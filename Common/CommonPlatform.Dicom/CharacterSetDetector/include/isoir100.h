// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include "IDetector.h"
#include "unicodeblock.h"
#include <vector>
#include <memory>

namespace CommonPlatform {	namespace Dicom
{

class ISOIR100 : public IDetector
{
	public:
		ISOIR100();
		virtual ~ISOIR100();
		virtual bool matchCharacterSet(const std::wstring& c) override;


	private:
		ISOIR100(const ISOIR100&) = delete;
		ISOIR100& operator= (const ISOIR100&) = delete;

	private:
		std::vector<std::unique_ptr<UnicodeBlock>> m_isoir100Block;

};

}}