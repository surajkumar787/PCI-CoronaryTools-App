# iAcquisitionInterface library

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

add_library(iAcquisitionInterface STATIC src/iacquisitionnotification.cpp)

target_include_directories(iAcquisitionInterface PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_include_directories(iAcquisitionInterface PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Sscf.1.4.2/include/)

