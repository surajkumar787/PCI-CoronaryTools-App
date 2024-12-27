// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "unicodeblock.h"
#include "stringutil.h"

namespace CommonPlatform {	namespace Dicom {

UnicodeBlock::UnicodeBlock(const std::string& blockName, unsigned int startVal, unsigned int endVal) :
	m_blockName(blockName),
	m_endVal(endVal), 
	m_startVal(startVal)
{	

}

bool UnicodeBlock::checkRange(const std::wstring& c) const
{
	bool retVal = false;	
	unsigned int val = StringUtil::getDecimalValue(c);
	if ((val >= m_startVal) && (val <= m_endVal))
	{
		retVal = true;
	}
	return retVal;
}

std::string UnicodeBlock::getBlockName() const
{
	return m_blockName;
}

}}