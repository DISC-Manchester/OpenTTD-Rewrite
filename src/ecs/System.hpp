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
#include "Component.hpp"
#include "Entity.hpp"
namespace openttd
{
namespace ecs
{
template<typename T>
    requires std::is_base_of<openttd::ecs::Component, T>::value
class ISystem
{
  public:
    virtual void run(EntityManager *ecs_in)
    {
        for (auto &component : ecs_in->getListOfComponents())
            if (component.component_count != 0)
                for (uint32_t i = 0; i < component.component_count; i++)
                {
                    if (typeid(*component.component[i]) == typeid(T))
                        component.component[i]->callback(component.component[i]);
                }
    }
};
} // namespace ecs
} // namespace openttd