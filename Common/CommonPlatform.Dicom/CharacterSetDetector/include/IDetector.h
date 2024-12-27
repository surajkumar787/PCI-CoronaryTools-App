// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include <string>
namespace CommonPlatform {	namespace Dicom{

	class IDetector
	{
		public:
			virtual bool matchCharacterSet(const std::wstring& inputChar) = 0;
			virtual ~IDetector() {}
	};

}}
