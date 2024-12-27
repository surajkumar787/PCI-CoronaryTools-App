# CommonPlatform.Dicom library

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
# Make sure the compiler supports the specified standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(CommonPlatform.Dicom STATIC 
							src/charsetmanager.cpp
							src/CharSetManagerSystemLanguage.cpp
							src/Dicom.cpp
							src/jpegencoder.cpp
							src/UidUtils.cpp
							Merge/application.cpp
							Merge/association.cpp
							Merge/exception.cpp
							Merge/file.cpp
							Merge/item.cpp
							Merge/library.cpp
							Merge/message.cpp
							Merge/messagebase.cpp
							Merge/messagebaseimpl.cpp
							Merge/proposedservicelist.cpp
							Merge/secureassociation.cpp
							Merge/service.cpp
							Merge/syntaxlist.cpp
							Merge/util.cpp
							TLS/applicationcontext.cpp
							TLS/mergetlsadapter.cpp
							TLS/schannelcontroller.cpp
							TLS/socket.cpp
							TLS/tlsutilities.cpp		
							CharacterSetDetector/charactersetdetector.cpp
							CharacterSetDetector/charsetutility.cpp
							CharacterSetDetector/detectorfactory.cpp
							CharacterSetDetector/gb18030.cpp
							CharacterSetDetector/isoir100.cpp
							CharacterSetDetector/isoir13.cpp
							CharacterSetDetector/isoir159.cpp
							CharacterSetDetector/isoir192.cpp
							CharacterSetDetector/isoir6.cpp
							CharacterSetDetector/isoir87.cpp
							CharacterSetDetector/stringutil.cpp
							CharacterSetDetector/unicodeblock.cpp)

target_include_directories(CommonPlatform.Dicom PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_include_directories(CommonPlatform.Dicom PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Merge/include/)
target_include_directories(CommonPlatform.Dicom PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/TLS/include/)
target_include_directories(CommonPlatform.Dicom PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/CharacterSetDetector/include/)
target_include_directories(CommonPlatform.Dicom PUBLIC ../CommonPlatform.Logging/include/)
target_include_directories(CommonPlatform.Dicom PUBLIC ../CommonPlatform.Utilities/include/)
target_include_directories(CommonPlatform.Dicom PUBLIC ../CommonPlatform.Xray/include/)
target_include_directories(CommonPlatform.Dicom PUBLIC ../../Application/Core/include/)
target_include_directories(CommonPlatform.Dicom PUBLIC ../../Externals/Ipp/IPP-release-17.3/ipp/include/)
target_include_directories(CommonPlatform.Dicom PUBLIC ../../Externals/Ipp/IEngineIpp/src/include/)
target_include_directories(CommonPlatform.Dicom PUBLIC ../../Externals/Ipp/IEngineIpp/src/jpeg/include/)
target_include_directories(CommonPlatform.Dicom PUBLIC ../../Externals/Ipp/IEngineIpp/src/png/include/)
target_include_directories(CommonPlatform.Dicom PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.CoronaryTools.Thirdparty.0.0.35/MergeCom3/mc3inc/)

