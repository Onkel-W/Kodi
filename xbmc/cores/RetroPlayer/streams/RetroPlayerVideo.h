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
#pragma once

#include "IRetroPlayerStream.h"
#include "RetroPlayerStreamTypes.h"

extern "C" {
#include "libavutil/pixfmt.h"
}

namespace KODI
{
namespace RETRO
{

class CRPProcessInfo;
class CRPRenderManager;

struct VideoStreamPacket : public StreamPacket
{
  VideoStreamPacket(unsigned int width,
                    unsigned int height,
                    VideoRotation rotation,
                    const uint8_t *data,
                    size_t size) :
    StreamPacket{StreamType::VIDEO},
    width{width},
    height{height},
    rotation{rotation},
    data{data},
    size{size}
  {
  }

  unsigned int width;
  unsigned int height;
  VideoRotation rotation;
  const uint8_t *data;
  size_t size;
};

  /*!
   * \brief Renders video frames provided by the game loop
   *
   * \sa CRPRenderManager
   */
  class CRetroPlayerVideo : public IRetroPlayerStream
  {
  public:
    CRetroPlayerVideo(CRPRenderManager& m_renderManager, CRPProcessInfo& m_processInfo);
    ~CRetroPlayerVideo() override;

    /*!
     * \brief Open video stream
     *
     * If aspect_ratio is <= 0.0, an aspect ratio of (nominalWidth / nominalHeight)
     * is assumed.
     */
    bool OpenStream(AVPixelFormat pixfmt, unsigned int nominalWidth, unsigned int nominalHeight, unsigned int maxWidth, unsigned int maxHeight, float aspectRatio);

    // implementation of IRetroPlayerStream
    bool GetStreamBuffer(unsigned int width, unsigned int height, StreamBuffer& buffer) override { return false; }
    void AddStreamData(const StreamPacket &packet) override;
    void CloseStream() override;

  private:
    // Construction parameters
    CRPRenderManager& m_renderManager;
    CRPProcessInfo& m_processInfo;

    // Stream properties
    bool m_bOpen = false;
  };

}
}
