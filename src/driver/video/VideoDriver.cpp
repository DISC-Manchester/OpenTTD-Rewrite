#include "VideoDriver.hpp"

namespace openttd
{
namespace drivers
{

IVideoDriver::IVideoDriver(uint8_t priority_in)
    : priority(priority_in)
{
    DriverRegistry::get()->registerDriver(this);
}

VideoDriver::VideoDriver()
{
    // TODO: add code to respect driver priority
    if (DriverRegistry::get()->getRef().empty())
    {
        puts("No Drivers registered");
    }

    IVideoDriver * last_highest = nullptr;;
    for (auto *driver_ref : DriverRegistry::get()->getRef())
    {
        if (driver_ref->getType() == DriverType::VIDEO)
        {
            if (last_highest == nullptr || (((IVideoDriver *)driver_ref)->getPriority() < last_highest->getPriority()))
            {
                last_highest = (IVideoDriver *)driver_ref;
            }
        }
    }

    try
    {
        const char *return_str = last_highest->start();
        if (return_str)
        {
            puts(return_str);
			return;
        }
        else
        {
			last_highest->stop();
            last_highest->driverTested();
            driver_thread = new std::thread(&IDriver::run, last_highest);
            puts(last_highest->getName());
        }
    }
    catch (std::exception e)
    {
        puts(e.what());
        puts("trying next driver");
    }
}

VideoDriver::~VideoDriver()
{
    driver_thread->join();
}
} // namespace drivers
} // namespace openttd