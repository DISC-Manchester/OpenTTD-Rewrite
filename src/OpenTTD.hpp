#pragma once
#include "driver/Video/VideoDriver.hpp"
namespace openttd
{
	class OpenTTD
	{
		drivers::VideoDriver* video;

	public:
		OpenTTD()
		{
			video = new drivers::VideoDriver();
		}

		void run()
		{
			while (true);
		}
	};
}