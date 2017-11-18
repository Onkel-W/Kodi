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

namespace PERIPHERALS
{
  class CAndroidJoystickTranslator
  {
  public:
    /*!
     * \brief Translate an axis ID to an Android enum suitable for logging
     *
     * \param axisId The axis ID given in <android/input.h>
     *
     * \return The translated enum label, or "unknown" if unknown
     */
    static const char *TranslateAxis(int axisId);

    /*!
     * \brief Translate a key code to an Android enum suitable for logging
     *
     * \param keyCode The key code given in <android/keycodes.h>
     *
     * \return The translated enum label, or "unknown" if unknown
     */
    static const char *TranslateKeyCode(int keyCode);
  };
}
