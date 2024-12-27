# ErrLib library
add_library(ErrLib STATIC src/ErrLibFuncs.c)

target_include_directories(ErrLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/Private/)
target_include_directories(ErrLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/Public/)
target_include_directories(ErrLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/Published/)

