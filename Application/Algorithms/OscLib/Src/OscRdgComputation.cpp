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

#include <OscDev.h>

//#ifdef OSC_USE_SSE_RDG
//extern void OscRdgMonoScaleRidgeSSE(short *Ori, short *Gau, float *Rdg, float *Dir, int Iw, int Ih, COscRoiCoord *RoiCoord, float KernelSig);
//#endif

#include <BxGaussian.h>

#include <mutex> 
//#include "MxLab.h"

static float arctan2( float y, float x );

// ***************************************************
// ************* Multiscale extraction ***************
// ***************************************************

void OscRdgMultiScaleRidge(COscHdl OscHdl, int Iw, int Ih)
{
    COsc                *This = (COsc*)OscHdl.Pv;
    COscExtractProc     *ExtractProc = &(This->ExtractProc);
    std::mutex          mutex;

	float		        *Rdg = ExtractProc->ImRdg, *Dir = ExtractProc->ImDir;
    
	OSC_MEMZERO(Rdg, Iw*Ih); OSC_MEMZERO(Dir, Iw*Ih);
    
    ExtractProc->IwRdg = Iw;
    ExtractProc->IhRdg = Ih;

  //  if (ExtractProc->t == 24){
		//float               *localRdg = (float*)malloc(ExtractProc->IwOriAngio*ExtractProc->IhOriAngio*sizeof(float))
  //                          , *localDir = (float*)malloc(ExtractProc->IwOriAngio*ExtractProc->IhOriAngio*sizeof(float))
  //                          , *localTmpRdg = (float*)malloc(ExtractProc->IwOriAngio*ExtractProc->IhOriAngio*sizeof(float));
		//short               *localGau = (short*)malloc(ExtractProc->IwOriAngio*ExtractProc->IhOriAngio*sizeof(short));
  //      COscRoiCoord        RoiHere;

  //      RoiHere.XMin = 10; RoiHere.XMax = ExtractProc->IwOriAngio-10;
  //      RoiHere.YMin = 10; RoiHere.YMax = ExtractProc->IhOriAngio-10;
  //      OscRdgMonoScaleRidgeSSE(ExtractProc->ImOriWorking, localGau, localRdg, localDir, ExtractProc->IwOriAngio, ExtractProc->IhOriAngio
		//					, &RoiHere, 10);

  //      MxOpen(); MxSetVisible(1);
  //      MxPutMatrixFloat(localRdg,ExtractProc->IwOriAngio, ExtractProc->IhOriAngio,"Rdg");
  //      MxCommand("figure(1); imagesc(Rdg'); colormap gray; axis image; axis off;");
  //      MxCommand("figure(1); imagesc(Rdg'); colormap gray; axis image; axis off;");
  //      MxCommand("save(\'T:\\PourPatrick\\Im_4080_10_Res24.mat\',\'Rdg\');");

  //      free(localRdg); free(localDir); free(localTmpRdg); free(localGau);
  //  }

	ExtractProc->pool->execute([&mutex,&OscHdl](int threadIndex, int threadCount)
	{
	    COsc                *This = (COsc*)OscHdl.Pv;
	    COscExtractProc     *ExtractProc = &(This->ExtractProc);

        int			        Iw = ExtractProc->IwRdg, Ih = ExtractProc->IhRdg;
        int			        xMin = OSC_MAX(ExtractProc->ShuttersRoiSafe.XMin, 0), xMax = OSC_MIN(ExtractProc->ShuttersRoiSafe.XMax, Iw-1);
	    int			        yMin = OSC_MAX(ExtractProc->ShuttersRoiSafe.YMin, 0), yMax = OSC_MIN(ExtractProc->ShuttersRoiSafe.YMax, Ih-1);

        int                 threadCountHere = OSC_MIN(threadCount, ExtractProc->nKernelSigs);


        if (threadIndex < ExtractProc->nKernelSigs) {

            int                 firstKernelIndice = (int)OSC_RND((float)threadIndex*ExtractProc->nKernelSigs/threadCountHere);
            int                 lastKernelIndice = (int)OSC_RND((float)(threadIndex+1)*ExtractProc->nKernelSigs/threadCountHere);
        
	        float		        *Rdg = ExtractProc->ImRdg, *Dir = ExtractProc->ImDir;

		    float               *localRdg = ExtractProc->BufsF[3* threadIndex + 0], *localDir = ExtractProc->BufsF[3* threadIndex + 1]
                                , *localTmpRdg = ExtractProc->BufsF[3* threadIndex + 2];
		    short               *localGau = ExtractProc->BufsS[threadIndex];

	        for (int k=firstKernelIndice;k <lastKernelIndice; k++) 
	        {
		        OSC_MEMZERO(localRdg, Iw*Ih);

		        OscRdgMonoScaleRidge(ExtractProc->ImSub, localGau, localRdg, localDir, Iw, Ih
							        , &ExtractProc->ShuttersRoiSafe, ExtractProc->KernelSigmas[k], localTmpRdg);

		        // diaphragm correction
		        OscDiaphragmCorrection(OscHdl, localGau, localRdg, localDir, Iw, Ih);

		        std::unique_lock<std::mutex> lock(mutex,std::defer_lock);
		
		        lock.lock();
		        {
			        int			x,y;

			        // Keep normalized max over scales
			        float		normalizationFactor = (float) exp(ExtractProc->gammaRdg*log(ExtractProc->KernelSigmas[k]));

			        for (y=yMin; y<=yMax; y++)
				        for (x=xMin; x<=xMax; x++){
					        int p = y*Iw+x;
					        float ridgeness = normalizationFactor * localRdg[p];
					        if (Rdg[p]<ridgeness) {
						        Rdg[p] = ridgeness; Dir[p] = localDir[p];
					        }
			        }
		        }
		       lock.unlock();
	        }
        }
    });
}

