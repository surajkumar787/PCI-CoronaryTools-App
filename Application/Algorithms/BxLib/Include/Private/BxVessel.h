// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#ifndef _BXVESSEL_INCLUDE
#define _BXVESSEL_INCLUDE

#include <BxCurve.h>

#define BXVESSEL_MAX_NKNOTS           128

typedef struct
{
	int   nknots                           ;
	int   sampling                         ;

	float xknot   [BXVESSEL_MAX_NKNOTS]    ;
	float yknot   [BXVESSEL_MAX_NKNOTS]    ;
	float wknot   [BXVESSEL_MAX_NKNOTS]    ;

	BxCurve centerline ; 

	float width   [BXCURVE_MAX_NPOINTS]    ; 

} BxVessel ;

int BxVesselInit                       (BxVessel* vessel, float* xknot, float* yknot, float * wknot, int nknots, int sampling ) ;
int BxVesselCopy                       (BxVessel* vessel, BxVessel* newvessel) ;
int BxVesselSetWidth                   (BxVessel* vessel, float * wknot ) ;
int BxVesselSetConstantWidth           (BxVessel* vessel, float   width ) ;
int BxVesselNormalAtKnot               (BxVessel* vessel, int knot, float range, float* xn, float* yn, int npoints) ;
int BxVesselFieldOfView                (BxVessel* vessel, int extremity, float extension, float viewAngle, float* xf, float* yf, int npoints) ;
int BxVesselSlide                      (BxVessel* vessel, int knot, float* xslide, float* yslide, int halfnpoints) ;
int BxVesselArcLengthResample          (BxVessel* vessel, int nknots, int sampling ) ;
int BxVesselArcLengthResampleCurve     (BxVessel* vessel, BxCurve * curve, int nknots, int sampling ) ;
int BxVesselInsertKnotAtExtremity      (BxVessel* vessel, int extremity);
int BxVesselTranslate                  (BxVessel* vessel, float tx, float ty) ;
int BxVesselRotate                     (BxVessel* vessel, float theta) ;
int BxVesselCenterInImage              (BxVessel* vessel, int dimx, int dimy);
int BxVesselAffineTransform            (BxVessel* vessel, float xc, float yc, float a, float b, float c, float d, float e, float f) ;
int BxVesselMarkersTransform           (BxVessel* vessel, float x11, float y11, float x12, float y12, float x21, float y21, float x22, float y22) ;
int BxVesselZoom                       (BxVessel* vessel, float zoom, float xcenter, float ycenter) ;
int BxVesselCenteredZoom               (BxVessel* vessel, float zoom) ;
int BxVesselShiftedCardinalSpline      (BxVessel* vessel, int knot, float* cardinalSpline, int normalize);

#endif
