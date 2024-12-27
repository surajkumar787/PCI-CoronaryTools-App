
set_property(GLOBAL PROPERTY USE_FOLDERS ON) 

# AuditTrailInterface library
add_subdirectory(AuditTrailInterface)

# CommonPlatform.Logging library
add_subdirectory(CommonPlatform.Logging)

# CommonPlatform.AppLauncherNative library
add_subdirectory(CommonPlatform.AppLauncherNative)

# CommonPlatform.Coregistration library
add_subdirectory(CommonPlatform.Coregistration)

# CommonPlatform.Utilities library
add_subdirectory(CommonPlatform.Utilities)

# CommonPlatform.TSM library
add_subdirectory(CommonPlatform.TSM)

# CommonPlatform.Xray library
add_subdirectory(CommonPlatform.Xray)

# CommonPlatform.Dicom library
add_subdirectory(CommonPlatform.Dicom)

# iAcquisitionInterface library
add_subdirectory(iAcquisitionInterface)

# iControlInterface library
add_subdirectory(iControlInterface)

# iControlLib library
add_subdirectory(iControlLib)

set_target_properties(AuditTrailInterface PROPERTIES FOLDER "Platform")
set_target_properties(CommonPlatform.Logging PROPERTIES FOLDER "Platform")
set_target_properties(CommonPlatform.AppLauncherNative PROPERTIES FOLDER "Platform")
set_target_properties(CommonPlatform.Coregistration PROPERTIES FOLDER "Platform")
set_target_properties(CommonPlatform.Utilities PROPERTIES FOLDER "Platform")
set_target_properties(CommonPlatform.TSM PROPERTIES FOLDER "Platform")
set_target_properties(CommonPlatform.Xray PROPERTIES FOLDER "Platform")
set_target_properties(CommonPlatform.Dicom PROPERTIES FOLDER "Platform")
set_target_properties(iAcquisitionInterface PROPERTIES FOLDER "Platform")
set_target_properties(iControlInterface PROPERTIES FOLDER "Platform")
set_target_properties(iControlLib PROPERTIES FOLDER "Platform")


