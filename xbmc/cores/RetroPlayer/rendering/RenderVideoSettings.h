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

#include "cores/IPlayer.h"

namespace KODI
{
namespace RETRO
{
  /*!
   * \brief Video settings provided by the rendering system
   */
  class CRenderVideoSettings
  {
  public:
    CRenderVideoSettings() { Reset(); }

    void Reset();

    bool operator==(const CRenderVideoSettings &rhs) const;
    bool operator!=(const CRenderVideoSettings &rhs) const { return !(*this == rhs); }
    bool operator<(const CRenderVideoSettings &rhs) const;
    bool operator>(const CRenderVideoSettings &rhs) const { return !(*this == rhs || *this < rhs); }

    const std::string &GetShaderPreset() const { return m_shaderPreset; }
    void SetShaderPreset(const std::string &shaderPreset) { m_shaderPreset = shaderPreset; }

    ESCALINGMETHOD GetScalingMethod() const { return m_scalingMethod; }
    void SetScalingMethod(ESCALINGMETHOD method) { m_scalingMethod = method; }

    ViewMode GetRenderViewMode() const { return m_viewMode; }
    void SetRenderViewMode(ViewMode mode) { m_viewMode = mode; }

  private:
    std::string m_shaderPreset;
    ESCALINGMETHOD m_scalingMethod;
    ViewMode m_viewMode;
  };
}
}
