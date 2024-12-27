// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "charsetmanager.h"

namespace CommonPlatform {	namespace Dicom{

CharsetManager::CharsetManager():m_bypassDetection(false)
{
}

CharsetManager::~CharsetManager()
{
}

void CharsetManager::setPossibleCharacterset(std::vector<std::string>& charsets)
{	
	if (m_bypassDetection)
	{
		m_detectedCharsets = charsets;
	}
	m_charsetDetector.setAvailableCharacterSets(charsets);
}

void CharsetManager::setBypassDetection(bool value)
{
	m_bypassDetection = value;
}

std::wstring CharsetManager::getEncodableString(const std::wstring& inputString)
{
	std::wstring outputString;
	if (!m_bypassDetection)
	{
		std::vector<std::string> detectedCharsets = m_charsetDetector.detectCharacterSet(inputString, outputString);
		m_detectedCharsets.insert(m_detectedCharsets.end(), detectedCharsets.begin(), detectedCharsets.end());
	}
	else
	{
		outputString = inputString;		
	}
	return outputString;
}

std::vector<std::string> CharsetManager::getDetectedCharacterSets() const
{
	return optimizeGivenCharactersets(m_detectedCharsets);
}

void CharsetManager::clearDetectedCharacterSets()
{
	m_detectedCharsets.clear();
}

std::vector<std::string> CharsetManager::optimizeGivenCharactersets(const std::vector<std::string>& charsetsToOptimize) const
{	
	std::vector<std::string> optimizedCharSets;
	if(!m_bypassDetection)
		optimizedCharSets = m_charsetDetector.retrieveOptimalCharactersets(charsetsToOptimize);
	else
	{
		optimizedCharSets = m_detectedCharsets;
	}
	return optimizedCharSets;
}

}}