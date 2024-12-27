# DiaLib library
add_library(DiaLib STATIC src/DiaBxGaussian.c
						src/DiaExtractDiaphragm.c
						src/DiaExtractShutters.c
						src/DiaLib.c
						src/DiaLibClassification.c
						src/DiaLibHough.c
						src/DiaLibTracking.c
						src/DiaMxDisplay.c
						src/DiaUtil.c)

target_include_directories(DiaLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/Private/)
target_include_directories(DiaLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/Public/)
target_include_directories(DiaLib PUBLIC ../ErrLib/include/Public/)