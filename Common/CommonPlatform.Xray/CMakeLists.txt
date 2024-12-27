# CommonPlatform.Xray library

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

add_compile_definitions(dllexport_sense=) 
add_compile_definitions(dllexport_experienceidentity=)

add_library(CommonPlatform.Xray STATIC 
						src/DvlpReceiverMonitor.cpp
						src/XrayEpx.cpp
						src/XrayGeometry.cpp
						src/XrayGeometryController.cpp
						src/XrayImage.cpp
						src/XraySinkFxd.cpp
						src/XraySourceFxd.cpp
						src/XraySourceRto.cpp
						src/XrayStudy.cpp)

target_include_directories(CommonPlatform.Xray PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_include_directories(CommonPlatform.Xray PUBLIC ../CommonPlatform.Logging/include/)
target_include_directories(CommonPlatform.Xray PUBLIC ../CommonPlatform.Utilities/include/)
target_include_directories(CommonPlatform.Xray PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.IpSimplicity.1.2.2/IpSimplicity/include/)
target_include_directories(CommonPlatform.Xray PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Sense.ExperienceIdentity.1.13.1.104054/include/)
target_include_directories(CommonPlatform.Xray PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Sense.1.13.1.104054/include/)
target_include_directories(CommonPlatform.Xray PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Cwis.1.1.1.10148/include/)
target_include_directories(CommonPlatform.Xray PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Dvlp.1.0.0.100109-master/include/)
target_include_directories(CommonPlatform.Xray PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.IpSimplicity.1.2.2/Medisys/include/)







