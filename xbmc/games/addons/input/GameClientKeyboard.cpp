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

#include "GameClientKeyboard.h"
#include "GameClientInput.h"
#include "addons/kodi-addon-dev-kit/include/kodi/kodi_game_types.h"
#include "games/addons/GameClient.h"
#include "games/addons/GameClientTranslator.h"
#include "input/keyboard/interfaces/IKeyboardInputProvider.h"
#include "input/Key.h"
#include "utils/log.h"

#include <utility>

using namespace KODI;
using namespace GAME;

#define BUTTON_INDEX_MASK  0x01ff

CGameClientKeyboard::CGameClientKeyboard(CGameClient &gameClient,
                                         std::string controllerId,
                                         KEYBOARD::IKeyboardInputProvider *inputProvider) :
  m_gameClient(gameClient),
  m_controllerId(std::move(controllerId)),
  m_inputProvider(inputProvider)
{
  m_inputProvider->RegisterKeyboardHandler(this, false);
}

CGameClientKeyboard::~CGameClientKeyboard()
{
  m_inputProvider->UnregisterKeyboardHandler(this);
}

std::string CGameClientKeyboard::ControllerID() const
{
  return m_controllerId;
}

bool CGameClientKeyboard::HasKey(const KEYBOARD::KeyName &key) const
{
  return m_gameClient.Input().HasFeature(ControllerID(), key);
}

bool CGameClientKeyboard::OnKeyPress(const KEYBOARD::KeyName &key, KEYBOARD::Modifier mod, uint32_t unicode)
{
  // Only allow activated input in fullscreen game
  if (!m_gameClient.Input().AcceptsInput())
  {
    CLog::Log(LOGDEBUG, "GAME: key press ignored, not in fullscreen game");
    return false;
  }

  //! @todo
  return false;
}

void CGameClientKeyboard::OnKeyRelease(const KEYBOARD::KeyName &key, KEYBOARD::Modifier mod, uint32_t unicode)
{
  //! @todo
}
