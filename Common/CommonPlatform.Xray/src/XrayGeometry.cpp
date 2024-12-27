// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "XrayGeometry.h"

namespace CommonPlatform { namespace Xray{

static const double BeamIsoCenterHeight = 1065.0;
static const double BeamLongitudinalOffset = 1200.0;
static const double DetectorFieldSize = 110.0;
static const double SourceImageDistance = 1195.0;
static const double SourceObjectDistance = 810.0;
static const double TableHeight = 963.0;
static const double TableLongitudinalOffset = 820.0;
static const double MinimumDiference = 0.0001;
static const double Deg90 = 90.0;
static const double Deg180 = 180.0;

XrayGeometry::XrayGeometry()
:
	rotation				        (0.0),
	angulation				        (0.0),
	beamIsoCenterHeight		        (BeamIsoCenterHeight),
	beamLateralOffset		        (0.0),
	beamLongitudinalOffset	        (BeamLongitudinalOffset),
	detectorFieldSizeX		        (DetectorFieldSize),
	detectorFieldSizeY		        (DetectorFieldSize),
	detectorHorizontalFlip	        (false),
	detectorImageRotated	        (false),
	detectorRotationAngle	        (0.0),
	detectorVerticalFlip	        (false),
	sourceImageDistance		        (SourceImageDistance),
	sourceObjectDistance	        (SourceObjectDistance),
	standLArmAngle			        (0.0),
	standPropellerAngle		        (0.0),
	standRollAngle			        (0.0),
	tableCradleAngle		        (0.0),
	tableHeight				        (TableHeight),
	tableLateralOffset		        (0.0),
	tableLongitudinalOffset	        (TableLongitudinalOffset),
	tablePivotAngle			        (0.0),
	tableSwivelAngle		        (0.0),
	tableTiltAngle			        (0.0),
	noseTowardsDetector		        (false),
	patientLegsToRoomOrientation    (LegsOrientation::HeadFirst),
    patientLegsToTableOrientation   (LegsOrientation::HeadFirst),
	patientNoseOrientation	        (NoseOrientation::NoseUp)
{
	updateSpaces();
}


bool XrayGeometry::changed(const XrayGeometry &geo) const
{
    return changedStatic(geo) || changedMoving(geo);
}

bool XrayGeometry::changedStatic(const XrayGeometry &geo) const
{
    if (fabs(detectorFieldSizeX		        -  geo.detectorFieldSizeX)		> MinimumDiference) return true;
    if (fabs(detectorFieldSizeY		        -  geo.detectorFieldSizeY)		> MinimumDiference) return true;
    if (fabs(sourceObjectDistance	        -  geo.sourceObjectDistance)	> MinimumDiference) return true;
    if (     detectorHorizontalFlip	        != geo.detectorHorizontalFlip			          ) return true;
    if (     detectorImageRotated	        != geo.detectorImageRotated				          ) return true;
    if (     detectorVerticalFlip	        != geo.detectorVerticalFlip				          ) return true;
    if (     patientLegsToRoomOrientation	!= geo.patientLegsToRoomOrientation		          ) return true;
    if (     patientLegsToTableOrientation	!= geo.patientLegsToTableOrientation              ) return true;
    if (     patientNoseOrientation	        != geo.patientNoseOrientation			          ) return true;

    return false;
}

bool XrayGeometry::changedMoving(const XrayGeometry &geo) const
{
    if (fabs(rotation				        -  geo.rotation)				> MinimumDiference) return true;
    if (fabs(angulation				        -  geo.angulation)				> MinimumDiference) return true;
    if (fabs(beamIsoCenterHeight	        -  geo.beamIsoCenterHeight)		> MinimumDiference) return true;
    if (fabs(beamLateralOffset		        -  geo.beamLateralOffset)		> MinimumDiference) return true;
    if (fabs(beamLongitudinalOffset         -  geo.beamLongitudinalOffset)	> MinimumDiference) return true;
    if (fabs(detectorRotationAngle	        -  geo.detectorRotationAngle)	> MinimumDiference) return true;
    if (fabs(sourceImageDistance	        -  geo.sourceImageDistance)		> MinimumDiference) return true;
    if (fabs(standLArmAngle			        -  geo.standLArmAngle)			> MinimumDiference) return true;
    if (fabs(standPropellerAngle	        -  geo.standPropellerAngle)		> MinimumDiference) return true;
    if (fabs(standRollAngle			        -  geo.standRollAngle)			> MinimumDiference) return true;
    if (fabs(tableCradleAngle		        -  geo.tableCradleAngle)		> MinimumDiference) return true;
    if (fabs(tableHeight			        -  geo.tableHeight)				> MinimumDiference) return true;
    if (fabs(tableLateralOffset		        -  geo.tableLateralOffset)		> MinimumDiference) return true;
    if (fabs(tableLongitudinalOffset        -  geo.tableLongitudinalOffset)	> MinimumDiference) return true;
    if (fabs(tablePivotAngle		        -  geo.tablePivotAngle)			> MinimumDiference) return true;
    if (fabs(tableSwivelAngle		        -  geo.tableSwivelAngle)		> MinimumDiference) return true;
    if (fabs(tableTiltAngle			        -  geo.tableTiltAngle)			> MinimumDiference) return true;
 
    return false;
}

static const Vector3d X(1, 0, 0);
static const Vector3d Y(0, 1, 0);
static const Vector3d Z(0, 0, 1);

static Space3d frustumToImage(const XrayGeometry &geo)
{
	return	Space3d::projection(-geo.sourceImageDistance);
}

static Space3d imageToDetector(const XrayGeometry &geo)
{
	return	Space3d::rotate(geo.detectorImageRotated   ? Deg90 : 0, Z) *
			Space3d::rotate(geo.detectorVerticalFlip   ? Deg180 : 0, X) *
			Space3d::rotate(geo.detectorHorizontalFlip ? Deg180 : 0, Y);
}

static Space3d detectorToCArc(const XrayGeometry &geo)
{
	return	Space3d::translate(Vector3d(0, 0, geo.sourceImageDistance - geo.sourceObjectDistance)) *
			Space3d::rotate(geo.detectorRotationAngle, Z);
}

static Space3d CArcToLArm(const XrayGeometry &geo)
{
	return	Space3d::rotate(geo.standPropellerAngle, Y) *
			Space3d::rotate(geo.standRollAngle,      X);
}

static Space3d LArmToIso(const XrayGeometry &geo)
{
	return	Space3d::rotate(geo.standLArmAngle, Z);
}

static Space3d isoToRoom(const XrayGeometry &geo)
{
	return	Space3d::translate(Vector3d(geo.beamLateralOffset, geo.beamLongitudinalOffset, geo.beamIsoCenterHeight));
}

static bool isTableBasePivotted(const XrayGeometry &geo)
{
    return fabs(geo.tablePivotAngle) > 175.0;
}

static bool isTableTopMountedInReverse(const XrayGeometry &geo) 
{
    return !isTableBasePivotted(geo) && (geo.beamLongitudinalOffset < 0.0);
}

static Space3d tableToRoom(const XrayGeometry &geo)
{
    return	Space3d::rotate	(geo.tablePivotAngle, Z) *
			Space3d::translate(Vector3d(geo.tableLateralOffset, geo.tableLongitudinalOffset, geo.tableHeight)) *
			Space3d::rotate	(isTableTopMountedInReverse(geo) ? Deg180 : 0.0, Z);
}

static Space3d patientToTable(const XrayGeometry &geo)
{
	return	Space3d::rotate(geo.patientNoseOrientation == XrayGeometry::NoseOrientation::NoseDown ? Deg180 : 0, Y) *
			Space3d::rotate(geo.patientLegsToTableOrientation == XrayGeometry::LegsOrientation::FeetFirst ? Deg180 : 0, Z);
}

void XrayGeometry::updateSpaces()
{
    updatePatientLegsToTableOrientation();

	table    = room		* tableToRoom		(*this);
	patient  = table	* patientToTable	(*this);
	iso      = room     * isoToRoom			(*this);
	LArm     = iso      * LArmToIso			(*this);
	CArc     = LArm     * CArcToLArm		(*this);
	detector = CArc     * detectorToCArc	(*this);
	image    = detector * imageToDetector	(*this);
	frustum  = image    * frustumToImage	(*this);
}

static XrayGeometry::LegsOrientation invertLegsOrientation(XrayGeometry::LegsOrientation legsOrientation)
{
    if ( legsOrientation == XrayGeometry::LegsOrientation::FeetFirst) return XrayGeometry::LegsOrientation::HeadFirst;
    if ( legsOrientation == XrayGeometry::LegsOrientation::HeadFirst) return XrayGeometry::LegsOrientation::FeetFirst;
    return XrayGeometry::LegsOrientation::Unknown;
}

void XrayGeometry::updatePatientLegsToTableOrientation()
{
    if ( isTableTopMountedInReverse(*this) || isTableBasePivotted((*this)))
    {
        patientLegsToTableOrientation = invertLegsOrientation(patientLegsToRoomOrientation);
    }
    else
    {
        patientLegsToTableOrientation = patientLegsToRoomOrientation;
    }
}
}}