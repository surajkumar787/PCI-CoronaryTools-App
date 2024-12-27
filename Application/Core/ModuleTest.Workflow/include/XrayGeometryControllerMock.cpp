// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "XrayGeometryControllerMock.h"

XrayGeometryControllerMock::XrayGeometryControllerMock() :
	m_lastAngulation(0.0),
	m_lastRotation(0.0),
	m_lastActivate(false)
{
}

bool XrayGeometryControllerMock::enableApc(double angulation, double rotation, bool activate)
{
	m_lastAngulation = angulation;
	m_lastRotation = rotation;
	m_lastActivate = activate;
	return true;
}

bool XrayGeometryControllerMock::disableApc()
{
	m_lastAngulation = 0.0;
	m_lastRotation = 0.0;
	m_lastActivate = false;
	return true;
}

double XrayGeometryControllerMock::getLastAngulation() const
{
	return m_lastAngulation;
}

double XrayGeometryControllerMock::getLastRotation()const
{
	return m_lastRotation;
}

bool XrayGeometryControllerMock::getLastActivate()const
{
	return m_lastActivate;
}
