// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#ifndef _BXSNAKE_INCLUDE
#define _BXSNAKE_INCLUDE

#include <BxCurve.h>

#define BXSNAKE_MAX_NKNOTS           64

typedef struct
{
	int   nknots                            ;
	int   sampling                          ;
	float xknot   [BXSNAKE_MAX_NKNOTS]      ;
	float yknot   [BXSNAKE_MAX_NKNOTS]      ;

	BxCurve curve ; 

} BxSnake ;

int BxSnakeInit                       (BxSnake* snake, float* xknot, float* yknot, int nknots, int sampling ) ;
int BxSnakeCopy                       (BxSnake* snake, BxSnake* newsnake) ;
int BxSnakeNormalAtKnot               (BxSnake* snake, int knot, float range, float* xn, float* yn, int npoints) ;
int BxSnakeFieldOfView                (BxSnake* snake, int extremity, float extension, float viewAngle, float* xf, float* yf, int npoints) ;
int BxSnakeSlide                      (BxSnake* snake, int knot, float* xslide, float* yslide) ;
int BxSnakeArcLengthResample          (BxSnake* snake, int nknots, int sampling ) ;
int BxSnakeArcLengthResampleCurve     (BxSnake* snake, BxCurve * curve, int nknots, int sampling ) ;
int BxSnakeInsertKnotAtExtremity      (BxSnake* snake, int extremity);
int BxSnakeTranslate                  (BxSnake* snake, float tx, float ty) ;
int BxSnakeCenterInImage              (BxSnake* snake, int dimx, int dimy);
int BxSnakeAffineTransform            (BxSnake* snake, float xc, float yc, float a, float b, float c, float d, float e, float f) ;
int BxSnakeMarkersTransform           (BxSnake* snake, float x11, float y11, float x12, float y12, float x21, float y21, float x22, float y22) ;
int BxSnakeZoom                       (BxSnake* snake, float zoom, float xcenter, float ycenter) ;
int BxSnakeShiftedCardinalSpline      (BxSnake* snake, int knot, float* cardinalSpline, int normalize);

#endif