// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "isoir159.h"
#include "charsetconstants.h"
#include "stringutil.h"
#include "charsetutility.h"

namespace CommonPlatform {	namespace Dicom 
{

ISOIR159::ISOIR159()
{
	m_isoir159Block.push_back(std::unique_ptr<UnicodeBlock>(new UnicodeBlock(BasicLatin, BasicLatinStartVal, BasicLatinEndVal)));
	m_isoir159Block.push_back(std::unique_ptr<UnicodeBlock>(new UnicodeBlock(Latin1Supplement, Latin1SupplementStartVal, Latin1SupplementEndVal)));
	m_isoir159Block.push_back(std::unique_ptr<UnicodeBlock>(new UnicodeBlock(LatinExtendedA, LatinExtendedAStartVal, LatinExtendedAEndVal)));
	m_isoir159Block.push_back(std::unique_ptr<UnicodeBlock>(new UnicodeBlock(LatinExtendedB, LatinExtendedBStartVal, LatinExtendedBEndVal)));
	m_isoir159Block.push_back(std::unique_ptr<UnicodeBlock>(new UnicodeBlock(SpacingModifierLetters, SpacingModifierLettersStartVal, SpacingModifierLettersEndVal)));
	m_isoir159Block.push_back(std::unique_ptr<UnicodeBlock>(new UnicodeBlock(GreekAndCoptic, GreekAndCopticStartVal, GreekAndCopticEndVal)));
	m_isoir159Block.push_back(std::unique_ptr<UnicodeBlock>(new UnicodeBlock(Cyrillic, CyrillicStartVal, CyrillicEndVal)));
	m_isoir159Block.push_back(std::unique_ptr<UnicodeBlock>(new UnicodeBlock(LetterLikeSymbols, LetterLikeSymbolsStartVal, LetterLikeSymbolsEndVal)));
	m_isoir159Block.push_back(std::unique_ptr<UnicodeBlock>(new UnicodeBlock(CJKUnifiedIdeographs, CJKUnifiedIdeographsStartVal, CJKUnifiedIdeographsEndVal)));
}

ISOIR159::~ISOIR159()
{
}

bool ISOIR159::hasSpecialCharacters(const std::wstring& str) const
{
	bool retVal = false;
	//0x2116 Numero sign and 0x007E Tilde are special characters which cannot be detected using bytesequence	
	//to be on safer side, covert to hex and compare the value, so that we are sure its not a half character
	unsigned int decVal = StringUtil::getDecimalValue(str);
	if ((decVal == TILDE) || (decVal == NUMEROSIGN))
	{
		retVal = true;
	}	
	return retVal;
}

bool ISOIR159::matchCharacterSet(const std::wstring& str)
{	
	bool retVal = checkUnicodeBlock(str);
	if ((retVal) && (!hasSpecialCharacters(str)))
	{
		std::string multibyteString("");
		retVal = detectWithCodePage(str, multibyteString);
		retVal = retVal && checkByteSequence(multibyteString);
	}
	return retVal;
}

bool ISOIR159::checkUnicodeBlock(const std::wstring& c)
{
	bool retVal = false;
	for (const auto& block : m_isoir159Block)
	{
		if (block->checkRange(c))
		{
			retVal = true;
			break;
		}
	}
	return retVal;
}
bool ISOIR159::detectWithCodePage(const std::wstring& c, std::string& multibyteOutString)
{
	std::string jap159Str = StringUtil::wstringToString(c, JapCodePageId);
	std::wstring revjap159Str = StringUtil::stringToWString(jap159Str, JapCodePageId);
	if (revjap159Str.compare(c) == 0)
	{
		multibyteOutString = jap159Str;
		return true;
	}
	return false;
}

bool ISOIR159::isIR159SpecificByteSequence(const std::string& str)const
{
	bool retValue = false;
	const char* c = str.c_str();

	if ((c[1] == '$') && (c[2] == '(') && (c[3] == 'D'))
	{
		retValue = true;
	}
	return retValue;
}

bool ISOIR159::checkByteSequence(const std::string& str)
{
	bool retValue = false;
	if (CharSetUtility::isEscSequenceForJapCharset(str) && isIR159SpecificByteSequence(str))
	{
		retValue = true;
	}	
	return retValue;
}


}}