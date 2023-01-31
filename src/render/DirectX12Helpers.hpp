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
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
namespace openttd
{
namespace render
{
namespace dx12helpers
{
using namespace Microsoft::WRL;
template<const D3D12_COMMAND_LIST_TYPE type>
class Directx12Commands
    : public IUnknown
    , stm::controlled_copy<Directx12Commands<type>>
{
    ComPtr<ID3D12CommandQueue> command_queue = nullptr;
    ComPtr<ID3D12GraphicsCommandList7> command_list = nullptr;
    ComPtr<ID3D12Fence1> queue_fence;
    ULONG refs = 0;

  public:
    stm::stmuint fence_value;
    controlled_copy_f(Directx12Commands<type>);
    constRef(ComPtr<ID3D12GraphicsCommandList7>) Get() const
    {
        return command_list;
    }

    constRef(ComPtr<ID3D12Fence1>) GetFence() const
    {
        return queue_fence;
    }

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,
                                                     _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject) override
    {
        return 0;
    }

    virtual ULONG STDMETHODCALLTYPE AddRef() override
    {
        return ++refs;
    }

    virtual ULONG STDMETHODCALLTYPE Release() override
    {
        if (command_queue.Get())
            command_queue.Reset();
        if (command_list.Get())
            command_list.Reset();
        return --refs;
    }
    Directx12Commands(_In_opt_ constRef(ComPtr<ID3D12Device10>) device,
                      _In_opt_ constRef(ComPtr<ID3D12CommandAllocator>) frame_allocator)
    {
        D3D12_COMMAND_QUEUE_DESC description{};
        description.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        description.NodeMask = 0;
        description.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        description.Type = type;
        if (FAILED(device->CreateCommandQueue(&description, IID_PPV_ARGS(&command_queue))))
        {
            Release();
            return;
        }

        if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&queue_fence))))
        {
            Release();
            return;
        }

        if (FAILED(device->CreateCommandList(0, type, frame_allocator.Get(), nullptr, IID_PPV_ARGS(&command_list))))
        {
            Release();
            return;
        }
        command_list->Close();
    }

    void Reset(_In_opt_ constRef(ComPtr<ID3D12CommandAllocator>) frame_allocator)
    {
        if (FAILED(command_list->Reset(frame_allocator.Get(), nullptr)))
        {
            std::puts("DirectX12: <ERROR> -> failed to reset command list");
        }
    }

    void Excute(stm::stmuint f_value)
    {
        ID3D12CommandList *const lists[]{command_list.Get()};
        command_queue->ExecuteCommandLists(_countof(lists), &lists[0]);
        command_queue->Signal(queue_fence.Get(), f_value);
    }

    void wait(HANDLE fence_event)
    {
        if (queue_fence->GetCompletedValue() < fence_value)
        {
            if (FAILED(queue_fence->SetEventOnCompletion(fence_value, fence_event)))
            {
                std::puts("DirectX12 <ERROR> -> failed to set fence event");
            }
            WaitForSingleObject(fence_event, INFINITE);
        }
    }
};

struct Directx12Frame
    : public IUnknown
    , stm::controlled_copy<Directx12Frame>
{
    ComPtr<ID3D12CommandAllocator> frame_allocator = nullptr;
    ComPtr<Directx12Commands<D3D12_COMMAND_LIST_TYPE_DIRECT>> commands = nullptr;
    ULONG refs = 0;
    controlled_copy_f(Directx12Frame);
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,
                                                     _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject) override
    {
        return 0;
    }

    void Reset()
    {
        if (FAILED(frame_allocator->Reset()))
        {
            std::puts("DirectX12: <ERROR> -> failed to reset frame_allocator");
        }
        commands->Reset(frame_allocator);
    }
    virtual ULONG STDMETHODCALLTYPE AddRef() override
    {
        return ++refs;
    }

    virtual ULONG STDMETHODCALLTYPE Release() override
    {
        if (frame_allocator.Get())
            frame_allocator.Reset();
        if (commands.Get())
            commands.Reset();
        return --refs;
    }

    explicit Directx12Frame(_In_opt_ constRef(ComPtr<ID3D12Device10>) device)
    {
        if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&frame_allocator))))
        {
            Release();
            return;
        }

        commands = new Directx12Commands<D3D12_COMMAND_LIST_TYPE_DIRECT>(device, frame_allocator);
    }
};

struct Directx12UniformManager
    : public IUnknown
    , stm::controlled_copy<Directx12UniformManager>
{
    ULONG refs = 0;
  public:
    controlled_copy_f(Directx12UniformManager);

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,
                                                     _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject) override
    {
        return 0;
    }

    virtual ULONG STDMETHODCALLTYPE AddRef() override
    {
        return ++refs;
    }

    virtual ULONG STDMETHODCALLTYPE Release() override
    {
        return --refs;
    }
    explicit Directx12UniformManager(_In_opt_ constRef(ComPtr<ID3D12Device10>) device)
    {
    }
};
} // namespace dx12helpers
} // namespace render
} // namespace openttd