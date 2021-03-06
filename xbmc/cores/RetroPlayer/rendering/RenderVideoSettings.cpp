/*
 *  Copyright (C) 2017-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "RenderVideoSettings.h"

using namespace KODI;
using namespace RETRO;

#define VIDEO_FILTER_NEAREST  "nearest"
#define VIDEO_FILTER_LINEAR   "linear"

void CRenderVideoSettings::Reset()
{
  m_shaderPreset.clear();
  m_scalingMethod = SCALINGMETHOD::AUTO;
  m_viewMode = VIEWMODE::Normal;
  m_rotationDegCCW = 0;
}

bool CRenderVideoSettings::operator==(const CRenderVideoSettings &rhs) const
{
  return m_shaderPreset == rhs.m_shaderPreset &&
         m_scalingMethod == rhs.m_scalingMethod &&
         m_viewMode == rhs.m_viewMode &&
         m_rotationDegCCW == rhs.m_rotationDegCCW;
}

bool CRenderVideoSettings::operator<(const CRenderVideoSettings &rhs) const
{
  if (m_shaderPreset < rhs.m_shaderPreset) return true;
  if (m_shaderPreset > rhs.m_shaderPreset) return false;

  if (m_scalingMethod < rhs.m_scalingMethod) return true;
  if (m_scalingMethod > rhs.m_scalingMethod) return false;

  if (m_viewMode < rhs.m_viewMode) return true;
  if (m_viewMode > rhs.m_viewMode) return false;

  if (m_rotationDegCCW < rhs.m_rotationDegCCW) return true;
  if (m_rotationDegCCW > rhs.m_rotationDegCCW) return false;

  return false;
}

std::string CRenderVideoSettings::GetVideoFilter() const
{
  switch (m_scalingMethod)
  {
  case SCALINGMETHOD::NEAREST:
    return VIDEO_FILTER_NEAREST;
  case SCALINGMETHOD::LINEAR:
    return VIDEO_FILTER_LINEAR;
  default:
    break;
  }

  if (!m_shaderPreset.empty())
    return m_shaderPreset;

  return "";
}

void CRenderVideoSettings::SetVideoFilter(const std::string &videoFilter)
{
  if (videoFilter == VIDEO_FILTER_NEAREST)
  {
    m_shaderPreset.clear();
    m_scalingMethod = SCALINGMETHOD::NEAREST;
  }
  else if (videoFilter == VIDEO_FILTER_LINEAR)
  {
    m_shaderPreset.clear();
    m_scalingMethod = SCALINGMETHOD::LINEAR;
  }
  else
  {
    m_shaderPreset = videoFilter;
    m_scalingMethod = SCALINGMETHOD::AUTO;
  }
}
