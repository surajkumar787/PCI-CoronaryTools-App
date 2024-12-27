// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <string>
#include "IUserPrefs.h"
#include "ArchivingSettings.h"

namespace Pci {	namespace Core {


class UserPrefs : public IUserPrefs
{
public:
	UserPrefs(CommonPlatform::Settings&);

	virtual	~UserPrefs( ) = default;

	virtual const IArchivingSettings& getArchivingSettings() const override;
	virtual bool getDefaultShowInvertedStent() const override;
	virtual bool getApcAutoActivation() const override;
private:
	ArchivingSettings	m_archivingSettings;
	bool m_defaultShowInvertedStent;
	bool m_apcAutoActivation;

};

}}
