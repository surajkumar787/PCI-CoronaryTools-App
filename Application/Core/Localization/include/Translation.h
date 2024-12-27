﻿/*----------------------------------------------------
 |      This file was generated by Res2Sense 2.0     |
 |              Do not modify this file.             |
 ----------------------------------------------------*/

#pragma once

#include <Sense.h>

namespace Pci { namespace Core { namespace Localization { 

class Translation
{
public:

	
	enum class Language { English, Danish, German, Spanish, French, Italian, Dutch, Norwegian, Swedish, Japanese, ChineseSimplified, ChineseTraditional, TEST };
	
	static void setLanguage(Language language);
	
	static std::wstring IDS_Abort;
	static std::wstring IDS_AboutDCR;
	static std::wstring IDS_AboutSBL;
	static std::wstring IDS_Acquisition;
	static std::wstring IDS_ActivateCrmEpx;
	static std::wstring IDS_ActivateStbEpx;
	static std::wstring IDS_ActivateStbOrCrmEpx;
	static std::wstring IDS_AngulationCAUD;
	static std::wstring IDS_AngulationCRAN;
	static std::wstring IDS_Apc;
	static std::wstring IDS_ArchivingMovie;
	static std::wstring IDS_ArchivingScreenshot;
	static std::wstring IDS_ArchivingSnapshot;
	static std::wstring IDS_BalloonMarkersLost;
	static std::wstring IDS_CaptureThreeHeartbeats;
	static std::wstring IDS_CaptureThreeHeartbeatsWithContrast;
	static std::wstring IDS_CArcAndTableMovedDuringAngio;
	static std::wstring IDS_CArcMovedDuringAngio;
	static std::wstring IDS_ConfirmBalloonMarkersVisible;
	static std::wstring IDS_ConnectionEstablishing;
	static std::wstring IDS_ConnectionFailure;
	static std::wstring IDS_ContactServiceProvider;
	static std::wstring IDS_ContrastFillAllVessels;
	static std::wstring IDS_CoregLicenseExpired;
	static std::wstring IDS_CoronaryTools;
	static std::wstring IDS_CreateAngioWithoutMoving;
	static std::wstring IDS_CreateAngioWithoutMovingCArc;
	static std::wstring IDS_CreateAngioWithoutMovingCArcAndTable;
	static std::wstring IDS_CreateAngioWithoutMovingTable;
	static std::wstring IDS_CrmLicenseExpired;
	static std::wstring IDS_Distal;
	static std::wstring IDS_DoNotMoveTableCarc;
	static std::wstring IDS_DynamicCoronaryRoadmap;
	static std::wstring IDS_EnableAPCForAutoPositiong;
	static std::wstring IDS_EnableAPCForAutoPositiongSmS;
	static std::wstring IDS_EnhancementFailed;
	static std::wstring IDS_ExportBusy;
	static std::wstring IDS_ExportBusyTitle;
	static std::wstring IDS_ExportRemaining;
	static std::wstring IDS_ExposureTooShort;
	static std::wstring IDS_FailedToSaveRoadmap;
	static std::wstring IDS_FFR;
	static std::wstring IDS_Help;
	static std::wstring IDS_HoldACCToMoveToRoadmap;
	static std::wstring IDS_IFR;
	static std::wstring IDS_KeepWirePosStable;
	static std::wstring IDS_LegalAllRightsReserved;
	static std::wstring IDS_LegalCopyright;
	static std::wstring IDS_LegalMedicalDevice;
	static std::wstring IDS_LegalOwnership;
	static std::wstring IDS_LegalReverseEngineer;
	static std::wstring IDS_ManuallyMoveCarc;
	static std::wstring IDS_ManuallyMoveTable;
	static std::wstring IDS_MinimumImagesForEnhancement;
	static std::wstring IDS_Movie;
	static std::wstring IDS_MovieArchived;
	static std::wstring IDS_MovieArchivingFailed;
	static std::wstring IDS_MovieRecord;
	static std::wstring IDS_NearestRoadmap;
	static std::wstring IDS_NewRoadmapFailed;
	static std::wstring IDS_NoContrast;
	static std::wstring IDS_NoLicense;
	static std::wstring IDS_NoLicenseStbLive;
	static std::wstring IDS_NoRoadmapAvailable;
	static std::wstring IDS_NotEnoughContrast;
	static std::wstring IDS_Pa;
	static std::wstring IDS_PatientBarDob;
	static std::wstring IDS_PatientBarDobUnknown;
	static std::wstring IDS_PatientBarId;
	static std::wstring IDS_Pd;
	static std::wstring IDS_PressFluoroForWirePosition;
	static std::wstring IDS_PreviousRoadmap;
	static std::wstring IDS_ProblemReport;
	static std::wstring IDS_ProblemReportReady;
	static std::wstring IDS_ProcessingAngio;
	static std::wstring IDS_ReadyForFFRRoadmap;
	static std::wstring IDS_ReadyForiFRRoadmap;
	static std::wstring IDS_ReadyToReceiveRoadmap;
	static std::wstring IDS_ReadyToStentboost;
	static std::wstring IDS_RestartOrContactServiceProvider;
	static std::wstring IDS_RoadmapAvailable;
	static std::wstring IDS_RoadmapFailed;
	static std::wstring IDS_RoadmapNotReady;
	static std::wstring IDS_RoadmapReady;
	static std::wstring IDS_RotationLAO;
	static std::wstring IDS_RotationRAO;
	static std::wstring IDS_RunTooShort;
	static std::wstring IDS_ScreenshotArchived;
	static std::wstring IDS_ScreenshotArchivingFailed;
	static std::wstring IDS_Service;
	static std::wstring IDS_ShortMonth_April;
	static std::wstring IDS_ShortMonth_August;
	static std::wstring IDS_ShortMonth_December;
	static std::wstring IDS_ShortMonth_February;
	static std::wstring IDS_ShortMonth_January;
	static std::wstring IDS_ShortMonth_July;
	static std::wstring IDS_ShortMonth_June;
	static std::wstring IDS_ShortMonth_March;
	static std::wstring IDS_ShortMonth_May;
	static std::wstring IDS_ShortMonth_November;
	static std::wstring IDS_ShortMonth_October;
	static std::wstring IDS_ShortMonth_September;
	static std::wstring IDS_SnapshotArchived;
	static std::wstring IDS_SnapshotArchivingFailed;
	static std::wstring IDS_Start;
	static std::wstring IDS_StbLiveLicenseExpired;
	static std::wstring IDS_SteadyContrastInjection;
	static std::wstring IDS_StentBoostLive;
	static std::wstring IDS_Stop;
	static std::wstring IDS_Store;
	static std::wstring IDS_System;
	static std::wstring IDS_TabledMovedDuringAngio;
	static std::wstring IDS_TableMoved;
	static std::wstring IDS_TooManyContrastFluctuations;
	static std::wstring IDS_Trend;
	static std::wstring IDS_UnknownRoadmapFailure;
	static std::wstring IDS_UnreliableIFRConnection;
	static std::wstring IDS_UseCinePedalToActivateSBL;
	static std::wstring IDS_WirePositionCaptured;
	static std::wstring IDS_OK;
	static std::wstring IDS_DoNotMoveStentMarkers;
	static std::wstring IDS_UseCinePedalToCreateSBLPostDeployment;
	static std::wstring IDS_ReadyToPostDeployStentboost;
	static std::wstring IDS_Series;
	static std::wstring IDS_Image;
	static std::wstring IDS_CoronaryToolsHasStoppedWorking;
	static std::wstring IDS_CoronaryToolsHasStoppedWorkingFull;
	static std::wstring IDS_Retry;
	static std::wstring IDS_Cancel;
	static std::wstring IDS_AdjustROI;
	static std::wstring IDS_CopyToReference;
	static std::wstring IDS_Apply;
	static std::wstring IDS_Size;
	static std::wstring IDS_StbPostDeploymentLicenseExpired;
	static std::wstring IDS_NoLicenseStbPostDeployment;
	static std::wstring IDS_StbSubtractLicenseExpired;
	static std::wstring IDS_NoLicenseStbSubtract;
	static std::wstring IDS_ReadyToReceiveStbSubtract;
	static std::wstring IDS_UseCinePedalToCreateSbSubtract;
	static std::wstring IDS_UseTSMToSelectDifferentImage;
	static std::wstring IDS_ChangeContrastImage;
	static std::wstring IDS_UseTSMForAdditionalFunctions;
	static std::wstring IDS_UseTSMToSelectRoadmap;
	static std::wstring IDS_LockRoadmap;
	static std::wstring IDS_RoadmapLocked;
	static std::wstring IDS_Boosted;
	static std::wstring IDS_Images;
	static std::wstring IDS_BoostedImage;
	static std::wstring IDS_ContrastImage;
	static std::wstring IDS_InjectContrast;
	static std::wstring IDS_Seconds;
	static std::wstring IDS_HoldCinePedalUntillVesselsFilledWithContrast;
	static std::wstring IDS_MakeSureMarkersAreVisible;
	static std::wstring IDS_RoadmapsCanBeLockerThroughTSM;
	static std::wstring IDS_EnhanceStent;
	static std::wstring IDS_ResetToDefault;
	static std::wstring IDS_RoadmapOptions;
	static std::wstring IDS_ChangeBoostImage;
	static std::wstring IDS_ProcessingStentBoost;
	static std::wstring IDS_UseTSMToSelectLastImageBoostedPhase;
	static std::wstring IDS_EnableAPCForAutoPositiongAzurion21;
	
	struct TranslationDescription
	{
		const wchar_t *name;
		const std::wstring *translation;
	};
	
	static const TranslationDescription translationList[161];

};

} } } 