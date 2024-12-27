cmake_minimum_required(VERSION 3.19)

project(pciapp)

set(CMAKE_BUILD_TYPE Debug) 

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/PciApp)

enable_testing()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Find Nuget (install the latest CLI here: https://www.nuget.org/downloads).
find_program(NUGET nuget)
if(NOT NUGET)
    message(FATAL "CMake could not find the nuget command line tool. Please install it!")
else()
    # Copy the Nuget config file from source location to the CMake build directory.
    #configure_file(packages.config.in packages.config COPYONLY)
    # Run Nuget using the .config file to install any missing dependencies to the build directory.
    execute_process(COMMAND 
        ${NUGET} restore ${CMAKE_CURRENT_SOURCE_DIR}/packages.config -SolutionDirectory ${CMAKE_BINARY_DIR}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

# Algorithms library
add_subdirectory(Algorithms)

# Platform library
add_subdirectory(../Common/ Common)

# Core library
add_subdirectory(Core)
#add_subdirectory(Core/FlexLM)

# TestAutomation library
add_subdirectory(../TestAutomation/TAF/PciExpressTestInterface/ TestAutomation)

# CrashRecovery executable
add_subdirectory(CrashRecovery)

# PciApp executable
add_subdirectory(PciApp)




