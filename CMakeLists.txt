cmake_minimum_required(VERSION 3.10)
project(HapticHID)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

# Specify the path to the ViGEmClient include directory
include_directories(
	"C:/MyRepos/CPP/ViGEmClient/include"
	"C:/Program Files/Force Dimension/sdk-3.17.1/include"
	)

# Specify the path to the ViGEmClient library
link_directories(
	"C:/MyRepos/CPP/ViGEmClient/lib"
	"C:/Program Files/Force Dimension/sdk-3.17.1/lib"
	)

# Add the source file for the test project
add_executable(DummyHapticHID src/DummyHapticHID.cpp)

# Link test project against the ViGEmClient library
target_link_libraries(DummyHapticHID setupapi.lib ViGEmClient.lib)

# Add source file for the actual project
add_executable(HapticHID src/HapticHID.cpp)

# Link test project against the ViGEmClient and drdms64 libraries
target_link_libraries(HapticHID setupapi.lib ViGEmClient.lib drdms64.lib)