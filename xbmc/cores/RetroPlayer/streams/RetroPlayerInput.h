/*
 *      Copyright (C) 2017 Team Kodi
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
#include "games/addons/GameClientCallbacks.h"
#include "peripherals/PeripheralTypes.h"

namespace PERIPHERALS
{
  class CPeripherals;
}

namespace KODI
{
namespace RETRO
{
  class CRetroPlayerInput : public IRetroPlayerStream,
                            public GAME::IGameInputCallback
  {
  public:
    CRetroPlayerInput();
    ~CRetroPlayerInput() override;

    // Implementation of IRetroPlayerStream
    bool OpenStream(const StreamProperties& properties) override;
    bool GetStreamBuffer(unsigned int width, unsigned int height, StreamBuffer& buffer) override { return false; }
    void AddStreamData(const StreamPacket& packet) override;
    void CloseStream() override;
    void SetSpeed(double speed) override;

    // implementation of IGameInputCallback
    void PollInput() override;

  private:
    PERIPHERALS::CPeripherals &m_peripheralManager;

    // Input variables
    PERIPHERALS::EventPollHandlePtr m_inputPollHandle;
  };
}
}