// ***************************************************
// ************* Monoscale extraction ****************
// ***************************************************

int OscRdgMonoScaleRidge(short *Ori, short *Gau, float *Rdg, float *Dir, int Iw, int Ih
							, COscRoiCoord *RoiCoord, float KernelSig, float *TmpRdg)
{
    int ret = 0;

	// Compute ridges and directions 
#ifdef OSC_USE_SSE_RDG
	OscRdgMonoScaleRidgeSSE(Ori, Gau, Rdg, Dir, Iw, Ih, RoiCoord, KernelSig);
#else
	ret = OscRdgComputation(Ori, Gau, Rdg, Dir, Iw, Ih, RoiCoord, KernelSig);
#endif

	// Anisotropic filtering of the ridgeness
	OscRdgAnisoFilter(Rdg,Dir,Iw, Ih, TmpRdg);

    return ret;
}

// *** Monoscale

// Differential ridge (based on determination of kernels from Hessian eigenvalues)
int OscRdgComputation(short *Ori, short *GauBuf, float *Rdg, float *Dir, int Iw, int Ih
						, COscRoiCoord *RoiCoord, float KernelSig)
{
	int    x, y;
	float lxx, lyy, lxy, L1, L2, Det;
	float Vx, Vy;

	int dBo = (int)(OSC_RDG_BORDER_MIN_FACTOR*KernelSig) + 1;
	int yMin = OSC_MAX(RoiCoord->YMin - dBo, OSC_RDG_BORDER_MIN_ABS);
	int yMax = OSC_MIN(RoiCoord->YMax + dBo, Ih-1-OSC_RDG_BORDER_MIN_ABS);
	int xMin = OSC_MAX(RoiCoord->XMin - dBo, OSC_RDG_BORDER_MIN_ABS);
	int xMax = OSC_MIN(RoiCoord->XMax + dBo, Iw-1-OSC_RDG_BORDER_MIN_ABS);

	OSC_MEMZERO(Rdg, Iw*Ih);
	OSC_MEMZERO(Dir, Iw*Ih);

    if (Ori != NULL)	// Gaussian image
    {
        if (BxIsoGaussFilterShort2D(Ori, GauBuf, KernelSig, Iw, Ih) != 0){
            printf("OscRdgComputation: allocation error in BxIsoGaussFilterShort2D\n");
            return -1;
        }
    }

	for (y=yMin; y<=yMax; y++)
		for (x=xMin; x<=xMax; x++){

			// Hessian
			lxx = (float)	(GauBuf[y*Iw+x+1]		+ GauBuf[y*Iw+x-1]		- 2*GauBuf[y*Iw+x]);
			lyy = (float)	(GauBuf[(y+1)*Iw+x]		+ GauBuf[(y-1)*Iw+x]	- 2*GauBuf[y*Iw+x]);
			lxy = 0.25f *	(GauBuf[(y+1)*Iw+x+1]	+ GauBuf[(y-1)*Iw+x-1]	- GauBuf[(y-1)*Iw+x+1] - GauBuf[(y+1)*Iw+x-1]);

			// Direction
			Vx =- (lxx - lyy); 
			Vy =- 2*lxy; 
			Dir[y*Iw+x] = (0.5f * arctan2(Vy, Vx));
			
			// Eigenvalues
			Det = (float)sqrtf( ( OSC_SQ(lxx-lyy) + 4*OSC_SQ(lxy) ) );
			L1  = 0.5f * (lxx + lyy + Det);
			L2  = 0.5f * (lxx + lyy - Det);

			if (OSC_ABS_F(L1) < OSC_ABS_F(L2))
			{
				float tmp = L1;
				L1 = L2;
				L2 = tmp;
			}

			if(L1 <= 0)
				continue;
			
			// Ridgeness
			Rdg[y*Iw+x] = L1 - L2 * OSC_RDG_LAMBDA;
		}
    return 0;
}

