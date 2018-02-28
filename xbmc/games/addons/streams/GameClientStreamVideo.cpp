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

#include "GameClientStreamVideo.h"
#include "cores/RetroPlayer/streams/RetroPlayerVideo.h"
#include "games/addons/GameClientCallbacks.h"
#include "games/addons/GameClientTranslator.h"
#include "utils/log.h"

using namespace KODI;
using namespace GAME;

CGameClientStreamVideo::CGameClientStreamVideo(RETRO::StreamPtr stream,
                                               const game_stream_video_properties &properties) :
  m_stream(std::move(stream)),
  m_format(properties.format),
  m_nominalWidth(properties.nominal_width),
  m_nominalHeight(properties.nominal_height),
  m_maxWidth(properties.max_width),
  m_maxHeight(properties.max_height),
  m_aspectRatio(properties.aspect_ratio)
{
}

bool CGameClientStreamVideo::Open()
{
  if (!m_stream)
    return false;

  const AVPixelFormat pixelFormat = CGameClientTranslator::TranslatePixelFormat(m_format);
  if (pixelFormat == AV_PIX_FMT_NONE)
  {
    CLog::Log(LOGERROR, "GAME: Unknown pixel format: %d", m_format);
    return false;
  }

  if (m_nominalWidth == 0 || m_nominalHeight == 0)
  {
    CLog::Log(LOGERROR, "GAME: Invalid nominal dimensions: %ux%u", m_nominalWidth, m_nominalHeight);
    return false;
  }

  if (m_maxWidth == 0 || m_maxHeight == 0)
  {
    CLog::Log(LOGERROR, "GAME: Invalid max dimensions: %ux%u", m_maxWidth, m_maxHeight);
    return false;
  }

  RETRO::CRetroPlayerVideo* videoStream = static_cast<RETRO::CRetroPlayerVideo*>(m_stream.get());
  return videoStream->OpenStream(pixelFormat, m_nominalWidth, m_nominalHeight, m_maxWidth, m_maxHeight, m_aspectRatio);
}

void CGameClientStreamVideo::Close()
{
  if (m_stream)
    m_stream->CloseStream();
}

void CGameClientStreamVideo::AddData(const game_stream_packet &packet)
{
  if (packet.type != GAME_STREAM_VIDEO)
    return;

  if (m_stream)
  {
    const game_stream_video_packet &video = packet.video;

    RETRO::VideoRotation rotation = CGameClientTranslator::TranslateRotation(video.rotation);

    RETRO::VideoStreamPacket videoPacket{
      video.width,
      video.height,
      rotation,
      video.data,
      video.size,
    };

    m_stream->AddStreamData(videoPacket);
  }
}

RETRO::StreamPtr CGameClientStreamVideo::ReleaseStream()
{
  return std::move(m_stream);
}
