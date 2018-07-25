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

#include "games/controllers/types/ControllerTree.h"
#include "games/GameTypes.h"

#include <memory>

struct game_input_topology;
struct game_input_port;
struct game_input_device;

namespace KODI
{
namespace GAME
{
  class CGameClientTopology
  {
  public:
    CGameClientTopology() = default;
    CGameClientTopology(GameClientPortVec ports, int playerLimit);

    void Clear();

    int PlayerLimit() const { return m_playerLimit; }

    const CControllerTree &ControllerTree() const { return m_controllers; }
    CControllerTree &ControllerTree() { return m_controllers; }

    game_input_topology *GetTopology() const;
    void FreeTopology(game_input_topology *inputTopology) const;

  private:
    static CControllerTree GetControllerTree(const GameClientPortVec &ports);
    static CControllerPortNode GetPortNode(const GameClientPortPtr &port, const std::string &address);
    static CControllerNode GetControllerNode(const GameClientDevicePtr &device, const std::string &portAddress);

    static game_input_topology *GetTopology(const ControllerPortVec &ports, int playerLimit);
    static void GetPort(const CControllerPortNode &portNode, game_input_port &gamePort);
    static void FreePort(game_input_port &gamePort);
    static void GetDevice(const CControllerNode &controllerNode, game_input_device &gameDevice);
    static void FreeDevice(game_input_device &gameDevice);

    // Utility function
    static std::string MakeAddress(const std::string &baseAddress, const std::string &nodeId);

    // Game API parameters
    GameClientPortVec m_ports;
    int m_playerLimit = -1;

    // Controller parameters
    CControllerTree m_controllers;
  };
}
}
