#pragma once
/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2. OpenTTD is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public
 * License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */
#include "../../istd/Stm.hpp"
#include "../Driver.hpp"
namespace openttd
{
namespace drivers
{
class IVideoDriver : public IDriver
{
    uint8_t priority = 0;

  public:
    IVideoDriver(uint8_t priority_in);
    virtual DriverType getType() const noexcept override final
    {
        return DriverType::VIDEO;
    }

    uint8_t getPriority() const noexcept
    {
        return priority;
    }
};

class VideoDriver
{
    std::thread *driver_thread;
  public:
    VideoDriver();
    ~VideoDriver();
};
} // namespace drivers
} // namespace openttd