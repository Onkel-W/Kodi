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

#include "GameClientTopology.h"
#include "GameClientDevice.h"
#include "GameClientPort.h"
#include "addons/kodi-addon-dev-kit/include/kodi/kodi_game_types.h"
#include "games/addons/GameClientTranslator.h"
#include "games/controllers/Controller.h"

#include <sstream>
#include <utility>

using namespace KODI;
using namespace GAME;

#define CONTROLLER_ADDRESS_SEPARATOR  "/"

CGameClientTopology::CGameClientTopology(GameClientPortVec ports, int playerLimit) :
  m_ports(std::move(ports)),
  m_playerLimit(playerLimit),
  m_controllers(GetControllerTree(m_ports))
{
}

void CGameClientTopology::Clear()
{
  m_ports.clear();
  m_controllers.Clear();
}

CControllerTree CGameClientTopology::GetControllerTree(const GameClientPortVec &ports)
{
  CControllerTree tree;

  ControllerPortVec controllerPorts;
  for (const GameClientPortPtr &port : ports)
  {
    CControllerPortNode portNode = GetPortNode(port, "");
    controllerPorts.emplace_back(std::move(portNode));
  }

  tree.SetPorts(std::move(controllerPorts));

  return tree;
}

CControllerPortNode CGameClientTopology::GetPortNode(const GameClientPortPtr &port, const std::string &address)
{
  CControllerPortNode portNode;

  std::string portAddress = MakeAddress(address, port->ID());

  portNode.SetConnected(false);
  portNode.SetPortType(port->PortType());
  portNode.SetPortID(port->ID());
  portNode.SetAddress(portAddress);

  ControllerNodeVec nodes;
  for (const GameClientDevicePtr &device : port->Devices())
  {
    CControllerNode controllerNode = GetControllerNode(device, portAddress);
    nodes.emplace_back(std::move(controllerNode));
  }
  portNode.SetCompatibleControllers(std::move(nodes));

  return portNode;
}

CControllerNode CGameClientTopology::GetControllerNode(const GameClientDevicePtr &device, const std::string &address)
{
  CControllerNode controllerNode;

  std::string controllerAddress = MakeAddress(address, device->Controller()->ID());

  controllerNode.SetController(device->Controller());
  controllerNode.SetAddress(controllerAddress);

  ControllerPortVec ports;
  for (const GameClientPortPtr &port : device->Ports())
  {
    CControllerPortNode portNode = GetPortNode(port, controllerAddress);
    ports.emplace_back(std::move(portNode));
  }

  CControllerHub controllerHub;
  controllerHub.SetPorts(std::move(ports));
  controllerNode.SetHub(std::move(controllerHub));

  return controllerNode;
}

game_input_topology *CGameClientTopology::GetTopology() const
{
  game_input_topology *inputTopology = nullptr;

  const ControllerPortVec &ports = m_controllers.Ports();
  if (!ports.empty())
    inputTopology = GetTopology(ports, m_playerLimit);

  return inputTopology;
}

game_input_topology *CGameClientTopology::GetTopology(const ControllerPortVec &ports, int playerLimit)
{
  game_input_topology *topology = new game_input_topology;

  topology->port_count = ports.size();
  topology->ports = new game_input_port[topology->port_count];
  topology->player_limit = playerLimit;

  for (unsigned int i = 0; i < topology->port_count; i++)
  {
    const CControllerPortNode &portNode = ports.at(i);
    GetPort(portNode, topology->ports[i]);
  }

  return topology;
}

void CGameClientTopology::FreeTopology(game_input_topology *inputTopology) const
{
  if (inputTopology)
  {
    for (unsigned int i = 0; i < inputTopology->port_count; i++)
      FreePort(inputTopology->ports[i]);

    delete[] inputTopology->ports;
    delete inputTopology;
  }
}

void CGameClientTopology::GetPort(const CControllerPortNode &portNode, game_input_port &gamePort)
{
  gamePort.type = CGameClientTranslator::TranslatePortType(portNode.PortType());
  gamePort.port_id = const_cast<char*>(portNode.PortID().c_str());

  const CControllerNode &controllerNode = portNode.ActiveController();
  const ControllerPtr &controller = controllerNode.Controller();

  if (controller)
  {
    gamePort.accepted_devices = new game_input_device;
    gamePort.device_count = 1;

    game_input_device &gameDevice = gamePort.accepted_devices[0];
    GetDevice(controllerNode, gameDevice);
  }
}

void CGameClientTopology::FreePort(game_input_port &gamePort)
{
  for (unsigned int i = 0; i < gamePort.device_count; i++)
  {
    game_input_device &gameDevice = gamePort.accepted_devices[i];
    FreeDevice(gameDevice);
  }

  delete[] gamePort.accepted_devices;
}

void CGameClientTopology::GetDevice(const CControllerNode &controllerNode, game_input_device &gameDevice)
{
  gameDevice.controller_id = const_cast<char*>(controllerNode.Controller()->ID().c_str());

  const ControllerPortVec &ports = controllerNode.Hub().Ports();
  if (!ports.empty())
  {
    gameDevice.port_count = ports.size();
    gameDevice.available_ports = new game_input_port[gameDevice.port_count];

    for (unsigned int i = 0; i < gameDevice.port_count; i++)
    {
      game_input_port &gamePort = gameDevice.available_ports[i];
      GetPort(ports[i], gamePort);
    }
  }
}

void CGameClientTopology::FreeDevice(game_input_device &gameDevice)
{
  for (unsigned int j = 0; j < gameDevice.port_count; j++)
    FreePort(gameDevice.available_ports[j]);

  delete[] gameDevice.available_ports;
}

std::string CGameClientTopology::MakeAddress(const std::string &baseAddress, const std::string &nodeId)
{
  std::ostringstream address;

  if (!baseAddress.empty())
    address << baseAddress;

  address << CONTROLLER_ADDRESS_SEPARATOR << nodeId;

  return address.str();
}
