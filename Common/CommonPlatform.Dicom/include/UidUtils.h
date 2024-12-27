// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <string>
#include <stdint.h>

namespace CommonPlatform { namespace Xray
{
	struct XrayStudy;
}}

namespace CommonPlatform { namespace Dicom { namespace UidUtils
{
	std::string getSopInstanceUIDBase();
	std::string getStudyInstanceUID(const CommonPlatform::Xray::XrayStudy& study);
	std::string getSeriesInstanceUID(const CommonPlatform::Xray::XrayStudy& study, int seriesNumber);
	std::string getSOPInstanceUID(const std::string& instanceCreationDateTime, bool isMultiFrame);

	std::string getTransactionUID(const std::string& date, const std::string& time, uint32_t nrOfItems);
}}}