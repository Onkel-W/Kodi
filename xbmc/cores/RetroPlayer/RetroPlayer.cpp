/*
 *      Copyright (C) 2012-2017 Team Kodi
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

#include "RetroPlayer.h"
#include "RetroPlayerAudio.h"
#include "RetroPlayerAutoSave.h"
#include "RetroPlayerClock.h"
#include "RetroPlayerInput.h"
#include "RetroPlayerVideo.h"
#include "addons/AddonManager.h"
#include "cores/RetroPlayer/environment/Environment.h"
#include "cores/RetroPlayer/playback/RealtimePlayback.h"
#include "cores/RetroPlayer/playback/ReversiblePlayback.h"
#include "cores/RetroPlayer/rendering/RPRenderManager.h"
#include "cores/VideoPlayer/Process/ProcessInfo.h"
#include "games/addons/GameClient.h"
#include "games/dialogs/osd/DialogGameVideoSelect.h"
#include "games/ports/PortManager.h"
#include "games/tags/GameInfoTag.h"
#include "games/GameServices.h"
#include "games/GameUtils.h"
#include "guilib/GUIDialog.h"
#include "guilib/GUIWindowManager.h"
#include "guilib/LocalizeStrings.h"
#include "guilib/WindowIDs.h"
#include "input/Action.h"
#include "input/ActionIDs.h"
#include "settings/MediaSettings.h"
#include "threads/SingleLock.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include "FileItem.h"
#include "ServiceBroker.h"
#include "URL.h"

using namespace KODI;
using namespace RETRO;

#define REWIND_FACTOR  0.25  // Rewind at 25% of gameplay speed

CRetroPlayer::CRetroPlayer(IPlayerCallback& callback) :
  IPlayer(callback),
  m_renderManager(new CRPRenderManager(m_clock, this)),
  m_processInfo(CProcessInfo::CreateInstance())
{
  m_processInfo->SetDataCache(&CServiceBroker::GetDataCacheCore());
  m_processInfo->SetTempo(1.0);
}

CRetroPlayer::~CRetroPlayer()
{
  CloseFile();
}

bool CRetroPlayer::OpenFile(const CFileItem& file, const CPlayerOptions& options)
{
  std::string redactedPath = CURL::GetRedacted(file.GetPath());
  CLog::Log(LOGINFO, "RetroPlayer: Opening: %s", redactedPath.c_str());

  // Reset game settings
  CMediaSettings::GetInstance().GetCurrentGameSettings() = CMediaSettings::GetInstance().GetDefaultGameSettings();

  //! @todo - Remove this when RetroPlayer has a renderer
  CVideoSettings &videoSettings = CMediaSettings::GetInstance().GetCurrentVideoSettings();
  videoSettings.m_ScalingMethod = CMediaSettings::GetInstance().GetCurrentGameSettings().ScalingMethod();
  videoSettings.m_ViewMode = CMediaSettings::GetInstance().GetCurrentGameSettings().ViewMode();

  CSingleLock lock(m_mutex);

  if (IsPlaying())
    CloseFile();

  PrintGameInfo(file);

  bool bSuccess = false;

  m_gameClient = GAME::CGameUtils::OpenGameClient(file);
  if (m_gameClient)
  {
    CLog::Log(LOGERROR, "Can't play game, no game client was passed to RetroPlayer!");
  }
  else if (!ADDON::CAddonMgr::GetInstance().GetAddon(gameClientId, addon, ADDON::ADDON_GAMEDLL))
  {
    CLog::Log(LOGERROR, "Can't find add-on %s for game file!", gameClientId.c_str());
  }
  else
  {
    m_gameClient = std::static_pointer_cast<CGameClient>(addon);
    if (m_gameClient->Initialize())
    {
      m_audio.reset(new CRetroPlayerAudio(*m_processInfo));
      m_video.reset(new CRetroPlayerVideo(*m_renderManager, *m_processInfo, m_clock));
      m_input.reset(new CRetroPlayerInput(CServiceBroker::GetPeripherals()));

      if (!file.GetPath().empty())
        bSuccess = m_gameClient->OpenFile(file, m_audio.get(), m_video.get(), m_input.get());
      else
        bSuccess = m_gameClient->OpenStandalone(m_audio.get(), m_video.get(), m_input.get());

      if (bSuccess)
        CLog::Log(LOGDEBUG, "RetroPlayer: Using game client %s", gameClientId.c_str());
      else
        CLog::Log(LOGERROR, "RetroPlayer: Failed to open file using %s", gameClientId.c_str());
    }
    else
      CLog::Log(LOGERROR, "RetroPlayer: Failed to initialize %s", gameClientId.c_str());
  }

  if (bSuccess)
  {
    m_environment.reset(new CEnvironment(*m_gameClient));
    if (m_gameClient->RequiresGameLoop())
      m_playback.reset(new CReversiblePlayback(m_environment.get(), m_gameClient->GetFrameRate()));
    else
      m_playback.reset(new CRealtimePlayback);
    m_clock.reset(new CRetroPlayerClock(m_playback.get(), m_gameClient->GetFrameRate()));

    CSavestate save;
    if (save.Deserialize(savestatePath))
    {
      // Check if game client is the same
      if (save.GameClient() != m_gameClient->ID())
      {
        ADDON::AddonPtr addon;
        if (ADDON::CAddonMgr::GetInstance().GetAddon(save.GameClient(), addon))
        {
          // Warn the user that continuing with a different game client will
          // overwrite the save
          bool dummy;
          if (!CGUIDialogYesNo::ShowAndGetInput(438, StringUtils::Format(g_localizeStrings.Get(35217), addon->Name()), dummy, 222, 35218, 0))
            bSuccess = false;
        }
      }
    }

    if (bSuccess)
    {
      std::string redactedSavestatePath = CURL::GetRedacted(savestatePath);
      CLog::Log(LOGDEBUG, "RetroPlayer: Loading savestate %s", redactedSavestatePath.c_str());

      if (!SetPlayerState(savestatePath))
        CLog::Log(LOGERROR, "RetroPlayer: Failed to load savestate");
    }
  }

  if (bSuccess)
  {
    RegisterWindowCallbacks();
    SetSpeed(1);
    m_environment.reset(new CEnvironment(*m_gameClient));
    m_clock.reset(new CRetroPlayerClock(m_environment.get(), m_gameClient->GetFrameRate()));
    m_clock->Start();
    m_callback.OnPlayBackStarted();
    m_autoSave.reset(new CRetroPlayerAutoSave(*m_gameClient));
  }
  else
  {
    m_gameClient.reset();
    m_audio.reset();
    m_video.reset();
  }

  return bSuccess;
}

bool CRetroPlayer::CloseFile(bool reopen /* = false */)
{
  CLog::Log(LOGDEBUG, "RetroPlayer: Closing file");

  // Stop the game
  m_autoSave.reset();
  if (m_clock)
    m_clock->Stop();

  CSingleLock lock(m_mutex);

  if (m_gameClient)
  {
    UnregisterWindowCallbacks();
    m_clock.reset();
    m_playback.reset();
    m_environment.reset();
    m_gameClient->CloseFile();
    m_gameClient->Unload();
    m_gameClient.reset();
    m_callback.OnPlayBackEnded();
  }

  m_audio.reset();
  m_video.reset();

  return true;
}

