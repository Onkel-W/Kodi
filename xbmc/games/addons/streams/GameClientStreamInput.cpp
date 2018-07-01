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

#include "GameClientStreamInput.h"
#include "addons/kodi-addon-dev-kit/include/kodi/kodi_game_types.h"
#include "cores/RetroPlayer/streams/RetroPlayerInput.h"
#include "utils/log.h"

using namespace KODI;
using namespace GAME;

bool CGameClientStreamInput::OpenStream(RETRO::IRetroPlayerStream* stream, const game_stream_properties& properties)
{
  RETRO::CRetroPlayerInput *inputStream = dynamic_cast<RETRO::CRetroPlayerInput*>(stream);
  if (inputStream == nullptr)
  {
    CLog::Log(LOGERROR, "GAME: RetroPlayer stream is not an input stream");
    return false;
  }

  RETRO::StreamProperties properties{};
  if (inputStream->OpenStream(properties))
    m_stream = stream;

  return m_stream != nullptr;
}

void CGameClientStreamInput::CloseStream()
{
  if (m_stream != nullptr)
  {
    m_stream->CloseStream();
    m_stream = nullptr;
  }
}

void CGameClientStreamInput::AddData(const game_stream_packet& packet)
{
  if (packet.type != GAME_STREAM_INPUT)
    return;

  if (m_stream != nullptr)
  {
    const game_stream_input_packet &input = packet.input;

    RETRO::InputStreamPacket inputPacket{
    };

    m_stream->AddStreamData(reinterpret_cast<const RETRO::StreamPacket&>(inputPacket));
  }
}
