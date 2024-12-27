// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "IsoIr192.h"
#include "stringutil.h"
#include "charsetconstants.h"

namespace CommonPlatform {	namespace Dicom
{
	
IsoIr192Detector::IsoIr192Detector()
{
}

IsoIr192Detector::~IsoIr192Detector()
{
}

bool IsoIr192Detector::matchCharacterSet(const std::wstring& str)
{	
	std::string multibyteString("");
	if (str.empty())
	{
		return true;
	}
	bool retVal = detectWithCodePage(str, multibyteString);	
	retVal = retVal && checkByteSequence(multibyteString);
	return retVal;	
}

bool IsoIr192Detector::detectWithCodePage(const std::wstring& c, std::string& multibyteOutString)
{
	std::string utf8String = StringUtil::wstringToString(c, Utf8CodePageId);
	std::wstring revUtf8String = StringUtil::stringToWString(utf8String, Utf8CodePageId);
	if (revUtf8String == c)
	{
		multibyteOutString = utf8String;
		return true;
	}
	return false;
}

bool IsoIr192Detector::checkIfTwoByteChar(std::vector<unsigned char>& c)
{
	bool retVal = false;
	if (c.size() < 2)
	{
		return false;
	}
	//Second byte range: 0x40 - 0x7E OR 0x80 - 0xFE
	if (((c[1] >= ATSIGN) && (c[1] <= TILDE)) || 
		((c[1] >= HEX80) && (c[1] <= SMALLTHORN)))
	{
		retVal = true;
	}	
	return retVal;
}

bool IsoIr192Detector::checkIfSingleByteCharacter(std::vector<unsigned char>& c)
{
	bool retVal = false;	
	if (c.empty())
	{
		return false;
	}

	if (c[0] <= HEX80)
	{	
		retVal = true;
	}
	return retVal;
}

bool IsoIr192Detector::checkIfFourByteCharacter(std::vector<unsigned char>& c)
{
	bool retVal = false;
	if (c.size() < 4)
	{
		return false;
	}	
	// check 2nd, 3rd and 4th byte	
	if (checkEvenByteOfMultibyteCharacter(c[1]))
	{		
		if (checkOddByteOfMultibyteCharacter(c[2]))
		{		
			if (checkEvenByteOfMultibyteCharacter(c[3]))
			{
				retVal = true;
			}
		}
	}
	
	return retVal;
}

bool IsoIr192Detector::checkEvenByteOfMultibyteCharacter(unsigned char c) const
{
	bool retVal = false;
	//range for 2nd and 4th byte : 0x30 - 0x39
	if ((c >= HEX30) && (c <= HEX39))
	{
		retVal = true;
	}
	return retVal;
}

bool IsoIr192Detector::checkOddByteOfMultibyteCharacter(unsigned char c) const
{
	bool retVal = false;
	//range for 1st and 3rd byte : 0x81 - 0xFE
	if ((c >= HEX81) && (c <= SMALLTHORN))
	{
		retVal = true;
	}
	return retVal;
}

bool IsoIr192Detector::checkIfMultiByteCharacter(std::vector<unsigned char>& bytes)
{
	bool retVal = false;
	//check the 1st byte. This range is common to both 2 byte and 4 byte characters
	if (checkOddByteOfMultibyteCharacter(bytes.front()))
	{
		retVal = checkIfTwoByteChar(bytes);
		if (!retVal)
		{
			retVal = checkIfFourByteCharacter(bytes);
		}
	}
	return retVal;
}

bool IsoIr192Detector::checkByteSequence(const std::string& str)
{	
	bool retVal = true;
	const unsigned char* c = reinterpret_cast<const unsigned char*>(str.c_str());
	int size = static_cast<int>(str.length());
	
	std::vector<unsigned char> bytes(c, c+size);	
	if (!checkIfSingleByteCharacter(bytes))
	{
		retVal = checkIfMultiByteCharacter(bytes);
	}
	return retVal;
}

}}