bool CRetroPlayer::IsPlaying() const
{
  if (m_gameClient)
    return m_gameClient->IsPlaying();

  return false;
}

bool CRetroPlayer::CanPause()
{
  if (m_gameClient)
    return m_gameClient->RequiresGameLoop();

  return false;
}

void CRetroPlayer::Pause()
{
  if (!CanPause())
    return;

  if (m_gameClient)
  {
    if (m_clock->GetSpeed() == 0.0)
      m_clock->SetSpeed(1.0);
    else
      m_clock->SetSpeed(0.0);

    m_environment->GetPlayback()->PauseUnpause();
    OnSpeedChange(m_gameClient->GetPlayback()->GetSpeed());
  }
}

bool CRetroPlayer::CanSeek()
{
  if (m_gameClient)
    return m_gameClient->RequiresGameLoop();

  return false;
}

void CRetroPlayer::Seek(bool bPlus /* = true */,
                        bool bLargeStep /* = false */,
                        bool bChapterOverride /* = false */)
{
  if (!CanSeek())
    return;

  if (m_environment)
  {
    //! @todo
    /*
    if (bPlus)
    {
      if (bLargeStep)
        m_gameClient->GetPlayback()->BigSkipForward();
      else
        m_gameClient->GetPlayback()->SmallSkipForward();
    }
    else
    {
      if (bLargeStep)
        m_gameClient->GetPlayback()->BigSkipBackward();
      else
        m_gameClient->GetPlayback()->SmallSkipBackward();
    }
    */
  }
}

