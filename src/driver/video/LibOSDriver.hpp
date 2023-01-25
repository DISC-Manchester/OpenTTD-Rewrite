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
#include "VideoDriver.hpp"
#include <libos/Window.h>
namespace openttd::drivers
{
class LibOSBaseWindowDriver : public IVideoDriver
{
  protected:
    losWindow window;

  public:
    LibOSBaseWindowDriver(int priority_in)
        : IVideoDriver(priority_in)
    {
    }

    const char *start() override
    {
        losResult result{};
        losWindowInfo info;
        info.title = "test";
        info.title_size = 4;
        info.window_size = {500, 500};
        if ((result = losCreateWindow(&window, info)) != LOS_SUCCESS)
            return "Error creating window";
        return nullptr;
    }
    void stop() override
    {
        losDestroyWindow(window);
    }

    const char *getName() override
    {
        return "LIBOS -> Video Driver";
    }
};
} // namespace drivers