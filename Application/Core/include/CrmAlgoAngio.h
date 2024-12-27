// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
//
// Filename: CrmAlgoAngio.h
#pragma once

#include "XrayImage.h"
#include "CrmRoadmap.h"
#include "CrmErrors.h"
#include "CrmParameters.h"
#include "CrmThreadPool.h"
#include <IpSimplicity.h>
#include <Log.h>

using namespace PciSuite;

namespace Pci { namespace Core {

class CrmAlgoAngio
{
public:
	CrmAlgoAngio( CommonPlatform::Log & log, Simplicity::ThreadPool &pool, const CrmParameters &params);
	CrmAlgoAngio(Simplicity::ThreadPool &pool, const CrmParameters &params);
	~CrmAlgoAngio();

	bool process(const std::shared_ptr<const XrayImage> &src);
	CrmErrors::Error end(const std::shared_ptr<CrmRoadmap> &dst);

	CrmAlgoAngio(const CrmAlgoAngio&) = delete;
	CrmAlgoAngio& operator=(const CrmAlgoAngio&) = delete;

protected:
	void init(const XrayImage &src);
	void exit();

	short					*buffer;
	void					*handle;
	CrmThreadPool			threadPool;
	int						count;
		  
	Rect					overlayShutters;
	Matrix					overlayMatrix;

	bool					tableOrLArmMoved;
	bool					cArmMoved;
	CrmErrors::Error		error;
	CrmRoadmap				roadmap;
	
	const CrmParameters &params;


private:
	int CrmAlgoAngio::GetInternalSubsamplingFactor(const XrayImage &src) const;
	float getMmPerPixelInIsoCenter(const std::shared_ptr<const XrayImage> & src) const;
    CommonPlatform::Log&      m_log;
};


}
}