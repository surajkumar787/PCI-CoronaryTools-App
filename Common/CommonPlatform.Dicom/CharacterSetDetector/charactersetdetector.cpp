// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "charactersetdetector.h"
#include "charsetutility.h"
#include "stringutil.h"
#include "detectorfactory.h"
#include "charsetconstants.h"
#include <Shlwapi.h>

namespace CommonPlatform {	namespace Dicom
{
	CharacterSetDetector::CharacterSetDetector(bool replaceIllegalCharacters, wchar_t replacementCharacter):
		m_replaceIllegalCharacters(replaceIllegalCharacters),
		m_replacementCharUsed(false), 
		m_replacementCharacter(L'?'),
		m_charsetDetectorMap(DetectorFactory::createAllDetectors())
	{		
		if (replacementCharacter != L'\0')
		{
			m_replacementCharacter = replacementCharacter;
		}	
	}

	void CharacterSetDetector::addAllSupportedCharacterSets(std::vector <std::string>& charSetList) 
	{
		charSetList.clear();
		charSetList.emplace_back(IsoIR6);
		charSetList.emplace_back(IsoIR100);
		charSetList.emplace_back(IsoIR13);
		charSetList.emplace_back(IsoIR87);
		charSetList.emplace_back(IsoIR159);
		charSetList.emplace_back(gb18030CharSet);
		charSetList.emplace_back(IsoIR192CharSet);
	}

	void CharacterSetDetector::findCharsetOfReplacementCharacter()
	{
		std::vector<std::string> charsets;
		if (searchCharacterSets(std::wstring(1, m_replacementCharacter), charsets))
		{
			m_charsetOfReplacementChar = charsets.front();
		}
		else
		{
			m_replacementCharacter = L'?';
			m_charsetOfReplacementChar = IsoIR6;
		}
	}

	std::vector<std::string> CharacterSetDetector::removeUnsupportedCharacterSets(const std::vector<std::string>& possibleCharacterSets)
	{
		std::vector<std::string> suppurtedCharacterSets;
		for (const auto& characterSet : possibleCharacterSets)
		{
			auto itr = m_charsetDetectorMap.find(characterSet);
			if (itr != m_charsetDetectorMap.end())
			{
				suppurtedCharacterSets.emplace_back(characterSet);
			}
		}
		return suppurtedCharacterSets;
	}

	void CharacterSetDetector::setAvailableCharacterSets(const std::vector<std::string>& possibleCharacterSets)
	{	
		m_replacementCharUsed = false;
		m_availableCharacterSets.clear();
		
		auto charSets = CharSetUtility::removeAllCodeExtensions(possibleCharacterSets);
		charSets = StringUtil::removeDuplicatesWithOrder(charSets);
		m_availableCharacterSets = removeUnsupportedCharacterSets(charSets);
		
		findCharsetOfReplacementCharacter();
	}

	void CharacterSetDetector::contructModifiedOutputString( std::wstring& modifiedString, std::wstring& character, bool detected )
	{
		if (detected || !m_replaceIllegalCharacters)
		{
			modifiedString.append(character);
		}
		else
		{
			modifiedString.append(std::wstring(1,m_replacementCharacter));
			m_replacementCharUsed = true;
		}
	}

	bool CharacterSetDetector::searchCharacterSets(const std::wstring& inputChar, std::vector<std::string>& detectedCharSets)
	{
		bool found = false;
		for (auto& charSet : m_availableCharacterSets)
		{
			auto itr = m_charsetDetectorMap.find(charSet);
			if (itr != m_charsetDetectorMap.end())
			{
				if (itr->second->matchCharacterSet(inputChar))
				{
					detectedCharSets.push_back(itr->first);
					found = true;
					break;
				}
			}			
		}		
		return found;
	}

	std::vector<std::string> CharacterSetDetector::detectCharacterSet(const std::wstring& inputString, std::wstring& modifiedOutputString)
	{
		if (m_availableCharacterSets.empty())
		{
			addAllSupportedCharacterSets(m_availableCharacterSets);
			findCharsetOfReplacementCharacter();
		}

		std::vector<std::string> detectedCharSets;
		int charCount = 0;	
		while (charCount < static_cast<int>(inputString.size()))
		{
			std::wstring nextChar = getValidCharacter(inputString, charCount);
			bool found = searchCharacterSets(nextChar, detectedCharSets);
			contructModifiedOutputString(modifiedOutputString, nextChar, found);
		}
		
		CharSetUtility::removeRedundantCharsets(detectedCharSets);
		return detectedCharSets;
	}

	std::vector<std::string> CharacterSetDetector::retrieveOptimalCharactersets(const std::vector<std::string>& inputCharacterSets) const
	{	
		std::vector<std::string> optimalCharacterSets;
		if (!inputCharacterSets.empty())
		{
			optimalCharacterSets = CharSetUtility::removeAllCodeExtensions(inputCharacterSets);

			if (m_replacementCharUsed)
			{
				optimalCharacterSets.push_back(m_charsetOfReplacementChar);
			}
	
			CharSetUtility::removeRedundantCharsets(optimalCharacterSets);
			CharSetUtility::reorderCharacterSets(optimalCharacterSets);
			CharSetUtility::addCorrectionsToCharset(optimalCharacterSets);
		}
		return optimalCharacterSets;
	}

	std::wstring CharacterSetDetector::getValidCharacter(const std::wstring& inputString, int& count)
	{	
		std::wstring nextValidChar(L"");		
		
		// 4 byte characters, check if they are valid surrogates and then Increment count by 2
		if ((StringUtil::isValidHighSurrogate(inputString[count])) && (inputString[count + 1] != L'\0'))
		{
			bool retVal = StringUtil::isValidLowSurrogate(inputString[count + 1]);
			if (retVal)
			{
				nextValidChar = inputString.substr(count, 2);								
			}
			count = count + 2;
		}
		else
		{
			// 1 or 2 byte characters. Increment count by 1
			nextValidChar = inputString.substr(count,1);
			count++;			
		}					
		return nextValidChar;
	}
	
}}