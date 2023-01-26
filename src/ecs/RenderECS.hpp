#pragma once
/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2. OpenTTD is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public
 * License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */
#include "../istd/Stm.hpp"
#include "../render/Renderer.hpp"
#include "Component.hpp"
namespace openttd
{
namespace ecs
{
namespace renderer
{
class RenderComponent : public openttd::ecs::Component
{
  public:
    RenderComponent()
        : openttd::ecs::Component([]() {
          puts("RenderComponent Activated");
        })
    {
    }

    virtual ~RenderComponent() override = default;
};

class RenderSystem : public openttd::ecs::ISystem<RenderComponent>
{
  public:
    void run(ECSManager *ecs_in) final override
    {
        openttd::render::Renderer::get()->begin();
        try
        {
            RenderSystem::ISystem<RenderComponent>::run(ecs_in);
        }
        catch (const std::exception)
        {
            return;
        }
        openttd::render::Renderer::get()->end();
        submitEvent(newEvent(openttd::drivers::DBusEventData::DBusEvent::FINISHED_DRAW));
    }
};
} // namespace renderer
} // namespace ecs
} // namespace openttd