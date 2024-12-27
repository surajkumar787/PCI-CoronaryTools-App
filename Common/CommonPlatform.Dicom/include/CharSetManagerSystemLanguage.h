// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "charsetmanager.h"

namespace CommonPlatform {	namespace Dicom 
{
	class CharsetManagerSystemLanguage : public CharsetManager
	{
	public:
		explicit CharsetManagerSystemLanguage(const std::wstring& systemLanguage);
		virtual  ~CharsetManagerSystemLanguage() = default;
	};
}}