// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include <map>
#include<memory>
#include "IDetector.h"

namespace CommonPlatform {	namespace Dicom{

	class DetectorFactory
	{
	public:
		static std::map<std::string,std::shared_ptr<IDetector>> createAllDetectors();
	};

}}
