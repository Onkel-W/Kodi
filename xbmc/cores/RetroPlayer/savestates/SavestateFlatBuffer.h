/*
 *      Copyright (C) 2012-2017 Team Kodi
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

#include "ISavestate.h"

#include <memory>

namespace flatbuffers
{
  class FlatBufferBuilder;
}

namespace KODI
{
namespace RETRO
{
  struct Savestate;
  struct SavestateBuilder;

  class CSavestateFlatBuffer : public ISavestate
  {
  public:
    CSavestateFlatBuffer();
    ~CSavestateFlatBuffer();

    // Implementation of ISavestate
    void Reset() override;
    bool GetData(uint8_t *&data, size_t &size) override;
    SAVE_TYPE Type() const override;
    uint8_t Slot() const override;
    std::string Label() const override;
    CDateTime Created() const override;
    std::string GameFileName() const override;
    uint64_t TimestampFrames() const override;
    double TimestampWallClock() const override;
    std::string GameClientID() const override;
    std::string GameClientVersion() const override;
    const uint8_t *GetMemoryData() const override;
    size_t GetMemorySize() const override;
    void SetType(SAVE_TYPE type) override;
    void SetSlot(uint8_t slot) override;
    void SetLabel(const std::string &label) override;
    void SetCreated(const CDateTime &created) override;
    void SetGameFileName(const std::string &gameFileName) override;
    void SetTimestampFrames(uint64_t timestampFrames) override;
    void SetTimestampWallClock(double timestampWallClock) override;
    void SetGameClientID(const std::string &gameClient) override;
    void SetGameClientVersion(const std::string &gameClient) override;
    uint8_t *GetMemoryBuffer(size_t size) override;
    void Finalize() override;
    bool Deserialize(std::vector<uint8_t> data) override;

  private:
    /*!
     * \brief Helper class to hold data needed in creation of a FlatBuffer
     *
     * The builder is used when deserializing from individual fields.
     */
    std::unique_ptr<flatbuffers::FlatBufferBuilder> m_builder;

    /*!
    * \brief Helper class to build the nested Savestate table
    */
    std::unique_ptr<SavestateBuilder> m_savestateBuilder;

    /*!
     * \brief System memory storage (for deserializing savestates)
     *
     * This memory is used when deserializing from a vector.
     */
    std::vector<uint8_t> m_data;

    /*!
     * \brief FlatBuffer struct used for accessing data
     */
    const Savestate *m_savestate = nullptr;
  };
}
}
