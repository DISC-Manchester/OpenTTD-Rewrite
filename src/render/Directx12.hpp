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
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
namespace openttd
{
namespace render
{
using namespace Microsoft::WRL;
template<bool cpu_side> class Directx12CommandList : public ICommandList
{
  public:
    Directx12CommandList()
    {
    }

    // TEMP: add a commands here

    ~Directx12CommandList()
    {
    }
};

class Directx12Buffer : public IBuffer
{
};
class Directx12Renderer : public IRender
{

    ComPtr<IDXGIFactory7> &instance;
    ComPtr<ID3D12Device10> &device;

  protected:
    virtual void transfer() final override
    {
    }

  public:
    Directx12Renderer(ComPtr<IDXGIFactory7> &instance_in, ComPtr<ID3D12Device10> &device_in)
        : instance(instance_in)
        , device(device_in)
    {
    }

    virtual ~Directx12Renderer() final override
    {
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
    }

    void present() final override
    {
    }
};
} // namespace render
} // namespace openttd