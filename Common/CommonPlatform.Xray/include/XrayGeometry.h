// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "Space3d.h"

namespace CommonPlatform {	namespace Xray {


struct XrayGeometry
{
	XrayGeometry();
	
	bool changed(const XrayGeometry &geo) const;
    bool changedStatic(const XrayGeometry &geo) const;
    bool changedMoving(const XrayGeometry &geo) const; 

	enum class NoseOrientation
	{
		Unknown			= -1,
		NoseUp			=  0,
		NoseDown		=  1,
		DecubitusLeft	=  2,
		DecubitusRight	=  3
	};

	enum class LegsOrientation
	{
		Unknown		= -1,
		FeetFirst	=  0,
		HeadFirst	=  1
	};

	double rotation;
	double angulation;	
	double beamIsoCenterHeight;
	double beamLateralOffset;
	double beamLongitudinalOffset;
	double detectorFieldSizeX;
	double detectorFieldSizeY;
	bool   detectorHorizontalFlip;
	bool   detectorImageRotated;
	double detectorRotationAngle;
	bool   detectorVerticalFlip;
	double sourceImageDistance;
	double sourceObjectDistance;
	double standLArmAngle;
	double standPropellerAngle;
	double standRollAngle;
	double tableCradleAngle;
	double tableHeight;
	double tableLateralOffset;
	double tableLongitudinalOffset;
	double tablePivotAngle;
	double tableSwivelAngle;
	double tableTiltAngle;
    bool   noseTowardsDetector;

	LegsOrientation patientLegsToRoomOrientation;
    LegsOrientation patientLegsToTableOrientation;
	NoseOrientation patientNoseOrientation;
	
	void updateSpaces();
    void updatePatientLegsToTableOrientation();

	Space3d room;
	Space3d table;
	Space3d patient;
	Space3d iso;
	Space3d LArm;
	Space3d CArc;
	Space3d detector;
	Space3d image;
    Space3d frustum;
};

}}