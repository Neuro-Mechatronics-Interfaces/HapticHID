cmake_minimum_required(VERSION 3.10)
project(HapticHID)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

# Set the output directory for executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/examples)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/examples)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/examples)

# Allow the user to specify the path to the Force Dimension SDK and ViGEmClient root folders 
set(FORCE_DIMENSION_SDK_DIR "path/to/fForce/Dimension/SDK")
set(VIGEMCLIENT_SDK_DIR "path/to/ViGEmClient")

# Convert paths to absolute paths
get_filename_component(FORCE_DIMENSION_SDK_DIR_ABS "${FORCE_DIMENSION_SDK_DIR}" ABSOLUTE)
get_filename_component(VIGEMCLIENT_SDK_DIR_ABS "${VIGEMCLIENT_SDK_DIR}" ABSOLUTE)

# Derive the include, lib, and bin paths for Force Dimension SDK and ViGEmClient
# Helps to construct absolute paths dynamically for compatibility between Windows/Linux PCs
set(FORCE_DIMENSION_INCLUDE_DIR "${FORCE_DIMENSION_SDK_DIR_ABS}/include")
set(FORCE_DIMENSION_LIB_DIR "${FORCE_DIMENSION_SDK_DIR_ABS}/lib")
set(FORCE_DIMENSION_BIN_DIR "${FORCE_DIMENSION_SDK_DIR_ABS}/bin")
set(VIGEMCLIENT_INCLUDE_DIR "${VIGEMCLIENT_SDK_DIR_ABS}/include")
set(VIGEMCLIENT_LIB_DIR "${VIGEMCLIENT_SDK_DIR_ABS}/bin/debug/x64")

# Specify the include directories
include_directories(
    ${VIGEMCLIENT_INCLUDE_DIR}
    ${FORCE_DIMENSION_INCLUDE_DIR}
)

# Specify the link directories
link_directories(
    ${VIGEMCLIENT_LIB_DIR}
    ${FORCE_DIMENSION_LIB_DIR}
    ${FORCE_DIMENSION_BIN_DIR}
)

# Create test project executable and link ViGEmClient library
add_executable(DummyHapticHID src/DummyHapticHID.cpp)
target_link_libraries(DummyHapticHID 
  setupapi.lib  
  "${VIGEMCLIENT_LIB_DIR}/ViGEmClient.lib"
)

# Add source file for the actual project
add_executable(HapticHID src/HapticHID.cpp)
target_link_libraries(HapticHID 
  setupapi.lib 
  "${VIGEMCLIENT_LIB_DIR}/ViGEmClient.lib" 
  "${FORCE_DIMENSION_LIB_DIR}/drdms64.lib"
)

# Add source file for the actual project
add_executable(RotationHapticHID src/RotationHapticHID.cpp)
target_link_libraries(RotationHapticHID 
  setupapi.lib 
  "${VIGEMCLIENT_LIB_DIR}/ViGEmClient.lib" 
  "${FORCE_DIMENSION_LIB_DIR}/drdms64.lib"
)


