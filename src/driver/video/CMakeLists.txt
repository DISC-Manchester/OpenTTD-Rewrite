add_files(
	VideoDriver.hpp
	VideoDriver.cpp
)

add_files(
	LibOSDriver.hpp
)

add_files(
	LibOSVulkanDriver.cpp
	CONDITION Vulkan_FOUND
)

add_files(
	LibOSDX12Driver.cpp
	CONDITION ${CMAKE_SYSTEM_NAME} MATCHES "Windows"
)