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

#include "GameClientStreamAudio.h"
#include "cores/RetroPlayer/streams/RetroPlayerAudio.h"
#include "games/addons/GameClientTranslator.h"
#include "utils/log.h"

using namespace KODI;
using namespace GAME;

CGameClientStreamAudio::CGameClientStreamAudio(RETRO::StreamPtr stream, const game_stream_audio_properties &properties, double sampleRate) :
  m_stream(std::move(stream)),
  m_format(properties.format),
  m_sampleRate(sampleRate),
  m_channelLayout(GetChannelLayout(properties.channel_map))
{
}

bool CGameClientStreamAudio::Open()
{
  if (!m_stream)
    return false;

  const RETRO::PCMFormat pcmFormat = CGameClientTranslator::TranslatePCMFormat(m_format);
  if (pcmFormat == RETRO::PCMFormat::FMT_UNKNOWN)
  {
    CLog::Log(LOGERROR, "GAME: Unknown PCM format: %d", static_cast<int>(m_format));
    return false;
  }

  RETRO::AudioChannelMap channelMap = { RETRO::AudioChannel::CH_NULL };
  unsigned int i = 0;
  for (GAME_AUDIO_CHANNEL gameChannel : m_channelLayout)
  {
    RETRO::AudioChannel channel = CGameClientTranslator::TranslateAudioChannel(gameChannel);
    if (channel == RETRO::AudioChannel::CH_NULL)
    {
      CLog::Log(LOGERROR, "GAME: Unknown channel ID: %d", static_cast<int>(gameChannel));
      return false;
    }

    channelMap[i++] = channel;
    if (i + 1 >= channelMap.size())
      break;
  }
  channelMap[i] = RETRO::AudioChannel::CH_NULL;

  if (channelMap[0] == RETRO::AudioChannel::CH_NULL)
  {
    CLog::Log(LOGERROR, "GAME: Empty channel layout");
    return false;
  }

  RETRO::CRetroPlayerAudio* audioStream = static_cast<RETRO::CRetroPlayerAudio*>(m_stream.get());
  return audioStream->OpenStream(pcmFormat, m_sampleRate, channelMap);
}

void CGameClientStreamAudio::Close()
{
  if (m_stream)
    m_stream->CloseStream();
}

void CGameClientStreamAudio::AddData(const game_stream_packet &packet)
{
  if (packet.type != GAME_STREAM_AUDIO)
    return;

  if (m_stream)
  {
    const game_stream_audio_packet &audio = packet.audio;

    RETRO::AudioStreamPacket audioPacket{
      audio.data,
      audio.size,
    };

    m_stream->AddStreamData(audioPacket);
  }
}

RETRO::StreamPtr CGameClientStreamAudio::ReleaseStream()
{
  return std::move(m_stream);
}

std::vector<GAME_AUDIO_CHANNEL> CGameClientStreamAudio::GetChannelLayout(const GAME_AUDIO_CHANNEL* channelMap)
{
  std::vector<GAME_AUDIO_CHANNEL> channelLayout;

  if (channelMap != nullptr)
  {
    for (const GAME_AUDIO_CHANNEL* channelPtr = channelMap; *channelPtr != GAME_CH_NULL; channelPtr++)
      channelLayout.push_back(*channelPtr);
  }

  return channelLayout;
}