// *** Small filtering along the ridges direction
// -2 --> +2 pixels
void OscRdgAnisoFilter(float *Rdg, float *Dir, int Iw, int Ih, float *TmpRdg)
{
	int Border = 2;
	int x,y,l;
	float epsHere = 1E-3f;

	// Direction tables
	int DX[8] = {-1,-1,0,1,1,1,0,-1};
	int DY[8] = {0,1,1,1,0,-1,-1,-1};

	float weight[3] = {0.5f, 0.2f, 0.05f};
	float sector = (float)OSC_PI/4.f;

	OSC_MEMZERO(TmpRdg, Iw*Ih);

	for(y=Border; y<(Ih-Border); y++) {
		for(x=Border; x<(Iw-Border); x++) {	
			int X = x, Y = y;
			float val = weight[0]  * Rdg[y*Iw+x];
			
			if (val<epsHere) {
				TmpRdg[y*Iw+x] = val;
				continue;
			}
			
			// Filtering along directions in positive direction
			for(l=0; l<Border; l++) {
				int dir = (int)(Dir[Y*Iw+X]/sector) + 2;
				
				X += DX[dir]; Y += DY[dir];
				val += weight[l+1] * Rdg[Y*Iw+X];
			}		

			// Filtering along directions in negative direction
			X = x, Y = y;
			for(l=0; l<Border; l++) {
				int dir = (int)(Dir[Y*Iw+X]/sector) + 2;

				X -= DX[dir]; Y -= DY[dir];
				val += weight[l+1] * Rdg[Y*Iw+X];
			}		

			TmpRdg[y*Iw+x] = val;
		}
	}
	
	OSC_MEMCPY(Rdg, TmpRdg, Iw * Ih);
}


static float arctan2( float y, float x )
{
	const float ONEQTR_PI = (float)(OSC_PI / 4.0);
	const float THRQTR_PI = (float)(3.0 * OSC_PI / 4.0);
	float r, angle;
	float abs_y = (float)fabs(y) + 1e-10f;      // kludge to prevent 0/0 condition
	if ( x < 0.0f ) {
		r = (x + abs_y) / (abs_y - x);
		angle = THRQTR_PI;
	} else {
		r = (x - abs_y) / (x + abs_y);
		angle = ONEQTR_PI;
	}
	angle += (0.1963f * r * r - 0.9817f) * r;
	if ( y < 0.0f )
		 	return( -angle );     // negate if in quad III or IV
	else
		 	return( angle );
}
