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

struct DBusEventData : public stm::controlled_copy<DBusEventData>
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
    void *data;
    DBusEventData()
    {
        event = DBusEvent::NO_EVENT;
        data = nullptr;
    }

    DBusEventData(DBusEvent event_in, void *data_in)
        : event(event_in)
        , data(data_in)
    {
    }

    virtual DBusEventData copy() final override
    {
        return DBusEventData(*this);
    }
};

class DBus
{
    stm::queue<DBusEventData, 255> bus;
    std::atomic_bool is_submition_locked = false;
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

    bool clear()
    {
        return bus.empty();
    }

    void submit(DBusEventData event)
    {
        if (is_submition_locked)
            return;
        bus.push(std::move(event));
    }

    void submitImportant(DBusEventData event)
    {
        if (is_submition_locked)
            return;
        if (event.event == DBusEventData::DBusEvent::CLOSED_WINDOW)
            is_submition_locked = true;
        bus.push(std::move(event),true);
    }

    void flushBus()
    {
        bus.forceFlush();
    }

    const DBusEventData retrieve(DBusEventData::DBusEvent filter = DBusEventData::DBusEvent::NO_EVENT)
    {
        if (!bus.empty())
        {
            const openttd::drivers::DBusEventData* top = bus.front();
            if (top->event == filter || filter == DBusEventData::DBusEvent::NO_EVENT)
                return bus.pop();
            return {};
        }
        return {};
    }
};
#define flushBus() openttd::drivers::DBus::get()->flushBus()
#define newEventWithData(event, data_ptr) openttd::drivers::DBusEventData(event, data_ptr)
#define newEvent(event) newEventWithData(event, nullptr)
#define getEventToHandle(filter) openttd::drivers::DBus::get()->retrieve(filter)
#define eventToHandle(filter) getEventToHandle(filter).event != openttd::drivers::DBusEventData::DBusEvent::NO_EVENT
#define reSubmitEvent(event) openttd::drivers::DBus::get()->submit(event)
#define submitEvent(event) openttd::drivers::DBus::get()->submit(event)
#define submitImportantEvent(event) openttd::drivers::DBus::get()->submitImportant(event)
} // namespace drivers
} // namespace openttd