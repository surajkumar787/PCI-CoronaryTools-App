#include "SWLibDev.h"

//#define SW_TIMING_DETAILS

#ifdef SW_TIMING_DETAILS
#include "omp.h"
#endif


// ***************************************************************************
int MultiscaleHessianNoDir(float* fOri, int Iw, int Ih, CSWRoiCoord *RoiCoord, const float* TabSig, int nbSigmas, float Gamma, float* Wid, float* Ctr, float *wkBuf1, float *wkBuf2)
{
    float *Lamb1 = wkBuf1, *fGau = wkBuf2;
#ifdef SW_TIMING_DETAILS
    double durationRidge = 0, durationReste = 0, t0, t1, t2;
#endif

    memset(Lamb1, 0, Iw*Ih*sizeof(Lamb1[0]));
    memset(fGau, 0, Iw*Ih*sizeof(Lamb1[0]));
    memset(Ctr, 0, Iw*Ih*sizeof(Ctr[0]));
    memset(Wid, 0, Iw*Ih*sizeof(Wid[0]));

    for (int kSig = 0; kSig < nbSigmas; kSig++){
        float Sig = TabSig[kSig];

#ifdef SW_TIMING_DETAILS
        double t0, t1, t2;
        t0 = omp_get_wtime();
#endif
        SWMkxIsoGaussFilterFloat2D(fOri, fGau, Sig, Iw, Ih);

        HessianNoDir(fGau, Iw, Ih, RoiCoord, Lamb1);
#ifdef SW_TIMING_DETAILS
        t1 = omp_get_wtime();
        durationRidge += 1000 * (t1-t0);
#endif

        float Sig_Gamma = static_cast<float>(pow(Sig, Gamma));
        int xMinHere = RoiCoord->XMin+1, xMaxHere = RoiCoord->XMax - 1;
        int yMinHere = RoiCoord->YMin+1, yMaxHere = RoiCoord->YMax - 1;
        int width = xMaxHere-xMinHere+1, height = yMaxHere-yMinHere+1, stride = Iw-width;

        float *ptrLambda = &Lamb1[yMinHere*Iw+xMinHere];
        float *ptrCtrl = &Ctr[yMinHere*Iw+xMinHere];
        float *ptrWidth = &Wid[yMinHere*Iw+xMinHere];
        for (int y=0; y<height; y++){
            for (int x=0; x<width; x++){
                (*ptrLambda) *= Sig_Gamma;
                if ((*ptrLambda) > (*ptrCtrl)){
                    *ptrCtrl = *ptrLambda;
                    *ptrWidth = Sig;
                }
                ptrLambda++; ptrCtrl++; ptrWidth ++;
            }
            ptrLambda += stride; ptrCtrl += stride; ptrWidth += stride;
        }

#ifdef SW_TIMING_DETAILS        
        t2 = omp_get_wtime();
        durationReste += 1000 * (t2-t1);
#endif
    }
    
#ifdef SW_TIMING_DETAILS
    printf("Duration ridges %.1f duration reste %.1f\n", durationRidge, durationReste);
#endif

    return 0;
}

// ***************************************************************************
int HessianNoDir(float* fGau, int Iw, int Ih, CSWRoiCoord *RoiCoord, float* lambda1)
{
    float Lxx, Lyy, Lxy, Det;
    memset(lambda1,0,Iw*Ih*sizeof(float));

    //for (int z = 0; z < Ih*Iw; z++)
    //    fGau[z] = SW_LIB_RND(fGau[z]);

    for (int y = RoiCoord->YMin+1; y < RoiCoord->YMax- 1; y++)
        for (int x = RoiCoord->XMin+1; x < RoiCoord->XMax - 1; x++){
            // Hessian
            Lxx = fGau[y*Iw + x + 1] - 2 * fGau[y*Iw + x] + fGau[y*Iw + x - 1];
            Lyy = fGau[(y + 1)*Iw + x] - 2 * fGau[y*Iw + x] + fGau[(y - 1)*Iw + x];
            Lxy = 0.25f*(fGau[(y + 1)*Iw + x + 1] - fGau[(y + 1)*Iw + x - 1] - fGau[(y - 1)*Iw + x + 1] + fGau[(y - 1)*Iw + x - 1]);
            // Eigenvalue
            Det = sqrtf((Lxx - Lyy)*(Lxx - Lyy) + 4 * Lxy*Lxy);
            lambda1[y*Iw + x] = 0.5f*(Lxx + Lyy + Det);
        }
  return 0;
}

