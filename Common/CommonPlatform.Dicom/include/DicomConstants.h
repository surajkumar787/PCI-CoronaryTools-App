// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <string>
#include <mc3msg.h>

namespace CommonPlatform { namespace Dicom { namespace Constants
{
	namespace SopClass
	{
		const std::string SecondaryCaptureImageStorage = "1.2.840.10008.5.1.4.1.1.7";
		const std::string MultiFrameSecondaryCaptureTrueColor = "1.2.840.10008.5.1.4.1.1.7.4";
		const std::string Verification = "1.2.840.10008.1.1";
		const std::string StorageCommitPush = "1.2.840.10008.1.20.1";
	}

	namespace SopInstance
	{
		const std::string StorageCommitPush = "1.2.840.10008.1.20.1.1";
	}

	namespace ServiceList
	{
		const std::string StorageCommitScu = "Storage_Commit_SCU_Service_List";
	}

	const std::string PMS_DICOM_ROOT = "1.3.46.670589"; // Philips
	const std::string PMS_PRODUCT_ID = "7.9";           // IGT + PCI

	const std::string  ISO_IR_100 = "ISO_IR 100";
	const std::string  ISO_IR_6 = "ISO_IR 6";
	const std::string  ISO_IR_13 = "ISO_IR 13";
	const std::string  ISO_IR_87 = "ISO_IR 87";
	const std::string  ISO_IR_159 = "ISO_IR 159";
	const std::string  ISO_IR_192 = "ISO_IR 192";
	const std::string  GB18030 = "GB18030";
	const std::string  ISO_IR_6CE = "ISO 2022 IR 6";
	const std::string  ISO_IR_13CE = "ISO 2022 IR 13";
	const std::string  ISO_IR_87CE = "ISO 2022 IR 87";
	const std::string  ISO_IR_159CE = "ISO 2022 IR 159";

	const TRANSFER_SYNTAX BasicSyntaxList[] =
	{
		EXPLICIT_LITTLE_ENDIAN,
		IMPLICIT_LITTLE_ENDIAN,
		EXPLICIT_BIG_ENDIAN,
	};

	const TRANSFER_SYNTAX SnapshotSyntaxList[] =
	{
		JPEG_LOSSLESS_HIER_14,
		EXPLICIT_LITTLE_ENDIAN,
		IMPLICIT_LITTLE_ENDIAN,
		EXPLICIT_BIG_ENDIAN,
	};

	const TRANSFER_SYNTAX MovieSyntaxList[] =
	{
		JPEG_BASELINE,
		JPEG_LOSSLESS_HIER_14,
		EXPLICIT_LITTLE_ENDIAN,
		IMPLICIT_LITTLE_ENDIAN,
		EXPLICIT_BIG_ENDIAN
	};

}}}