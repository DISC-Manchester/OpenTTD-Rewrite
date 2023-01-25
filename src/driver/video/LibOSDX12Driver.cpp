#include "LibOSDriver.hpp"
#include <libos/NativeWindow.h>

namespace openttd::drivers
{

class LibOSDX12WindowDriver : public LibOSBaseWindowDriver
{
  public:
    LibOSDX12WindowDriver()
        : LibOSBaseWindowDriver(1)
    {
    }

    const char *start() override
    {
        const char *result = nullptr;
        if ((result = LibOSBaseWindowDriver::start()) != nullptr)
            return result;

        return nullptr;
    }

    void mainLoop() final override
    {
        while (losUpdateWindow(window) != LOS_WINDOW_CLOSE)
            ;

        DBusEventData data;
        data.event = DBusEventData::DBusEvent::CLOSED_WINDOW;
        data.data = nullptr;
        DBus::get()->submit(data);
    }

    void stop() override
    {
        LibOSBaseWindowDriver::stop();
    }
    const char *getName() override
    {
        return "LIBOS -> (DX12)Video Driver";
    }
};
static LibOSDX12WindowDriver libos_dx12_driver;
} // namespace openttd::drivers