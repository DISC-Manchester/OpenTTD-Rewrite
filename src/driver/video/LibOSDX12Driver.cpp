#include "../../render/Directx12.hpp"
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
    ComPtr<ID3D12Device10> device{nullptr};
    bool physicalDeviceFeatureCheck(ComPtr<IDXGIAdapter4> physical_device)
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
        : LibOSBaseWindowDriver(2)
    {
    }

    const char *start() final override
    {
        ComPtr<IDXGIAdapter4> physical_device{nullptr};
        const char *result = nullptr;
        if ((result = LibOSBaseWindowDriver::start()) != nullptr)
            return result;
        losWindowWin32 *native_window = (losWindowWin32 *)losGetWindowNativePointer(window);
        UINT graphics_interface_flags = 0;
#if WITH_DEBUG
        ComPtr<ID3D12Debug6> debug_layers{nullptr};
        if (FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(debug_layers.GetAddressOf()))))
            return "failed to get Debug layers for directx";
        debug_layers->EnableDebugLayer();
        debug_layers->SetEnableAutoName(TRUE);
        debug_layers->SetEnableSynchronizedCommandQueueValidation(TRUE);
        debug_layers.Reset();
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
            physical_device.Reset();
        }

        if (physicalDeviceFeatureCheck(physical_device))
            return "directx could not find a usable support gpu";

        if (FAILED(
                D3D12CreateDevice(physical_device.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(device.GetAddressOf()))))
            return "directx could not create device binding";
#if WITH_DEBUG
        ComPtr<ID3D12InfoQueue> queue_info;
        if (FAILED(device->QueryInterface(IID_PPV_ARGS(queue_info.GetAddressOf()))))
            return "directx debug could not set up device debug messages";
        queue_info->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
        queue_info->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        queue_info->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        queue_info.Reset();
#endif
        physical_device.Reset();
        delete native_window;
        if (tested)
            openttd::render::Renderer::get()->use(new openttd::render::Directx12Renderer(instance, device));
        return nullptr;
    }

    void mainLoop() final override
    {
        using namespace std::chrono;
        using namespace std::literals;
        while (losUpdateWindow(window) != LOS_WINDOW_CLOSE)
        {
            system_clock::time_point start_time = system_clock::now();
            if (eventToHandle(drivers::DBusEventData::DBusEvent::PRESENT_FRAME))
                openttd::render::Renderer::get()->present();
            system_clock::time_point end_time = system_clock::now();
            std::this_thread::sleep_for((end_time - start_time) - 16.5ms);
        }
        submitImportantEvent(newEvent(DBusEventData::DBusEvent::CLOSED_WINDOW));
        flushBus();
    }

    void stop() final override
    {
        if (tested)
            openttd::render::Renderer::get()->use(nullptr);
        if (instance)
            instance.Reset();
#if WITH_DEBUG
        ComPtr<ID3D12InfoQueue> queue_info;
        device->QueryInterface(IID_PPV_ARGS(queue_info.GetAddressOf()));
        queue_info->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, FALSE);
        queue_info->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, FALSE);
        queue_info->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, FALSE);
        queue_info.Reset();

        ComPtr<ID3D12DebugDevice2> debug;
        device->QueryInterface(IID_PPV_ARGS(debug.GetAddressOf()));

#endif
        if (device)
            device.Reset();

#if WITH_DEBUG
        debug->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
        debug.Reset();
#endif
        LibOSBaseWindowDriver::stop();
    }
    const char *getName() final override
    {
        return "LIBOS -> (DX12)Video Driver";
    }
};
static LibOSDX12WindowDriver libos_dx12_driver;
} // namespace drivers
} // namespace openttd