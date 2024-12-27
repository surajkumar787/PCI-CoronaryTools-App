# CommonPlatform.Logging library

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

add_library(CommonPlatform.Logging STATIC 
					src/AsyncLogWriter.cpp
					src/ILogWriter.cpp
					src/Log.cpp
					src/NullLogWriter.cpp
					src/SynchronousLogWriter.cpp
					src/UtilizationLogEvent.cpp)

target_include_directories(CommonPlatform.Logging PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/) 