void CRetroPlayer::SeekPercentage(float fPercent /* = 0 */)
{
  if (!CanSeek())
    return;

  if (fPercent < 0.0f  )
    fPercent = 0.0f;
  else if (fPercent > 100.0f)
    fPercent = 100.0f;

  uint64_t totalTime = GetTotalTime();
  if (totalTime != 0)
    SeekTime(static_cast<int64_t>(totalTime * fPercent / 100.0f));
}

float CRetroPlayer::GetCachePercentage()
{
  if (m_playback)
  {
    const float cacheMs = static_cast<float>(m_playback->GetCacheTimeMs());
    const float totalMs = static_cast<float>(m_playback->GetTotalTimeMs());

    if (totalMs != 0.0f)
      return cacheMs / totalMs * 100.0f;
  }
  return 0.0f;
}

void CRetroPlayer::SetMute(bool bOnOff)
{
  if (m_audio)
    m_audio->Enable(!bOnOff);
}

void CRetroPlayer::SeekTime(int64_t iTime /* = 0 */)
{
  if (!CanSeek())
    return;

  if (m_playback && iTime >= 0)
  {
    m_environment->GetPlayback()->SeekTimeMs(static_cast<unsigned int>(iTime));
    OnSpeedChange(m_gameClient->GetPlayback()->GetSpeed());
  }
}

bool CRetroPlayer::SeekTimeRelative(int64_t iTime)
{
  if (!CanSeek())
    return false;

  SeekTime(GetTime() + iTime);

  return true;
}

uint64_t CRetroPlayer::GetTime()
{
  if (m_playback)
    return m_playback->GetTimeMs();

  return 0;
}

uint64_t CRetroPlayer::GetTotalTime()
{
  if (m_playback)
    return m_playback->GetTotalTimeMs();

  return 0;
}

bool CRetroPlayer::GetStreamDetails(CStreamDetails &details)
{
  //! @todo
  return false;
}

void CRetroPlayer::SetSpeed(float speed)
{
  if (m_clock)
  {
    if (speed < 0.0f)
      speed *= REWIND_FACTOR;

    if (m_clock->GetSpeed() != speed)
    {
      if (speed == 1.0f)
        m_callback.OnPlayBackResumed();
      else if (speed == 0.0f)
        m_callback.OnPlayBackPaused();
    }

    m_gameClient->GetPlayback()->SetSpeed(speed);
    OnSpeedChange(speed);
  }
}

bool CRetroPlayer::OnAction(const ::CAction &action)
{
  switch (action.GetID())
  {
  case ACTION_PLAYER_RESET:
  {
    if (m_environment)
    {
      double speed = m_gameClient->GetPlayback()->GetSpeed();

      m_gameClient->GetPlayback()->SetSpeed(0.0);


      CServiceBroker::GetGameServices().PortManager().HardwareReset();

      // If rewinding or paused, begin playback
      if (speed <= 0.0)
        speed = 1.0;

      m_gameClient->GetPlayback()->SetSpeed(speed);
      OnSpeedChange(speed);
    }
    return true;
  }
  case ACTION_SHOW_OSD:
  {
    if (m_gameClient && m_gameClient->GetPlayback()->GetSpeed() == 0.0)
    {
      CloseOSD();
      return true;
    }
  }
  default:
    break;
  }

  return false;
}

