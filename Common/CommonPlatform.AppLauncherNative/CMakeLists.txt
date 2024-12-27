# CommonPlatform.AppLauncherNative library

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

add_library(CommonPlatform.AppLauncherNative STATIC 
					src/AppLauncher.cpp
					src/AppLauncherClient.cpp)

target_include_directories(CommonPlatform.AppLauncherNative PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_include_directories(CommonPlatform.AppLauncherNative PUBLIC ../CommonPlatform.Logging/include/) 
target_include_directories(CommonPlatform.AppLauncherNative PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Sscf.1.4.2/include/)

