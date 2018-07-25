/*
 *      Copyright (C) 2015-2017 Team Kodi
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

#include "GameClientMouse.h"
#include "GameClientInput.h"
#include "addons/kodi-addon-dev-kit/include/kodi/kodi_game_types.h"
#include "games/addons/GameClient.h"
#include "input/mouse/interfaces/IMouseInputProvider.h"
#include "input/Key.h"
#include "utils/log.h"

#include <utility>

using namespace KODI;
using namespace GAME;

CGameClientMouse::CGameClientMouse(CGameClient &gameClient,
                                   std::string controllerId,
                                   MOUSE::IMouseInputProvider *inputProvider) :
  m_gameClient(gameClient),
  m_controllerId(std::move(controllerId)),
  m_inputProvider(inputProvider)
{
  inputProvider->RegisterMouseHandler(this, false);
}

CGameClientMouse::~CGameClientMouse()
{
  m_inputProvider->UnregisterMouseHandler(this);
}

std::string CGameClientMouse::ControllerID(void) const
{
  return m_controllerId;
}

bool CGameClientMouse::OnMotion(const std::string& relpointer, int dx, int dy)
{
  // Only allow activated input in fullscreen game
  if (!m_gameClient.Input().AcceptsInput())
  {
    return false;
  }

  //! @todo
  return false;
}

bool CGameClientMouse::OnButtonPress(const std::string& button)
{
  // Only allow activated input in fullscreen game
  if (!m_gameClient.Input().AcceptsInput())
  {
    return false;
  }

  //! @todo
  return false;
}

void CGameClientMouse::OnButtonRelease(const std::string& button)
{
  //! @todo
}
