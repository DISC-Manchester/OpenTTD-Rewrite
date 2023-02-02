#pragma once
/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2. OpenTTD is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public
 * License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */
#include "VulkanHelpers.hpp"
#include "Renderer.hpp"
#include <vulkan/vulkan_core.h>
namespace openttd
{
namespace render
{
using namespace openttd::render::vulkanhelpers;
class VulkanBuffer : public IBuffer
{
  public:
    controlled_copy_f(VulkanBuffer)
};

class VulkanRenderer : public IRender
{
    VkInstance &instance;
    VkPhysicalDevice &physical_device;
    VkDevice &device;
    VkSurfaceKHR &surface;
    VkQueue present_queue;
    VkQueue draw_queue;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkFormat swapchain_format;
    stm::vector<VkImage> swapchain_images;
    stm::vector<VkImageView> swapchain_image_views;
    bool separate_present_index = false;
    uint32_t present_family_index = 0;
    uint32_t graphics_family_index = 0;

  protected:
    virtual void transfer() final override
    {
    }

    void iCreateSwapchain()
    {
        VkSwapchainKHR old_swapchain = swapchain;

        // Retrieve properties of old swapchain
        VkSurfaceCapabilitiesKHR surface_capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_capabilities);

        uint32_t image_count = surface_capabilities.minImageCount + 1;
        if (surface_capabilities.maxImageCount > 0 && image_count > surface_capabilities.maxImageCount)
        {
            image_count = surface_capabilities.maxImageCount;
        }

        VkSurfaceFormatKHR surface_format;
        {
            uint32_t count;
            vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &count, nullptr);
            stm::vector<VkSurfaceFormatKHR> formats(count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &count, formats.data());

            if (count == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
            {
                surface_format = {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
            }

            for (const auto &format : formats)
            {
                if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                {
                    surface_format = format;
                    goto surfaceEnd;
                }
            }

            surface_format = formats[0];
        }
    surfaceEnd:
        // Create new swapchain
        VkSwapchainCreateInfoKHR swapchain_create_info = {};
        swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchain_create_info.surface = surface;
        swapchain_create_info.minImageCount = image_count;
        swapchain_create_info.imageFormat = surface_format.format;
        swapchain_create_info.imageColorSpace = surface_format.colorSpace;
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);
        if (capabilities.currentExtent.width != stm::numeric_limits<uint32_t>::max())
        {
            swapchain_create_info.imageExtent = capabilities.currentExtent;
        }
        else
        {
            // FIXME: add way to ask driver for the video size;
            int width = 500, height = 500;

            VkExtent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

            actualExtent.width =
                stm::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height =
                stm::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            swapchain_create_info.imageExtent = actualExtent;
        }
        swapchain_create_info.imageArrayLayers = 1;
        swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if (separate_present_index)
        {
            uint32_t index_ptr[2] = {graphics_family_index, present_family_index};
            swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchain_create_info.queueFamilyIndexCount = 2;
            swapchain_create_info.pQueueFamilyIndices = index_ptr;
        }
        else
        {
            swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }
        swapchain_create_info.preTransform = surface_capabilities.currentTransform;
        swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        {
            uint32_t present_mode_count;
            vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);
            stm::vector<VkPresentModeKHR> present_modes(present_mode_count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count,
                                                      present_modes.data());

            VkPresentModeKHR best_mode = VK_PRESENT_MODE_FIFO_KHR;
            for (const auto &mode : present_modes)
            {
                if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
                {
                    swapchain_create_info.presentMode = mode;
                }
                else if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
                {
                    best_mode = mode;
                }
            }
            swapchain_create_info.presentMode = best_mode;
        }
        swapchain_create_info.clipped = VK_TRUE;
        swapchain_create_info.oldSwapchain = old_swapchain;

        vkCreateSwapchainKHR(device, &swapchain_create_info, nullptr, &swapchain);

        if (old_swapchain != VK_NULL_HANDLE)
        {
            if (!swapchain_image_views.empty())
            {
                for (VkImageView view : swapchain_image_views)
                    vkDestroyImageView(device, view, nullptr);
                swapchain_image_views.clear();
            }
            if (!swapchain_images.empty())
                swapchain_image_views.clear();
            vkDestroySwapchainKHR(device, old_swapchain, nullptr);
        }
        {
            uint32_t count = 0;
            vkGetSwapchainImagesKHR(device, swapchain, &count, nullptr);
            swapchain_images.resize(count);
            vkGetSwapchainImagesKHR(device, swapchain, &count, swapchain_images.data());
        }

        swapchain_image_views.resize(swapchain_images.size());

        for (size_t i = 0; i < swapchain_images.size(); i++)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapchain_images[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = surface_format.format;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            vkCreateImageView(device, &createInfo, nullptr, &swapchain_image_views[i]);
        }
    }

    void iPresent()
    {
        vkQueueWaitIdle(draw_queue);
    }

  public:
    VulkanRenderer(VkInstance &instance_in, VkPhysicalDevice &physical_device_in, VkDevice &device_in,
                   VkSurfaceKHR &surface_in, uint32_t graphics_family_index_in)
        : instance(instance_in)
        , device(device_in)
        , physical_device(physical_device_in)
        , surface(surface_in)
        , graphics_family_index(graphics_family_index_in)
    {
        uint32_t count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, nullptr);
        stm::vector<VkQueueFamilyProperties> queues(count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, queues.data());
        int i = 0;
        for (const auto &queue : queues)
        {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &presentSupport);
            if (presentSupport)
            {
                present_family_index = i;
                break;
            }
            i++;
        }
        if (present_family_index != graphics_family_index)
            separate_present_index = true;
        iCreateSwapchain();
        vkGetDeviceQueue(device, graphics_family_index, 0, &draw_queue);
        vkGetDeviceQueue(device, present_family_index, 0, &present_queue);
    }

    virtual ~VulkanRenderer() override
    {
        vkDeviceWaitIdle(device);
        if (!swapchain_image_views.empty())
        {
            for (VkImageView view : swapchain_image_views)
                vkDestroyImageView(device, view, nullptr);
            swapchain_image_views.clear();
        }
        if (!swapchain_images.empty())
            swapchain_image_views.clear();

        vkDestroySwapchainKHR(device, swapchain, nullptr);
    }

    void begin() final override
    {
    }

    const IBuffer *createBuffer() final override
    {
        return nullptr;
    }

    void bindBuffer(const IBuffer *) final override
    {
    }

    void destroyBuffer(const IBuffer *) final override
    {
    }

    void draw() final override
    {
    }

    void end() final override
    {

        // transfer command list ownership
        {
            vkQueueWaitIdle(draw_queue);
        }
    }

    void present() final override
    {
        iPresent();
    }
};
} // namespace render
} // namespace openttd