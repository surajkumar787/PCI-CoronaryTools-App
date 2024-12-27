// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "Dicom.h"

#include "messagebase.h"
#include "exception.h"
#include "item.h"
#include "UidUtils.h"
#include "DicomConstants.h"
#include "XrayStudy.h"
#include "XrayGeometry.h"
#include "jpegencoder.h"
#include "stlstring.h"

#include <mergecom.h>
#include <assert.h>
#include <algorithm>
#include <windows.h> 
#include <Settings.h>
#include "PathUtil.h"
#include "Log.h"
#include "ProductInfoUtil.h"

using namespace CommonPlatform::Utilities;

namespace CommonPlatform {	namespace Dicom{
	// image attribute constants (PS3.3, A.8)
	static const int SamplesPerPixel = 3;
	static const int BitsAllocated = 8;
	static const int BitsStored = 8;
	static const int HighBit = 7;
	static const int PixelRepresentation = 0;
	static const int PlanarConfiguration = 0;
	static const std::string BurnedInAnnotation = "NO";

	// study constants
	static const std::string Modality = "XA"; // Other (PS3.3, C.7.3.1.1.1)
	static const std::string ConversionType = "WSD"; // Workstation (PS3.3, C.8-24)

	// general attribute constants
	static const std::string Manufacturer = "Philips";


	bool addNNull(Merge::MessageBase& m, unsigned long tag)
	{
		bool res = m.addAttribute(tag);
		res = res && m.setNull(tag);
		return res;
	}

	template<typename T>
	bool addNSet(Merge::MessageBase & m, unsigned long tag, const T & value)
	{
		bool res = m.addAttribute(tag);
		res = res && m.set(tag, value);
		return res;
	}

	std::string getPhotometricInterpretation(TRANSFER_SYNTAX ts)
	{
		switch (ts)
		{
		case EXPLICIT_LITTLE_ENDIAN:
		case IMPLICIT_LITTLE_ENDIAN:
		case EXPLICIT_BIG_ENDIAN:
		case IMPLICIT_BIG_ENDIAN:
		case JPEG_LOSSLESS_HIER_14:
			return "RGB";
		case JPEG_BASELINE:
			return "YBR_FULL_422";
		default:
			assert(false);
			return "";
		}
	}

	void ConvertRGBAToBGR(const char* rgbaSource, uint32_t width, uint32_t height, std::vector<char>& bgrTarget)
	{
		struct RGBA { unsigned char r, g, b, a; };
		struct BGR { unsigned char b, g, r; };

		int pixelCount = width * height;

		bgrTarget.resize(pixelCount * 3);
		auto rgba = reinterpret_cast<const RGBA*>(rgbaSource);
		auto bgr = reinterpret_cast<BGR*>(bgrTarget.data());

		for (int i = 0 ; i < pixelCount; i++)
		{
			bgr[i].r = rgba[i].r;
			bgr[i].g = rgba[i].g;
			bgr[i].b = rgba[i].b;
		}
	}

	bool setCompressedFrameData(Merge::MessageBase& msg, const std::vector<std::vector<char>>& frames, int32_t imgWidth, int32_t imgHeight, bool lossless)
	{
		struct UserData
		{
			bool lossless;
			int width, height;
			std::vector<char> bgrFrame;
			std::vector<char> encFrame;
			const std::vector<std::vector<char>>& frames;
			size_t curFrame;
		};
		UserData ctx{ lossless, imgWidth, imgHeight, {}, {}, frames, 0 };

		auto callback = [](int /*msg*/, unsigned long /*tag*/, int /*firstCall*/, void* ctx, int* bufferSize, void** buffer, int* isLast)
		{
			auto ud = static_cast<UserData*>(ctx);
			if (ud->frames.empty() || ud->curFrame >= ud->frames.size() || ud->frames[ud->curFrame].empty()) return MC_CANNOT_COMPLY;

			auto& frame = ud->frames[ud->curFrame];
			ConvertRGBAToBGR(frame.data(), ud->width, ud->height, ud->bgrFrame);
			ud->encFrame = encodeJpeg(ud->bgrFrame.data(), ud->width, ud->height, BitsAllocated, HighBit, SamplesPerPixel, ud->lossless);
			if (ud->encFrame.size() & 0x1) ud->encFrame.push_back(0); // add null byte in case of odd number (required by DICOM)

			(*bufferSize) = static_cast<int>(ud->encFrame.size());
			(*buffer) = ud->encFrame.data();
			(*isLast) = 1;

			ud->curFrame++;
			return MC_NORMAL_COMPLETION;
		};

		auto status = MC_Set_Encapsulated_Value_From_Function(msg.id(), MC_ATT_PIXEL_DATA, &ctx, callback);
		while (ctx.curFrame < ctx.frames.size() && status == MC_NORMAL_COMPLETION)
		{
			status = MC_Set_Next_Encapsulated_Value_From_Function(msg.id(), MC_ATT_PIXEL_DATA, &ctx, callback);
		}
		status = MC_Close_Encapsulated_Value(msg.id(), MC_ATT_PIXEL_DATA);
		return status == MC_NORMAL_COMPLETION;
	}

