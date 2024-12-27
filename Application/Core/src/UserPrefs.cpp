// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.


#include "UserPrefs.h"
#include "Settings.h"

namespace Pci {	namespace Core {

Core::UserPrefs::UserPrefs(CommonPlatform::Settings &settings):
	m_archivingSettings(settings),
	m_defaultShowInvertedStent(false),
	m_apcAutoActivation()
{
	m_defaultShowInvertedStent = settings.get(L"StentBoost", L"ShowStentInverted", false);
	m_apcAutoActivation = settings.get(L"Xray", L"Automatic3DAPC", true);
}

const IArchivingSettings & Core::UserPrefs::getArchivingSettings() const
{
	return m_archivingSettings;
}
bool Core::UserPrefs::getDefaultShowInvertedStent() const
{
	return m_defaultShowInvertedStent;
}
bool Core::UserPrefs::getApcAutoActivation() const
{
	return m_apcAutoActivation;
}

}}