// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#ifndef		_MKXFEATURES_H
#define		_MKXFEATURES_H

typedef enum { 

            MKX_CPL_Cx = 0,
            MKX_CPL_Cy,
            MKX_CPL_Br,
            MKX_CPL_An,
            MKX_CPL_Cx_Speed,
            MKX_CPL_Cy_Speed,
            MKX_CPL_Br_Speed,
            MKX_CPL_An_Speed,
            MKX_CPL_Cx_Accel,
            MKX_CPL_Cy_Accel,
            MKX_CPL_Br_Accel,
            MKX_CPL_An_Accel,
            MKX_CPL_Intens1,
            MKX_CPL_A1,
            MKX_CPL_B1,
            MKX_CPL_Dir1,
            MKX_CPL_LevelIn1,
            MKX_CPL_LevelOut1,
            MKX_CPL_Contrast1,
            MKX_CPL_Err1,
            MKX_CPL_Intens2,
            MKX_CPL_A2,
            MKX_CPL_B2,
            MKX_CPL_Dir2,
            MKX_CPL_LevelIn2,
            MKX_CPL_LevelOut2,
            MKX_CPL_Contrast2,
            MKX_CPL_Err2,

	MKX_CPL_N_FEATURES                      // Number of features

} CMkxCplFeatureType;

#endif   //_MKXFEATURES_H
