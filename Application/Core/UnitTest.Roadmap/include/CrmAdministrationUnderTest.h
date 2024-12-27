// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "CrmAdministration.h"

namespace Pci {	namespace Test {

class CrmAdministrationUnderTest : public ::Pci::Core::CrmAdministration
{
public:
	CrmAdministrationUnderTest(CommonPlatform::Log& log, const Pci::Core::CrmParameters &params, Pci::Core::CrmRepository &repository);
    virtual ~CrmAdministrationUnderTest(void);

    bool persistentViewCreated() const;
    void updatePersistent() override;
};

}}