	bool setUncompressedFrameData(Merge::MessageBase& msg, const std::vector<std::vector<char>>& frames, int32_t imgWidth, int32_t imgHeight)
	{
		struct UserData
		{
			int width, height;
			std::vector<char> bgrFrame;
			const std::vector<std::vector<char>>& frames;
			size_t curFrame;
		};
		UserData ctx{ imgWidth, imgHeight,{}, frames, 0 };

		auto callback = [](int /*msg*/, unsigned long /*tag*/, int /*firstCall*/, void* ctx, int* bufferSize, void** buffer, int* isLast)
		{
			auto ud = static_cast<UserData*>(ctx);
			if (ud->frames.empty() || ud->curFrame >= ud->frames.size() || ud->frames[ud->curFrame].empty()) return MC_CANNOT_COMPLY;

			auto& rgbaFrame = ud->frames[ud->curFrame];
			auto& bgrFrame = ud->bgrFrame;
			ConvertRGBAToBGR(rgbaFrame.data(), ud->width, ud->height, bgrFrame);
			if (bgrFrame.size() & 0x1) bgrFrame.push_back(0); // add null byte in case of odd number (required by DICOM)

			(*bufferSize) = static_cast<int>(bgrFrame.size());
			(*buffer) = bgrFrame.data();

			ud->curFrame++;
			if (ud->frames.size() == ud->curFrame) (*isLast) = 1;
			return MC_NORMAL_COMPLETION;
		};

		auto status = MC_Set_Value_From_Function(msg.id(), MC_ATT_PIXEL_DATA, &ctx, callback);
		return status == MC_NORMAL_COMPLETION;
	}

	bool setFrameData(Merge::MessageBase& msg, const std::vector<std::vector<char>>& frames, int frameWidth, int frameHeight)
	{
		auto ts = msg.getTransferSyntax();
		switch (ts)
		{
		case EXPLICIT_LITTLE_ENDIAN:
		case IMPLICIT_LITTLE_ENDIAN:
		case EXPLICIT_BIG_ENDIAN: // big endian ordering of the binary data is handled by mergeCOM
		case IMPLICIT_BIG_ENDIAN:
			return setUncompressedFrameData(msg, frames, frameWidth, frameHeight);
		case JPEG_LOSSLESS_HIER_14:
			return setCompressedFrameData(msg, frames, frameWidth, frameHeight, true);
		case JPEG_BASELINE:
			return setCompressedFrameData(msg, frames, frameWidth, frameHeight, false);
		default:
			{
				assert(false);
				return false;
			}
		}
	}

	bool setFrameData(Merge::MessageBase& msg, const char* rgbaFrameData, int frameWidth, int frameHeight)
	{
		const int32_t bytesPerPixel = 4;
		std::vector<std::vector<char>> frames = { { rgbaFrameData, rgbaFrameData + (frameWidth * frameHeight * bytesPerPixel)} };
		return setFrameData(msg, frames, frameWidth, frameHeight);
	}

	std::string getDicomDate(const SYSTEMTIME& sysTime)
	{
		char buffer[16] = {};
		sprintf_s(buffer, "%04u%02u%02u", // YYYYMMDD
							sysTime.wYear, sysTime.wMonth, sysTime.wDay);
		return buffer;
	}

