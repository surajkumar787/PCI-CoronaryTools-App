// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "charsetutility.h"
#include "charsetconstants.h"
#include "stringutil.h"
#include <algorithm>

namespace CommonPlatform {	namespace Dicom{

	void CharSetUtility::removeCodeExtensions(std::string& charSet)
	{	
		if (charSet.find(CODEEXTENSION) != std::string::npos)
		{
			charSet.replace(charSet.find(CODEEXTENSION), CODEEXTENSION.length(), "_");
		}	
	}

	void CharSetUtility::addCodeExtension(std::string& charSet)
	{
		if (charSet.find(ISO_) != std::string::npos)
		{
			//erase "_"
			charSet.erase(IsoLength, 1);
			charSet.insert(IsoLength, CODEEXTENSION);
		}
	}

	bool CharSetUtility::isEscSequenceForJapCharset(const std::string& str)
	{
		bool retValue = false;
		const char* c = str.c_str();
		if (static_cast<int>(strlen(c)) > 3)
		{
			if ((c[0] == ESC))
			{
				retValue = true;
			}
		}
		return retValue;
	}

	std::vector<std::string> CharSetUtility::removeAllCodeExtensions(const std::vector<std::string>& charSetList)
	{
		std::vector<std::string> resultVector = charSetList;
		for(auto& charset : resultVector)
		{
			removeCodeExtensions(charset);
		}
		return resultVector;
	}

	std::vector<std::string> CharSetUtility::getSupportedSingleByteCharacterset()
	{
		std::vector<std::string> supportedSingleByteCharsSets;
		supportedSingleByteCharsSets.emplace_back(IsoIR6);
		supportedSingleByteCharsSets.emplace_back(IsoIR100);
		supportedSingleByteCharsSets.emplace_back(IsoIR13);
		supportedSingleByteCharsSets.emplace_back(gb18030CharSet);
		supportedSingleByteCharsSets.emplace_back(IsoIR192CharSet);
		return supportedSingleByteCharsSets;
	}	

	void CharSetUtility::eliminateSubsets(std::vector<std::string>& characterSetList)
	{
		// If GB18030 is present, remove all other char sets
		if (StringUtil::contains(characterSetList, gb18030CharSet))
		{
			characterSetList.clear();
			characterSetList.emplace_back(gb18030CharSet);
			return;
		}
		// If ISO IR 192 is present, remove all other char sets
		else if (StringUtil::contains(characterSetList, IsoIR192CharSet))
		{
			characterSetList.clear();
			characterSetList.emplace_back(IsoIR192CharSet);
			return;
		}
		// If both ISO IR 6 and ISO IR 100 is present; or
		// If both ISO IR 6 and ISO IR 13 is present, then remove IR 6
		if (StringUtil::contains(characterSetList, IsoIR6) && (StringUtil::contains(characterSetList, IsoIR100) || (StringUtil::contains(characterSetList, IsoIR13))))
		{
			characterSetList.erase(std::remove(characterSetList.begin(), characterSetList.end(), IsoIR6), characterSetList.end());			
		}
	}

	void CharSetUtility::addCodeExtensionToCharSet(std::vector<std::string>& charSetList)
	{
		if (static_cast<int>(charSetList.size()) == 1)
		{
			return;
		}
		for (auto& element : charSetList)
		{
			addCodeExtension(element);
		}
	}

	void CharSetUtility::removeRedundantCharsets(std::vector<std::string>& charsetList)
	{
		std::sort(charsetList.begin(), charsetList.end());
		charsetList.erase(std::unique(charsetList.begin(), charsetList.end()), charsetList.end());
		eliminateSubsets(charsetList);		
	}
	
	void CharSetUtility::addSingleByteCharsetIfNotPresent(std::vector<std::string>& charsets)
	{
		bool found = false;
		std::vector<std::string> supportedSingleByteCharset = getSupportedSingleByteCharacterset();
		for (auto& item : charsets)
		{
			auto itr = std::find(supportedSingleByteCharset.begin(), supportedSingleByteCharset.end(), item);
			if (itr != supportedSingleByteCharset.end())
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			charsets.insert(charsets.begin(), IsoIR6);		
		}
	}

	void CharSetUtility::addCorrectionsToCharset(std::vector<std::string>& charSetList)
	{
		// ISO_IR 6 is not a defined term; clear list to indicate use of default repertoire
		// SOURCE: http://dicom.nema.org/medical/dicom/current/output/html/part03.html#sect_C.12.1.1.2
		if (charSetList.size() == 1 && charSetList[0].compare(IsoIR6) == 0)
		{
			charSetList.clear();
		}
	}

	bool CharSetUtility::reorderLatinCharacterSets(std::vector<std::string>& charsetList)
	{
		bool latinCharSetPresent = false;
		if (StringUtil::contains(charsetList, IsoIR6))
		{
			charsetList.erase(std::remove(charsetList.begin(), charsetList.end(), IsoIR6), charsetList.end());			
			charsetList.insert(charsetList.begin(),IsoIR6);
			latinCharSetPresent = true;
		}
		else if (StringUtil::contains(charsetList, IsoIR100))
		{			
			charsetList.erase(std::remove(charsetList.begin(), charsetList.end(), IsoIR100), charsetList.end());
			charsetList.insert(charsetList.begin(), IsoIR100);
			latinCharSetPresent = true;
		}
		return latinCharSetPresent;
	}	

	void CharSetUtility::reorderIR13(bool latinCharSetPresent, std::vector<std::string>& charsets)
	{
		if ((!latinCharSetPresent) && (StringUtil::contains(charsets, IsoIR13)))
		{
			charsets.erase(std::remove(charsets.begin(), charsets.end(), IsoIR13), charsets.end());
			charsets.insert(charsets.begin(), IsoIR13);
		}
	}	

	void CharSetUtility::reorderIR159(std::vector<std::string>& charsets)
	{
		if (StringUtil::contains(charsets, IsoIR87) && StringUtil::contains(charsets, IsoIR159))
		{
			charsets.erase(std::remove(charsets.begin(), charsets.end(), IsoIR159), charsets.end());
			charsets.insert(std::find(charsets.begin(), charsets.end(), IsoIR87) + 1, IsoIR159);
		}
	}

	// Preference for the first character set is given in this order
	// 1. IR 6
	// 2. IR 100
	// 3. IR 13
	void CharSetUtility::reorderCharacterSets(std::vector<std::string>& charSetList)
	{
		addSingleByteCharsetIfNotPresent(charSetList);
		if (static_cast<int>(charSetList.size()) == 1)
		{
			return;
		}		
		bool latinCharSetPresent = reorderLatinCharacterSets(charSetList);
		reorderIR13(latinCharSetPresent, charSetList);
		reorderIR159(charSetList);
		addCodeExtensionToCharSet(charSetList);	
	}	

}}