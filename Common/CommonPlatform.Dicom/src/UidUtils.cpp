// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "UidUtils.h"

#include "XrayStudy.h"
#include "DicomConstants.h"
#include "Dicom.h"

#include <assert.h>
#include <sstream>
#include <Windows.h>
#include <stdint.h>
#include <memory>
#include <algorithm>

#include <comdef.h>
//#import "progid:WbemScripting.SWbemLocator" named_guids
#import "C:\windows\system32\wbem\wbemdisp.tlb" named_guids

namespace CommonPlatform { namespace Dicom { namespace UidUtils
{
	std::string getMAC()
	{
		static std::string result;
		if (result.empty())
		{
			CoInitialize(NULL);
			try
			{
				// Use WMI to locate the Hospital Network adapter and retrieve its corresponding MAC address
				WbemScripting::ISWbemLocatorPtr locator;
				locator.CreateInstance(WbemScripting::CLSID_SWbemLocator);
				if (locator)
				{
					WbemScripting::ISWbemServicesPtr services = locator->ConnectServer(".", "root\\cimv2", "", "", "", "", 0, nullptr);
					WbemScripting::ISWbemObjectSetPtr objects = services->ExecQuery("Select * from Win32_NetworkAdapter", "WQL", 0x10, nullptr);
					IEnumVARIANTPtr obj_enum = objects->Get_NewEnum();
					ULONG fetched = 0;
					VARIANT var = {};
					while (obj_enum->Next(1, &var, &fetched) == S_OK)
					{
						WbemScripting::ISWbemObjectPtr object = var;
						WbemScripting::ISWbemPropertySetPtr properties = object->Properties_;
						WbemScripting::ISWbemPropertyPtr prop = properties->Item("NetConnectionID", 0);
						_variant_t value = prop->GetValue();
						if (value.vt == VT_BSTR && _bstr_t(value.bstrVal) == _bstr_t("Hospital Network"))
						{
							prop = properties->Item("MACAddress", 0);
							std::string mac = _bstr_t(prop->GetValue());
							auto newEnd = std::remove_if(mac.begin(), mac.end(), [](const char& c) { return c == ':'; });
							mac.erase(newEnd, mac.end());
							std::stringstream ss;
							ss << mac << std::hex;
							uint64_t macInt = 0;
							ss >> macInt;
							result = std::to_string(macInt);
							break;
						}
					}
				}
			}
			catch (_com_error err)
			{
				assert(false);
				return result;
			}
			CoUninitialize();
			assert(!result.empty());
		}
		return result;
	}

	std::string CwisToDicomTime(const std::string& time)
	{
		size_t pos = time.find_first_of('.');
		if (pos != std::string::npos)
		{
			return time.substr(0, pos);
		}
		else if (time.size() > 6)
		{
			return time.substr(0, 6);
		}
		return time;
	}

	std::string getSopInstanceUIDBase()
	{
		std::stringstream uid;
		uid << Constants::PMS_DICOM_ROOT;
		uid << "." << Constants::PMS_PRODUCT_ID;
		uid << "." << getMAC();
		return uid.str();
	}

	std::string getStudyInstanceUID(const CommonPlatform::Xray::XrayStudy& study)
	{
		std::stringstream uid;
		uid << getSopInstanceUIDBase();
		uid << ".";
		uid << study.studyDate;
		uid << CwisToDicomTime(study.studyTime);
		auto uidString = uid.str();
		assert(uidString.size() < 64); //63 char + null
		return uidString;
	}

	std::string getSeriesInstanceUID(const CommonPlatform::Xray::XrayStudy& study, int seriesNumber)
	{
		std::stringstream uid;
		uid << getStudyInstanceUID(study);
		uid << "." << seriesNumber;
		auto uidString = uid.str();
		assert(uidString.size() < 64); //63 char + null
		return uidString;
	}

	std::string getSOPInstanceUID(const std::string& instanceCreationDateTime, bool isMultiFrame)
	{
		std::stringstream uid;
		uid << getSopInstanceUIDBase();
		uid << "." << instanceCreationDateTime;
		uid << ".1" << isMultiFrame;
		auto uidString = uid.str();
		assert(uidString.size() < 64); //63 char + null
		return uidString;
	}

	std::string getTransactionUID(const std::string& date, const std::string& time, uint32_t nrOfItems)
	{
		std::stringstream uid;
		uid << getSopInstanceUIDBase();
		uid << "." << date << "." << time << "." << nrOfItems;
		auto uidString = uid.str();
		assert(uidString.size() < 64); //63 char + null
		return uidString;
	}

}}}