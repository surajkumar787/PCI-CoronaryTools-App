// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <Sense.h>
#include "XraySinkFxd.h"
#include "CrmRoadmap.h"
#include "StbMarkers.h"
#include "StbImage.h"
#include "Thread.h"


namespace CommonPlatform {namespace Xray
{
	struct XrayImage;
}}

namespace Pci { namespace Core {

using namespace Sense;

class PciThreadGrabber
{
public:
	PciThreadGrabber(IScheduler &scheduler, const std::wstring &folder, int maxSize, bool async, bool splitOutDir = true);
	~PciThreadGrabber();
	
	void addXray	(const std::shared_ptr<const CommonPlatform::Xray::XrayImage>  &image);
	void addBoost	(const std::shared_ptr<const StbImage>   &boost);
	void addRoadmap	(const std::shared_ptr<const CrmRoadmap> &newRoadmap);
	void addMarkers	(const std::shared_ptr<const CommonPlatform::Xray::XrayImage>  &image, const std::shared_ptr<const StbMarkers> &markers);

	void setRoadmap (const std::shared_ptr<const CrmRoadmap> &roadmap);
	void setStudy	(const CommonPlatform::Xray::XrayStudy &study);
	
	void endLive();
	void endMarkers();

	bool diskSpaceAvailable()	const;
	bool diskExists()			const;

	PciThreadGrabber(const PciThreadGrabber&) = delete;
	PciThreadGrabber& operator=(const PciThreadGrabber&) = delete;

protected:

	void flushLive();
	void flushMarkers();
	
	void write(const std::shared_ptr<const StbImage>   &boost);
	void write(const std::shared_ptr<const CommonPlatform::Xray::XrayImage>  &image);
	void write(const std::shared_ptr<const CrmRoadmap> &roadmap);
	void write(const std::shared_ptr<const CommonPlatform::Xray::XrayImage>  &image, const std::shared_ptr<const StbMarkers> &markers);
	
	enum class Type
	{
		Default, 
		RoadmapAngio,
		RoadmapPreview,
		RoadmapFluoro,
		StentBoostAngio,
		StentBoostPreview,
		StentBoostMarkers
	};
	
	std::wstring getFileName(Type type, int index);

private:
	std::unique_ptr<CommonPlatform::Thread>	  thread;
	CommonPlatform::Xray::XrayStudy		      study;
	std::unique_ptr<XraySinkFxd>		      sink;
	std::wstring						      folder;
	bool								      enabled;
	bool								      reset;
	volatile long						      count;
	bool								      splitOutDir;

	__int64							          maxSize;
	__int64							          currentSize;

	std::shared_ptr<const StbImage> currentBoost;
	std::shared_ptr<const CrmRoadmap>	     currentRoadmap;

	int									      markerRun;
	int									      markerFrame;
	std::vector<StbMarkers>	      markerList;

	std::wstring						      studyName;

};

}}