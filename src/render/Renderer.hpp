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
class IRender
{
  public:
    virtual void begin() = 0;
    virtual void end() = 0;
    virtual void present() = 0;
    virtual ~IRender() = default;
};

class Renderer
{
    std::atomic<openttd::render::IRender *> render_ = nullptr;
  public:
    Renderer()
    {
    }

    ~Renderer()
    {
        delete render_;
        render_ = nullptr;
    }

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
};
} // namespace render
} // namespace openttd
