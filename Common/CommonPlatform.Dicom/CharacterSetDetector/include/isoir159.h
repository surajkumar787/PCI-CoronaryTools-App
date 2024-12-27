// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include "IDetector.h"
#include "unicodeblock.h"
#include <vector>
#include <memory>

namespace CommonPlatform {	namespace Dicom{

class ISOIR159 : public IDetector
{
public:
	ISOIR159();
	virtual ~ISOIR159();
	virtual bool matchCharacterSet(const std::wstring& str) override;


private:
	ISOIR159(const ISOIR159&) = delete;
	ISOIR159& operator= (const ISOIR159&) = delete;

	bool checkUnicodeBlock(const std::wstring& c);
	bool detectWithCodePage(const std::wstring& c, std::string& multibyteOutString);
	bool checkByteSequence(const std::string& str);
	bool hasSpecialCharacters(const std::wstring& str) const;
	bool isIR159SpecificByteSequence(const std::string& str)const;

private:
	std::vector<std::unique_ptr<UnicodeBlock>>  m_isoir159Block;


};

}}
