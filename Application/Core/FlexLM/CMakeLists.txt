# FlexLM library

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

add_compile_definitions(dllexport_sense=) 

link_directories(${CMAKE_BINARY_DIR}/packages/Philips.CoronaryTools.Thirdparty.0.0.35/FlexLm/Lib/x64/)
#add_library(FlexLM STATIC LicenseCheck.cpp)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
add_library(FlexLM SHARED LicenseCheck.cpp)

target_compile_definitions(FlexLM PRIVATE FLEXLM_EXPORTS)

target_link_libraries(FlexLM lmgr
							libcrvs
							libnoact
							libsb
							legacy_stdio_definitions
							lmgr_dongle_stub
							wsock32
							netapi32
							advapi32
							comdlg32
							comctl32
							Wbemuuid
							shlwapi
							ws2_32)

target_include_directories(FlexLM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)
target_include_directories(FlexLM PUBLIC ../include/)
target_include_directories(FlexLM PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.CoronaryTools.Thirdparty.0.0.35/FlexLm/includes/)

