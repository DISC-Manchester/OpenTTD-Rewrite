#pragma once
/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2. OpenTTD is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public
 * License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */
#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <type_traits>
#include <typeinfo>
#include <utility>
namespace stm
{
#define stm_new new
#define stm_free delete
typedef uint16_t unicode;
typedef uint64_t stmuint;

template<typename T> struct controlled_copy
{
  protected:
    controlled_copy &operator=(const controlled_copy &) = default;
    controlled_copy(const controlled_copy &) = default;

  public:
    controlled_copy() = default;

    virtual T copy() = 0;
};

template<typename T, stmuint max_size> struct queue
{
  private:
    static_assert(std::is_base_of<controlled_copy<T>, T>::value || !std::is_copy_constructible<T>::value,
                  "STM ERROR<queue>: must be a controlled copy type or not copy constructible  as queue takes "
                  "ownership of the item");
    friend T;
    struct queue_item
    {
        T item;
        bool auto_remove = true;
        ~queue_item()
        {
            auto_remove = true;
        }
    };

    queue_item *slots;
    std::mutex queue_lock;
    std::atomic<stmuint> front_index = 0;
    std::atomic<stmuint> items = 0;

    stmuint i_incIndex()
    {
        if (front_index < max_size)
            front_index = 0;
        else
            front_index++;
        return front_index;
    }

    constexpr void i_flush()
    {
        front_index = 0;
        std::vector<queue_item> to_add_back;
        for (stmuint i = 0; i < max_size; i++)
        {
            if (!slots[i].auto_remove)
                to_add_back.push_back(slots[i]);
        }

        for (stmuint i = 0; i < max_size; i++)
        {
            slots[i].~queue_item();
            slots[i] = {};
        }

        for (queue_item item : to_add_back)
            slots[i_incIndex()] = item;

        items = to_add_back.size();
    }

  public:
    constexpr queue()
    {
        slots = stm_new queue_item[max_size];
    }

    constexpr bool empty()
    {
        return items == 0;
    }

    constexpr bool full()
    {
        return items == max_size;
    }

    constexpr void push(T &&value, bool important = false)
    {
        std::lock_guard lock(queue_lock);
        if (full())
            i_flush();
        items++;
        new (slots + front_index) queue_item(std::move(value), !important);
        i_incIndex();
    }

    constexpr void forceFlush()
    {
        std::lock_guard lock(queue_lock);
        i_flush();
    }

    constexpr T pop()
    {
        std::lock_guard lock(queue_lock);
        if (!empty())
            items--;
        queue_item value = std::move(slots[front_index]);
        slots[front_index].~queue_item();
        i_incIndex();
        return std::move(value.item);
    }

    constexpr const T *front()
    {
        std::lock_guard lock(queue_lock);
        i_incIndex();
        return &slots[front_index].item;
    }

    constexpr ~queue()
    {
        stm_free[] slots;
    }
};
} // namespace stm
