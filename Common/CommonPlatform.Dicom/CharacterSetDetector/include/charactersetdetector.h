// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <vector>
#include <map>
#include <memory>
#include <string>

namespace CommonPlatform {	namespace Dicom
{
class IDetector;

class CharacterSetDetector
{
public:
	explicit CharacterSetDetector(bool replaceIllegalCharacters = true, wchar_t replacementCharacter = L'?');
	~CharacterSetDetector() = default;
	
	// Set list character sets to be used during detection
	void setAvailableCharacterSets(const std::vector<std::string>& possibleCharacterSets);

	// Analyze input string and find character set(s) that can be used to encode the string. If not-encodable characters are found and replaceIlligalCharacter
	// is true, modifiedOutputString contains string where illigal characters are replaced by m_replacementCharacter
	std::vector<std::string> detectCharacterSet(const std::wstring& inputString, std::wstring& modifiedOutputString);	

	// Optimize inputCharacterSets by performing steps like removing duplicates, make sure the sets are in the correct order, etc.
	std::vector<std::string> retrieveOptimalCharactersets(const std::vector<std::string>& inputCharacterSets) const;

private:
	CharacterSetDetector(const CharacterSetDetector&) = delete;
	CharacterSetDetector& operator= (const CharacterSetDetector&) = delete;

	std::wstring getValidCharacter(const std::wstring& inputString, int& count);
	void contructModifiedOutputString(std::wstring& modifiedString, std::wstring& character, bool detected);
	bool searchCharacterSets(const std::wstring& inputChar, std::vector<std::string>& detectedCharSets);
	void findCharsetOfReplacementCharacter();
	void addAllSupportedCharacterSets(std::vector <std::string>& charSetList);
	std::vector<std::string> removeUnsupportedCharacterSets(const std::vector<std::string>& possibleCharacterSets);

	std::vector<std::string> m_availableCharacterSets;
	bool m_replaceIllegalCharacters;
	bool m_replacementCharUsed;
	wchar_t m_replacementCharacter;
	std::string m_charsetOfReplacementChar;	

	const std::map <std::string, std::shared_ptr<IDetector>> m_charsetDetectorMap;
};
}}