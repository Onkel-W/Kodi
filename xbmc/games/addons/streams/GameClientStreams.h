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

struct game_stream_properties;

namespace KODI
{
namespace RETRO
{
  class IStreamManager;
}

namespace GAME
{

class CGameClient;
class IGameClientStream;

class CGameClientStreams
{
public:
  CGameClientStreams(CGameClient &gameClient);

  void Initialize(RETRO::IStreamManager& streamManager);
  void Deinitialize();

  IGameClientStream *OpenStream(const game_stream_properties &properties);
  void CloseStream(IGameClientStream *stream);

private:
  // Construction parameters
  CGameClient &m_gameClient;

  // Initialization parameters
  RETRO::IStreamManager* m_streamManager = nullptr;
};

} // namespace GAME
} // namespace KODI
