// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

namespace CommonPlatform {	namespace Dicom{

	const std::string CODEEXTENSION = " 2022 ";

	const std::string ISO_ = "ISO_";
	const int IsoLength = 3;

	const unsigned int HighSurrogateStartVal = 0xD800;
	const unsigned int LowSurrogateStartVal = 0xDC00;
	const unsigned int LowestSurrogateChar = 0x10000;
	const unsigned int TenBits = 0x400;
	const unsigned int SurrogateMask = 0xFC00;

	//Yen and overline are special characters in ISO IR 13
	const unsigned int YEN = 0x00A5;
	const unsigned int SLASH = 0x005C;
	const unsigned int OVERLINE = 0x203E;
	const unsigned int TILDE = 0x007E;
	const unsigned int NUMEROSIGN = 0x2116;
	const unsigned int ATSIGN = 0x40;
	const unsigned int SMALLTHORN = 0xFE;
	const unsigned int HEX80 = 0x80;
	const unsigned int HEX81 = 0x81;
	const unsigned int HEX30 = 0x30;
	const unsigned int HEX39 = 0x39;

	const std::string IsoIR6 = "ISO_IR 6";
	const std::string Iso2022IR6 = "ISO 2022 IR 6";
	const std::string IsoIR100 = "ISO_IR 100";
	const std::string Iso2022IR100 = "ISO 2022 IR 100";
	const std::string IsoIR13 = "ISO_IR 13";
	const std::string Iso2022IR13 = "ISO 2022 IR 13";
	const std::string IsoIR87 = "ISO_IR 87";
	const std::string Iso2022IR87 = "ISO 2022 IR 87";
	const std::string IsoIR159 = "ISO_IR 159";
	const std::string Iso2022IR159 = "ISO 2022 IR 159";
	const std::string gb18030CharSet = "GB18030";
	const std::string IsoIR192CharSet = "ISO_IR 192";

	const int ESC = 0x1b;
	const int JapCodePageId = 50222;
	const int ChineseCodePageId = 54936;
	const int Utf8CodePageId = 65001;

	//*******************section for unicode blocks***********************//
	static const char* BasicLatin = "Basic Latin";
	const unsigned int BasicLatinStartVal = 0x0000;
	const unsigned int BasicLatinEndVal = 0x007F;

	static const char* Latin1Supplement = "Latin-1 Supplement";
	const unsigned int Latin1SupplementStartVal = 0x0080;
	const unsigned int Latin1SupplementEndVal = 0x00FF;

	static const char* GeneralPunctuation = "General Punctuation";
	const unsigned int GeneralPunctuationStartVal = 0x2000;
	const unsigned int GeneralPunctuationEndVal = 0x206F;

	static const char* HalfWidthFullWidthForm = "Halfwidth and Fullwidth Forms";
	const unsigned int HalfWidthFullWidthFormStartVal = 0xFF00;
	const unsigned int HalfWidthFullWidthFormEndVal = 0xFFEF;

	static const char* GreekAndCoptic = "Greek And Coptic";
	const unsigned int GreekAndCopticStartVal = 0x0370;
	const unsigned int GreekAndCopticEndVal = 0x03FF;

	static const char* Cyrillic = "Cyrillic";
	const unsigned int CyrillicStartVal = 0x0400;
	const unsigned int CyrillicEndVal = 0x04FF;

	static const char* LetterLikeSymbols = "Letterlike Symbols";
	const unsigned int LetterLikeSymbolsStartVal = 0x2100;
	const unsigned int LetterLikeSymbolsEndVal = 0x214F;

	static const char* Arrows = "Arrows";
	const unsigned int ArrowsStartVal = 0x2190;
	const unsigned int ArrowsEndVal = 0x21FF;

	static const char* MathematicalOperators = "Mathematical Operators";
	const unsigned int MathematicalOperatorsStartVal = 0x2200;
	const unsigned int MathematicalOperatorsEndVal = 0x22FF;

	static const char* MiscellaneousTechnical = "Miscellaneous Technical";
	const unsigned int MiscellaneousTechnicalStartVal = 0x2300;
	const unsigned int MiscellaneousTechnicalEndVal = 0x23FF;

	static const char* BoxDrawing = "Box Drawing";
	const unsigned int BoxDrawingStartVal = 0x2500;
	const unsigned int BoxDrawingEndVal = 0x257F;

	static const char* GeometricShapes = "Geometric Shapes";
	const unsigned int GeometricShapesStartVal = 0x25A0;
	const unsigned int GeometricShapesEndVal = 0x25FF;

	static const char* MiscellaneousSymbols = "Miscellaneous Symbols";
	const unsigned int MiscellaneousSymbolsStartVal = 0x2600;
	const unsigned int MiscellaneousSymbolsEndVal = 0x26FF;

	static const char* CJKSymbolsAndPunctuation = "CJK Symbols and Punctuation";
	const unsigned int CJKSymbolsAndPunctuationStartVal = 0x3000;
	const unsigned int CJKSymbolsAndPunctuationEndVal = 0x303F;

	static const char* Hiragana = "Hiragana";
	const unsigned int HiraganaStartVal = 0x3040;
	const unsigned int HiraganaEndVal = 0x309F;

	static const char* Katakana = "Katakana";
	const unsigned int KatakanaStartVal = 0x30A0;
	const unsigned int KatakanaEndVal = 0x30FF;

	static const char* CJKUnifiedIdeographs = "CJK Unified Ideographs";
	const unsigned int CJKUnifiedIdeographsStartVal = 0x4E00;
	const unsigned int CJKUnifiedIdeographsEndVal = 0x9FFF;

	static const char* LatinExtendedA = "Latin Extended-A";
	const unsigned int LatinExtendedAStartVal = 0x0100;
	const unsigned int LatinExtendedAEndVal = 0x017F;

	static const char* LatinExtendedB = "Latin Extended-B";
	const unsigned int LatinExtendedBStartVal = 0x0180;
	const unsigned int LatinExtendedBEndVal = 0x024F;

	static const char* SpacingModifierLetters = "Spacing Modifier Letters";
	const unsigned int SpacingModifierLettersStartVal = 0x02B0;
	const unsigned int SpacingModifierLettersEndVal = 0x02FF;


}}