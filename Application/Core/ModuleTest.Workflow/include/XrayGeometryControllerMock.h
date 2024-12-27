// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "IXrayGeometryController.h"

class XrayGeometryControllerMock : public CommonPlatform::Xray::IXrayGeometryController
{
public:
	XrayGeometryControllerMock();

	bool enableApc(double angulation, double rotation, bool activate) override;
	bool disableApc() override;

	double getLastAngulation()const;
	double getLastRotation()const;
	bool getLastActivate()const;
private:
	double m_lastAngulation;
	double m_lastRotation;
	bool m_lastActivate;
};

