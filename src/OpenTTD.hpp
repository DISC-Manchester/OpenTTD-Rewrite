#pragma once
#include "driver/video/VideoDriver.hpp"
namespace openttd
{
class OpenTTD
{
    drivers::VideoDriver *video;

  public:
    OpenTTD()
    {
        video = new drivers::VideoDriver();
    }

    void run()
    {
        bool running = true;
        while (running)
        {
            if (drivers::DBus::get()->retrieve(drivers::DBusEventData::DBusEvent::CLOSED_WINDOW).event !=
                drivers::DBusEventData::DBusEvent::NO_EVENT)
                running = false;
        }
    }
};
} // namespace openttd