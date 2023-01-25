#include "LibOSDriver.hpp"
#include <libos/NativeWindow.h>
#include <vulkan/vulkan.h>
#if __has_include(<wayland-client.h>)
#    include <wayland-client.h>
//-- don't flip
#    include <vulkan/vulkan_wayland.h>

#endif
#if __has_include(<xcb/xcb.h>)
#    include <xcb/xcb.h>
//-- don't flip
#    include <vulkan/vulkan_xcb.h>
#endif // DEBUG

namespace openttd::drivers
{

class LibOSVulkanWindowDriver : public LibOSBaseWindowDriver
{
    VkInstance instance;
    VkDevice device;
    VkPhysicalDevice physical_device;
    VkSurfaceKHR surface;
    uint32_t graphics_family;

    // NOTE: 24/jan/2023 Pheonixfirewingz(Digitech) this function generate by chatGPT as i'm only a vulkan novice
    VkPhysicalDevice findBestVulkanDevice(VkInstance instance)
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if (deviceCount == 0)
        {
            throw std::runtime_error("No physical devices with Vulkan support found");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        VkPhysicalDevice bestDevice = VK_NULL_HANDLE;
        int bestScore = -1;
        for (const VkPhysicalDevice &device : devices)
        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(device, &properties);

            int score = 0;
            // Check for discrete GPU
            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                score += 1000;
            }

            // Check for supported features
            VkPhysicalDeviceFeatures features;
            vkGetPhysicalDeviceFeatures(device, &features);
            if (features.geometryShader)
            {
                score += 500;
            }
            if (features.tessellationShader)
            {
                score += 400;
            }
            if (features.samplerAnisotropy)
            {
                score += 100;
            }

            // Check for suitable memory properties
            VkPhysicalDeviceMemoryProperties memoryProperties;
            vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);
            for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
            {
                if ((VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT & memoryProperties.memoryTypes[i].propertyFlags) ==
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
                {
                    score += 100;
                    break;
                }
            }

            // Check for suitable queue families
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
            for (const VkQueueFamilyProperties &queueFamily : queueFamilies)
            {
                if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    score += 10;
                }
            }

            if (score > bestScore)
            {
                bestDevice = device;
                bestScore = score;
            }
        }

        if (bestDevice == VK_NULL_HANDLE)
        {
            throw std::runtime_error("No suitable device found");
        }

        return bestDevice;
    }

  public:
    LibOSVulkanWindowDriver()
        : LibOSBaseWindowDriver(1)
    {
    }

    const char *start() override
    {
        const char *result = nullptr;
        if ((result = LibOSBaseWindowDriver::start()) != nullptr)
            return result;

        VkApplicationInfo app_info{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.apiVersion = VK_MAKE_VERSION(1, 0, 3);
        app_info.engineVersion = 1;
        app_info.pEngineName = "OpenTTD Integrated Engine";
        app_info.pApplicationName = "OpenTTD";

        const std::vector<const char *> validation_layers = {"VK_LAYER_KHRONOS_validation"};
        const std::vector<const char *> instance_enable_extendions = {
            VK_KHR_SURFACE_EXTENSION_NAME,
#    if __has_include(<wayland-client.h>)
            VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
#    endif
#    if __has_include(<xcb/xcb.h>)
            VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#    endif
        };

        VkInstanceCreateInfo inst_info{};
        inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        inst_info.enabledLayerCount = validation_layers.size();
        inst_info.enabledExtensionCount = instance_enable_extendions.size();
        inst_info.ppEnabledExtensionNames = instance_enable_extendions.data();
        inst_info.ppEnabledLayerNames = validation_layers.data();
        inst_info.pApplicationInfo = &app_info;
        if (vkCreateInstance(&inst_info, nullptr, &instance) != VK_SUCCESS)
            return "Vulkan API error creating instance";

        physical_device = findBestVulkanDevice(instance);

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queueFamilyCount, queueFamilies.data());

        uint32_t i = 0;
        for (const auto &queueFamily : queueFamilies)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                graphics_family = i;
            i++;
        }

        VkDeviceQueueCreateInfo queue_info{};
        queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info.queueFamilyIndex = graphics_family;
        queue_info.queueCount = 1;

        float queue_priority = 1.0f;
        queue_info.pQueuePriorities = &queue_priority;

        VkPhysicalDeviceFeatures device_features{};

        VkDeviceCreateInfo dev_info{};
        dev_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        dev_info.pEnabledFeatures = &device_features;
        dev_info.enabledExtensionCount = 0;
        dev_info.enabledLayerCount = 0;
        dev_info.ppEnabledExtensionNames = nullptr;
        dev_info.ppEnabledLayerNames = nullptr;
        dev_info.queueCreateInfoCount = 1;
        dev_info.pQueueCreateInfos = &queue_info;

        if (vkCreateDevice(physical_device, &dev_info, nullptr, &device) != VK_SUCCESS)
            return "Vulkan API error creating device";

        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(physical_device, &deviceProperties);
        if (tested)
            printf("Vulkan Device Created/Bound: %s\n", deviceProperties.deviceName);

        switch (losWhatApiIsWindowUsed(window))
        {
        case WIN32_API:
            throw new std::runtime_error("Vulkan wsi is not implemented on Windows");
#    if __has_include(<wayland-client.h>)
        case WAYLAND_API: {
            losWindowWayland *native_link = (losWindowWayland *)losGetWindowNativePointer(window);
            VkWaylandSurfaceCreateInfoKHR surface_info{};
            surface_info.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
            surface_info.surface = (struct wl_surface *)native_link->surface;
            surface_info.display = (struct wl_display *)native_link->display;
            vkCreateWaylandSurfaceKHR(instance, &surface_info, nullptr, &surface);
            if (tested)
                puts("Vulkan Created Wayland surface WSI");
            break;
        }
#    endif
#    if __has_include(<xcb/xcb.h>)
        case XCB_API: {
            losWindowXCB *native_link = (losWindowXCB *)losGetWindowNativePointer(window);
            VkXcbSurfaceCreateInfoKHR surface_info{};
            surface_info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
            surface_info.connection = (xcb_connection_t *)native_link->connection;
            surface_info.window = *(xcb_window_t *)native_link->window;
            vkCreateXcbSurfaceKHR(instance, &surface_info, nullptr, &surface);
            if (tested)
                puts("Vulkan Created Wayland surface WSI");
            break;
        }
#    endif
        }
        return nullptr;
    }

    void mainLoop() final override
    {
        while (losUpdateWindow(window) != LOS_WINDOW_CLOSE)
            ;

        DBusEventData data;
        data.event = DBusEventData::DBusEvent::CLOSED_WINDOW;
        data.data = nullptr;
        DBus::get()->submit(data);
    }

    void stop() override
    {
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyDevice(device, nullptr);
        vkDestroyInstance(instance, nullptr);
        LibOSBaseWindowDriver::stop();
    }
    const char *getName() override
    {
        return "LIBOS -> (Vulkan)Video Driver";
    }
};
static LibOSVulkanWindowDriver libos_vulkan_driver;
} // namespace openttd::drivers