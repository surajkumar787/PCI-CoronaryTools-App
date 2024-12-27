# Bxlib library

add_library(Bxlib STATIC src/BxCurve.c
						src/BxGaussian.c
						src/BxSnake.c
						src/BxSpline.c
						src/BxVessel.c
						src/BxWarp.c
						src/BxWire.c)

target_include_directories(Bxlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/Private/)
target_include_directories(Bxlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/Public/)

