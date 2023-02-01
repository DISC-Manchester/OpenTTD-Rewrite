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

class IRender : public stm::not_copyable
{
  protected:
    virtual void transfer() = 0;
    imutableInt(max_frames, 3);

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

class NullRender : public IRender
{
  protected:
    virtual void transfer() override{};

  public:
    virtual void begin() override{};
    virtual const IBuffer *createBuffer() override
    {
        return nullptr;
    };
    virtual void bindBuffer(const IBuffer *) override{};
    virtual void destroyBuffer(const IBuffer *) override{};
    virtual void draw() override{};
    virtual void end() override{};
    virtual void present() override{};
    virtual ~NullRender() override = default;
};

struct Renderer
{
  private:
    stm::atomic<openttd::render::IRender *> render_ = new NullRender();
    stm::mutex use_lock;
    stm::thread::id last_thread_to_lock;

    void trackLock()
    {
        use_lock.lock();
        last_thread_to_lock = stm::this_thread::get_id();
    }

    bool isUseable()
    {
        return render_.load();
    }

  public:

    static Renderer *get(bool free_it = false)
    {
        static openttd::render::Renderer *renderer;
        if (free_it)
        {
            renderer->tryUnlock();
            delete renderer;
            renderer = new openttd::render::Renderer();
            return renderer;
        }

        if (!renderer)
            renderer = new openttd::render::Renderer();
        return renderer;
    }

    void tryUnlock()
    {
        if (last_thread_to_lock == stm::this_thread::get_id())
            use_lock.unlock();
    }

    void set(openttd::render::IRender *render_in)
    {
        trackLock();
        if (render_)
            delete render_;
        render_ = render_in;
        tryUnlock();
    }

    void reset()
    {
        trackLock();
        if (render_)
            delete render_;
        render_ = new NullRender();
        tryUnlock();
    }

    void begin()
    {
        trackLock();
        if (isUseable())
            render_.load()->begin();
    }

    void present()
    {
        trackLock();
        if (isUseable())
            render_.load()->present();
        tryUnlock();
    }

    void end()
    {
        if (isUseable())
            render_.load()->end();
        tryUnlock();
    }

    const IBuffer *createBuffer()
    {
        if (isUseable())
            return render_.load()->createBuffer();
        return nullptr;
    }

    void bindBuffer(const IBuffer *buffer)
    {
        if (isUseable())
            return render_.load()->bindBuffer(buffer);
    }

    void destroyBuffer(const IBuffer *buffer)
    {
        if (isUseable())
            render_.load()->destroyBuffer(buffer);
    }

    void draw()
    {
        if (isUseable())
            render_.load()->draw();
    }
};
} // namespace render
} // namespace openttd