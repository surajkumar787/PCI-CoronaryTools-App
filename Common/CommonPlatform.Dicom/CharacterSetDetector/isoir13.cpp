// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "isoir13.h"
#include "charsetconstants.h"
#include "stringutil.h"
#include "charsetutility.h"

namespace CommonPlatform {	namespace Dicom{

ISOIR13::ISOIR13():m_isBasicLatin(false),m_latin1Supplement(false)
{
	m_isoir13Block.push_back(std::unique_ptr<UnicodeBlock>(new UnicodeBlock(BasicLatin, BasicLatinStartVal, BasicLatinEndVal)));
	m_isoir13Block.push_back(std::unique_ptr<UnicodeBlock>(new UnicodeBlock(Latin1Supplement, Latin1SupplementStartVal, Latin1SupplementEndVal)));
	m_isoir13Block.push_back(std::unique_ptr<UnicodeBlock>(new UnicodeBlock(GeneralPunctuation, GeneralPunctuationStartVal, GeneralPunctuationEndVal)));
	m_isoir13Block.push_back(std::unique_ptr<UnicodeBlock>(new UnicodeBlock(HalfWidthFullWidthForm, HalfWidthFullWidthFormStartVal, HalfWidthFullWidthFormEndVal)));
}

ISOIR13::~ISOIR13()
{
}

void ISOIR13::processIfLatinBlocks(const std::string& blockName)
{
	if (blockName.compare(BasicLatin) == 0)
	{
		m_isBasicLatin = true; 
	}
	else if (blockName.compare(Latin1Supplement) == 0)
	{
		m_latin1Supplement = true;
	}
}

bool ISOIR13::checkUnicodeBlock(const std::wstring& c)
{
	bool retVal = false;
	for (const auto& block : m_isoir13Block)
	{
		if (block->checkRange(c))
		{
			processIfLatinBlocks(block->getBlockName());
			retVal = true;
			break;
		}
	}
	return retVal;
}

bool ISOIR13::detectWithCodePage(const std::wstring& c, std::string& multibyteOutString)
{		
	std::string jap13Str = StringUtil::wstringToString(c, JapCodePageId);
	std::wstring revJap13Str = StringUtil::stringToWString(jap13Str, JapCodePageId);
	if (revJap13Str.compare(c) == 0)
	{
		multibyteOutString = jap13Str;
		return true;
	}	
	return false;
}

bool ISOIR13::isIR13SpecificByteSequence(const std::string& str)const
{
	bool retValue = false;
	const char* c = str.c_str();
	if ((c[1] == '(') && (c[2] == 'J'))
	{
		retValue = true;
	}

	return retValue;
}

bool ISOIR13::checkByteSequence(const std::string& str) 
{
	bool retValue = false;	
	if ((CharSetUtility::isEscSequenceForJapCharset(str)) && isIR13SpecificByteSequence(str))
	{
		retValue = true;
	}	
	return retValue;
}

bool ISOIR13::isSpecialCharacter(const std::wstring& c) const 
{
	bool retVal = false;
	//0x00A5 YEN SIGN and 0x203E OVERLINE  are special characters which cannot be detected using code page	
    //to be on safer side, covert to hex and compare the value, so that we are sure its not a half character
	unsigned int decVal = StringUtil::getDecimalValue(c);	
	if ((decVal == YEN) || (decVal == OVERLINE))
	{
		retVal = true;
	}		
	
	return retVal;
}

bool ISOIR13::checkIfFurtherProcessingRequired(const std::wstring& str) const
{
	bool retVal = false;
	//if not a special character (yen or overline) and not a basic latin char, 
	//then further processing is required 
	if ((!isSpecialCharacter(str)) && (!m_isBasicLatin))
	{		
		retVal = true;
	}
	return retVal;
}

bool ISOIR13::isTilde(const std::wstring& c)const
{
	bool retVal = false;

	//to be on safer side, convert to hex and compare the value, so that we are sure its not a half character
	unsigned int decVal = StringUtil::getDecimalValue(c);
	if (decVal == TILDE)
	{
		retVal = true;
	}

	return retVal;
}

bool ISOIR13::isSlash(const std::wstring& c)
{
	bool retVal = false;	
	//to be on safer side, convert to hex and compare the value, so that we are sure its not a half character
	unsigned int decVal = StringUtil::getDecimalValue(c);
	if (decVal == SLASH)
	{
		retVal = true;
	}	
	return retVal;
}

bool ISOIR13::isYen(const std::wstring& c)
{
	bool retVal = false;
	//to be on safer side, convert to hex and compare the value, so that we are sure its not a half character
	unsigned int decVal = StringUtil::getDecimalValue(c);
	if (decVal == YEN)
	{
		retVal = true;
	}

	return retVal;
}

bool ISOIR13::isNonYenCharacterInLatin1Supplement(const std::wstring& str)
{
	bool retVal = false;	
	if ((m_latin1Supplement) && (!isYen((str))))
	{
		retVal =  true;
	}
	return retVal;
}

//The following characters requires special handling
bool ISOIR13::isInvalidCharacter(const std::wstring& str)
{
	bool retVal = false; 
	if (isTilde(str) || isSlash(str) || isNonYenCharacterInLatin1Supplement(str))
	{
		retVal = true;
	}
	return retVal;
}

bool ISOIR13::isCharPresentInIR13(const std::wstring& str)
{	
	bool retVal = checkUnicodeBlock(str);	
	if ((!retVal) || isInvalidCharacter(str))
	{
		return false;
	}		
	return retVal;
}

void ISOIR13::resetMemberVariables()
{
	m_latin1Supplement = false;
	m_isBasicLatin = false;

}

//Steps to match character set
//1.Check unicode block
//2.Check if Tilde is present. It is not part of IR 13 and cannot bre caught using code page and byte sequence
//3.If special character is present or if it is BasicLatin chars - further checking not required
//4.Detect with code page
//5.Do byte sequence check
bool ISOIR13::matchCharacterSet(const std::wstring& str)
{	
	bool retVal = isCharPresentInIR13(str);
	if (retVal && checkIfFurtherProcessingRequired(str))
	{
		std::string multibyteString("");
		retVal = detectWithCodePage(str, multibyteString);
		//Byte sequence check is only for non Basic Latin characters
		retVal = retVal && checkByteSequence(multibyteString);
	}	
	resetMemberVariables();
	return retVal;
}
}}