// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "isoir100.h"
#include "charsetconstants.h"

namespace CommonPlatform {	namespace Dicom
{
	ISOIR100::ISOIR100()
	{		
		m_isoir100Block.push_back(std::unique_ptr<UnicodeBlock>(new UnicodeBlock(BasicLatin, BasicLatinStartVal, BasicLatinEndVal)));
		m_isoir100Block.push_back(std::unique_ptr<UnicodeBlock>(new UnicodeBlock(Latin1Supplement, Latin1SupplementStartVal, Latin1SupplementEndVal)));
	}

	ISOIR100::~ISOIR100()
	{
	}

	bool ISOIR100::matchCharacterSet(const std::wstring& c)
	{
		bool retVal = false;
		for(const auto& block : m_isoir100Block)
		{
			if (block->checkRange(c))
			{
				retVal = true;
				break;
			}
		}	
		return retVal;
	}
}}