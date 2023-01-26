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
namespace drivers
{
typedef enum DriverType : uint8_t
{
    VIDEO,
    SOUND
} DriverType;

class IDriver
{
  protected:
    bool tested = false;

  public:
    virtual DriverType getType() const noexcept = 0;
    virtual const char *start() = 0;
    virtual void mainLoop() = 0;
    virtual void stop() = 0;
    virtual const char *getName() = 0;
    void driverTested()
    {
        tested = true;
    }

    static void run(IDriver *driver)
    {
        try
        {
            driver->start();
            driver->mainLoop();
            driver->stop();
        }
        catch (std::exception e)
        {
            printf("OpenTTD Driver (%s) CRASH: %s\n", driver->getName(), e.what());
        }
    }
};

class DriverRegistry
{
    std::vector<drivers::IDriver *> driver_registry;

  public:
    DriverRegistry() = default;
    ~DriverRegistry() = default;

    static DriverRegistry *get(bool done = false)
    {
        static DriverRegistry *reg;
        if (done)
        {
            delete reg;
            return nullptr;
        }
        if (!reg)
            reg = new DriverRegistry();
        return reg;
    }

    void registerDriver(drivers::IDriver *driver)
    {
        driver_registry.push_back(driver);
    }

    const std::vector<drivers::IDriver *> &getRef()
    {
        return driver_registry;
    }
};

struct DBusEventData
{
    enum class DBusEvent
    {
        NO_EVENT,
        CLOSED_WINDOW,
        ON_KEY_DOWN,
        ON_MOUSE_DOWN,
        FINISHED_DRAW,
        PRESENT_FRAME,
    } event;
    std::shared_ptr<void *> data;
    DBusEventData()
    {
        event = DBusEvent::NO_EVENT;
        data = nullptr;
    }

    DBusEventData(DBusEvent event_in, void *data_in)
        : event(event_in)
        , data(std::make_shared<void *>(data_in))
    {
    }
};

class DBus
{
    std::queue<DBusEventData> bus;
    std::mutex bus_lock;
    std::atomic_bool is_lock = false;

  public:
    DBus() = default;
    ~DBus() = default;

    static DBus *get()
    {
        static DBus *dbus;
        if (!dbus)
            dbus = new DBus();
        return dbus;
    }

    void lockSubmit()
    {
        std::lock_guard<std::mutex> lock(bus_lock);
        is_lock = true;
    }

    bool clear()
    {
        std::lock_guard<std::mutex> lock(bus_lock);
        return bus.empty();
    }

    void dump()
    {
        std::lock_guard<std::mutex> lock(bus_lock);
        while (!bus.empty())
            bus.pop();
    }

    void submit(DBusEventData event)
    {
        using namespace std::literals;
        std::lock_guard<std::mutex> lock(bus_lock);
        if (!is_lock)
            bus.push(event);
    }

    const DBusEventData retrieve(DBusEventData::DBusEvent filter = DBusEventData::DBusEvent::NO_EVENT)
    {
        if (!bus.empty())
        {
            std::lock_guard<std::mutex> lock(bus_lock);
            auto top = bus.front();
            if (top.event == filter || filter == DBusEventData::DBusEvent::NO_EVENT)
            {
                bus.pop();
                return top;
            }
            return {};
        }
        return {};
    }
};
#define dumpBus() openttd::drivers::DBus::get()->dump();
#define lockBus() openttd::drivers::DBus::get()->lockSubmit();
#define newEventWithData(event, data_ptr) openttd::drivers::DBusEventData(event, data_ptr)
#define newEvent(event) newEventWithData(event, nullptr)
#define getEventToHandle(filter) openttd::drivers::DBus::get()->retrieve(filter)
#define eventToHandle(filter) getEventToHandle(filter).event != openttd::drivers::DBusEventData::DBusEvent::NO_EVENT
#define reSubmitEvent(event) openttd::drivers::DBus::get()->submit(event)
#define submitEvent(event) openttd::drivers::DBus::get()->submit(event)
} // namespace drivers
} // namespace openttd