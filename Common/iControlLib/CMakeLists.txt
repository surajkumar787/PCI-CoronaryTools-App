# iControlLib library

add_library(iControlLib STATIC 
			${CMAKE_BINARY_DIR}/packages/Philips.iControl.33.0.0/src-cpp/base.cpp
			${CMAKE_BINARY_DIR}/packages/Philips.iControl.33.0.0/src-cpp/datetime.cpp
			${CMAKE_BINARY_DIR}/packages/Philips.iControl.33.0.0/src-cpp/iacquisitionsettings.cpp
			${CMAKE_BINARY_DIR}/packages/Philips.iControl.33.0.0/src-cpp/iacquisitionsettingsV2.cpp
			${CMAKE_BINARY_DIR}/packages/Philips.iControl.33.0.0/src-cpp/igeometryapc.cpp
			${CMAKE_BINARY_DIR}/packages/Philips.iControl.33.0.0/src-cpp/igeometrybase.cpp
			${CMAKE_BINARY_DIR}/packages/Philips.iControl.33.0.0/src-cpp/igeometrystate.cpp
			${CMAKE_BINARY_DIR}/packages/Philips.iControl.33.0.0/src-cpp/igeometrystateV2.cpp
			${CMAKE_BINARY_DIR}/packages/Philips.iControl.33.0.0/src-cpp/igeometrytrajectory.cpp
			${CMAKE_BINARY_DIR}/packages/Philips.iControl.33.0.0/src-cpp/iorchestration.cpp
			${CMAKE_BINARY_DIR}/packages/Philips.iControl.33.0.0/src-cpp/ireview.cpp
			${CMAKE_BINARY_DIR}/packages/Philips.iControl.33.0.0/src-cpp/iuserguidance.cpp
			${CMAKE_BINARY_DIR}/packages/Philips.iControl.33.0.0/src-cpp/iuserguidancedatatypes.cpp
			${CMAKE_BINARY_DIR}/packages/Philips.iControl.33.0.0/src-cpp/scenegraph.cpp
			${CMAKE_BINARY_DIR}/packages/Philips.iControl.33.0.0/src-cpp/utils.cpp)

#target_include_directories(iControlLib PUBLIC ../CommonPlatform.Logging/include/)
target_include_directories(iControlLib PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.iControl.33.0.0/)
target_include_directories(iControlLib PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.iControl.33.0.0/src-cpp/)
target_include_directories(iControlLib PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Sscf.1.4.2/include/)

