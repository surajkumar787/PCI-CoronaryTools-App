# CommonPlatform.Coregistration library

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

add_compile_definitions(dllexport_sense=) 
add_compile_definitions(dllexport_experienceidentity=)

add_library(CommonPlatform.Coregistration STATIC 
						src/CoRegistrationError.cpp
						src/CoRegistrationIfrPullbackCycle.cpp
						src/CoRegistrationIfrSpotCycle.cpp
						src/CoRegistrationOptions.cpp
						src/CoRegistrationPdPaInfo.cpp
						src/CoRegistrationPressureAndEcg.cpp
						src/CoRegistrationServerState.cpp
						src/CoRegistrationSource.cpp
						src/CoRegistrationSpotMeasurement.cpp
						src/CoRegistrationSystemInfo.cpp
						src/TcpClient.cpp
						src/TimeStampValidator.cpp)

target_include_directories(CommonPlatform.Coregistration PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_include_directories(CommonPlatform.Coregistration PUBLIC ../CommonPlatform.Logging/include/)
target_include_directories(CommonPlatform.Coregistration PUBLIC ../CommonPlatform.Utilities/include/)
target_include_directories(CommonPlatform.Coregistration PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Sense.1.13.1.104054/include/)
target_include_directories(CommonPlatform.Coregistration PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Sense.ExperienceIdentity.1.13.1.104054/include/)

