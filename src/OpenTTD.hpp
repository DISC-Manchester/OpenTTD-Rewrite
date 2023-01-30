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
#include "ecs/Systems.hpp"
#include "istd/Stm.hpp"
namespace openttd
{
static ecs::renderer::RenderSystem render_system;
static ecs::EntityManager entity_manager;
int gameMain()
{
    drivers::VideoDriver video{};
    (void)drivers::DriverRegistry::get(true);
    ecs::Entity entity = entity_manager.getNewEntity();
    entity_manager.addComponent(entity, new ecs::renderer::SpriteRenderComponent(nullptr));
    while (true)
    {
        using namespace std::chrono;
        using namespace std::literals;
        system_clock::time_point start_time = system_clock::now();
        if (eventToHandle(drivers::DBusEventData::DBusEvent::CLOSED_WINDOW))
            break;
        //start of game loop

        render_system.run(&entity_manager);

        //end the game loop
        if (eventToHandle(drivers::DBusEventData::DBusEvent::FINISHED_DRAW))
            submitEvent(newEvent(drivers::DBusEventData::DBusEvent::PRESENT_FRAME));
        system_clock::time_point end_time = system_clock::now();
        std::this_thread::sleep_for((end_time - start_time) - 16.5ms);
    }
    entity_manager.returnEntity(entity);
    (void)openttd::render::Renderer::get(true);
    return 0;
}
} // namespace openttd