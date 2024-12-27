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

	class ISOIR13 : public IDetector
	{
	public:
		ISOIR13();
		virtual ~ISOIR13();
		virtual bool matchCharacterSet(const std::wstring& str) override;


	private:
		ISOIR13(const ISOIR13&) = delete;
		ISOIR13& operator= (const ISOIR13&) = delete;

		bool checkUnicodeBlock(const std::wstring& c);
		bool detectWithCodePage(const std::wstring& c, std::string& multibyteOutString);
		bool checkByteSequence(const std::string& str);
		bool isIR13SpecificByteSequence(const std::string& str)const;
		//Yen sign and overline are special characters in ISO IR 13. They cannot be detected using code page.
		//Hence require special handling
		bool isSpecialCharacter(const std::wstring& c) const;
		//Tilde is the only character in Basic Latin which is not part of IR 13
		bool isTilde(const std::wstring& c)const;
		bool isYen(const std::wstring& c);
		bool isSlash(const std::wstring& c);
		//do basic checking for the presence of the character in IR 13 (unicode block and tilde check)
		bool isCharPresentInIR13(const std::wstring& str);
		//check if code page detection and byte sequence check is required
		bool checkIfFurtherProcessingRequired(const std::wstring& str) const;
		void resetMemberVariables();
		//Do further processing if the block  belongs to BasicLatin or Latin1Supplement
		void processIfLatinBlocks(const std::string& blockName);
		bool isNonYenCharacterInLatin1Supplement(const std::wstring& str);
		bool isInvalidCharacter(const std::wstring& str);
	private:
		std::vector<std::unique_ptr<UnicodeBlock>>  m_isoir13Block;
		//This variable is used to decide whether byte sequence check is required or not.
		//It is required only for non basic latin characters
		bool m_isBasicLatin;
		bool m_latin1Supplement;

	};

}}