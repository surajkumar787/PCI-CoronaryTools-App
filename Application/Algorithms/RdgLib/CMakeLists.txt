# RdgLib library
add_library(RdgLib STATIC src/Rdg.c
						src/RdgExtract.c
						src/RdgExtractAngio.c
						src/RdgExtractBackground.c
						src/RdgExtractCatheters.c
						src/RdgExtractFluoro.c
						src/RdgExtractLabel.c
						src/RdgExtractLabelfeatures.c
						src/RdgExtractMask.c
						src/RdgExtractMorphFilter.c
						src/RdgExtractRidgeDifferential.c
						src/RdgExtractRidgeFilter.c
						src/RdgExtractThreshold.c
						src/RdgGaussian.c
						src/RdgParam.c
						src/RdgUtil.c
						src/RdgUtilImage.c)

target_include_directories(RdgLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/Private/)
target_include_directories(RdgLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/Public/)

