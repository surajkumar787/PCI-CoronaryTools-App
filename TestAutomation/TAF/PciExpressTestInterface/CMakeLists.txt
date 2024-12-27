# PciExpressTestInterface library

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

add_compile_definitions(PCI_EXPRESS_TAF_USE_SSCF) 

add_library(PciExpressTestInterface STATIC 
						IPciExpressTestfInterface.cpp
						PciExpressTestClient.cpp
						PciExpressTestServer.cpp)

target_include_directories(PciExpressTestInterface PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_include_directories(PciExpressTestInterface PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Sscf.1.4.2/include/)


set_target_properties(PciExpressTestInterface PROPERTIES FOLDER "TAF")
