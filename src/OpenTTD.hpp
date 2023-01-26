#pragma once
/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2. OpenTTD is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public
 * License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */
#include "driver/video/VideoDriver.hpp"
#include "ecs/ECS.hpp"
#include "ecs/RenderECS.hpp"
#include "istd/Stm.hpp"
namespace openttd
{
static ecs::renderer::RenderSystem render_system;
static ecs::ECSManager ecs_manager;
int gameMain()
{
    drivers::VideoDriver video{};
    (void)drivers::DriverRegistry::get(true);
    ecs::Entity entity = ecs_manager.getNewEntity();
    ecs_manager.addComponent(entity,new ecs::renderer::RenderComponent()); 
    bool running = true;
    while (running)
    {
        using namespace std::chrono;
        using namespace std::literals;
        system_clock::time_point start_time = system_clock::now();
        render_system.run(&ecs_manager);
        if (eventToHandle(drivers::DBusEventData::DBusEvent::CLOSED_WINDOW))
        {
            running = false;
        }
        if (eventToHandle(drivers::DBusEventData::DBusEvent::FINISHED_DRAW))
        {
            submitEvent(newEvent(drivers::DBusEventData::DBusEvent::PRESENT_FRAME));
        }
        system_clock::time_point end_time = system_clock::now();
        std::this_thread::sleep_for((end_time - start_time) - 16.5ms);
    }
    ecs_manager.returnEntity(entity);
    return 0;
}
} // namespace openttd