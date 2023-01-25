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
enum class DriverType
{
    VIDEO,
    SOUND,
    NETWORK,
    FILE
};

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

    static DriverRegistry *get()
    {
        static DriverRegistry *reg;
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
        ON_MOUSE_BUTTON_DOWN,
    } event;
    void *data;
    DBusEventData()
    {
        event = DBusEvent::NO_EVENT;
        data = nullptr;
    }
};

class DBus
{
    std::queue<DBusEventData> bus;
    std::mutex bus_lock;

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
        std::lock_guard<std::mutex> lock(bus_lock);
        return bus.empty();
    }

    void submit(DBusEventData event)
    {
        std::lock_guard<std::mutex> lock(bus_lock);
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
} // namespace drivers
} // namespace openttd