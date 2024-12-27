// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include "..\charactersetdetector\include\charactersetdetector.h"

namespace CommonPlatform {	namespace Dicom{

class CharsetManager
{
public:
	CharsetManager();
	virtual ~CharsetManager();
	
	void setBypassDetection(bool value);
	void setPossibleCharacterset(std::vector<std::string>& charsets);
	
	// Detects charactersets for the input string and gives the output string to be used by the dicom attributes.
	// Also collects the characters set detected for each input string
	std::wstring getEncodableString(const std::wstring& inputString);
	
	// Get detected character sets based on the calls to getEncodableString
	std::vector<std::string> getDetectedCharacterSets() const;

	void clearDetectedCharacterSets();

private:
	std::vector<std::string> optimizeGivenCharactersets(const std::vector<std::string>& charsetsToOptimize) const;

	std::vector<std::string> m_detectedCharsets;
	CharacterSetDetector m_charsetDetector;
	bool m_bypassDetection;
};

}}