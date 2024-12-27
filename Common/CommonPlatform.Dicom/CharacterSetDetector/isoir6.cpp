// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "isoir6.h"
#include "charsetconstants.h"

namespace CommonPlatform {	namespace Dicom{

	ISOIR6::ISOIR6()
	{
		m_isoir6Block.reset(new UnicodeBlock(BasicLatin, BasicLatinStartVal, BasicLatinEndVal));
	}

	ISOIR6::~ISOIR6()
	{		
	}

	bool ISOIR6::matchCharacterSet(const std::wstring& str)
	{	
		return m_isoir6Block->checkRange(str);		
	}
}}