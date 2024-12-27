// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "detectorfactory.h"
#include "charsetconstants.h"
#include "isoir6.h"
#include "isoir100.h"
#include "isoir13.h"
#include "isoir87.h"
#include "isoir159.h"
#include "gb18030.h"
#include "isoir192.h"

namespace CommonPlatform {	namespace Dicom{

std::map<std::string, std::shared_ptr<IDetector>> DetectorFactory::createAllDetectors()
{
	std::map<std::string, std::shared_ptr<IDetector>> concreteDetector;	
	concreteDetector.emplace(std::make_pair(IsoIR6, std::shared_ptr<IDetector>(new ISOIR6())));	
	concreteDetector.emplace(std::make_pair(IsoIR100, std::shared_ptr<IDetector>(new ISOIR100())));
	concreteDetector.emplace(std::make_pair(IsoIR13, std::shared_ptr<IDetector>(new ISOIR13())));
	concreteDetector.emplace(std::make_pair(IsoIR87, std::shared_ptr<IDetector>(new ISOIR87())));
	concreteDetector.emplace(std::make_pair(IsoIR159, std::shared_ptr<IDetector>(new ISOIR159())));
	concreteDetector.emplace(std::make_pair(gb18030CharSet, std::shared_ptr<IDetector>(new Gb18030())));
	concreteDetector.emplace(std::make_pair(IsoIR192CharSet, std::shared_ptr<IDetector>(new IsoIr192Detector())));
	return concreteDetector;
}
	

}}