// ***************************************************************************
int VertiWeight(float* Rt, int Iw, int Ih, float* weight, float sig)
{
    memset(weight, 0, Iw*sizeof(float));

  //vertical projection
  for (int y = 0; y < Ih; y++)  for (int x = 0; x < Iw; x++)   weight[x] += Rt[x + y*Iw];
  for (int x = 0; x < Iw; x++)    weight[x] /= (float)Ih;

  //horizontal smoothing
  SWMkxIsoGaussFilterFloat2D(weight, weight, sig, Iw, 1);

  //normalization
  float weightMax = weight[0];
  for (int x = 0; x < Iw; x++)  if (weight[x]>weightMax)    weightMax = weight[x];
  for (int x = 0; x < Iw; x++)    weight[x] /= weightMax;

  return 0;
}


// ***************************************************************************
int ImDilate(unsigned char *ImIn, int Iw, int Ih, int xMin, int xMax, int yMin, int yMax, unsigned char* ImOut, int strelSize, unsigned char* buf)
{
    int xMinHere, xMaxHere, width, yMinHere, yMaxHere, height, stride;

    memcpy(buf, ImIn, Iw*Ih*sizeof(unsigned char));
    for (int i = -strelSize; i <= strelSize; i++){
        //for (int y=yMin; y<= yMax; y++)
        //    for (int x=SW_LIB_MAX(strelSize,xMin); x<= SW_LIB_MIN(Iw-1-strelSize,xMax); x++)
        //        buf[y*Iw+x] += ImIn[y*Iw+x+i];
        xMinHere = SW_LIB_MAX(strelSize,xMin); xMaxHere = SW_LIB_MIN(Iw-1-strelSize,xMax); width = xMaxHere-xMinHere+1; stride = Iw-width;
        yMinHere = yMin; yMaxHere = yMax; height = yMaxHere-yMinHere+1;
        unsigned char* ptrBuf = &buf[yMinHere*Iw+xMinHere];
        unsigned char* ptrIm = &ImIn[yMinHere*Iw+xMinHere+i];
        for (int yy =0; yy<height; yy++){
            for (int xx =0; xx<width; xx++){
                *ptrBuf += *ptrIm; ptrBuf++; ptrIm++;
            }
            ptrBuf += stride; ptrIm += stride;
        }
    }
  
  
    memcpy(ImOut, buf, Iw*Ih*sizeof(unsigned char));
    for (int i = -strelSize; i <= strelSize; i++){
        //for (int y=SW_LIB_MAX(strelSize,yMin); y<= SW_LIB_MIN(Ih-1-strelSize,yMax); y++)
        //    for (int x=xMin; x<= xMax; x++)
        //        ImOut[y*Iw+x] += buf[(y+i)*Iw+x];
        xMinHere = xMin; xMaxHere = xMax; width = xMaxHere-xMinHere+1; stride = Iw-width;
        yMinHere = SW_LIB_MAX(strelSize,yMin); yMaxHere = SW_LIB_MIN(Ih-1-strelSize,yMax); height = yMaxHere-yMinHere+1;
        unsigned char* ptrImOut = &ImOut[yMinHere*Iw+xMinHere];
        unsigned char* ptrBuf = &buf[(yMinHere+i)*Iw+xMinHere];
        for (int yy =0; yy<height; yy++){
            for (int xx =0; xx<width; xx++){
                *ptrImOut += *ptrBuf; ptrImOut++; ptrBuf++;
            }
            ptrImOut += stride; ptrBuf += stride;
        }
    }

    for (int y=yMin; y<= yMax; y++)
        for (int x=xMin; x<= xMax; x++)
            if (ImOut[y*Iw+x] > 0)
                ImOut[y*Iw+x] = 1;

  return 0;
}

