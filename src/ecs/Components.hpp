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
#include "System.hpp"
namespace openttd
{
namespace ecs
{
namespace renderer
{
struct IRenderComponent : public openttd::ecs::Component
{
    IRenderComponent(void (*callback_in)(openttd::ecs::Component*))
        : openttd::ecs::Component(callback_in)
    {
    }
    virtual ~IRenderComponent() override = default;
};

class SpriteRenderComponent : public IRenderComponent
{
  public:
    const openttd::render::IBuffer* vertex_buffer;
  
    SpriteRenderComponent(const char *)
        : IRenderComponent(SpriteRenderComponent::run)
    {
            const float g_vertex_buffer_data[9] = {
                -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            };
            vertex_buffer = openttd::render::Renderer::get()->createBuffer();
    }
    static void run(openttd::ecs::Component* componant)
    {
        openttd::render::Renderer::get()->bindBuffer(static_cast<SpriteRenderComponent*>(componant)->vertex_buffer);
        openttd::render::Renderer::get()->draw();
    }

    virtual ~SpriteRenderComponent() override
    {
        openttd::render::Renderer::get()->destroyBuffer(vertex_buffer);
    }
};
} // namespace renderer
} // namespace ecs
} // namespace openttd