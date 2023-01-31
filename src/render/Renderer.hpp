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
namespace openttd
{
namespace render
{
struct IBuffer : public stm::controlled_copy<IBuffer>
{
};


class IRender : public stm::controlled_copy<IRender>
{
  protected:
    virtual void transfer() = 0;
    imutableInt(max_frames,3);
  public:
    virtual void begin() = 0;
    virtual const IBuffer *createBuffer() = 0;
    virtual void bindBuffer(const IBuffer *buffer) = 0;
    virtual void destroyBuffer(const IBuffer *buffer) = 0;
    virtual void draw() = 0;
    virtual void end() = 0;
    virtual void present() = 0;
    virtual ~IRender() = default;
};

struct Renderer
{
  private:
    std::atomic<openttd::render::IRender *> render_;
  public:
    static Renderer *get(bool free_it = false)
    {
        static openttd::render::Renderer *renderer;
        if (free_it)
        {
            delete renderer;
            renderer = new openttd::render::Renderer();
            return renderer;
        }

        if (!renderer)
            renderer = new openttd::render::Renderer();
        return renderer;
    }
    void use(openttd::render::IRender *render_in)
    {
        if (render_)
            delete render_;
        render_ = render_in;
    }

    void begin()
    {
        if (render_.load())
            render_.load()->begin();
    }

    void present()
    {
        if (render_.load())
            render_.load()->present();
    }

    void end()
    {
        if (render_.load())
            render_.load()->end();
    }

    const IBuffer *createBuffer()
    {
        if (render_.load())
            return render_.load()->createBuffer();
        return nullptr;
    }

    void bindBuffer(const IBuffer *buffer)
    {
        if (render_.load())
            return render_.load()->bindBuffer(buffer);
    }

    void destroyBuffer(const IBuffer *buffer)
    {
        if (render_.load())
            render_.load()->destroyBuffer(buffer);
    }

    void draw()
    {
        if (render_.load())
            render_.load()->draw();
    }
};
} // namespace render
} // namespace openttd