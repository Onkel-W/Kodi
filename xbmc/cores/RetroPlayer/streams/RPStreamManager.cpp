/*
 *      Copyright (C) 2018 Team Kodi
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

#include "RPStreamManager.h"
#include "IRetroPlayerStream.h"
#include "RetroPlayerAudio.h"
//#include "RetroPlayerHardwareBuffer.h" //! @todo
#include "RetroPlayerInput.h"
#include "RetroPlayerMemory.h"
//#include "RetroPlayerSoftwareBuffer.h" //! @todo
#include "RetroPlayerVideo.h"

using namespace KODI;
using namespace RETRO;

CRPStreamManager::CRPStreamManager(CRPRenderManager& renderManager, CRPProcessInfo& processInfo) :
  m_renderManager(renderManager),
  m_processInfo(processInfo)
{
}

CRPStreamManager::~CRPStreamManager()
{
  for (const StreamPtr &stream : m_streams)
    stream->CloseStream();
}

void CRPStreamManager::SetSpeed(double speed)
{
  for (const StreamPtr &stream : m_streams)
    stream->SetSpeed(speed);
}

StreamPtr CRPStreamManager::CreateStream(StreamType streamType)
{
  StreamPtr stream;

  switch (streamType)
  {
  case StreamType::AUDIO:
  {
    stream.reset(new CRetroPlayerAudio(m_processInfo));
    break;
  }
  case StreamType::VIDEO:
  case StreamType::SW_BUFFER:
  {
    stream.reset(new CRetroPlayerVideo(m_renderManager, m_processInfo));
    break;
  }
  case StreamType::HW_BUFFER:
  {
    //stream.reset(new CRetroPlayerHardware(m_renderManager, m_processInfo)); //! @todo
    break;
  }
  case StreamType::MEMORY:
  {
    stream.reset(new CRetroPlayerMemory);
    break;
  }
  case StreamType::INPUT:
  {
    stream.reset(new CRetroPlayerInput);
    break;
  }

  default:
    break;
  }

  if (stream)
    m_streams.push_back(stream);

  return stream;
}

void CRPStreamManager::CloseStream(StreamPtr stream)
{
  if (stream)
  {
    auto it = std::find(m_streams.begin(), m_streams.end(), stream);
    if (it != m_streams.end())
      m_streams.erase(it);

    stream->CloseStream();
  }
}