// ***************************************************************************
int SWConnectedComponents(const unsigned char *MaskIn, int Iw, int Ih, int xMin, int xMax, int yMin, int yMax, int NbRangePix
                          , short *stack_x, short *stack_y, short *stack_x0, short *stack_y0, int *LabOut, int *nbPelPerLab, int *NbLabelsOut)
{
	int x, y, x0, y0, etiquette = 2;
	int recursion_level, max_recursion_level;

	max_recursion_level = (xMax-xMin+1)*(yMax-yMin+1);

	// initialization of LabOut to ThrIn
	for(y=0; y<Ih; y++)
		for(x=0; x<Iw; x++)
			LabOut[y*Iw+x] = (int)MaskIn[y*Iw+x];

	// *** Loop on the image
	// When new unlabeled pixel met, initialize new label and perform its labeling completely.
	// It is done by storing the next points to discuss into the different stacks

	for(y = yMin; y <= yMax; y++)
		for(x = xMin; x <= xMax; x++){

			// * Start new label
			if(MaskIn[y*Iw+x]==1 && LabOut[y*Iw+x]==1){

				// Initializations
				int   nbPixLabel = 0;
				recursion_level = 0; x0 = x; y0 = y;

				// * Labeling the complete image with that label

				// Pixel of interest
				// 1- being thresholded and unlabeled 2- being on the mask 
				// or 3- if there are points to be processed stored in the stacks
				while (LabOut[y*Iw+x]==1 || recursion_level>0){

					// Case 1- thresholded pixel
					if (LabOut[y*Iw+x]==1){

						// Labelling pixel
                        nbPixLabel++;
						LabOut[y*Iw+x] = etiquette;

						// Store the current pixel as a point to be studied later
						// And iterate x and y
						if (recursion_level<max_recursion_level){

							// Store the current pixel as a point to be studied later
							stack_x[recursion_level] = x; stack_y[recursion_level] = y;
							stack_x0[recursion_level] = x0; stack_y0[recursion_level] = y0;

							// And iterate x and y
							x0 = x; y0 = y;
							x = SW_LIB_MAX(x0-NbRangePix, xMin); y = SW_LIB_MAX(y0-NbRangePix, yMin);

							recursion_level++;
						}
						continue;
					}
                    
					// Increment x and y in the search range
					x++;
					if (x>SW_LIB_MIN(x0+NbRangePix,xMax))
					{
						y++;
						if (y>SW_LIB_MIN(y0+NbRangePix,yMax))
						{       
							// We reach the end of the search region (from x0,y0)
							// We change the reference to the stacked x,y (and x0, y0)                      

							//end of scanning in this recursion level
							recursion_level--;

							// restore i, j, i0 and j0
							x  = stack_x [recursion_level];
							y  = stack_y [recursion_level];
							x0 = stack_x0[recursion_level];
							y0 = stack_y0[recursion_level];

							continue;
						}
						else
						{
							x= SW_LIB_MAX(x0-NbRangePix, xMin);
						}                              
					}
				}
                nbPelPerLab[etiquette-2] = nbPixLabel;
				etiquette++;
			}
		}

	*NbLabelsOut = etiquette-2;

	return 0;
}

