# MkxLib library

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
# Make sure the compiler supports the specified standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

add_library(MkxLib STATIC src/Mkx.cpp
						src/MkxAdapt.cpp
						src/MkxApplyRigidMotionMask.cpp
						src/MkxBlobEnhance.cpp
						src/MkxBlobSelect.cpp
						src/MkxBxMarkerCoupleWireRescore.cpp
						src/MkxConsolidate.cpp
						src/MkxConsolidateFunc.cpp
						src/MkxCouplesSelect.cpp
						src/MkxExtract.cpp
						src/MkxFillResults.cpp
						src/MkxLabelling.cpp
						src/MkxMarkerDirection.cpp
						src/MkxParam.cpp
						src/MkxPixSelect.cpp
						src/MkxPolarize.cpp
						src/MkxPredict.cpp
						src/MkxRefine.cpp
						src/MkxRefining.cpp
						src/MkxSetParaCtx.cpp
						src/MkxStat.cpp
						src/MkxSubSampleImage.cpp
						src/MkxSysTime.cpp
						src/MkxTracking.cpp
						src/MkxUtil.cpp
						src/MkxWire.cpp
						src/Tdi.cpp
						src/Tr1Analyse.cpp
						src/Tr1InOut.cpp
						src/Tr1Main.cpp
						src/Tr1PredErr.cpp)

target_include_directories(MkxLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/Private/)
target_include_directories(MkxLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/Public/)
target_include_directories(MkxLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/Published/)
target_include_directories(MkxLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/AlgoFragments/BasicFilters/)
target_include_directories(MkxLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/AlgoFragments/Common/)
target_include_directories(MkxLib PUBLIC ../ErrLib/include/Public/)
target_include_directories(MkxLib PUBLIC ../PrmLib/include/Public/)
target_include_directories(MkxLib PUBLIC ../BxLib/include/Public/)
target_include_directories(MkxLib PUBLIC ../BxLib/include/Private/)
target_include_directories(MkxLib PUBLIC ../SwLib/include/Public/)
target_include_directories(MkxLib PUBLIC ../)

