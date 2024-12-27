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

class IsoIr192Detector : public IDetector
{
public:
	IsoIr192Detector();
	virtual ~IsoIr192Detector();
	
	virtual bool matchCharacterSet(const std::wstring& str) override;

private:
	bool checkByteSequence(const std::string& str);
	bool detectWithCodePage(const std::wstring& c, std::string& multibyteOutString);	
	bool checkIfSingleByteCharacter(std::vector<unsigned char>& c);

	bool checkIfMultiByteCharacter(std::vector<unsigned char>& bytes);
	bool checkIfTwoByteChar(std::vector<unsigned char>& c);
	bool checkIfFourByteCharacter(std::vector<unsigned char>& c);

	bool checkOddByteOfMultibyteCharacter(unsigned char c) const;
	bool checkEvenByteOfMultibyteCharacter(unsigned char c) const;
private:
	IsoIr192Detector(const IsoIr192Detector&) = delete;
	IsoIr192Detector& operator= (const IsoIr192Detector&) = delete;
};

}}