// ***************************************************************************
void SWConnectedComponentsAndDistance(CSWHdl *Hdl, int *LabOut, int *nbPixPerLab, int *NbLabelsOut)
{
    CSW* hdl = (CSW*)(Hdl->Pv);
    int Iw = hdl->Iw, Ih = hdl->Ih;
    int xMin = hdl->Roi.XMin, xMax = hdl->Roi.XMax, yMin = hdl->Roi.YMin, yMax = hdl->Roi.YMax;
    int *nbPixPerLabDist1 = hdl->WkBufI2;

    int nbLabsDist1;

    // * Connected components on loose mask
    SWConnectedComponents((const unsigned char *)hdl->MaskSeqLoose, Iw, Ih, xMin, xMax, yMin, yMax, 1
                          , hdl->WkBuf1S, hdl->WkBuf2S, hdl->WkBuf3S, hdl->WkBuf4S, LabOut, nbPixPerLabDist1, &nbLabsDist1);

    if (nbLabsDist1 > SW_LAB_REGION_FOR_EFFICIENT_REGION_MERGING){
        // Too many labels - regular connceted components more efficient
        SWConnectedComponents((const unsigned char *)hdl->MaskSeqLoose, Iw, Ih, xMin, xMax, yMin, yMax, hdl->RangeSameRegion
                              , hdl->WkBuf1S, hdl->WkBuf2S, hdl->WkBuf3S, hdl->WkBuf4S, LabOut, nbPixPerLab, NbLabelsOut);
    } else {

        // * Compute labels ROI and fronteer image
        int *LabFronteerDist1      = hdl->WkBufI4;
        int *xMinLab = hdl->WVectI1, *xMaxLab = hdl->WVectI2, *yMinLab = hdl->WVectI3, *yMaxLab = hdl->WVectI4;
        for (int lab=0; lab <nbLabsDist1; lab++){
            xMinLab[lab] = xMax; xMaxLab[lab] = xMin; yMinLab[lab] = yMax; yMaxLab[lab] = yMin;
        }
        memset(LabFronteerDist1,0,Iw*Ih*sizeof(int));
        for (int y=SW_LIB_MAX(1,yMin); y<=SW_LIB_MIN(Ih-2,yMax); y++)
            for (int x=SW_LIB_MAX(1,xMin); x<=SW_LIB_MIN(Iw-2,xMax); x++)
                if (LabOut[y*Iw+x] > 1){
                    xMinLab[LabOut[y*Iw+x]-2] = SW_LIB_MIN(x, xMinLab[LabOut[y*Iw+x]-2]);
                    xMaxLab[LabOut[y*Iw+x]-2] = SW_LIB_MAX(x, xMaxLab[LabOut[y*Iw+x]-2]);
                    yMinLab[LabOut[y*Iw+x]-2] = SW_LIB_MIN(y, yMinLab[LabOut[y*Iw+x]-2]);
                    yMaxLab[LabOut[y*Iw+x]-2] = SW_LIB_MAX(y, yMaxLab[LabOut[y*Iw+x]-2]);

                    if ( (LabOut[(y-1)*Iw+x] <= 1) || (LabOut[(y+1)*Iw+x] <= 1)
                        || (LabOut[y*Iw+x-1] <= 1) || (LabOut[y*Iw+x+1] <= 1) )
                        LabFronteerDist1[y*Iw+x] = LabOut[y*Iw+x];
                } 

        // * Checking which labs have to be merged
        bool * merge = (bool*)hdl->WMatrixBool;
        memset(merge, 0, nbLabsDist1*nbLabsDist1*sizeof(bool));
        for (int lab=0; lab<nbLabsDist1; lab++)
            merge[lab*nbLabsDist1+lab] = true;
        int limDist2 = hdl->RangeSameRegion*hdl->RangeSameRegion;

        for (int lab=0; lab<nbLabsDist1; lab++){

            for (int llab=lab+1; llab<nbLabsDist1; llab++){
                // Should they be merged?

                if ( (xMaxLab[llab] >= xMinLab[lab]-hdl->RangeSameRegion) && (xMinLab[llab] <= xMaxLab[lab]+hdl->RangeSameRegion)
                    && (yMaxLab[llab] >= yMinLab[lab]-hdl->RangeSameRegion) && (yMinLab[llab] <= yMaxLab[lab]+hdl->RangeSameRegion) ){
                        // Otherwise, the bounding boxes are further than RangeSameRegion --> No merging possible

                        for (int y=yMinLab[lab]; y<=yMaxLab[lab]; y++)
                            for (int x=xMinLab[lab]; x<=xMaxLab[lab]; x++)
                                if (LabOut[y*Iw+x]-2 == lab){

                                    for (int yy=SW_LIB_MAX(yMinLab[llab],y-hdl->RangeSameRegion); yy<=SW_LIB_MIN(yMaxLab[llab],y+hdl->RangeSameRegion); yy++)
                                        for (int xx=SW_LIB_MAX(xMinLab[llab],x-hdl->RangeSameRegion); xx<=SW_LIB_MIN(xMaxLab[llab],x+hdl->RangeSameRegion); xx++)
                                            // Region compatible with both labels ROI (where pixls distant by less than RangeSameRegion are possible)
                                            if ( (LabOut[yy*Iw+xx]-2 == llab) && ((xx-x)*(xx-x)+(yy-y)*(yy-y) <= limDist2) ){
                                                merge[lab*nbLabsDist1+llab] = true;
                                                merge[llab*nbLabsDist1+lab] = true;
                                                break;
                                            }

                                    if (merge[lab*nbLabsDist1+llab])
                                        // Regions already merged
                                        break;
                                }
                }
            }
        }

        // * Establish new labelling
        // Translating the merge matrix into a vector of new labels, recursively
        int *newLabs = hdl->WVectI3;
        memset(newLabs, 0, nbLabsDist1*sizeof(int));
        memset(nbPixPerLab, 0, nbLabsDist1*sizeof(int));
        int currentLab = 2;
        for (int lab=0; lab<nbLabsDist1; lab++)
            if (newLabs[lab] == 0){
                // Otherwise, already processed (merged with previous label)
                RelabelRecursive(lab, currentLab, nbLabsDist1, (const bool*)merge, (const int*)nbPixPerLabDist1, newLabs, nbPixPerLab);
                currentLab++;
            }
        *NbLabelsOut = currentLab-2;

        // * Final relabel
        for (int y=yMin; y<=yMax; y++)
            for (int x=xMin; x<=xMax; x++)
                if (LabOut[y*Iw+x] > 1){
                    LabOut[y*Iw+x] = newLabs[LabOut[y*Iw+x]-2];
                } 
    }
}

