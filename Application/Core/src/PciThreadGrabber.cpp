// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#include "PciThreadGrabber.h"
#include "PciEpx.h"
#include "PathUtil.h"
#include <Shlwapi.h>


namespace Pci { namespace Core{
using namespace PciSuite;
using namespace CommonPlatform;

static const int MaximumMarkerListSize = 2000;
static const ULONGLONG  MinimumFreeDiskSpace = 5368709120; // (5 * 1024 * 1024 * 1024);

PciThreadGrabber::PciThreadGrabber(IScheduler &scheduler, const std::wstring &Folder, int maxSize, bool async, bool _splitOutDir)
:
	thread		(async && !folder.empty() ? std::make_unique<Thread>(scheduler) : nullptr),
	study		(),
	sink		(nullptr),
	folder		(Utilities::AbsolutePathFromExeDir(Folder)),
	enabled		(!Folder.empty()),
	reset		(true),
	count		(0),
	splitOutDir (_splitOutDir),
	maxSize		( static_cast<__int64>(maxSize * 1024 * 1024)),
	currentSize	(0),
	markerRun	(0),
	markerFrame	(0),
	markerList	(),
	studyName	()
{
}

PciThreadGrabber::~PciThreadGrabber(void)
{
	thread = nullptr;
}


void PciThreadGrabber::setStudy(const XrayStudy &Study)
{
	if (!enabled) return;

	SYSTEMTIME time;
	GetLocalTime(&time);

	wchar_t name[1024];
	swprintf_s(name, L"case_%.2d-%.2d-%.2d_%.2d-%.2d-%.2d_%s", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, study.studyId.c_str());			

	std::wstring caseName = name;

	if (thread != nullptr)
	{
		thread->executeAsync([this, Study, caseName] {	flushLive(); study = Study;	currentRoadmap = nullptr;	studyName = caseName; });
	}
	else
	{
		flushLive(); study = Study;	currentRoadmap = nullptr;	studyName = caseName; 
	};
}

static bool equal(const std::shared_ptr<const CrmRoadmap> &a, const std::shared_ptr<const CrmRoadmap> &b)
{
	if ( a == nullptr && b == nullptr)					return true;
	if ( a != nullptr && b == nullptr)					return false;
	if ( a == nullptr &&  b != nullptr)					return false;
	if (a->runIndex == b->runIndex) return true;
									return false;
}

void PciThreadGrabber::setRoadmap(const std::shared_ptr<const CrmRoadmap> &Roadmap)
{
	if (!enabled) return;

	if (thread != nullptr)
	{
		thread->executeAsync([this, Roadmap] {
								            	if (equal(Roadmap, currentRoadmap)) return;   
								            	flushLive();   
												currentRoadmap = Roadmap; 
											});
	}
	else 
	{
		if (equal(Roadmap, currentRoadmap)) return;   
		flushLive();   
		currentRoadmap = Roadmap;  
	};
}


void PciThreadGrabber::addXray(const std::shared_ptr<const XrayImage> &image)
{
	if (!enabled)								return;
	if (!image)									return;
	//if (image->epx.type == XrayEpx::Type::None)	return;
	if (count > 100)							return;		//don't buffer too many images...

	InterlockedIncrement(&count);

	if (thread != nullptr)
	{
		thread->executeAsync([this, image] {	write(image); 
											    InterlockedDecrement(&count);	
										   });
	}
	else
	{
		write(image); 
		InterlockedDecrement(&count);	
	}
}


void PciThreadGrabber::addMarkers(const std::shared_ptr<const XrayImage> &image, const std::shared_ptr<const StbMarkers> &markers)
{
	if (!enabled) return;
	if (!markers) return;
	if (!image)   return;

	if (thread != nullptr)
	{
		thread->executeAsync([this, image, markers] {	if (markerList.size() < MaximumMarkerListSize)
														{
															write(image, markers);
														}
														});
	}
	else 	
	{
		if ( static_cast<int>( markerList.size()) < MaximumMarkerListSize)
		{
			write(image, markers);
		}
	}
}	



void PciThreadGrabber::addRoadmap(const std::shared_ptr<const CrmRoadmap> &newRoadmap)
{
	if (!enabled) return;
	if (!newRoadmap) return;

	if (thread != nullptr)
	{
		thread->executeAsync([this, newRoadmap] {	write(newRoadmap);		});
	}
	else 
	{
		write(newRoadmap);
	}
}

void PciThreadGrabber::addBoost(const std::shared_ptr<const StbImage> &Boost)
{
	if (!enabled) return;
	if ( Boost == nullptr)   return;

	currentBoost = Boost;
}

void PciThreadGrabber::endLive()
{
	if (!enabled) return;
	
	if (thread != nullptr)
	{
		thread->executeAsync([this] { flushLive(); });
	}
	else
	{
		flushLive();
	};

	if (currentBoost != nullptr)
	{
		std::shared_ptr<const StbImage> image = currentBoost;

		if (thread != nullptr)
		{
			thread->executeAsync([this, image] { write(image); });
		}
		else
		{
			write(image);	
		}
	
		currentBoost = nullptr;
	}
}

void PciThreadGrabber::endMarkers()
{
	if (!enabled) return;
	
	if (thread != nullptr)
	{
		thread->executeAsync([this] { flushMarkers(); });
	}
	else
	{
		flushMarkers();
	};
}


void PciThreadGrabber::write(const std::shared_ptr<const XrayImage>  &image, const std::shared_ptr<const StbMarkers> &markers)
{
	if (image->imageIndex < markerFrame)						 return;
	if (image->imageIndex > markerFrame + MaximumMarkerListSize) return;

	//add empty markers for frames that were skipped by the live thread
	StbMarkers skipped;
	for (int i = markerFrame; i < image->imageIndex - 1; i++)
	{
		markerList.push_back(skipped);
	}

	markerList.push_back(*markers);
	markerFrame = image->imageIndex;
	markerRun   = image->runIndex;
}


void PciThreadGrabber::write(const std::shared_ptr<const XrayImage> &image)
{
	if (reset)
	{
		sink = nullptr;
		reset = false;
		currentSize = 0;

		Type type = Type::Default;

		PciEpx epx(image->epx);

		if (epx.getType() == PciEpx::Type::StentBoost && image->type == XrayImage::Type::Exposure)	type = Type::StentBoostAngio;
		if (epx.getType() == PciEpx::Type::StentBoost && image->type == XrayImage::Type::Fluoro)	type = Type::RoadmapFluoro;
		if (epx.getType() == PciEpx::Type::Roadmap    && image->type == XrayImage::Type::Exposure)	type = Type::RoadmapAngio;
		if (epx.getType() == PciEpx::Type::Roadmap    && image->type == XrayImage::Type::Fluoro)	type = Type::RoadmapFluoro;

		std::wstring dst = getFileName(type, image->runIndex);
		if (!dst.empty())
		{
			sink = std::make_unique<XraySinkFxd>(dst);
		}
	}

	if ((image != nullptr) && (sink != nullptr ) && ((maxSize == 0) || (currentSize < maxSize)))
	{
		sink->addImage(image);
		currentSize += static_cast<__int64>(image->width * image->height * sizeof(short)); 
	}

}


void PciThreadGrabber::write(const std::shared_ptr<const CrmRoadmap> &roadmap)
{
	std::wstring dst = getFileName(Type::RoadmapPreview, roadmap->runIndex);
	
	if (!dst.empty())
	{
		FILE *f = nullptr;

		if (_wfopen_s(&f, dst.c_str(), L"wb") == 0)
		{
			roadmap->save(f);
			fclose(f);
		}
	}
}

void PciThreadGrabber::write(const std::shared_ptr<const StbImage> &boost)
{
	std::wstring dst = getFileName(Type::StentBoostPreview, boost->runIndex);
	
	if (!dst.empty())
	{
		XraySinkFxd fxd(dst);
		fxd.addImage(boost);
	}
}


bool PciThreadGrabber::diskExists() const
{
	return GetFileAttributes(folder.c_str()) == FILE_ATTRIBUTE_DIRECTORY;
}


bool PciThreadGrabber::diskSpaceAvailable()	const
{
	ULARGE_INTEGER free;
	
	if (!diskExists())													return false;
	if (!GetDiskFreeSpaceEx(folder.c_str(), &free, nullptr, nullptr))	return false;
	if (free.QuadPart < MinimumFreeDiskSpace)						    return false;

	return true;
}


std::wstring PciThreadGrabber::getFileName(Type type, int index)
{
	if (!diskSpaceAvailable()) return L"";

	wchar_t file[MAX_PATH];
	wchar_t dir [MAX_PATH];
	
	//double-check that we have a valid roadmap...
	if ((type == Type::RoadmapFluoro) && currentRoadmap == nullptr)
	{
		type = Type::Default;
	}

	switch(type)
	{
		case Type::Default:			   swprintf_s(dir, L"%s",			studyName.c_str());						swprintf_s(file, L"%s_run%.3d.fxd",						study.studyId.c_str(), index);						break;
		case Type::RoadmapAngio:	   swprintf_s(dir, L"%s_crm%.3d",   studyName.c_str(), index);				swprintf_s(file, L"%s_run%.3d_crm%.3d_angio.fxd",		study.studyId.c_str(), index, index);				break;
		case Type::RoadmapPreview:	   swprintf_s(dir, L"%s_crm%.3d",   studyName.c_str(), index);				swprintf_s(file, L"%s_run%.3d_crm%.3d_preview.fxd",		study.studyId.c_str(), index, index);				break;
		case Type::RoadmapFluoro:	   swprintf_s(dir, L"%s_crm%.3d",   studyName.c_str(), currentRoadmap->runIndex);	swprintf_s(file, L"%s_run%.3d_crm%.3d_fluoro.fxd",		study.studyId.c_str(), index, currentRoadmap->runIndex);	break;
		case Type::StentBoostAngio:	   swprintf_s(dir, L"%s_stb",		studyName.c_str());						swprintf_s(file, L"%s_run%.3d_stb_angio.fxd",			study.studyId.c_str(), index);						break;
		case Type::StentBoostPreview:  swprintf_s(dir, L"%s_stb",		studyName.c_str());						swprintf_s(file, L"%s_run%.3d_stb_preview.fxd",			study.studyId.c_str(), index);						break;
		case Type::StentBoostMarkers:  swprintf_s(dir, L"%s_stb",		studyName.c_str());						swprintf_s(file, L"%s_run%.3d_stb_angio.mkx",			study.studyId.c_str(), index);						break;
		default: return L"";
	};
	
	wchar_t path[MAX_PATH];
	wcscpy_s(path, folder.c_str());

	PathAppend(path, studyName.c_str());
	CreateDirectory(path, nullptr);
	if (splitOutDir) {
		PathAppend(path, dir);
		CreateDirectory(path, nullptr);
	}
	if (!PathIsDirectory(path)) return L"";
	
	PathAppend(path, file);
	return path;
}


void PciThreadGrabber::flushLive()
{
	if (sink != nullptr)
	{
		sink = nullptr;
	}

	reset = true;
	currentSize = 0;
}

void PciThreadGrabber::flushMarkers()
{
	//we write the markers all at once; this minimizes hard drive seek times while writing the image data...
	if (!markerList.empty() && markerRun != 0)
	{
		std::wstring dst = getFileName(Type::StentBoostMarkers, markerRun);

		if (!dst.empty())
		{
			FILE *f = nullptr;

			if (_wfopen_s(&f, dst.c_str(), L"wb") == 0)
			{
				for (auto &i:markerList) if (!i.save(f)) break;
				fclose(f);
			}
		}
	}
	
	markerList.clear();
	markerRun   = 0;
	markerFrame = 0;  
}
}}