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
class Directx12GraphicsCommands : public stm::controlled_copy<Directx12GraphicsCommands>
{
    ComPtr<ID3D12CommandQueue> command_queue = nullptr;
    ComPtr<ID3D12GraphicsCommandList7> command_list = nullptr;
    ComPtr<ID3D12Fence1> queue_fence;

  public:
    stm::stmuint fence_value = 0;
    controlled_copy_f(Directx12GraphicsCommands);

    const ComPtr<ID3D12Fence1> GetFence() const
    {
        return queue_fence;
    }

     const HRESULT Close()
    {
        return command_list->Close();
    }

    ~Directx12GraphicsCommands()
    {
        if (command_queue.Get())
            command_queue.Reset();
        if (command_list.Get())
            command_list.Reset();
    }

    Directx12GraphicsCommands(_In_opt_ const ComPtr<ID3D12Device10> device)
    {
        D3D12_COMMAND_QUEUE_DESC description{};
        description.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        description.NodeMask = 0;
        description.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        description.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        if (FAILED(device->CreateCommandQueue(&description, IID_PPV_ARGS(&command_queue))))
            return;

        if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&queue_fence))))
            return;

        if (FAILED(device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE,
                                              IID_PPV_ARGS(&command_list))))
            return;
    }

    void Reset(_In_opt_ const ComPtr<ID3D12CommandAllocator> frame_allocator)
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

struct Directx12Frame : public stm::not_copyable
{
  public:
    ComPtr<ID3D12CommandAllocator> frame_allocator = nullptr;
    stm::unique_ptr<Directx12GraphicsCommands> commands = nullptr;

    void Reset()
    {
        if (FAILED(frame_allocator->Reset()))
        {
            std::puts("DirectX12: <ERROR> -> failed to reset frame_allocator");
        }
        commands->Reset(frame_allocator);
    }

    ~Directx12Frame()
    {
        if (frame_allocator.Get())
            frame_allocator.Reset();
        if (commands)
            commands.reset();
    }

    explicit Directx12Frame(_In_opt_ const ComPtr<ID3D12Device10> device)
    {
        if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&frame_allocator))))
            return;
        commands = stm::make_unique<Directx12GraphicsCommands>(device);
    }
};

struct Directx12UniformManager : stm::controlled_copy<Directx12UniformManager>
{
  public:
    controlled_copy_f(Directx12UniformManager);
    explicit Directx12UniformManager(_In_opt_ const ComPtr<ID3D12Device10> device)
    {
    }
};
} // namespace dx12helpers
} // namespace render
} // namespace openttd