std::string CRetroPlayer::GetPlayerState()
{
  //! @todo
  return "";
}

bool CRetroPlayer::SetPlayerState(const std::string& state)
{
  //! @todo
  return false;
}

void CRetroPlayer::FrameMove()
{
  m_renderManager->FrameMove();

  if (m_gameClient)
  {
    const bool bFullscreen = (g_windowManager.GetActiveWindowID() == WINDOW_FULLSCREEN_GAME);

    switch (m_state)
    {
    case State::STARTING:
    {
      if (bFullscreen)
        m_state = State::FULLSCREEN;
      break;
    }
    case State::FULLSCREEN:
    {
      if (!bFullscreen)
      {
        m_priorSpeed = m_clock->GetSpeed();
        m_clock->SetSpeed(0.0);
        OnSpeedChange(0.0);
        m_state = State::BACKGROUND;
      }
      break;
    }
    case State::BACKGROUND:
    {
      if (bFullscreen)
      {
        if (m_clock->GetSpeed() == 0.0)
          m_clock->SetSpeed(m_priorSpeed);
        {
          OnSpeedChange(m_priorSpeed);
        }
        m_state = State::FULLSCREEN;
      }
      break;
    }
    }

    m_processInfo->SetPlayTimes(0, GetTime(), 0, GetTotalTime());
  }
}

void CRetroPlayer::Render(bool clear, uint32_t alpha /* = 255 */, bool gui /* = true */)
{
  m_renderManager->Render(clear, 0, alpha, gui);
}

void CRetroPlayer::FlushRenderer()
{
  m_renderManager->Flush(true);
}

void CRetroPlayer::SetRenderViewMode(int mode)
{
  m_renderManager->SetViewMode(mode);
}

float CRetroPlayer::GetRenderAspectRatio()
{
  return m_renderManager->GetAspectRatio();
}

void CRetroPlayer::TriggerUpdateResolution()
{
  m_renderManager->TriggerUpdateResolution(0.0f, 0, 0);
}

bool CRetroPlayer::IsRenderingVideo()
{
  return m_renderManager->IsConfigured();
}

bool CRetroPlayer::Supports(EINTERLACEMETHOD method)
{
  return m_processInfo->Supports(method);
}

EINTERLACEMETHOD CRetroPlayer::GetDeinterlacingMethodDefault()
{
  return m_processInfo->GetDeinterlacingMethodDefault();
}
bool CRetroPlayer::Supports(ESCALINGMETHOD method)
{
  return m_renderManager->Supports(method);
}

bool CRetroPlayer::Supports(ERENDERFEATURE feature)
{
  return m_renderManager->Supports(feature);
}

unsigned int CRetroPlayer::RenderCaptureAlloc()
{
  return m_renderManager->AllocRenderCapture();
}

void CRetroPlayer::RenderCaptureRelease(unsigned int captureId)
{
  m_renderManager->ReleaseRenderCapture(captureId);
}

void CRetroPlayer::RenderCapture(unsigned int captureId, unsigned int width, unsigned int height, int flags)
{
  m_renderManager->StartRenderCapture(captureId, width, height, flags);
}

bool CRetroPlayer::RenderCaptureGetPixels(unsigned int captureId, unsigned int millis, uint8_t *buffer, unsigned int size)
{
  return m_renderManager->RenderCaptureGetPixels(captureId, millis, buffer, size);
}

void CRetroPlayer::UpdateClockSync(bool enabled)
{
  m_processInfo->SetRenderClockSync(enabled);
}

void CRetroPlayer::UpdateRenderInfo(CRenderInfo &info)
{
  m_processInfo->UpdateRenderInfo(info);
}

void CRetroPlayer::UpdateRenderBuffers(int queued, int discard, int free)
{
  m_processInfo->UpdateRenderBuffers(queued, discard, free);
}

