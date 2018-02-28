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
#pragma once

#include "IGameClientStream.h"
#include "addons/kodi-addon-dev-kit/include/kodi/kodi_game_types.h"
#include "cores/RetroPlayer/streams/RetroPlayerStreamTypes.h"

namespace KODI
{
namespace RETRO
{
  class CRetroPlayerVideo;
}

namespace GAME
{

class CGameClientStreamVideo : public IGameClientStream
{
public:
  CGameClientStreamVideo(RETRO::StreamPtr stream, const game_stream_video_properties &properties);
  ~CGameClientStreamVideo() override { Close(); }

  // Implementation of IGameClientStream
  GAME_STREAM_TYPE Type() const override { return GAME_STREAM_VIDEO; }
  bool Open() override;
  void Close() override;
  void AddData(const game_stream_packet &packet) override;
  RETRO::StreamPtr ReleaseStream() override;

private:
  // Construction parameters
  RETRO::StreamPtr m_stream;
  const GAME_PIXEL_FORMAT m_format;
  const unsigned int m_nominalWidth;
  const unsigned int m_nominalHeight;
  const unsigned int m_maxWidth;
  const unsigned int m_maxHeight;
  const float m_aspectRatio;
};

} // namespace GAME
} // namespace KODI
