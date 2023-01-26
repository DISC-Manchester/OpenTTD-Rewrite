#pragma once
/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2. OpenTTD is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public
 * License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */
#include "Renderer.hpp"
#include <vulkan/vulkan_core.h>
namespace openttd
{
namespace render
{
class VulkanRenderer : public IRender
{
    VkInstance &instance;
    VkDevice &device;
    VkSurfaceKHR &surface;
  public:
    VulkanRenderer(VkInstance &instance_in, VkDevice &device_in, VkSurfaceKHR &surface_in)
        : instance(instance_in)
        , device(device_in)
        , surface(surface_in)
    {
    }

    void begin() final override
    {
    }

    void end() final override
    {
    }

    void present() final override
    {
    }
};
} // namespace render
} // namespace openttd