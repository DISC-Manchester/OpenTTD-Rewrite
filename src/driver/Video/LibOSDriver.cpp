#include <libos/Window.h>
#include "VideoDriver.hpp"

namespace openttd::drivers
{
	class LibOSWindowDriver : public IVideoDriver
	{
		losWindow window;
	protected:
		void mainLoop() final override
		{
			while (losUpdateWindow(window) != LOS_WINDOW_CLOSE)
			{
			}
		}
	public:
		LibOSWindowDriver() : IVideoDriver(0)
		{
		}

		const char* start() final override
		{
			losResult result{};
			losWindowInfo info;
			info.title = "test";
			info.title_size = 4;
			info.window_size = { 500,500 };
			if((result = losCreateWindow(&window,info)) != LOS_SUCCESS)
				return nullptr;
		}
		void stop() final override
		{
			losDestroyWindow(window);
		}

		const char* getName() final override
		{
			return "LIBOS -> Video Driver";
		}
	};


	static LibOSWindowDriver libos_driver;
}