void RelabelRecursive(int ind, int newLab, int maxInd, const bool*merge, const int *nbPixPerLabPrev, int *newLabs, int *nbPixPerLab){
    
    newLabs[ind] = newLab;
    nbPixPerLab[newLab-2] += nbPixPerLabPrev[ind-2];
    for (int i=0; i<maxInd; i++)
        if ( (newLabs[i]== 0) && (merge[ind*maxInd+i]) )
            RelabelRecursive(i, newLab, maxInd, merge, nbPixPerLabPrev, newLabs, nbPixPerLab);
}

// ***************************************************************************
float SWMedianPosRidgeByHisto(float *Rdg, int Iw, int Ih, int xMin, int xMax, int yMin, int yMax, int *histBuf, int nbBins){

    float rdgMax = 0.f;
    for (int y=yMin; y<=yMax; y++)
        for (int x=xMin; x<=xMax; x++){
            rdgMax = SW_LIB_MAX(rdgMax, Rdg[y*Iw+x]);
        }

    if (rdgMax < 0)
        return -1.f;
    else {

        memset(histBuf, 0, nbBins*sizeof(float));
        int nbElts = 0;
        float coeff = (float)(nbBins-1)/rdgMax;

        for (int y=yMin; y<=yMax; y++)
            for (int x=xMin; x<=xMax; x++)
                if (Rdg[y*Iw+x] > 0){
                    histBuf[SW_LIB_RND(coeff * Rdg[y*Iw+x])] ++;
                    nbElts++;
                }

        int nbEltsCum = 0;
        for (int i=0; i<nbBins; i++){
            nbEltsCum += histBuf[i];
            if (nbEltsCum >= nbElts/2){
                return (i*rdgMax/nbBins);
            }
        }
    }
    return -1.f;
}

// ***************************************************************************
void SWSubSample(short *ImIn, int IwIn, int IhIn, int SSFactor, short *ImOut){

    int IhOut = IhIn/SSFactor, IwOut = IwIn/SSFactor;
    //short *Buf = (short*)malloc(IwIn*IhIn*sizeof(short));

    //SWMkxIsoGaussFilterShort2D(ImIn, Buf, 1.4f, IwIn, IhIn);

    //for (int y=0; y<IhOut; y++)
    //    for (int x=0; x<IwOut; x++)
    //        ImOut[y*IwOut+x] = Buf[(SSFactor*y)*IwIn+SSFactor*x];

    //free(Buf);

    for (int y=0; y<IhOut; y++)
        for (int x=0; x<IwOut; x++){
            ImOut[y*IwOut+x] = ImIn[(SSFactor*y)*IwIn+SSFactor*x];

            for (int yy=SSFactor*y; yy<SSFactor*(y+1); yy++)
                for (int xx=SSFactor*x; xx<SSFactor*(x+1); xx++){
                    ImOut[y*IwOut+x] = SW_LIB_MIN(ImOut[y*IwOut+x],ImIn[yy*IwIn+xx]);
                }
        }
}

