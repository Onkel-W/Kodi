/*
 *      Copyright (C) 2016-2017 Team Kodi
 *      http://kodi.tv
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this Program; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "ReversiblePlayback.h"
#include "cores/RetroPlayer/savestates/SavestateReader.h"
#include "cores/RetroPlayer/savestates/SavestateWriter.h"
#include "cores/RetroPlayer/streams/memory/BasicMemoryStream.h"
#include "cores/RetroPlayer/streams/memory/DeltaPairMemoryStream.h"
#include "games/addons/GameClient.h"
#include "games/GameServices.h"
#include "games/GameSettings.h"
#include "threads/SingleLock.h"
#include "utils/MathUtils.h"
#include "ServiceBroker.h"

#include <algorithm>

using namespace KODI;
using namespace RETRO;

#define REWIND_FACTOR  0.25  // Rewind at 25% of gameplay speed

CReversiblePlayback::CReversiblePlayback(GAME::CGameClient* gameClient, double fps, size_t serializeSize) :
  m_gameClient(gameClient),
  m_gameLoop(this, fps),
  //m_savestateWriter(new CSavestateWriter),
  //m_savestateReader(new CSavestateReader),
  m_totalFrameCount(0),
  m_pastFrameCount(0),
  m_futureFrameCount(0),
  m_playTimeMs(0),
  m_totalTimeMs(0),
  m_cacheTimeMs(0)
{
  UpdateMemoryStream();

  GAME::CGameSettings &gameSettings = CServiceBroker::GetGameServices().GameSettings();
  gameSettings.RegisterObserver(this);
}

CReversiblePlayback::~CReversiblePlayback()
{
  GAME::CGameSettings &gameSettings = CServiceBroker::GetGameServices().GameSettings();
  gameSettings.UnregisterObserver(this);

  Deinitialize();
}

void CReversiblePlayback::Initialize()
{
  m_gameLoop.Start();
}

void CReversiblePlayback::Deinitialize()
{
  m_gameLoop.Stop();
}

void CReversiblePlayback::SeekTimeMs(unsigned int timeMs)
{
  const int offsetTimeMs = timeMs - GetTimeMs();
  const int offsetFrames = MathUtils::round_int(offsetTimeMs / 1000.0 * m_gameLoop.FPS());

  if (offsetFrames > 0)
  {
    const uint64_t frames = std::min(static_cast<uint64_t>(offsetFrames), m_futureFrameCount);
    if (frames > 0)
    {
      m_gameLoop.SetSpeed(0.0);
      AdvanceFrames(frames);
      m_gameLoop.SetSpeed(1.0);
    }
  }
  else if (offsetFrames < 0)
  {
    const uint64_t frames = std::min(static_cast<uint64_t>(-offsetFrames), m_pastFrameCount);
    if (frames > 0)
    {
      m_gameLoop.SetSpeed(0.0);
      RewindFrames(frames);
      m_gameLoop.SetSpeed(1.0);
    }
  }
}

double CReversiblePlayback::GetSpeed() const
{
  return m_gameLoop.GetSpeed();
}

void CReversiblePlayback::SetSpeed(double speedFactor)
{
  if (speedFactor >= 0.0)
    m_gameLoop.SetSpeed(speedFactor);
  else
    m_gameLoop.SetSpeed(speedFactor * REWIND_FACTOR);
}

void CReversiblePlayback::PauseAsync()
{
  m_gameLoop.PauseAsync();
}

std::string CReversiblePlayback::CreateSavestate()
{
  std::string empty;

  // Game client must support serialization
  if (m_gameClient->SerializeSize() == 0)
    return empty;

  /*! @todo
  if (!m_savestateWriter->Initialize(m_gameClient->GetGamePath()))
    return empty;
  */

  std::unique_ptr<IMemoryStream> memoryStream;
  bool bHasMemoryStream = false;

  {
    CSingleLock lock(m_mutex);
    if (m_memoryStream)
    {
      memoryStream = std::move(m_memoryStream);
      bHasMemoryStream = true;
    }
    else
    {
      lock.Leave();
      memoryStream.reset(new CBasicMemoryStream);
      memoryStream->Init(m_gameClient->SerializeSize(), 1);
    }
  }

  // If memory stream is empty, ask the game client for a frame
  if (memoryStream->CurrentFrame() == nullptr)
  {
    if (m_gameClient->Serialize(memoryStream->BeginFrame(), memoryStream->FrameSize()))
      memoryStream->SubmitFrame();
  }

  bool bSuccess = false;

  /*! @todo
  if (m_savestateWriter->WriteSave(memoryStream->CurrentFrame(), memoryStream->FrameSize()))
  {
    m_savestateWriter->WriteThumb();
    bSuccess = true;
  }
  */

  if (bHasMemoryStream)
  {
    CSingleLock lock(m_mutex);
    m_memoryStream = std::move(memoryStream);
  }

  /*! @todo
  if (bSuccess)
    return m_savestateWriter->GetPath();
  */

  return "";
}

