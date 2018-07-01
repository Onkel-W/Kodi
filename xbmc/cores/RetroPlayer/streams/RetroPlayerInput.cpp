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

#include "RetroPlayerInput.h"
#include "peripherals/EventPollHandle.h"
#include "peripherals/Peripherals.h"
#include "utils/log.h"
#include "ServiceBroker.h"

using namespace KODI;
using namespace RETRO;

CRetroPlayerInput::CRetroPlayerInput() :
  m_peripheralManager(CServiceBroker::GetPeripherals())
{
  CLog::Log(LOGDEBUG, "RetroPlayer[INPUT]: Initializing input");

}

CRetroPlayerInput::~CRetroPlayerInput()
{
  CLog::Log(LOGDEBUG, "RetroPlayer[INPUT]: Deinitializing input");
}

bool CRetroPlayerInput::OpenStream(const StreamProperties& properties)
{
  CLog::Log(LOGDEBUG, "RetroPlayer[INPUT]: Acquiring input handle");
  m_inputPollHandle = m_peripheralManager.RegisterEventPoller();
  return true;
}

void CRetroPlayerInput::AddStreamData(const StreamPacket& packet)
{
  //! @todo
}

void CRetroPlayerInput::CloseStream()
{
  CLog::Log(LOGDEBUG, "RetroPlayer[INPUT]: Releasing input handle");
  m_inputPollHandle.reset();
}

void CRetroPlayerInput::SetSpeed(double speed)
{
  if (speed != 0)
    m_inputPollHandle->Activate();
  else
    m_inputPollHandle->Deactivate();
}

void CRetroPlayerInput::PollInput()
{
  m_inputPollHandle->HandleEvents(true);
}
