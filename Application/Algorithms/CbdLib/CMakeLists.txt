# CbdLib library
add_library(CbdLib STATIC src/CBDBxGaussian.cpp
						src/CBDExtractShutters.cpp
						src/CBDLib.cpp
						src/CBDRidgeComputation.cpp
						src/CBDSeqLib.cpp
						src/CBDSubfunctions.cpp
						src/CBDSubtract.cpp
						src/CBDTrack.cpp
						src/CBDTubeModel.cpp)

target_include_directories(CbdLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/Private/)
target_include_directories(CbdLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/Public/)
target_include_directories(CbdLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/LineTracker/)
target_include_directories(CbdLib PUBLIC ../ErrLib/include/Public/)

