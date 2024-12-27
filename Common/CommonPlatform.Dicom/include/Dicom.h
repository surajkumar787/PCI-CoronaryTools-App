// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <vector>
#include <string>
#include <stdint.h>
#include "Build.hpp"
#include "ApplicationType.h"

using namespace Pci::Core::PACS;
struct _SYSTEMTIME;

namespace CommonPlatform { namespace Xray
{
	struct XrayStudy;
	struct XrayGeometry;
}}

namespace CommonPlatform { namespace Dicom
{

	namespace Merge
	{
		class MessageBase;
		class Service;
	}

	/*
	 * Adds a XrayGeometry to the given message.
	 * 
	 * @param msg				The message to which the image is added
	 * @param geometry			The current geometry of the Xray device
	 * 
	 * @return true when the geometry is added, false otherwise
	 */
	bool writeGeometry(Merge::MessageBase& msg, const CommonPlatform::Xray::XrayGeometry& geometry);

	/*
	 * Add the common SOP specific information to a Dicom message
	 * 
	 * @param msg              The message to which the attributes are added
	 * @param instanceCreation The time at which the instance was created
	 * @param timeZoneOffset   The current offset (in minutes) from UTC w.r.t. the local time
	 * @param isMultiFrame     Indicates if the message will be used for a single frame (snapshot) or multi frame (movie)
	 * 
	 * @return true when all attributes were set, false otherwise
	 */
	bool writeSOPCommon(Merge::MessageBase& msg, const _SYSTEMTIME& instanceCreation, int timeZoneOffset, bool isMultiFrame, std::vector<std::string>& charSets);


/*
 * Add the Product information to a Dicom message
 *
 * @param msg              The message to which the attributes are added
 * @param Runtype          The type of Run acquired DCR or SBL *
 * @return true when all attributes were set, false otherwise
 */
	bool writeSCProductInfo(Merge::MessageBase& msg, RunType runType);

	/*
	 * Adds the patient, study and series information to a Dicom message
	 * 
	 * @param msg			The message to which the attributes are added
	 * @param study			The study used to set the required attributes
	 * @param seriesNumber	The series used to set the series attribute
	 * 
	 * @return true when all attributes were set, false otherwise
	 */
	bool writeStudy(Merge::MessageBase& msg, const CommonPlatform::Xray::XrayStudy& study, int seriesNumber);

	/*
	 * Adds Institution Name details to Dicom message
	 *
	 * @param msg			The message to which the attributes are added
	 * @param study			The study used to set the required attributes 	
	 * @return true when all attributes were set, false otherwise
	 */
	bool writeInstitutionName(Merge::MessageBase& msg, const CommonPlatform::Xray::XrayStudy& study);

	/*
	 * Adds a Secondary Capture image (and accompanying attributes) to the given message.
	 * The image data will be encoded based on the transfer syntax of the message.
	 * This also sets the SOP Class UID of the message to Secondary Capture Image
	 * 
	 * @param msg				The message to which the image is added
	 * @param instanceNumber	The image/instance number for this capture
	 * @param pixelData			A pointer to the RGBA32 pixel data to added
	 * @param imgWidth			The width of the image in pixels
	 * @param imgHeight			The height of the image in pixels
	 * 
	 * @return true when the image is added, false otherwise
	 */
	bool writeSCImage(Merge::MessageBase& msg, int instanceNumber, const char* pixelData, int imgWidth, int imgHeight);

	/*
	 * Adds the common image metadata for a Multi Frame Secondary Capture image to the given message
	 * This also sets the SOP Class UID of the message to Multi Frame Secondary Capture True Color
	 * 
	 * @param msg				The message to which the image is added
	 * @param instanceNumber	The image/instance number for this capture
	 * @param imgWidth			The width of the images in pixels
	 * @param imgHeight			The height of the images in pixels
	 * @param startOfCapture	The date/time on which the first image is captured
	 * 
	 * @return true when the metadata is added, false otherwise
	 */
	bool writeSCMFImageCommon(Merge::MessageBase& msg, uint32_t instanceNumber, int32_t imgWidth, int32_t imgHeight, const _SYSTEMTIME& startOfCapture);

	bool writeSCMFImage(Merge::MessageBase& msg, const std::vector<std::vector<char>>& frames, int32_t imgWidth, int32_t imgHeight);

	/*
	 * Adds storage commit request info to the given message.
	 * This includes the transaction UID
	 *
	 * @param msg		The message to which the information is added
	 * @param messages	A pair of the SOP Class and SOP Instance UID of each message for which a storage request must be made
	 *
	 * @return true when the info is added, false otherwise
	 */
	bool writeStorageCommitRequest(Merge::MessageBase& msg, const std::vector<std::pair<std::string, std::string>>& sops);
}}
