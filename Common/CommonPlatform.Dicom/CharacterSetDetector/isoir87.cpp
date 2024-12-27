// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "isoir87.h"
#include "charsetconstants.h"
#include "stringutil.h"
#include "charsetutility.h"

namespace CommonPlatform {	namespace Dicom{

ISOIR87::ISOIR87()
{
	m_isoir87Block.push_back(std::unique_ptr<UnicodeBlock>(new UnicodeBlock(BasicLatin, BasicLatinStartVal, BasicLatinEndVal)));
	m_isoir87Block.push_back(std::unique_ptr<UnicodeBlock>(new UnicodeBlock(Latin1Supplement, Latin1SupplementStartVal, Latin1SupplementEndVal)));
	m_isoir87Block.push_back(std::unique_ptr<UnicodeBlock>(new UnicodeBlock(GreekAndCoptic, GreekAndCopticStartVal, GreekAndCopticEndVal)));
	m_isoir87Block.push_back(std::unique_ptr<UnicodeBlock>(new UnicodeBlock(Cyrillic, CyrillicStartVal, CyrillicEndVal)));
	m_isoir87Block.push_back(std::unique_ptr<UnicodeBlock>(new UnicodeBlock(GeneralPunctuation, GeneralPunctuationStartVal, GeneralPunctuationEndVal)));
	m_isoir87Block.push_back(std::unique_ptr<UnicodeBlock>(new UnicodeBlock(LetterLikeSymbols, LetterLikeSymbolsStartVal, LetterLikeSymbolsEndVal)));
	m_isoir87Block.push_back(std::unique_ptr<UnicodeBlock>(new UnicodeBlock(Arrows, ArrowsStartVal, ArrowsEndVal)));
	m_isoir87Block.push_back(std::unique_ptr<UnicodeBlock>(new UnicodeBlock(MathematicalOperators, MathematicalOperatorsStartVal, MathematicalOperatorsEndVal)));
	m_isoir87Block.push_back(std::unique_ptr<UnicodeBlock>(new UnicodeBlock(MiscellaneousTechnical, MiscellaneousTechnicalStartVal, MiscellaneousTechnicalEndVal)));
	m_isoir87Block.push_back(std::unique_ptr<UnicodeBlock>(new UnicodeBlock(BoxDrawing, BoxDrawingStartVal, BoxDrawingEndVal)));
	m_isoir87Block.push_back(std::unique_ptr<UnicodeBlock>(new UnicodeBlock(GeometricShapes, GeometricShapesStartVal, GeometricShapesEndVal)));
	m_isoir87Block.push_back(std::unique_ptr<UnicodeBlock>(new UnicodeBlock(MiscellaneousSymbols, MiscellaneousSymbolsStartVal, MiscellaneousSymbolsEndVal)));	
	m_isoir87Block.push_back(std::unique_ptr<UnicodeBlock>(new UnicodeBlock(CJKSymbolsAndPunctuation, CJKSymbolsAndPunctuationStartVal, CJKSymbolsAndPunctuationEndVal)));
	m_isoir87Block.push_back(std::unique_ptr<UnicodeBlock>(new UnicodeBlock(Hiragana, HiraganaStartVal, HiraganaEndVal)));
	m_isoir87Block.push_back(std::unique_ptr<UnicodeBlock>(new UnicodeBlock(Katakana, KatakanaStartVal, KatakanaEndVal)));
	m_isoir87Block.push_back(std::unique_ptr<UnicodeBlock>(new UnicodeBlock(CJKUnifiedIdeographs, CJKUnifiedIdeographsStartVal, CJKUnifiedIdeographsEndVal)));
	m_isoir87Block.push_back(std::unique_ptr<UnicodeBlock>(new UnicodeBlock(HalfWidthFullWidthForm, HalfWidthFullWidthFormStartVal, HalfWidthFullWidthFormEndVal)));
}

ISOIR87::~ISOIR87()
{
}

bool ISOIR87::isSlash(const std::wstring& str)
{
	bool retVal = false;	
	//to be on safer side, convert to hex and compare the value, so that we are sure its not a half character
	unsigned int decVal = StringUtil::getDecimalValue(str);
	if (decVal == SLASH)
	{
		retVal = true;
	}	
	return retVal;
}

bool ISOIR87::checkIfFurtherPorcessingRequired(const std::wstring& str)
{
	bool retVal = true;
	//slash cannot be detetcted using code page but it is part of IR 87
	if (isSlash(str))
	{
		retVal = false;
	}
	return retVal;
}

bool ISOIR87::isNumeroSign(const std::wstring& str)
{
	bool retVal = false;
	unsigned int decVal = StringUtil::getDecimalValue(str);
	if (decVal == NUMEROSIGN)
	{
		retVal = true;
	}
	return retVal;
}

bool ISOIR87::isInvalidCharacter(const std::wstring& str)
{
	bool retVal = false;
	//Numero sign passes throught the detection but it is not part of IR 87
	if (isNumeroSign(str))
	{
		retVal = true;
	}
	return retVal;
}

bool ISOIR87::isCharPresentinIR87(const std::wstring& str)
{
	bool retVal = checkUnicodeBlock(str);
	retVal = retVal && (!isInvalidCharacter(str));
	
	return retVal;
}


bool ISOIR87::matchCharacterSet(const std::wstring& str)
{	
	bool retVal = isCharPresentinIR87(str);
	if (retVal && checkIfFurtherPorcessingRequired(str))
	{
		std::string multibyteString("");
		retVal = detectWithCodePage(str, multibyteString);
		retVal = retVal && checkByteSequence(multibyteString);
	}
	return retVal;
}

bool ISOIR87::checkUnicodeBlock(const std::wstring& c)
{
	bool retVal = false;
	for (const auto& block : m_isoir87Block)
	{
		if (block->checkRange(c))
		{			
			retVal = true;
			break;
		}
	}
	return retVal;
}

bool ISOIR87::detectWithCodePage(const std::wstring& c, std::string& multibyteOutString)
{
	std::string jap87Str = StringUtil::wstringToString(c, JapCodePageId);
	std::wstring revjap87Str = StringUtil::stringToWString(jap87Str, JapCodePageId);
	if (revjap87Str.compare(c) == 0)
	{
		multibyteOutString = jap87Str;
		return true;
	}
	return false;
}

bool ISOIR87::isIR87SpecificByteSequence(const std::string& str)const
{
	bool retValue = false;
	const char* c = str.c_str();
	if ((c[1] == '$') && ((c[2] == '@') || (c[2] == 'B')))
	{
		retValue = true;
	}

	return retValue;
}

bool ISOIR87::checkByteSequence(const std::string& str) 
{
	bool retValue = false;	
	if ((CharSetUtility::isEscSequenceForJapCharset(str)) && isIR87SpecificByteSequence(str))
	{
		retValue = true;
	}
	
	return retValue;
}

}}