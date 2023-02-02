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
#include "Renderer.hpp"
#include <vulkan/vulkan.h>
namespace openttd
{
namespace render
{
namespace vulkanhelpers
{

class VulkanGraphicsCommands : public stm::controlled_copy<VulkanGraphicsCommands>
{
  public:
    controlled_copy_f(VulkanGraphicsCommands);

    ~VulkanGraphicsCommands()
    {
    }

    VulkanGraphicsCommands(const VkDevice &)
    {
    }
};

struct VulkanFrame : public stm::not_copyable
{
  public:
    stm::unique_ptr<VulkanGraphicsCommands> commands = nullptr;
    ~VulkanFrame()
    {
    }

    explicit VulkanFrame(const VkDevice &device)
    {
        commands = stm::make_unique<VulkanGraphicsCommands>(device);
    }
};

} // namespace vulkanhelpers
} // namespace render
} // namespace openttd