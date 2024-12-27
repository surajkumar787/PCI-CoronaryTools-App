# SwLib library
add_library(SwLib STATIC src/SWBxGaussian.c
						src/SWExtractShutters.cpp
						src/SWLib.cpp
						src/SWLibComponentsAnalysis.cpp
						src/SWLibProcess.cpp
						src/SWLibSubfunctions.cpp
						src/SWLibTracking.cpp
						src/SWMkxGaussian.cpp
						src/SWRdgComputationSSE.cpp)

target_include_directories(SwLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/Private/)
target_include_directories(SwLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/Public/)
target_include_directories(SwLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/AlgoFragments/BasicFilters/)
target_include_directories(SwLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/AlgoFragments/Common/)
target_include_directories(SwLib PUBLIC ../)

