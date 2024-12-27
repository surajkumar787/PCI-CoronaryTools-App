#PCI-CRONARY-TOOLS CORE PART

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

add_compile_definitions(dllexport_sense=) 
add_compile_definitions(dllexport_experienceidentity=)

# Translation library
add_subdirectory(Localization)

# FlexLM library
#add_subdirectory(FlexLM)

# StenboostAlgo library
add_subdirectory(StenboostAlgo)

add_library(Core STATIC 
src/AboutInfo.cpp
src/AlluraSeparator.cpp
src/ArchivingSettings.cpp
src/arrowcontrol.cpp
src/ButtonTransparent.cpp
src/ConnectionSettings.cpp
src/CoRegistrationAdministration.cpp
src/CoRegistrationControl.cpp
src/CoRegistrationGuidanceState.cpp
src/CoRegistrationState.cpp
src/CoRegParameters.cpp
src/CoregUiAdapter.cpp
src/countdowncontrol.cpp
src/CrmAdministration.cpp
src/CrmAlgoAngio.cpp
src/CrmAlgoFluoro.cpp
src/CrmApcController.cpp
src/CrmApcGuidanceState.cpp
src/CrmCreationGuidanceState.cpp
src/CrmErrorGuidanceState.cpp
src/CrmErrors.cpp
src/CrmGuidanceBaseState.cpp
src/CrmMovieGrabber.cpp
src/CrmOverlay.cpp
src/CrmOverlayState.cpp
src/CrmParameters.cpp
src/CrmPictorialItem.cpp
src/CrmPictorialListControl.cpp
src/CrmProcessingAngioState.cpp
src/CrmRecording.cpp
src/CrmRepository.cpp
src/CrmRoadmap.cpp
src/CrmRoadmapActive.cpp
src/CrmRoadmapSelectViewer.cpp
src/CrmTableGuidanceState.cpp
src/CrmThumbnail.cpp
src/CrmTsmPage.cpp
src/CrmViewer.cpp
src/FmTrendDisplay.cpp
src/FrameGrabber.cpp
src/GraphDisplay.cpp
src/LicensesCollection.cpp
src/Line.cpp
src/LineSubsampler.cpp
src/MainController.cpp
src/MainControllerTestServer.cpp
src/MainModel.cpp
src/MainViewer.cpp
src/MenuButton.cpp
src/MenuButtonAllura.cpp
src/MenuButtonSmartSuite.cpp
src/MessageBuilder.cpp
src/MovieGrabber.cpp
src/MovieGrabberStbSubtract.cpp
src/NoConnectionState.cpp
src/NodeSettings.cpp
src/NoLicenseState.cpp
src/NonPciEpxState.cpp
src/PACSArchiver.cpp
src/PACSPush.cpp
src/patientbar.cpp
src/PciBaseViewer.cpp
src/PciEpx.cpp
src/PciEpxSelectionView.cpp
src/PciGuidanceView.cpp
src/PciProgressBar.cpp
src/PciThreadBackground.cpp
src/PciThreadGrabber.cpp
src/PciThreadLive.cpp
src/PciThreadStentBoost.cpp
src/PciThumbnail.cpp
src/PciTsmPage.cpp
src/PciViewContext.cpp
src/PciXrayGeometryControl.cpp
src/PciXrayPositionViewer.cpp
src/PciXrayViewer.cpp
src/PictorialItem.cpp
src/PictorialItemAllura.cpp
src/PictorialItemAzurion.cpp
src/PictorialListControl.cpp
src/PressureDisplay.cpp
src/ProblemReportDialog.cpp
src/PullbackRecording.cpp
src/Resources.cpp
src/SmartSuiteAboutBox.cpp
src/SmartSuiteLicenseAgreement.cpp
src/Snapshot.cpp
src/StateControlUtility.cpp
src/StbAdministration.cpp
src/StbAlgoBoost.cpp
src/StbAlgoContrast.cpp
src/StbAlgoLive.cpp
src/StbAlgoMarkers.cpp
src/StbAlgoTransform.cpp
src/StbBoostFrameSelectionState.cpp
src/StbBoostViewer.cpp
src/StbContrastFrameSelectionState.cpp
src/StbContrastGuidanceState.cpp
src/StbContrastProcessingState.cpp
src/StbContrastReviewState.cpp
src/StbContrastRoiDefinitionState.cpp
src/StbContrastViewer.cpp
src/StbGuidanceState.cpp
src/StbImage.cpp
src/StbImageSelectTsmPage.cpp
src/StbImageSelectViewer.cpp
src/StbLiveGuidanceState.cpp
src/StbLiveProcessingState.cpp
src/StbLiveReviewState.cpp
src/StbMarkers.cpp
src/StbMarkersViewer.cpp
src/StbMask.cpp
src/StbParameters.cpp
src/StbPictorialItem.cpp
src/StbPictorialListControl.cpp
src/StbPostDeployGuidanceState.cpp
src/StbPostDeployProcessingState.cpp
src/StbPostDeployReviewState.cpp
src/StbPostDeployRoiDefinitionState.cpp
src/StbProcessingState.cpp
src/StbRecording.cpp
src/StbReviewState.cpp
src/StbRoiDefinitionState.cpp
src/StbRoiTsmPage.cpp
src/StbRoiViewer.cpp
src/StbTsmPage.cpp
src/StbUIHelpers.cpp
src/StbViewer.cpp
src/StorageCommitListener.cpp
src/TextureSimplicity.cpp
src/UiState.cpp
src/UiStateController.cpp
src/UserMouseKeyboardMonitor.cpp
src/UserPrefs.cpp
src/ViewStateHelper.cpp
src/ViewTypeDetector.cpp
src/XRayImageRecorder.cpp
src/XRayImageRecording.cpp)


