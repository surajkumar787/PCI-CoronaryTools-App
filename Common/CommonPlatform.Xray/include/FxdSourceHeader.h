// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

namespace CommonPlatform {	namespace Xray {

#pragma pack(1)

struct FxdSourceHeader
{
	int		id;
	int		width;
	int		height;
	int		count;
	int		type;
	int		bitdepth;
	float	maxValue;
	float	minValue;
	int		offset;
	int		reserved0[23];
};



struct FxdSourceHeaderEx
{
	int	  zero;
	int	  metaId;
	int   version;
	int   lost;
	short start[8];
	short stop[8];
	char  type[32];
	char  channel[32];
	char  technique[32];
	char  appName[32];
	char  procName[32];
	char  kV[32];
	char  mA[32];
	char  mS[32];
	char  aGL[32];
	char  testKV[32];
	char  testMA[32];
	char  testMS[32];
	char  subImages[32];
	char  tickness[32];
	char  dose[32];
	char  doseRatio[32];
	char  framespeed[32];
	char  EDL[32];
	char  senseTaste[32];
	char  phase1Speed[32];
	char  phase1Duration[32];
	char  phase2Speed[32];
	char  phase2Duration[32];
	char  filterType[32];
	char  detectorFormat[32];
	char  measuringField[32];
	char  flavor[32];
	char  length[32];
	char  angulation[32];
	char  rotation[32];
	char  SID[32];
	char  cArm[32];
	char  detectorFrontal[32];
	char  propellor[32];
	char  frontalBeamLongitudinal[32];
	char  frontalRotateDetector[32];
	char  frontalSwing[32];
	char  frontalZRotation[32];
	char  tableHeight[32];
	char  shutters[32];
	char  skinDose[32];
	char  fluoroAreaDose[32];
	char  exposureAreaDose[32];
	char  matrixSize[32];
	char  validRectangle[32];
	char  clusterMode[32];
	char  focus[32];
	short validLeft, validTop, validRight, validBottom;
	short shutterLeft, shutterTop, shutterRight, shutterBottom;
	char  estimatedSkinDose[32];
	char  estimatedFluoDAP[32];
	char  estimatedExpoDAP[32];
	char  pBL[32];
	char  phase3Speed[32];
	char  phase3Duration[32];
	char  lostStrips[32];
	char  tag[128];
};

struct FxdSourceMeta
{
	int				runIndex;
	int				imageIndex;
	XrayImage::Type	type;
	Rect			validRect;
	Rect			shutters;
	Matrix			frustumToPixels;
	Point			mmPerPixel;
	double			frameTime;
	XrayGeometry	geometry;
	// XrayEpx			epx;   contains std::wstring that seems to gives access violation when read (when actual data was missing) and then destructed.
};


#pragma pack()
}}