// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include "detectorfactory.h"
#include <vector>

namespace CommonPlatform {	namespace Dicom{

	class CharSetUtility
	{
	public:	
		static std::vector<std::string> removeAllCodeExtensions(const std::vector<std::string>& charSetList);			
		static void reorderCharacterSets(std::vector<std::string>& charSetList);
		static void removeRedundantCharsets(std::vector<std::string>& charsetList);	
		static bool isEscSequenceForJapCharset(const std::string& str);
		static void addCorrectionsToCharset(std::vector<std::string>& charSetList);

	private:
		static void eliminateSubsets(std::vector<std::string>& characterSetList);	
		static bool reorderLatinCharacterSets(std::vector<std::string>& charsetList);		
		static void reorderIR13(bool latinCharSetPresent, std::vector<std::string>& charsets);		
		static void reorderIR159(std::vector<std::string>& charsets);
		static void addCodeExtensionToCharSet(std::vector<std::string>& charSetList);
		static void addCodeExtension(std::string& charSet);		
		static void addSingleByteCharsetIfNotPresent(std::vector<std::string>& charsets);
		//Fills all supported single byte character sets with and without code extensions
		static std::vector<std::string> getSupportedSingleByteCharacterset();
		static void removeCodeExtensions(std::string& charSet);
				
	};

}}