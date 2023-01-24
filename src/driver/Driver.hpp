#pragma once
/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */
#include "../istd/Stm.hpp"
namespace openttd::drivers
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
	public:
		virtual DriverType getType() const noexcept = 0;
		virtual const char* start() = 0;
		virtual void mainLoop() = 0;
		virtual void stop() = 0;
		virtual const char* getName() = 0;

		static void run(IDriver* driver)
		{
			driver->mainLoop();
		}
	};

	class DriverRegistery
	{
		std::vector<drivers::IDriver*> driver_registery;
	public:
		DriverRegistery() = default;
		~DriverRegistery() = default;

		static DriverRegistery* get()
		{
			static DriverRegistery* reg;
			if(!reg)
				reg = new DriverRegistery();
			return reg;
		}

		void registerDriver(drivers::IDriver* driver)
		{
			driver_registery.push_back(driver);
		}

		const std::vector<drivers::IDriver*>& getRef()
		{
			return driver_registery;
		}
	};
}