// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include "IDetector.h"
#include "unicodeblock.h"
#include <vector>

namespace CommonPlatform {	namespace Dicom{

class Gb18030 : public IDetector
{
public:
	Gb18030();
	virtual ~Gb18030();
	virtual bool matchCharacterSet(const std::wstring& str) override;

private:
	bool checkByteSequence(const std::string& str);
	bool detectWithCodePage(const std::wstring& c, std::string& multibyteOutString);	
	bool checkIfSingleByteCharacter(std::vector<unsigned char>& c);
	//check if the character is 2 or 4 bytes
	bool checkIfMultiByteCharacter(std::vector<unsigned char>& bytes);
	bool checkIfTwoByteChar(std::vector<unsigned char>& c);
	bool checkIfFourByteCharacter(std::vector<unsigned char>& c);
	//checks the 1st and 3rd byte of a multibyte character
	bool checkOddByteOfMultibyteCharacter(unsigned char c) const;
	//checks the 2nd and 4th byte of a multibyte character
	bool checkEvenByteOfMultibyteCharacter(unsigned char c) const;
private:
	Gb18030(const Gb18030&) = delete;
	Gb18030& operator= (const Gb18030&) = delete;

};

}}