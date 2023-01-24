#include "VideoDriver.hpp"

namespace openttd::drivers
{

IVideoDriver::IVideoDriver(uint8_t priority_in): priority(priority_in)
{
	DriverRegistery::get()->registerDriver(this);
}

VideoDriver::VideoDriver()
{
	//TODO: add code to respect driver priority
	if (DriverRegistery::get()->getRef().empty())
	{
		puts("No Drivers regisitered");
	}

	for (auto* driver_ref : DriverRegistery::get()->getRef())
	{
		if (driver_ref->getType() == DriverType::VIDEO)
		{
			try
			{
				const char * return_str  = driver_ref->start();
				if (return_str != nullptr)
				{
					puts(return_str);
					continue;
				}
				else
				{
					driver = (IVideoDriver*)driver_ref;
					puts(driver->getName());
				}
			}
			catch (std::exception e)
			{
				puts(e.what());
				puts("trying next driver");
				continue;
			}
		}
	}

	driver_thread = new std::thread(&IDriver::run,driver);
}

VideoDriver::~VideoDriver()
{
	if (!driver)
	{
		driver->stop();
		driver = nullptr;
	}
}
}