bool CReversiblePlayback::LoadSavestate(const std::string& path)
{
  // Game client must support serialization
  if (m_gameClient->SerializeSize() == 0)
    return false;

  /*! @todo
  if (!m_savestateReader->Initialize(path, m_gameClient))
    return false;
  */

  std::unique_ptr<IMemoryStream> memoryStream;
  bool bHasMemoryStream = false;

  {
    CSingleLock lock(m_mutex);
    if (m_memoryStream)
    {
      memoryStream = std::move(m_memoryStream);
      bHasMemoryStream = true;
    }
    else
    {
      lock.Leave();
      memoryStream.reset(new CBasicMemoryStream);
      memoryStream->Init(m_gameClient->SerializeSize(), 1);
    }
  }

  bool bSuccess = false;

  /*! @todo
  if (CSavestateReader::ReadSave(memoryStream->BeginFrame(), memoryStream->FrameSize()))
  {
    memoryStream->SubmitFrame();

    m_gameClient->Deserialize(memoryStream->CurrentFrame(), memoryStream->FrameSize());
    m_totalFrameCount = m_savestateReader->GetFrameCount();

    bSuccess = true;
  }
  */

  if (bHasMemoryStream)
  {
    CSingleLock lock(m_mutex);
    m_memoryStream = std::move(memoryStream);
  }

  return bSuccess;
}

void CReversiblePlayback::FrameEvent()
{
  m_gameClient->RunFrame();

  AddFrame();
}

void CReversiblePlayback::RewindEvent()
{
  RewindFrames(1);

  m_gameClient->RunFrame();
}

void CReversiblePlayback::AddFrame()
{
  CSingleLock lock(m_mutex);

  if (m_memoryStream)
  {
    if (m_gameClient->Serialize(m_memoryStream->BeginFrame(), m_memoryStream->FrameSize()))
    {
      m_memoryStream->SubmitFrame();
      UpdatePlaybackStats();
    }
  }

  m_totalFrameCount++;
}

void CReversiblePlayback::RewindFrames(uint64_t frames)
{
  CSingleLock lock(m_mutex);

  if (m_memoryStream)
  {
    m_memoryStream->RewindFrames(frames);
    m_gameClient->Deserialize(m_memoryStream->CurrentFrame(), m_memoryStream->FrameSize());
    UpdatePlaybackStats();
  }

  m_totalFrameCount -= std::min(m_totalFrameCount, static_cast<uint64_t>(frames));
}

void CReversiblePlayback::AdvanceFrames(uint64_t frames)
{
  CSingleLock lock(m_mutex);

  if (m_memoryStream)
  {
    m_memoryStream->AdvanceFrames(frames);
    m_gameClient->Deserialize(m_memoryStream->CurrentFrame(), m_memoryStream->FrameSize());
    UpdatePlaybackStats();
  }

  m_totalFrameCount += frames;
}

void CReversiblePlayback::UpdatePlaybackStats()
{
  m_pastFrameCount = m_memoryStream->PastFramesAvailable();
  m_futureFrameCount = m_memoryStream->FutureFramesAvailable();

  const uint64_t played = m_pastFrameCount + (m_memoryStream->CurrentFrame() ? 1 : 0);
  const uint64_t total = m_memoryStream->MaxFrameCount();
  const uint64_t cached = m_futureFrameCount;

  m_playTimeMs = MathUtils::round_int(1000.0 * played / m_gameLoop.FPS());
  m_totalTimeMs = MathUtils::round_int(1000.0 * total / m_gameLoop.FPS());
  m_cacheTimeMs = MathUtils::round_int(1000.0 * cached / m_gameLoop.FPS());
}

void CReversiblePlayback::Notify(const Observable &obs, const ObservableMessage msg)
{
  switch (msg)
  {
  case ObservableMessageSettingsChanged:
    UpdateMemoryStream();
    break;
  default:
    break;
  }
}

void CReversiblePlayback::UpdateMemoryStream()
{
  CSingleLock lock(m_mutex);

  bool bRewindEnabled = false;

  GAME::CGameSettings &gameSettings = CServiceBroker::GetGameServices().GameSettings();

  gameSettings.UnregisterObserver(this);

  if (m_gameClient->SerializeSize() > 0)
    bRewindEnabled = gameSettings.RewindEnabled();

  if (bRewindEnabled)
  {
    unsigned int rewindBufferSec = gameSettings.MaxRewindTimeSec();
    if (rewindBufferSec < 10)
      rewindBufferSec = 10; // Sanity check

    unsigned int frameCount = MathUtils::round_int(rewindBufferSec * m_gameLoop.FPS());

    if (!m_memoryStream)
    {
      m_memoryStream.reset(new CDeltaPairMemoryStream);
      m_memoryStream->Init(m_gameClient->SerializeSize(), frameCount);
    }

    if (m_memoryStream->MaxFrameCount() != frameCount)
    {
      m_memoryStream->SetMaxFrameCount(frameCount);
    }
  }
  else
  {
    m_memoryStream.reset();

    // Reset playback stats
    m_pastFrameCount = 0;
    m_futureFrameCount = 0;
    m_playTimeMs = 0;
    m_totalTimeMs = 0;
    m_cacheTimeMs = 0;
  }
}