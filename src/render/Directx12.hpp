#pragma once
/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2. OpenTTD is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public
 * License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */
#include "DirectX12Helpers.hpp"
#include "Renderer.hpp"
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>

namespace openttd
{
namespace render
{
using namespace Microsoft::WRL;
using namespace openttd::render::dx12helpers;

struct DX12Window : public stm::controlled_copy<DX12Window>
{
    const uint32_t width;
    const uint32_t height;
    const void *ptr;

    explicit DX12Window(const uint32_t width_in, const uint32_t height_in, const void *ptr_in)
        : width(width_in)
        , height(height_in)
        , ptr(ptr_in)
    {
    }

    controlled_copy_f(DX12Window)

        ~DX12Window()
    {
    }
};

class Directx12Buffer : public IBuffer
{
  public:
    controlled_copy_f(Directx12Buffer)
};

class Directx12Renderer : public IRender
{
    const ComPtr<IDXGIFactory7> instance;
    const ComPtr<ID3D12Device10> device;
    stm::unique_ptr<Directx12Frame> frames[max_frames]{nullptr};
    stm::unique_ptr<Directx12Frame> window_frame{nullptr};
    stm::unique_ptr<Directx12UniformManager> uniforms_manager;
    stm::stmuint frame_index = 0;
    stm::stmuint fence_value = 0;
    HANDLE fence_event;
    bool render_ready = false;

  protected:
    virtual void transfer() final override
    {
    }

  public:
    //---
    Directx12Renderer(_In_opt_ const ComPtr<IDXGIFactory7> instance_in, _In_opt_ const DX12Window *window,
                      _In_opt_ const ComPtr<ID3D12Device10> device_in, bool win32 = true)
        : instance(instance_in)
        , device(device_in)
    {
        for (stm::stmuint i = 0; i < max_frames; i++)
            frames[i] = stm::make_unique<Directx12Frame>(device);

        window_frame = stm::make_unique<Directx12Frame>(device);
        fence_event = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);

        uniforms_manager = stm::make_unique<Directx12UniformManager>(device);

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount = max_frames;
        swapChainDesc.Width = window->width;
        swapChainDesc.Height = window->height;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1;

        ComPtr<IDXGISwapChain1> swapChain;
        if (win32)
        {
            if (instance->CreateSwapChainForHwnd(window_frame->frame_allocator.Get(), (HWND)window->ptr, &swapChainDesc,
                                                 nullptr, nullptr, &swapChain))
            {
                stm::puts("DX12 <ERROR> -> failed to create swapchain");
            }
        }
    }

    virtual ~Directx12Renderer() final override
    {
        frame_index = 0;
        for (stm::stmuint i = 0; i < max_frames; i++)
            frames[i]->commands->wait(fence_event);

        window_frame->commands->wait(fence_event);
        if (fence_event)
            CloseHandle(fence_event);

        openttd::render::Renderer::get()->tryUnlock();
    }

    void begin() final override
    {
        frames[frame_index]->commands->wait(fence_event);
        frames[frame_index]->Reset();
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
        if (FAILED(frames[frame_index]->commands->Close()))
        {
            std::puts("DirectX12: <ERROR> -> failed to close command list");
        }
        stm::stmuint &fence_value_ref = fence_value;
        ++fence_value_ref;
        frames[frame_index]->commands->fence_value = fence_value_ref;
        frames[frame_index]->commands->Excute(fence_value_ref);
        frame_index = (frame_index + 1) % max_frames;
    }

    void present() final override
    {
    }
};
} // namespace render
} // namespace openttd