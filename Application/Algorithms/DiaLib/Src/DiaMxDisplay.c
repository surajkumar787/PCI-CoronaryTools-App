//Copyright (c)2014 Koninklijke Philips Electronics N.V.,
//All Rights Reserved.
//
//This source code and any compilation or derivative thereof is the
//proprietary information of Koninklijke Philips Electronics N.V.
//and is confidential in nature.
//Under no circumstances is this software to be combined with any Open Source
//Software in any way or placed under an Open Source License of any type 
//without the express written permission of Koninklijke Philips 
//Electronics N.V.

#include <DiaLibDev.h>

void MxDisPlaySetBackgroundImageUC(unsigned char* im, int Iw, int Ih, char *name){
#ifdef DISPLAY_INT
	int p;
	float* newim = (float*)malloc(Iw*Ih*sizeof(float));
	for (p=0; p<Iw*Ih; p++)
		newim[p] = (float)im[p];

	MxPutMatrixFloat(newim,Iw,Ih,name);
	free(newim);
#endif
}
void MxDisPlaySetBackgroundImageS(short* im, int Iw, int Ih, char *name){
#ifdef DISPLAY_INT
	int p;
	float* newim = (float*)malloc(Iw*Ih*sizeof(float));
	for (p=0; p<Iw*Ih; p++)
		newim[p] = (float)im[p];

	MxPutMatrixFloat(newim,Iw,Ih,name);
	free(newim);
#endif
}
void MxDisPlaySetBackgroundImageI(int* im, int Iw, int Ih, char *name){
#ifdef DISPLAY_INT
	int p;
	float* newim = (float*)malloc(Iw*Ih*sizeof(float));
	for (p=0; p<Iw*Ih; p++)
		newim[p] = (float)im[p];

	MxPutMatrixFloat(newim,Iw,Ih,name);
	free(newim);
#endif
}
void MxDisPlaySetBackgroundImageF(float* im, int Iw, int Ih, char *name){
#ifdef DISPLAY_INT
	MxPutMatrixFloat(im,Iw,Ih,name);
#endif
}