	std::string getDicomTime(const SYSTEMTIME& sysTime)
	{
		char buffer[16] = {};
		sprintf_s(buffer, "%02u%02u%02u", // hhmmss
							sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
		return buffer;
	}


	static const int HoursToMinutes = 60;

	std::string getDicomTimeZoneOffset(int timeZoneOffset)
	{
		int hours = timeZoneOffset / HoursToMinutes;
		int minutes = abs(timeZoneOffset % HoursToMinutes);
		char buffer[16] = {};
		sprintf_s(buffer, "%+03d%02d", hours, minutes);

		return buffer;
	}

	bool writeGeometry(Merge::MessageBase& msg, const Xray::XrayGeometry& /*geometry*/)
	{
		return addNNull(msg, MC_ATT_PATIENT_ORIENTATION);
	}

	std::wstring GetManufacturerModelName(RunType runType)
	{
		std::wstring manufacturerModelName = L"";

		if (runType == RunType::DCR)
		{
			manufacturerModelName = L"Dynamic Coronary Roadmap";
		}
		else if (runType == RunType::SBL)
		{
			manufacturerModelName = L"StentBoost Live";
		}
		
		return  manufacturerModelName;
	}
		

	

	std::wstring GetSoftwareVersion(RunType runType)
	{
		bool success = false;		
		std::wstring softwareVersion = L"";
		int major = 0, minor = 0,level = 0;

		if (runType == RunType::DCR)
		{			
			success = getProductVersion(DcrSoftwareVersion, major, minor, level);
		}
		else if(runType == RunType::SBL)
		{			
			success = getProductVersion(SblSoftwareVersion, major, minor, level);
		}

		if (success)
		{
			softwareVersion = getProductVersionLabel(major, minor, level);
		}
		return softwareVersion;
	}

	
		
	bool writeSCProductInfo(Merge::MessageBase& msg, RunType runType)
	{
		bool success = true;
		std::wstring manufacturerModelName = GetManufacturerModelName(runType);
		std::wstring softwareVersion = GetSoftwareVersion(runType);		
			
		success = success && addNSet(msg, MC_ATT_SOFTWARE_VERSIONS, softwareVersion);
		success = success && addNSet(msg, MC_ATT_MANUFACTURERS_MODEL_NAME, manufacturerModelName);

		char hostname[MAX_COMPUTERNAME_LENGTH + 1];
		if (gethostname(hostname, sizeof hostname) == 0)
		{
			success = success && addNSet(msg, MC_ATT_STATION_NAME, hostname);
		}
		
		return success;
	}

	

	bool writeSOPCommon(Merge::MessageBase& msg, const _SYSTEMTIME& instanceCreation, int timeZoneOffset, bool isMultiFrame, std::vector<std::string>& charSets)
	{
		bool success = true;

		char buffer[32] = {};
		sprintf_s(buffer, "%04u%02u%02u%02u%02u%02u%03u", // YYYYMMDDhhmmssSSS
						   instanceCreation.wYear, instanceCreation.wMonth, instanceCreation.wDay,
						   instanceCreation.wHour, instanceCreation.wMinute, instanceCreation.wSecond, instanceCreation.wMilliseconds);
		auto sopInstanceUid = UidUtils::getSOPInstanceUID(buffer, isMultiFrame);

		auto creationDate = getDicomDate(instanceCreation);
		auto creationTime = getDicomTime(instanceCreation);

		std::vector<std::string> imageType { "DERIVED", "SECONDARY" };

		success = success && addNSet(msg, MC_ATT_SPECIFIC_CHARACTER_SET, charSets);
		success = success && addNSet(msg, MC_ATT_IMAGE_TYPE, imageType);
		success = success && addNNull(msg, MC_ATT_SOP_CLASS_UID);
		success = success && addNSet(msg, MC_ATT_SOP_INSTANCE_UID, sopInstanceUid);

		// Affected SOP Instance UID, required by mergecom3 for sending to a PACS
		success = success && addNSet(msg, MC_ATT_AFFECTED_SOP_INSTANCE_UID, sopInstanceUid);

		success = success && addNSet(msg, MC_ATT_INSTANCE_CREATION_TIME, creationTime);
		success = success && addNSet(msg, MC_ATT_INSTANCE_CREATION_DATE, creationDate);

		// DICOM timezone offset is the offset from UTC where the windows timezone offset is offset from local time, thus the sign has to be inverted
		auto timeOffset = getDicomTimeZoneOffset(-timeZoneOffset);
		success = success && addNSet(msg, MC_ATT_TIMEZONE_OFFSET_FROM_UTC, timeOffset);
		success = success && addNSet(msg, MC_ATT_MANUFACTURER, Manufacturer);

		return success;
	}

	bool writeSCImageCommon(Merge::MessageBase& msg, int instanceNumber, int imgWidth, int imgHeight)
	{
		bool success = true;
		success = success && addNSet(msg, MC_ATT_IMAGE_NUMBER, instanceNumber);
		success = success && addNSet(msg, MC_ATT_SAMPLES_PER_PIXEL, SamplesPerPixel);
		success = success && addNSet(msg, MC_ATT_PHOTOMETRIC_INTERPRETATION, getPhotometricInterpretation(msg.getTransferSyntax()));
		success = success && addNSet(msg, MC_ATT_ROWS, imgHeight);
		success = success && addNSet(msg, MC_ATT_COLUMNS, imgWidth);
		success = success && addNSet(msg, MC_ATT_BITS_ALLOCATED, BitsAllocated);
		success = success && addNSet(msg, MC_ATT_BITS_STORED, BitsStored);
		success = success && addNSet(msg, MC_ATT_HIGH_BIT, HighBit);
		success = success && addNSet(msg, MC_ATT_PIXEL_REPRESENTATION, PixelRepresentation);
		success = success && addNSet(msg, MC_ATT_PLANAR_CONFIGURATION, PlanarConfiguration);
		success = success && addNSet(msg, MC_ATT_BURNED_IN_ANNOTATION, BurnedInAnnotation);
		return success;
	}

	bool writeSCImage(Merge::MessageBase& msg, int instanceNumber, const char* pixelData, int imgWidth, int imgHeight)
	{
		bool success = true;
		if (!pixelData || !imgWidth || !imgHeight) return false;

		success = success && writeSCImageCommon(msg, instanceNumber, imgWidth, imgHeight);
		success = success && setFrameData(msg, pixelData, imgWidth, imgHeight);
		success = success && addNSet(msg, MC_ATT_SOP_CLASS_UID, Constants::SopClass::SecondaryCaptureImageStorage);

		return success;
	}

	bool writeSCMFImageCommon(Merge::MessageBase& msg, uint32_t instanceNumber, int32_t imgWidth, int32_t imgHeight, const _SYSTEMTIME& startOfCapture)
	{
		bool success = true;
		success = success && writeSCImageCommon(msg, instanceNumber, imgWidth, imgHeight);
		success = success && addNSet(msg, MC_ATT_SOP_CLASS_UID, Constants::SopClass::MultiFrameSecondaryCaptureTrueColor);
	
		success = success && addNSet(msg, MC_ATT_CONTENT_DATE, getDicomDate(startOfCapture));
		success = success && addNSet(msg, MC_ATT_CONTENT_TIME, getDicomTime(startOfCapture));

		return success;
	}

	bool writeSCMFImage(Merge::MessageBase& msg, const std::vector<std::vector<char>>& frames, int imgWidth, int imgHeight)
	{
		bool success = true;
		success = success && setFrameData(msg, frames, imgWidth, imgHeight);
		success = success && addNSet(msg, MC_ATT_NUMBER_OF_FRAMES, frames.size());

		return success;
	}

	bool writeRequestedAttributeSequence(Merge::MessageBase& msg, const Xray::XrayStudy& study)
	{
		bool success = true;
		std::vector<Merge::Item> items;

		Merge::Item item(L"REQUESTED_ATTR_SEQUENCE");		

		if (!study.requestedProcedureId.empty())
		{
			success = success && item.set(MC_ATT_REQUESTED_PROCEDURE_ID, str_ext::wtos((study.requestedProcedureId)));
			if (success)
			{
				items.push_back(item);
				success = success && msg.set(MC_ATT_REQUEST_ATTRIBUTES_SEQUENCE, items);
			}			
		}	
					
		return success;
	}

	bool writeInstitutionName(Merge::MessageBase& msg, const Xray::XrayStudy& study)
	{
		bool success = true;
		// Institution name to be set to hospital name when institution name from source is empty 
		static const std::wstring configFolder = L"..\\Configuration\\";
		auto configFolderFullPath = Utilities::AbsolutePathFromExeDir(configFolder);	
		auto& log = CommonPlatform::Logger(CommonPlatform::LoggerType::Platform);

		if (GetFileAttributes((configFolderFullPath + L"Workstation.ini").c_str()) == INVALID_FILE_ATTRIBUTES)
		{
			log.DeveloperWarning(L"Workstation.ini does not exist, default settings will be used");
		}

		CommonPlatform::Settings workstation_ini(configFolderFullPath + L"Workstation.ini");
		std::wstring hospitalName = workstation_ini.get(L"General", L"HospitalName", L"Philips Medical Systems");

		if (!study.institutionName.empty())
		{
			success = success && addNSet(msg, MC_ATT_INSTITUTION_NAME, study.institutionName);
		}
		else
		{			
			success = success && addNSet(msg, MC_ATT_INSTITUTION_NAME, hospitalName);
		}

		return success;
	}

	bool writeStudy(Merge::MessageBase & msg, const Xray::XrayStudy& study, int seriesNumber)
	{
		bool success = true;

		//Patient's Name (0010,0010) 2 Patient's full name.
		success = success && addNSet(msg, MC_ATT_PATIENTS_NAME, study.patientName);

		//Patient ID (0010,0020) 2 Primary hospital identification number or code for the patient.
		success = success && addNSet(msg, MC_ATT_PATIENT_ID, study.patientId);

		//Patient's Birth Date (0010,0030) 2 Birth date of the patient.
		if (std::all_of(study.patientBirthdate.begin(), study.patientBirthdate.end(), [](wchar_t c) { return c == '0'; }))
		{
			success = success && addNNull(msg, MC_ATT_PATIENTS_BIRTH_DATE);
		}
		else
		{
			success = success && addNSet(msg, MC_ATT_PATIENTS_BIRTH_DATE, study.patientBirthdate);
		}
		
		//Patient's Sex (0010,0040) 2 Sex of the named patient.
		success = success && addNSet(msg, MC_ATT_PATIENTS_SEX, study.patientGender);

		// general study
		success = success && addNSet(msg, MC_ATT_STUDY_ID, study.studyId);
		success = success && addNSet(msg, MC_ATT_STUDY_INSTANCE_UID, study.studyInstanceUid);
		success = success && addNSet(msg, MC_ATT_STUDY_DATE, study.studyDate);
		success = success && addNSet(msg, MC_ATT_STUDY_TIME, study.studyTime);
		success = success && addNSet(msg, MC_ATT_ACCESSION_NUMBER, study.accessionNumber);
		success = success && addNSet(msg, MC_ATT_REFERRING_PHYSICIANS_NAME, study.referringPhysician);					
		success = success && addNSet(msg, MC_ATT_SERIES_NUMBER, seriesNumber);
		
		// general series
		success = success && addNSet(msg, MC_ATT_SERIES_INSTANCE_UID, UidUtils::getSeriesInstanceUID(study, seriesNumber));
		success = success && addNSet(msg, MC_ATT_MODALITY, Modality);

		// sc equipment
		success = success && addNSet(msg, MC_ATT_CONVERSION_TYPE, ConversionType);

		if(!study.otherPatientIds.empty())
			success = success && addNSet(msg, MC_ATT_OTHER_PATIENT_IDS_RETIRED, study.otherPatientIds);

		//InstitutionName
		success = success && writeInstitutionName(msg, study);
		
		//requested attribute sequence 
		success = success && writeRequestedAttributeSequence(msg, study);	
		
		return success;
	}

	bool writeStorageCommitRequest(Merge::MessageBase& msg, const std::vector<std::pair<std::string, std::string>>& sops)
	{
		bool success = true;

		char curTime[16] = {};
		char curDate[16] = {};

		SYSTEMTIME sysTime = {};
		GetSystemTime(&sysTime);

		success = success && sprintf_s(curDate, "%04u%02u%02u", sysTime.wYear, sysTime.wMonth, sysTime.wDay) != 0;
		success = success && sprintf_s(curTime, "%02u%02u%02u", sysTime.wHour, sysTime.wMinute, sysTime.wSecond) != 0;

		std::vector<Merge::Item> items;
		items.reserve(sops.size());
		for (const auto& sop : sops)
		{
			Merge::Item item(L"REF_SOP_MEDIA");

			success = success && item.set(MC_ATT_REFERENCED_SOP_CLASS_UID, sop.first);
			success = success && item.set(MC_ATT_REFERENCED_SOP_INSTANCE_UID, sop.second);
			if (success)
			{
				items.push_back(item);
			}
			else
			{
				return success;
			}
		}

		success = success && msg.set(MC_ATT_REFERENCED_SOP_SEQUENCE, items);

		success = success && msg.set(MC_ATT_TRANSACTION_UID, UidUtils::getTransactionUID(curDate, curTime, static_cast<uint32_t>(items.size())));
		success = success && msg.set(MC_ATT_REQUESTED_SOP_INSTANCE_UID, Constants::SopInstance::StorageCommitPush);
		success = success && msg.set(MC_ATT_ACTION_TYPE_ID, 1);

		return success;
	}
}}
//TICS +CON#007	