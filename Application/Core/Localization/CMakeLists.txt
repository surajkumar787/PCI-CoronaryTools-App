# Localization library

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

add_compile_definitions(dllexport_sense=) 

add_library(Localization STATIC Translation.cpp)

target_include_directories(Localization PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_include_directories(Localization PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Sense.ExperienceIdentity.1.13.1.104054/include/)
target_include_directories(Localization PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Sense.1.13.1.104054/include/)

