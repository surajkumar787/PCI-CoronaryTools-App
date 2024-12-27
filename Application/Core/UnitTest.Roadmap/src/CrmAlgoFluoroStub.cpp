// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
//
// Filename: CrmAlgoFluoroStub.cpp

#include "CrmAlgoFluoroStub.h"
#include "SystemType.h"
namespace Pci {	namespace Test {

CrmAlgoFluoroStub::CrmAlgoFluoroStub(Simplicity::ThreadPool &pool, const CrmParameters &params)
	: CrmAlgoFluoro(pool, params, SystemType::Allura)
{
}


CrmAlgoFluoroStub::~CrmAlgoFluoroStub()
{
}

Matrix CrmAlgoFluoroStub::getTransformationMatrix( const std::shared_ptr<const XrayImage> & src )
{
	return CrmAlgoFluoro::getTransformationMatrix(src);
}
}}