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

#include "GameClientJoystick.h"
#include "GameClientInput.h"
#include "games/addons/GameClient.h"
#include "games/controllers/Controller.h"
#include "games/ports/Port.h"
#include "input/joysticks/interfaces/IInputReceiver.h"
#include "utils/log.h"

#include <assert.h>

using namespace KODI;
using namespace GAME;

CGameClientJoystick::CGameClientJoystick(CGameClient &gameClient,
                                         const std::string &portAddress,
                                         const ControllerPtr& controller) :
  m_gameClient(gameClient),
  m_portAddress(portAddress),
  m_controller(controller),
  m_port(new CPort(this))
{
  assert(m_controller.get() != NULL);
}

CGameClientJoystick::~CGameClientJoystick() = default;

void CGameClientJoystick::RegisterInput(JOYSTICK::IInputProvider *inputProvider)
{
  m_port->RegisterInput(inputProvider);
}

void CGameClientJoystick::UnregisterInput(JOYSTICK::IInputProvider *inputProvider)
{
  m_port->UnregisterInput(inputProvider);
}

std::string CGameClientJoystick::ControllerID(void) const
{
  return m_controller->ID();
}

bool CGameClientJoystick::HasFeature(const std::string& feature) const
{
  return m_gameClient.Input().HasFeature(m_controller->ID(), feature);
}

bool CGameClientJoystick::AcceptsInput(const std::string &feature) const
{
  return m_gameClient.Input().AcceptsInput();
}

bool CGameClientJoystick::OnButtonPress(const std::string& feature, bool bPressed)
{
  game_input_event event;

  std::string controllerId = m_controller->ID();

  event.type                   = GAME_INPUT_EVENT_DIGITAL_BUTTON;
  event.controller_id          = controllerId.c_str();
  event.port_type              = GAME_PORT_CONTROLLER;
  event.port_address           = m_portAddress.c_str();
  event.feature_name           = feature.c_str();
  event.digital_button.pressed = bPressed;

  return m_gameClient.Input().InputEvent(event);
}

bool CGameClientJoystick::OnButtonMotion(const std::string& feature, float magnitude, unsigned int motionTimeMs)
{
  game_input_event event;

  std::string controllerId = m_controller->ID();

  event.type                    = GAME_INPUT_EVENT_ANALOG_BUTTON;
  event.controller_id           = controllerId.c_str();
  event.port_type               = GAME_PORT_CONTROLLER;
  event.port_address            = m_portAddress.c_str();
  event.feature_name            = feature.c_str();
  event.analog_button.magnitude = magnitude;


  return m_gameClient.Input().InputEvent(event);
}

bool CGameClientJoystick::OnAnalogStickMotion(const std::string& feature, float x, float y, unsigned int motionTimeMs)
{
  game_input_event event;

  std::string controllerId = m_controller->ID();

  event.type           = GAME_INPUT_EVENT_ANALOG_STICK;
  event.controller_id  = controllerId.c_str();
  event.port_type      = GAME_PORT_CONTROLLER;
  event.port_address   = m_portAddress.c_str();
  event.feature_name   = feature.c_str();
  event.analog_stick.x = x;
  event.analog_stick.y = y;

  return m_gameClient.Input().InputEvent(event);
}

bool CGameClientJoystick::OnAccelerometerMotion(const std::string& feature, float x, float y, float z)
{
  game_input_event event;

  std::string controllerId = m_controller->ID();

  event.type            = GAME_INPUT_EVENT_ACCELEROMETER;
  event.controller_id   = controllerId.c_str();
  event.port_type       = GAME_PORT_CONTROLLER;
  event.port_address    = m_portAddress.c_str();
  event.feature_name    = feature.c_str();
  event.accelerometer.x = x;
  event.accelerometer.y = y;
  event.accelerometer.z = z;

  return m_gameClient.Input().InputEvent(event);
}

bool CGameClientJoystick::OnWheelMotion(const std::string& feature, float position, unsigned int motionTimeMs)
{
  game_input_event event;

  std::string controllerId = m_controller->ID();

  event.type                    = GAME_INPUT_EVENT_AXIS;
  event.controller_id           = controllerId.c_str();
  event.port_type               = GAME_PORT_CONTROLLER;
  event.port_address            = m_portAddress.c_str();
  event.feature_name            = feature.c_str();
  event.axis.position           = position;

  return m_gameClient.Input().InputEvent(event);
}

bool CGameClientJoystick::OnThrottleMotion(const std::string& feature, float position, unsigned int motionTimeMs)
{
  game_input_event event;

  std::string controllerId = m_controller->ID();

  event.type                    = GAME_INPUT_EVENT_AXIS;
  event.controller_id           = controllerId.c_str();
  event.port_type               = GAME_PORT_CONTROLLER;
  event.port_address            = m_portAddress.c_str();
  event.feature_name            = feature.c_str();
  event.axis.position           = position;

  return m_gameClient.Input().InputEvent(event);
}

bool CGameClientJoystick::SetRumble(const std::string& feature, float magnitude)
{
  bool bHandled = false;

  if (InputReceiver())
    bHandled = InputReceiver()->SetRumbleState(feature, magnitude);

  return bHandled;
}
