# CrashRecovery executable 

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

add_compile_definitions(dllexport_sense=)

link_directories(${CMAKE_BINARY_DIR}/packages/Philips.IpSimplicity.1.2.2/IpSimplicity/lib/v140/${CMAKE_VS_PLATFORM_NAME}/)
link_directories(${CMAKE_BINARY_DIR}/packages/Philips.Sense.1.13.1.104054/lib/native/${CMAKE_BUILD_TYPE}_${CMAKE_VS_PLATFORM_NAME}_v140_Dynamic/Sense/)
link_directories(${CMAKE_BINARY_DIR}/packages/Philips.Sscf.1.4.2/lib/native/)
link_directories(${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/)

add_executable(CrashRecovery WIN32 main.cpp)

target_link_libraries(CrashRecovery Shlwapi 
									Sscf1.142dlld.x64 
									CommonPlatform.AppLauncherNative
									CommonPlatform.Logging 
									CommonPlatform.Utilities 
									Localization)

target_include_directories(CrashRecovery PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Sense.1.13.1.104054/include/)
target_include_directories(CrashRecovery PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Sense.ExperienceIdentity.1.13.1.104054/include/)
target_include_directories(CrashRecovery PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Sscf.1.4.2/include/)
target_include_directories(CrashRecovery PUBLIC ../../Common/CommonPlatform.Logging/include/)
target_include_directories(CrashRecovery PUBLIC ../../Common/CommonPlatform.Utilities/include/)
target_include_directories(CrashRecovery PUBLIC ../Core/Localization/include/)
target_include_directories(CrashRecovery PUBLIC ../../Common/CommonPlatform.AppLauncherNative/include/)


set_target_properties(CrashRecovery PROPERTIES FOLDER "Application")

