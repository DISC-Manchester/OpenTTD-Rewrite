#include "LibOSDriver.hpp"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <libos/NativeWindow.h>
#include <wrl/client.h>
#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "D3D12.lib")
namespace openttd
{
namespace drivers
{
using namespace Microsoft::WRL;
class LibOSDX12WindowDriver : public LibOSBaseWindowDriver
{
    ComPtr<IDXGIFactory7> instance{nullptr};
    ComPtr<IDXGIAdapter4> physical_device{nullptr};
    ComPtr<ID3D12Device10> device{nullptr};
    bool physicalDeviceFeatureCheck()
    {
        if (physical_device == nullptr)
            return true;

        const D3D_FEATURE_LEVEL levels[2]{D3D_FEATURE_LEVEL_12_0, D3D_FEATURE_LEVEL_12_1};
        D3D12_FEATURE_DATA_FEATURE_LEVELS level_info{};
        level_info.NumFeatureLevels = _countof(levels);
        level_info.pFeatureLevelsRequested = levels;
        ComPtr<ID3D12Device10> temp_device;
        if (FAILED(D3D12CreateDevice(physical_device.Get(), D3D_FEATURE_LEVEL_12_0,
                                     IID_PPV_ARGS(temp_device.GetAddressOf()))))
            return true;

        
        if (FAILED(temp_device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &level_info, sizeof(level_info))))
            return true;
        return false;
    }

  public:
    LibOSDX12WindowDriver()
        : LibOSBaseWindowDriver(1)
    {
    }

    const char *start() override
    {
        const char *result = nullptr;
        if ((result = LibOSBaseWindowDriver::start()) != nullptr)
            return result;
        losWindowWin32 *native_window = (losWindowWin32 *)losGetWindowNativePointer(window);
        UINT graphics_interface_flags = 0;
#if WITH_DEBUG
        graphics_interface_flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
        if (FAILED(CreateDXGIFactory2(graphics_interface_flags, IID_PPV_ARGS(instance.GetAddressOf()))))
        {
            stop();
            return "failed to create directx 12 instance";
        }

        for (UINT i = 0;
             instance->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                                                  IID_PPV_ARGS(physical_device.GetAddressOf())) != DXGI_ERROR_NOT_FOUND;
             i++)
        {
            if (SUCCEEDED(D3D12CreateDevice(physical_device.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device10),
                                            nullptr)))
                break;
            physical_device->Release();
            physical_device = nullptr;
        }

        if (physicalDeviceFeatureCheck())
            return "directx could not find a usable support gpu";

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
        if (instance)
            instance->Release();
        if (physical_device)
            physical_device->Release();
        if (device)
            device->Release();
        LibOSBaseWindowDriver::stop();
    }
    const char *getName() override
    {
        return "LIBOS -> (DX12)Video Driver";
    }
};
static LibOSDX12WindowDriver libos_dx12_driver;
} // namespace drivers
} // namespace openttd