link_directories(${CMAKE_BINARY_DIR}/packages/Philips.IpSimplicity.1.2.2/IpSimplicity/lib/v140/${CMAKE_VS_PLATFORM_NAME}/)
target_link_libraries(Core IpSimplicityd)

target_include_directories(Core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_include_directories(Core PUBLIC ../../Common/CommonPlatform.Logging/include/)
target_include_directories(Core PUBLIC ../../Common/CommonPlatform.Utilities/include/)
target_include_directories(Core PUBLIC ../../Common/CommonPlatform.TSM/include/)
target_include_directories(Core PUBLIC ../../Common/CommonPlatform.Dicom/include/)
target_include_directories(Core PUBLIC ../../Common/CommonPlatform.Dicom/Merge/include/)
target_include_directories(Core PUBLIC ../../Common/CommonPlatform.Dicom/TLS/include/)
target_include_directories(Core PUBLIC ../../Common/CommonPlatform.Dicom/CharacterSetDetector/include/)
target_include_directories(Core PUBLIC ../../Common/CommonPlatform.Coregistration/include/)
target_include_directories(Core PUBLIC ../../Common/CommonPlatform.Xray/include/)
target_include_directories(Core PUBLIC ../../Common/CommonPlatform.iControlInterface/include/)
target_include_directories(Core PUBLIC ../../Common/CommonPlatform.AppLauncherNative/include/)
target_include_directories(Core PUBLIC ../../Common/iControlInterface/include/)
target_include_directories(Core PUBLIC ../../Common/iAcquisitionInterface/include/)
target_include_directories(Core PUBLIC ../../Common/AuditTrailInterface/include/)
target_include_directories(Core PUBLIC ../../TestAutomation/TAF/PciExpressTestInterface/include/)
target_include_directories(Core PUBLIC Localization/include/)
target_include_directories(Core PUBLIC StenboostAlgo/include/)
target_include_directories(Core PUBLIC ../Algorithms/PrmLib/include/Public/)
target_include_directories(Core PUBLIC ../Algorithms/ErrLib/include/Public/)
target_include_directories(Core PUBLIC ../Algorithms/MkxLib/include/Public/)
target_include_directories(Core PUBLIC ../Algorithms/BxLib/include/Public/)
target_include_directories(Core PUBLIC ../Algorithms/OscLib/include/Public/)
target_include_directories(Core PUBLIC ../Algorithms/CbdLib/include/Public/)
target_include_directories(Core PUBLIC ../Algorithms/DiaLib/include/Public/)
target_include_directories(Core PUBLIC ../Algorithms/SwLib/include/Public/)
target_include_directories(Core PUBLIC ../Algorithms/RdgLib/include/Public/)
target_include_directories(Core PUBLIC ../Algorithms/)
target_include_directories(Core PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Sense.1.13.1.104054/include/)
target_include_directories(Core PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.IpXres3.1.2.2/IpXres3/include/)
target_include_directories(Core PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.IpSimplicity.1.2.2/IpSimplicity/include/)
target_include_directories(Core PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.IpSimplicity.1.2.2/Medisys/include/)
target_include_directories(Core PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Sense.ExperienceIdentity.1.13.1.104054/include/)
target_include_directories(Core PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Cwis.1.1.1.10148/include/)
target_include_directories(Core PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.Sscf.1.4.2/include/)
target_include_directories(Core PUBLIC ${CMAKE_BINARY_DIR}/packages/Philips.CoronaryTools.Thirdparty.0.0.35/MergeCom3/mc3inc/)


# Core.Archiving.ModuleTest executable
#add_subdirectory(Core.Archiving.ModuleTest)

set_target_properties(Core PROPERTIES FOLDER "Core")
set_target_properties(Localization PROPERTIES FOLDER "Core")
#set_target_properties(FlexLM PROPERTIES FOLDER "Core")
set_target_properties(StenboostAlgo PROPERTIES FOLDER "Core")
#set_target_properties(Core.Archiving.ModuleTest PROPERTIES FOLDER "Core")

