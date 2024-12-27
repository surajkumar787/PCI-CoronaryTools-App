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

class ISOIR87 : public IDetector
{
public:
	ISOIR87();
	virtual ~ISOIR87();
	virtual bool matchCharacterSet(const std::wstring& str) override;


private:
	ISOIR87(const ISOIR87&) = delete;
	ISOIR87& operator= (const ISOIR87&) = delete;

	bool checkUnicodeBlock(const std::wstring& c);
	bool detectWithCodePage(const std::wstring& c, std::string& multibyteOutString);
	bool checkByteSequence(const std::string& str);
	bool checkIfFurtherPorcessingRequired(const std::wstring& str);
	bool isSlash(const std::wstring& str);
	bool isCharPresentinIR87(const std::wstring& str);
	bool isInvalidCharacter(const std::wstring& str);
	bool isNumeroSign(const std::wstring& str);
	bool isIR87SpecificByteSequence(const std::string& str)const;
private:
	std::vector<std::unique_ptr<UnicodeBlock>>  m_isoir87Block;
	

};

}}