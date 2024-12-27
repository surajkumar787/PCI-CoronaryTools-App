# PrmLib library
add_library(PrmLib STATIC src/PrmLibFuncs.c)

target_include_directories(PrmLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/Private/)
target_include_directories(PrmLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/Public/)
target_include_directories(PrmLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/Published/)
target_include_directories(PrmLib PUBLIC ../ErrLib/include/Public/)

