#pragma once
#include "driver/video/VideoDriver.hpp"
namespace openttd
{
int gameMain()
{
    drivers::VideoDriver video{};
    (void)drivers::DriverRegistry::get(true);
    bool running = true;
    while (running)
    {
        if (eventToHandle(drivers::DBusEventData::DBusEvent::CLOSED_WINDOW))
        {
            running = false;
        }
        if (eventToHandle(drivers::DBusEventData::DBusEvent::STATE_CHANGE))
        {
            submitEvent(newEvent(drivers::DBusEventData::DBusEvent::DRAW_NEW_FRAME));
        }
    }
    return 0;
}
} // namespace openttd