void CRetroPlayer::UpdateGuiRender(bool gui)
{
  m_processInfo->SetGuiRender(gui);
}

void CRetroPlayer::UpdateVideoRender(bool video)
{
  m_processInfo->SetVideoRender(video);
}

void CRetroPlayer::OnSpeedChange(double newSpeed)
{
  m_audio->Enable(newSpeed == 1.0);
  m_input->SetSpeed(newSpeed);
  m_processInfo->SetSpeed(static_cast<float>(newSpeed));
  if (newSpeed != 0.0)
    CloseOSD();
}

void CRetroPlayer::CloseOSD()
{
  g_windowManager.CloseDialogs(true);
}

void CRetroPlayer::RegisterWindowCallbacks()
{
  CDialogGameVideoSelect *dialogVideoFilter = dynamic_cast<CDialogGameVideoSelect*>(g_windowManager.GetWindow(WINDOW_DIALOG_GAME_VIDEO_FILTER));
  if (dialogVideoFilter != nullptr)
    dialogVideoFilter->RegisterCallback(m_renderManager.get());

  CDialogGameVideoSelect *dialogViewMode = dynamic_cast<CDialogGameVideoSelect*>(g_windowManager.GetWindow(WINDOW_DIALOG_GAME_VIEW_MODE));
  if (dialogViewMode != nullptr)
    dialogViewMode->RegisterCallback(m_renderManager.get());
}

void CRetroPlayer::UnregisterWindowCallbacks()
{
  CDialogGameVideoSelect *dialogVideoFilter = dynamic_cast<CDialogGameVideoSelect*>(g_windowManager.GetWindow(WINDOW_DIALOG_GAME_VIDEO_FILTER));
  if (dialogVideoFilter != nullptr)
    dialogVideoFilter->UnregisterCallback();

  CDialogGameVideoSelect *dialogViewMode = dynamic_cast<CDialogGameVideoSelect*>(g_windowManager.GetWindow(WINDOW_DIALOG_GAME_VIEW_MODE));
  if (dialogViewMode != nullptr)
    dialogViewMode->UnregisterCallback();
}

void CRetroPlayer::PrintGameInfo(const CFileItem &file) const
{
  const GAME::CGameInfoTag *tag = file.GetGameInfoTag();
  if (tag)
  {
    CLog::Log(LOGDEBUG, "RetroPlayer: ---------------------------------------");
    CLog::Log(LOGDEBUG, "RetroPlayer: Game tag loaded");
    CLog::Log(LOGDEBUG, "RetroPlayer: URL: %s", tag->GetURL().c_str());
    CLog::Log(LOGDEBUG, "RetroPlayer: Title: %s", tag->GetTitle().c_str());
    CLog::Log(LOGDEBUG, "RetroPlayer: Platform: %s", tag->GetPlatform().c_str());
    CLog::Log(LOGDEBUG, "RetroPlayer: Genres: %s", StringUtils::Join(tag->GetGenres(), ", ").c_str());
    CLog::Log(LOGDEBUG, "RetroPlayer: Developer: %s", tag->GetDeveloper().c_str());
    if (tag->GetYear() > 0)
      CLog::Log(LOGDEBUG, "RetroPlayer: Year: %u", tag->GetYear());
    CLog::Log(LOGDEBUG, "RetroPlayer: Game Code: %s", tag->GetID().c_str());
    CLog::Log(LOGDEBUG, "RetroPlayer: Region: %s", tag->GetRegion().c_str());
    CLog::Log(LOGDEBUG, "RetroPlayer: Publisher: %s", tag->GetPublisher().c_str());
    CLog::Log(LOGDEBUG, "RetroPlayer: Format: %s", tag->GetFormat().c_str());
    CLog::Log(LOGDEBUG, "RetroPlayer: Cartridge type: %s", tag->GetCartridgeType().c_str());
    CLog::Log(LOGDEBUG, "RetroPlayer: Game client: %s", tag->GetGameClient().c_str());
    CLog::Log(LOGDEBUG, "RetroPlayer: ---------------------------------------");
  }
}
