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
namespace openttd
{
namespace ecs
{
typedef uint32_t Entity;
#define max_entities 5000
#define max_components 50

struct ComponentList
{
    std::array<Component *, max_components> component;
    uint32_t component_count;
    ComponentList()
    {
        component_count = 0;
        for (uint32_t i = 0; i < max_components; i++)
        {
            component[i] = nullptr;
        }
    }
};

class ECSManager
{
    std::queue<Entity> entity_ids;
    std::array<ComponentList, max_entities> entity_components{};

  public:
    ECSManager()
    {
        for (Entity i = 1; i < max_entities; i++)
            entity_ids.emplace(i);
    }

    std::array<ComponentList, max_entities> &getListOfComponents()
    {
        return entity_components;
    }

    ~ECSManager()
    {
        while (!entity_ids.empty())
            entity_ids.pop();
    }

    const Entity getNewEntity()
    {
        assert(!entity_ids.empty());
        auto it = entity_ids.front();
        entity_ids.pop();
        return it;
    }

    void addComponent(Entity entity, Component *component)
    {
        entity_components[entity].component[entity_components[entity].component_count] = component;
        entity_components[entity].component_count++;
    }

    void returnEntity(const Entity entity)
    {
        entity_components[entity].component_count = 0;
        for (auto *ptr : entity_components[entity].component)
        {
            if (ptr)
                delete ptr;
            ptr = nullptr;
        }
        assert(entity_ids.size() < max_entities);
        entity_ids.push(entity);
    }
};

template<typename T>
    requires std::is_base_of<openttd::ecs::Component, T>::value
class ISystem
{
  public:
    virtual void run(ECSManager *ecs_in)
    {
        for (auto &component : ecs_in->getListOfComponents())
            for (uint32_t i = 0; i < component.component_count; i++)
            {
                if (typeid(*component.component[i]) == typeid(T))
                    component.component[i]->callback();
            }
    }
};
} // namespace ecs
} // namespace openttd