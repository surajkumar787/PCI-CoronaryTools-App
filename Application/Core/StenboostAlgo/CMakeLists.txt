# StenboostAlgo library

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

add_compile_definitions(dllexport_sense=) 
add_compile_definitions(dllexport_experienceidentity=)

add_library(StenboostAlgo STATIC 
					src/FxdSink.cpp
					src/NodeMarkerExtraction.cpp
					src/NodeRunningAvg.cpp
					src/StbContrastDetect.cpp
					src/StbContrastSelect.cpp
					src/StbFrangi.cpp
					src/StentMarkers.cpp)

target_include_directories(StenboostAlgo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_include_directories(StenboostAlgo PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Sense.1.13.1.104054/include/)
target_include_directories(StenboostAlgo PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.IpXres3.1.2.2/IpXres3/include/)
target_include_directories(StenboostAlgo PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.IpSimplicity.1.2.2/IpSimplicity/include/)
target_include_directories(StenboostAlgo PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.IpSimplicity.1.2.2/Medisys/include/)
target_include_directories(StenboostAlgo PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Sense.ExperienceIdentity.1.13.1.104054/include/)
target_include_directories(StenboostAlgo PUBLIC ../../Algorithms/PrmLib/include/Public/)
target_include_directories(StenboostAlgo PUBLIC ../../Algorithms/ErrLib/include/Public/)
target_include_directories(StenboostAlgo PUBLIC ../../Algorithms/MkxLib/include/Public/)
target_include_directories(StenboostAlgo PUBLIC ../../Algorithms/BxLib/include/Public/)
target_include_directories(StenboostAlgo PUBLIC ../../Algorithms/)
target_include_directories(StenboostAlgo PUBLIC ../../../Common/CommonPlatform.Xray/include/)

