// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
//
// Filename: CrmAlgoFluoroStub.h
#pragma once
#include "CrmAlgoFluoro.h"

namespace Pci {	namespace Test {
using namespace Pci::Core;

class CrmAlgoFluoroStub : public CrmAlgoFluoro
{
public:
	CrmAlgoFluoroStub(Simplicity::ThreadPool &pool, const CrmParameters &params);
	virtual ~CrmAlgoFluoroStub();

    virtual	Matrix getTransformationMatrix( const std::shared_ptr<const XrayImage> & src ) override;
};
}}
