# CommonPlatform.Utilities library

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

add_compile_definitions(dllexport_sense=)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
# Make sure the compiler supports the specified standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(CommonPlatform.Utilities STATIC 
						src/Bitmap.cpp
						src/Environment.cpp
						src/InvisibleWindow.cpp
						src/Network.cpp
						src/PathUtil.cpp
						src/ProductInfoUtil.cpp
						src/Settings.cpp
						src/StlString.cpp
						src/Thread.cpp
						src/Timing.cpp)

target_include_directories(CommonPlatform.Utilities PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_include_directories(CommonPlatform.Utilities PUBLIC ../CommonPlatform.Logging/include/)
target_include_directories(CommonPlatform.Utilities PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Net.1.3.5.101019/include/net)
target_include_directories(CommonPlatform.Utilities PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Net.1.3.5.101019/include/)
target_include_directories(CommonPlatform.Utilities PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Sense.1.13.1.104054/include/)

