// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
//
// Filename: CrmAlgoFluoro.h
#pragma once

#include "XrayImage.h"
#include "CrmOverlay.h"
#include "CrmRoadmap.h"
#include "CrmParameters.h"
#include "CrmErrors.h"
#include "CrmThreadPool.h"
#include <IpSimplicity.h>
#include <Osc.h>
#include <Log.h>

namespace CommonPlatform { namespace Xray { enum class SystemType; } }
namespace Pci { namespace Core {



class CrmAlgoFluoro
{
public:
	CrmAlgoFluoro(Simplicity::ThreadPool &pool, const CrmParameters &params, CommonPlatform::Xray::SystemType systemType);
	CrmAlgoFluoro(CommonPlatform::Log &log, Simplicity::ThreadPool &pool, const CrmParameters &params, SystemType systemType);
	virtual ~CrmAlgoFluoro();

	bool process(const std::shared_ptr<const CommonPlatform::Xray::XrayImage> &src, const std::shared_ptr<CrmOverlay> &dst, const std::shared_ptr<CommonPlatform::Xray::XrayImage> &live,bool skip);
	bool end();

	void set(const std::shared_ptr<const CrmRoadmap> &roadmap);

	CrmAlgoFluoro(const CrmAlgoFluoro&) = delete;
	CrmAlgoFluoro& operator=(const CrmAlgoFluoro&) = delete;
protected:
	
	void exit();

	virtual Matrix getTransformationMatrix(const std::shared_ptr<const CommonPlatform::Xray::XrayImage> & src);
	float getFramesPerSecond(const std::shared_ptr<const CommonPlatform::Xray::XrayImage> & src)const;
	float getMmPerPixelInIsoCenter(const std::shared_ptr<const CommonPlatform::Xray::XrayImage> & src)const;
	void correctShuttersRoi(const std::shared_ptr<const CommonPlatform::Xray::XrayImage> & src, Matrix &trans, COscRoiCoord &roi);
	void applyLutsToLive(const std::shared_ptr<const CommonPlatform::Xray::XrayImage> & src, const std::shared_ptr<CommonPlatform::Xray::XrayImage> & live);

	std::vector<short> buffer;
	CrmThreadPool threadPool;
	void  *handle;
	int	   count;
	PciSuite::CrmErrors::Error			error;
	Simplicity::NodeLut					nodeLut;
	Simplicity::NodeBasic				basic;
	std::shared_ptr<const CrmRoadmap>	roadmap;
	const CrmParameters					&params;
	const short transformationBorderValue;

	Simplicity::NodeAffineTransform	transform;
	const SystemType					_systemType;

private:
    ::CommonPlatform::Log&      m_log;
};


}
}