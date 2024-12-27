// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include "IDetector.h"
#include "unicodeblock.h"
#include <memory>

namespace CommonPlatform {	namespace Dicom{

class ISOIR6 : public IDetector
{
	public:
		ISOIR6();
		virtual ~ISOIR6();
		virtual bool matchCharacterSet(const std::wstring& str) override;


	private:
		ISOIR6(const ISOIR6&) = delete;
		ISOIR6& operator= (const ISOIR6&) = delete;

	private:
		std::unique_ptr<UnicodeBlock>  m_isoir6Block;

};

}}

