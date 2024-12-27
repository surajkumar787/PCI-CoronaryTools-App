# iControlInterface library

add_library(iControlInterface STATIC src/CopyToRefClient.cpp)

target_include_directories(iControlInterface PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_include_directories(iControlInterface PUBLIC ../CommonPlatform.Logging/include/)
target_include_directories(iControlInterface PUBLIC ../CommonPlatform.Utilities/include/)
target_include_directories(iControlInterface PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Sscf.1.4.2/include/)
target_include_directories(iControlInterface PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.iControl.33.0.0/)

