# PciApp executable 

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

set(CMAKE_BUILD_TYPE Debug) 

add_compile_definitions(dllexport_sense=)
add_compile_definitions(dllexport_experienceidentity=)
add_compile_definitions(PHILIPS_TRACE_INCLUDED)
add_compile_definitions(SSCF1_INCLUDED)
add_compile_definitions(__STDC_LIMIT_MACROS)


# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
# Make sure the compiler supports the specified standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)

link_directories(${CMAKE_BINARY_DIR}/packages/Philips.IpSimplicity.1.2.2/IpSimplicity/lib/v140/${CMAKE_VS_PLATFORM_NAME}/)
link_directories(${CMAKE_BINARY_DIR}/packages/Philips.Sscf.1.4.2/lib/native/)
link_directories(${CMAKE_BINARY_DIR}/packages/Philips.TraceWriter.1.3.0/lib/native/)
link_directories(${CMAKE_BINARY_DIR}/packages/Philips.Net.1.3.5.101019/lib/native/)
link_directories(${CMAKE_BINARY_DIR}/packages/Philips.Sense.1.13.1.104054/lib/native/${CMAKE_BUILD_TYPE}_${CMAKE_VS_PLATFORM_NAME}_v140_Dynamic/Sense/)
link_directories(${CMAKE_BINARY_DIR}/packages/Philips.Sense.ExperienceIdentity.1.13.1.104054/lib/native/${CMAKE_BUILD_TYPE}_${CMAKE_VS_PLATFORM_NAME}_v140_Dynamic/ExperienceIdentity/)
link_directories(${CMAKE_BINARY_DIR}/packages/Philips.Dvlp.1.0.0.100109-master/lib/native/${CMAKE_BUILD_TYPE}_Dynamic_${CMAKE_VS_PLATFORM_NAME}/)
link_directories(${CMAKE_BINARY_DIR}/packages/Philips.Cwis.1.1.1.10148/lib/native/${CMAKE_BUILD_TYPE}_${CMAKE_VS_PLATFORM_NAME}_v140_Dynamic/Cwis/)
link_directories(${CMAKE_BINARY_DIR}/packages/Philips.IpXres3.1.2.2/IpXres3/lib/v140/${CMAKE_VS_PLATFORM_NAME}/)
link_directories(${CMAKE_BINARY_DIR}/packages/Philips.CoronaryTools.Thirdparty.0.0.35/MergeCom3/mc3lib/)
link_directories(${CMAKE_BINARY_DIR}/packages/Philips.CoronaryTools.Thirdparty.0.0.35/FlexLm/Lib/x64/)
link_directories(${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/)
link_directories(../../Externals/Ipp/IPP-release-17.3-libraries/Lib/)
link_directories(../../Externals/Ipp/IPP-release-17.3-libraries/Lib/Extra/)
link_directories(../../Externals/Ipp/Bin)

add_executable(PciApp WIN32 Main.cpp
						LicenseManager.cpp
						NetworkIP.cpp
						Resources.cpp
						WindowKeyHandler.cpp)

target_link_libraries(PciApp Shlwapi 
							Sscf1.142dlld.x64 
							IpSimplicityd
							mswsock
							iphlpapi
							mfuuid
							Mfreadwrite
							Mfplat
							IpXres3d
							Sense
							ExperienceIdentity
							Dvlp
							Cwis
							Net.142dlld.x64
							TraceWriter.142dlld.x64
							Philips.IEngine.IEngineIppd
							ippcc90lgcmt
							ippccmt
							ippcoremt
							ippdcmt
							ippi90lgcmt
							ippimt
							ippj90lgcmt
							ippsmt
							ippvc90lgcmt
							ws2_32
							psapi
							mc3adll64d
							dinput8
							dxguid
							core
							Sense
							ExperienceIdentity
							gdiplus
							winmm
							usp10
							CommonPlatform.CoRegistration
							CommonPlatform.TSM
							CommonPlatform.Utilities
							CommonPlatform.Dicom
							CommonPlatform.Xray
							CommonPlatform.Logging
							AuditTrailInterface
							CommonPlatform.AppLauncherNative
							PciExpressTestInterface
							Localization
							StenboostAlgo
							iControlInterface
							iControlLib
							FlexLM
							iAcquisitionInterface
							PrmLib
							ErrLib
							MkxLib
							BxLib
							OscLib
							CbdLib
							DiaLib
							SwLib
							RdgLib)

target_include_directories(PciApp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_include_directories(PciApp PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Cwis.1.1.1.10148/include/)
target_include_directories(PciApp PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Sscf.1.4.2/include/)
target_include_directories(PciApp PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.TraceWriter.1.3.0/include/)
target_include_directories(PciApp PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Trace.1.3.0/include/)
target_include_directories(PciApp PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Dvlp.1.0.0.100109-master/include/)
target_include_directories(PciApp PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Net.1.3.5.101019/include/)
target_include_directories(PciApp PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Net.1.3.5.101019/include/net/)
target_include_directories(PciApp PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Sense.1.13.1.104054/include/)
target_include_directories(PciApp PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.IpXres3.1.2.2/IpXres3/include/)
target_include_directories(PciApp PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.IpSimplicity.1.2.2/IpSimplicity/include/)
target_include_directories(PciApp PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.IpSimplicity.1.2.2/Medisys/include/)
target_include_directories(PciApp PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Sense.ExperienceIdentity.1.13.1.104054/include/)
target_include_directories(PciApp PUBLIC ../../Common/iControlInterface/include/)
target_include_directories(PciApp PUBLIC ../../Common/CommonPlatform.Logging/include/)
target_include_directories(PciApp PUBLIC ../../Common/CommonPlatform.Utilities/include/)
target_include_directories(PciApp PUBLIC ../../Common/CommonPlatform.TSM/include/)
target_include_directories(PciApp PUBLIC ../../Common/CommonPlatform.Dicom/include/)
target_include_directories(PciApp PUBLIC ../../Common/CommonPlatform.Dicom/Merge/include/)
target_include_directories(PciApp PUBLIC ../../Common/CommonPlatform.Dicom/TLS/include/)
target_include_directories(PciApp PUBLIC ../../Common/CommonPlatform.Dicom/CharacterSetDetector/include/)
target_include_directories(PciApp PUBLIC ../../Common/CommonPlatform.Coregistration/include/)
target_include_directories(PciApp PUBLIC ../../Common/CommonPlatform.Xray/include/)
target_include_directories(PciApp PUBLIC ../../Common/CommonPlatform.iControlInterface/include/)
target_include_directories(PciApp PUBLIC ../../Common/CommonPlatform.AppLauncherNative/include/)
target_include_directories(PciApp PUBLIC ../../Common/iControlInterface/include/)
target_include_directories(PciApp PUBLIC ../../Common/iAcquisitionInterface/include/)
target_include_directories(PciApp PUBLIC ../../Common/AuditTrailInterface/include/)
target_include_directories(PciApp PUBLIC ../../TestAutomation/TAF/PciExpressTestInterface/include/)
target_include_directories(PciApp PUBLIC ../Core/include/)
target_include_directories(PciApp PUBLIC ../Core/Localization/include/)
target_include_directories(PciApp PUBLIC ../Core/StenboostAlgo/include/)
target_include_directories(PciApp PUBLIC ../Algorithms/PrmLib/include/Public/)
target_include_directories(PciApp PUBLIC ../Algorithms/ErrLib/include/Public/)
target_include_directories(PciApp PUBLIC ../Algorithms/MkxLib/include/Public/)
target_include_directories(PciApp PUBLIC ../Algorithms/BxLib/include/Public/)
target_include_directories(PciApp PUBLIC ../Algorithms/OscLib/include/Public/)
target_include_directories(PciApp PUBLIC ../Algorithms/CbdLib/include/Public/)
target_include_directories(PciApp PUBLIC ../Algorithms/DiaLib/include/Public/)
target_include_directories(PciApp PUBLIC ../Algorithms/SwLib/include/Public/)
target_include_directories(PciApp PUBLIC ../Algorithms/RdgLib/include/Public/)
target_include_directories(PciApp PUBLIC ../Algorithms/)
target_include_directories(PciApp PUBLIC ../../Externals/Ipp/IPP-release-17.3/ipp/include/)
target_include_directories(PciApp PUBLIC ../../Externals/Ipp/IEngineIpp/src/include/)
target_include_directories(PciApp PUBLIC ../../Externals/Ipp/IEngineIpp/src/jpeg/include/)
target_include_directories(PciApp PUBLIC ../../Externals/Ipp/IEngineIpp/src/png/include/)
target_include_directories(PciApp PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.CoronaryTools.Thirdparty.0.0.35/MergeCom3/mc3inc/)

set_target_properties(PciApp PROPERTIES FOLDER "Application")

