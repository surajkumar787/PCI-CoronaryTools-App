// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include <string>

namespace CommonPlatform {	namespace Dicom 
{
class UnicodeBlock
{
public:
	UnicodeBlock(const std::string& blockName, unsigned int startVal, unsigned int endVal);
	bool checkRange(const std::wstring& c) const;
	std::string getBlockName() const;

private:
	std::string m_blockName;
	unsigned int m_endVal;
	unsigned int m_startVal;
};

}}
