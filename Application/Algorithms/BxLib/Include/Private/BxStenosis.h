// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#ifndef		_BXSTENOSIS_INCLUDE
#define		_BXSTENOSIS_INCLUDE

#include <BxVessel.h>

typedef struct {

	float    OutputResolution  ;
	int      Sampling          ;
	float    FieldOfView       ;
	int      NormalSearch      ;
	float    NormalRange       ;
	int      SlideSearch       ;
	int      GlobalSlideSearch ;
	int      ExtensionSearch   ;
	float    ExtensionRange    ;
	float    WidthSearch       ;
	float    WidthRange        ;
	float    WidthMin          ;
	float    WidthMax          ;
	float    TranslationSearch ;
	float    TranslationRange  ;
	float    RotationSearch    ;
	float    RotationRange     ;
	float    KnotSpacing       ;
	float    ExpectedLength    ;
	float    ExpectedContrast  ;
	float    MaxLength         ;
	int      UseMxLab          ;
	float    CRWeight          ;
	float    WRWeight          ;
	float    CModelWeight      ;
	float    WModelWeight      ;
	float    IModelWeight      ;
	float    SamplingWeight    ;
	float    NormalResolution  ;
	int      ModelHalfSize     ;
	float    ModelRange        ;

} BxStenosisParameters ;

int BxStenosisDefaultParameters ( BxStenosisParameters * params, int nBitsDyn ) ; 

int BxStenosisCreate              ( void** handle, BxStenosisParameters * params ) ;
int BxStenosisDelete              ( void* handle ) ;
int BxStenosisExtract             ( void* handle, BxVessel* vessel, short* image, int dimx, int dimy, float * xinit, float *  yinit, int npoints ) ;
int BxStenosisRefine              ( void* handle, BxVessel* vessel, short* image, int dimx, int dimy ) ;
int BxStenosisExtend              ( void* handle, BxVessel* vessel, short* image, int dimx, int dimy ) ;
int BxStenosisRigidRegsitration   ( void* handle, BxVessel* vessel, short* image, int dimx, int dimy ) ;
int BxStenosisOneClickInit        ( void* handle, BxVessel* vessel, short* image, int dimx, int dimy, float  xclick, float yclick) ;
int BxStenosisSetShapeModel       ( void* handle, BxVessel* vessel ) ;
int BxStenosisSetImageModel       ( void* handle, BxVessel* vessel, short * im, int dimx, int dimy ) ;
int BxStenosisUpdateImageModel    ( void* handle, BxVessel* vessel, short * im, int dimx, int dimy, float forget ) ;
int BxStenosisUpdateShapeModel    ( void* handle, BxVessel* vessel, float forget ) ;
int BxStenosisPlotVessel          ( BxVessel * vessel) ; 

#endif