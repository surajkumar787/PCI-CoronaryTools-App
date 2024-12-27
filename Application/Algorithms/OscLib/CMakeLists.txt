# OscLib library

add_compile_definitions(__STDC_LIMIT_MACROS)

add_library(OscLib STATIC src/BxGaussian.c
						src/Osc.cpp
						src/OscAngioExtractPictures.cpp
						src/OscAngioExtractSequence.cpp
						src/OscCardiacCycle.cpp
						src/OscCatheterBody.cpp
						src/OscCleanVasculature.cpp
						src/OscDiaphragm.cpp
						src/OscExtractShutters.cpp
						src/OscFluoroExtractPictures.cpp
						src/OscGetImages.cpp
						src/OscInjectionCatheter.cpp
						src/OscMedisysOverlay.cpp
						src/OscMemoryManagement.cpp
						src/OscMotionEstimation.cpp
						src/OscParam.cpp
						src/OscPyramid.cpp
						src/OscRdgBackground.cpp
						src/OscRdgComputation.cpp
						src/OscRdgComputationSSE.cpp
						src/OscRdgThresholding.cpp
						src/OscROIs.cpp
						src/OscSternalWire.cpp
						src/OscUtilsImage.cpp
						src/OscViterbi.cpp)
						


target_include_directories(OscLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/Private/)
target_include_directories(OscLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/Public/)
target_include_directories(OscLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/Published/)
target_include_directories(OscLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/AlgoFragments/BasicFilters/)
target_include_directories(OscLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/AlgoFragments/Common/)
target_include_directories(OscLib PUBLIC ../ErrLib/include/Public/)
target_include_directories(OscLib PUBLIC ../PrmLib/include/Public/)
target_include_directories(OscLib PUBLIC ../CbdLib/include/Public/)
target_include_directories(OscLib PUBLIC ../DiaLib/include/Public/)
target_include_directories(OscLib PUBLIC ../SwLib/include/Public/)
target_include_directories(OscLib PUBLIC ../)

