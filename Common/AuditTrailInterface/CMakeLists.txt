# AuditTrailInterface library

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

add_library(AuditTrailInterface STATIC src/AuditTrailClient.cpp
						src/IAuditTrail.cpp)

target_include_directories(AuditTrailInterface PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_include_directories(AuditTrailInterface PUBLIC ../CommonPlatform.Logging/include/)
target_include_directories(AuditTrailInterface PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Sscf.1.4.2/include/)

