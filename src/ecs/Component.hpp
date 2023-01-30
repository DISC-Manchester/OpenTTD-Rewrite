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
namespace ecs
{

struct Component
{
  private:
    static const uint32_t incTypeID()
    {
        static uint32_t component_type_id;
        return component_type_id++;
    }
  public:
    const uint32_t component_type;
    void (*callback)(openttd::ecs::Component*);
    Component(void (*callback_in)(openttd::ecs::Component*))
        : component_type(incTypeID())
        , callback(callback_in)
    {
    }
    virtual ~Component() = default;
};
} // namespace ecs
} // namespace openttd