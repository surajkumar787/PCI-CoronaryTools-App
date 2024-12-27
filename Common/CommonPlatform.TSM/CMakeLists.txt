# CommonPlatform.TSM library

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

add_compile_definitions(dllexport_sense=)
add_compile_definitions(dllexport_experienceidentity=)

add_library(CommonPlatform.TSM STATIC 
						src/TsmButtonDefinitions.cpp
						src/TsmPage.cpp
						src/TsmSource.cpp)

target_include_directories(CommonPlatform.TSM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_include_directories(CommonPlatform.TSM PUBLIC ../CommonPlatform.Logging/include/)
target_include_directories(CommonPlatform.TSM PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Sense.1.13.1.104054/include/)
target_include_directories(CommonPlatform.TSM PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Sense.ExperienceIdentity.1.13.1.104054/include/)
target_include_directories(CommonPlatform.TSM PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Cwis.1.1.1.10148/include/)

