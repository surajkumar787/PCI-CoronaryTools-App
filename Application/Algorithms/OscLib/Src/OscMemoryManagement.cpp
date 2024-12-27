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

#include <crtdbg.h>
#include <malloc.h>

#include <OscDev.h>
#include <OscVersion.h>
#include <OscErrorMessages.h>

#define CHECK_WRITINGREADING_ERROR(num, targetSize, inputFile, funcName, Reason) if (num != targetSize){	ErrStartErrorReportFromLeaf(This->ErrHdl, funcName, OSC_ERR_WRITEREADTOMEMORY, Reason); fclose(inputFile); return OSC_ERR_WRITEREADTOMEMORY;}
#define WRITE_VALARRAY_TO_STREAM_AND_PROGRESS(arrVal, nbElts, type, streamUC) \
    memcpy(streamUC, arrVal, nbElts*sizeof(type));\
    streamUC += nbElts * sizeof(type) / sizeof(unsigned char);
#define READ_VALARRAY_FROM_STREAM_AND_PROGRESS(streamUC, nbElts, type, arr) \
    memcpy(arr, streamUC, nbElts*sizeof(type));\
    streamUC += nbElts * sizeof(type) / sizeof(unsigned char);    
#define CHECK_READINGERROR(val, valMin, valMax, valName, funcName)  if ( (val < valMin) || (val > valMax) ){ char ReasonHere[128]; sprintf_s(ReasonHere, 128, "Field %s %d out of acceptable range [%d,%d]. Memory probably corrupted", valName, val, valMin, valMax); ErrStartErrorReportFromLeaf(This->ErrHdl, funcName, OSC_ERR_LOAD_MEMORY, ReasonHere); return OSC_ERR_LOAD_MEMORY;}
#define CHECK_READINGERROR_F(val, valMin, valMax, valName, funcName)  if ( (val < valMin) || (val > valMax) ){ char ReasonHere[128]; sprintf_s(ReasonHere, 128, "Field %s %f out of acceptable range [%f,%f]. Memory probably corrupted", valName, val, valMin, valMax); ErrStartErrorReportFromLeaf(This->ErrHdl, funcName, OSC_ERR_LOAD_MEMORY, ReasonHere); return OSC_ERR_LOAD_MEMORY;}

#define OSC_IM_IW_MIN       128
#define OSC_IM_IW_MAX       4096
#define OSC_IM_PROC_IW_MIN  48
#define OSC_MAX_NB_PROC     32
#define OSC_MAX_SSFACTOR    16
#define OSC_MAX_NB_ANGIO_IMAGES 1000
#define OSC_MAX_PYR_LEVEL 10
#define OSC_MIN_FRAME_RATE 0.01f
#define OSC_MAX_FRAME_RATE 1000.f
#define OSC_MIN_ISO_PIXSIZE 0.f
#define OSC_MAX_ISO_PIXSIZE 10.f
#define OSC_MAX_TABPOS_PIX 10000

// ****************************************************
// READ / WRITE HANDLE NECESSARY PARTS TO MEMORY
//
int OscSaveToMemory(COscHdl OscHdl, void* memOut){

	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);
	COscExtractSequence *ExtractSequence = &(This->ExtractSequence);
	int					IwProc = ExtractProc->IwProcAngio, IhProc = ExtractProc->IhProcAngio
                        , IwOri = ExtractProc->IwOriAngio, IhOri = ExtractProc->IhOriAngio;
	
	int					scalarValI[43];
	float				scalarValF[10];
	unsigned char       * ptrUC = (unsigned char *)memOut;

	// For version check
	scalarValI[0] = (int)OSC_VERSION_MAJOR; scalarValI[1] = (int)OSC_VERSION_MINOR; scalarValI[2] = (int)OSC_VERSION_PATCH; scalarValI[3] = (int)OSC_VERSION_WRITE_HDL_TO_DISK;
	// Angio processed
	scalarValI[4] = ExtractProc->angioProcessed;
    WRITE_VALARRAY_TO_STREAM_AND_PROGRESS(scalarValI, 5, int, ptrUC);

	if (ExtractProc->angioProcessed){

        scalarValI[0] = ExtractProc->IwOriAngio; scalarValI[1] = ExtractProc->IhOriAngio; scalarValI[2] = ExtractProc->maxAngioLengthAlloc;
		scalarValI[3] = (int)ExtractProc->CArmAnglesAngioGiven; scalarValI[4] = (int)ExtractProc->ShuttersRoiPosGiven; scalarValI[5] = (int)ExtractProc->angioProcessed;
		scalarValI[6] = ExtractProc->ShuttersRoiFullScale.XMin; scalarValI[7] = ExtractProc->ShuttersRoiFullScale.XMax;
		scalarValI[8] = ExtractProc->ShuttersRoiFullScale.YMin; scalarValI[9] = ExtractProc->ShuttersRoiFullScale.YMax;
		scalarValI[10] = ExtractProc->ShuttersRoiSafeFullScale.XMin; scalarValI[11] = ExtractProc->ShuttersRoiSafeFullScale.XMax;
		scalarValI[12] = ExtractProc->ShuttersRoiSafeFullScale.YMin; scalarValI[13] = ExtractProc->ShuttersRoiSafeFullScale.YMax;
		scalarValI[14] = ExtractProc->ShuttersRoi.XMin; scalarValI[15] = ExtractProc->ShuttersRoi.XMax;
		scalarValI[16] = ExtractProc->ShuttersRoi.YMin; scalarValI[17] = ExtractProc->ShuttersRoi.YMax;
		scalarValI[18] = ExtractProc->ShuttersRoiSafe.XMin; scalarValI[19] = ExtractProc->ShuttersRoiSafe.XMax;
		scalarValI[20] = ExtractProc->ShuttersRoiSafe.YMin; scalarValI[21] = ExtractProc->ShuttersRoiSafe.YMax;
		scalarValI[22] = ExtractProc->NbProc; scalarValI[23] = ExtractProc->SSFactor;
		scalarValI[24] = ExtractProc->IwProcAngio; scalarValI[25] = ExtractProc->IhProcAngio; 
		scalarValI[26] = ExtractProc->detectedDiaphragm; scalarValI[27] = ExtractProc->detectedCatheter;
		scalarValI[28] = ExtractSequence->nExtractPictures; scalarValI[29] = ExtractSequence->CardiacCycleStart;
		scalarValI[30] = ExtractSequence->CardiacCycleEnd; scalarValI[31] = ExtractSequence->PyrNbBands;
		scalarValI[32] = ExtractProc->tableInfoFromtheStart; scalarValI[33] = ExtractProc->t;
		scalarValI[34] = ExtractProc->ShuttersRoiFullScaleRef.XMin; scalarValI[35] = ExtractProc->ShuttersRoiFullScaleRef.XMax;
		scalarValI[36] = ExtractProc->ShuttersRoiFullScaleRef.YMin; scalarValI[37] = ExtractProc->ShuttersRoiFullScaleRef.YMax;
		scalarValI[38] = (int)ExtractProc->CArmAnglesAngioGivenRef; 
        if (ExtractProc->TableInformationGiven == 1){
            scalarValI[39] = ExtractProc->OrigTablePosXPix; scalarValI[40] = ExtractProc->OrigTablePosYPix;
        } else {
            scalarValI[39] = 0; scalarValI[40] = 0;
        }
        scalarValI[41] = ExtractProc->lastDeltaXSubAngio;
        scalarValI[42] = ExtractProc->lastDeltaYSubAngio;

        scalarValF[0] = ExtractProc->frameRateAngio; scalarValF[1] = ExtractProc->frameRateFluoro;
		scalarValF[2] = ExtractProc->isoCenterPixSize; scalarValF[3] = ExtractProc->RotAngio; scalarValF[4] = ExtractProc->AngAngio;
		scalarValF[5] = ExtractProc->isoCenterPixSizeRef; scalarValF[6] = ExtractProc->RotAngioRef; scalarValF[7] = ExtractProc->AngAngioRef;
		scalarValF[8] = ExtractProc->OrigTablePosXMMPlane; scalarValF[9] = ExtractProc->OrigTablePosYMMPlane;

        WRITE_VALARRAY_TO_STREAM_AND_PROGRESS(scalarValI, 43, int, ptrUC);
        WRITE_VALARRAY_TO_STREAM_AND_PROGRESS(scalarValF, 10, float, ptrUC);

		if (ExtractProc->detectedDiaphragm){
			scalarValF[0] = ExtractProc->DiaphragmParams[0]; scalarValF[1] = ExtractProc->DiaphragmParams[1]; scalarValF[2] = ExtractProc->DiaphragmParams[2];
            scalarValF[3] = ExtractProc->DiaphragmParamsRefAngio[0]; 
            scalarValF[4] = ExtractProc->DiaphragmParamsRefAngio[1]; 
		    scalarValF[5] = ExtractProc->DiaphragmParams[2];
            WRITE_VALARRAY_TO_STREAM_AND_PROGRESS(scalarValF, 6, float, ptrUC);
		}
		for (int i=0; i<=ExtractSequence->PyrNbBands; i++){
			scalarValI[0] = ExtractSequence->PyrW[i]; scalarValI[1] = ExtractSequence->PyrH[i];
            WRITE_VALARRAY_TO_STREAM_AND_PROGRESS(scalarValI, 2, int, ptrUC);
		}

		for (int frame = ExtractSequence->CardiacCycleStart; frame <= ExtractSequence->CardiacCycleEnd; frame ++){
			COscExtractPicture ExtractPic = ExtractSequence->ExtractPictures[frame];

			scalarValI[0] = ExtractPic.deltaTabXSub; scalarValI[1] = ExtractPic.deltaTabYSub;
            WRITE_VALARRAY_TO_STREAM_AND_PROGRESS(scalarValI, 2, int, ptrUC);

            WRITE_VALARRAY_TO_STREAM_AND_PROGRESS(ExtractSequence->dXAngioToAngio[frame], ExtractProc->maxAngioLengthAlloc, int, ptrUC);
            WRITE_VALARRAY_TO_STREAM_AND_PROGRESS(ExtractSequence->dYAngioToAngio[frame], ExtractProc->maxAngioLengthAlloc, int, ptrUC);

            WRITE_VALARRAY_TO_STREAM_AND_PROGRESS(ExtractPic.ImOverlayMask, IwProc*IhProc, unsigned char, ptrUC);

			for (int i=0; i<= ExtractSequence->PyrNbBands; i++){
                memcpy(ptrUC, (void **)(ExtractPic.ImPyrDir1D[i])
                    , ExtractSequence->PyrW[i]*ExtractSequence->PyrH[i]*sizeof(ExtractPic.ImPyrDir1D[i][0]));
                ptrUC += ExtractSequence->PyrW[i]*ExtractSequence->PyrH[i] * sizeof(ExtractPic.ImPyrDir1D[i][0]) / sizeof(unsigned char);
            }
		}

		// Sternal wires
        scalarValI[0] = ExtractProc->SWRes.nbPixMask;
        WRITE_VALARRAY_TO_STREAM_AND_PROGRESS(scalarValI, 1, int, ptrUC);
        if (ExtractProc->SWRes.nbPixMask > 0){
            WRITE_VALARRAY_TO_STREAM_AND_PROGRESS(ExtractProc->SWRes.SWMask, IwOri*IhOri, unsigned char, ptrUC);
        }
	}

	return 0;
}

// ****************************************************
//
int OscLoadFromMemory(COscHdl OscHdl, void* memIn){

	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);
	COscExtractSequence *ExtractSequence = &(This->ExtractSequence);
	char				Reason[128];
	
	int					scalarValI[43];
	float				scalarValF[10];
	unsigned char       * ptrUC = (unsigned char *)memIn;

	// Version check
    READ_VALARRAY_FROM_STREAM_AND_PROGRESS(ptrUC, 5, int, scalarValI);	
    if ( (scalarValI[0] != OSC_VERSION_MAJOR) || (scalarValI[1] != OSC_VERSION_MINOR) || (scalarValI[2] != OSC_VERSION_PATCH) 
			|| (scalarValI[3] != OSC_VERSION_WRITE_HDL_TO_DISK) ) {
        ExtractProc->FatalErrorOnPreviousImage = 1;
		sprintf_s(Reason, 128, "Incompatible version between OscLoadFromMemory (Osc%d.%d.%d, write hdl version %d) and OscLoadFromMemory (Osc%d.%d.%d, write hdl version %d)"
				, scalarValI[0], scalarValI[1], scalarValI[2], scalarValI[3], (int)OSC_VERSION_MAJOR, (int)OSC_VERSION_MINOR, (int)OSC_VERSION_PATCH, (int)OSC_VERSION_WRITE_HDL_TO_DISK);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscLoadFromMemory", OSC_ERR_DLL_VERSION, Reason);
		return OSC_ERR_DLL_VERSION;
	}

	// Angio processed
    ExtractProc->angioProcessed = scalarValI[4];
    CHECK_READINGERROR(ExtractProc->angioProcessed, 0, 1, "angioProcessed", "OscLoadFromMemory");

	if (ExtractProc->angioProcessed){
        int	IwProc, IhProc, IwOri, IhOri;

        READ_VALARRAY_FROM_STREAM_AND_PROGRESS(ptrUC, 43, int, scalarValI);	

		ExtractProc->IwOriAngio = scalarValI[0]; ExtractProc->IhOriAngio = scalarValI[1];
		ExtractProc->CArmAnglesAngioGiven = (unsigned char)scalarValI[3]; ExtractProc->ShuttersRoiPosGiven = (unsigned char)scalarValI[4];
		ExtractProc->angioProcessed = (unsigned char)scalarValI[5];
		ExtractProc->ShuttersRoiFullScale.XMin = scalarValI[6]; ExtractProc->ShuttersRoiFullScale.XMax = scalarValI[7];
		ExtractProc->ShuttersRoiFullScale.YMin = scalarValI[8]; ExtractProc->ShuttersRoiFullScale.YMax = scalarValI[9];
		ExtractProc->ShuttersRoiSafeFullScale.XMin = scalarValI[10]; ExtractProc->ShuttersRoiSafeFullScale.XMax = scalarValI[11];
		ExtractProc->ShuttersRoiSafeFullScale.YMin = scalarValI[12]; ExtractProc->ShuttersRoiSafeFullScale.YMax = scalarValI[13];
		ExtractProc->ShuttersRoi.XMin = scalarValI[14]; ExtractProc->ShuttersRoi.XMax = scalarValI[15];
		ExtractProc->ShuttersRoi.YMin = scalarValI[16]; ExtractProc->ShuttersRoi.YMax = scalarValI[17];
		ExtractProc->ShuttersRoiSafe.XMin = scalarValI[18]; ExtractProc->ShuttersRoiSafe.XMax = scalarValI[19];
		ExtractProc->ShuttersRoiSafe.YMin = scalarValI[20]; ExtractProc->ShuttersRoiSafe.YMax = scalarValI[21];
		ExtractProc->NbProc = scalarValI[22]; ExtractProc->SSFactor = scalarValI[23];
		ExtractProc->IwProcAngio = scalarValI[24]; ExtractProc->IhProcAngio = scalarValI[25];
		ExtractProc->detectedDiaphragm = scalarValI[26]; ExtractProc->detectedCatheter = scalarValI[27];
		ExtractSequence->nExtractPictures = scalarValI[28]; ExtractSequence->CardiacCycleStart = scalarValI[29];
		ExtractSequence->CardiacCycleEnd = scalarValI[30]; ExtractSequence->PyrNbBands = scalarValI[31];
		ExtractProc->tableInfoFromtheStart = scalarValI[32]; ExtractProc->t = scalarValI[33];
		ExtractProc->ShuttersRoiFullScaleRef.XMin = scalarValI[34]; ExtractProc->ShuttersRoiFullScaleRef.XMax = scalarValI[35];
		ExtractProc->ShuttersRoiFullScaleRef.YMin = scalarValI[36]; ExtractProc->ShuttersRoiFullScaleRef.YMax = scalarValI[37];
		ExtractProc->CArmAnglesAngioGivenRef = (unsigned char)scalarValI[38];

        CHECK_READINGERROR(ExtractProc->IwOriAngio, OSC_IM_IW_MIN, OSC_IM_IW_MAX, "IwOriAngio", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->IhOriAngio, OSC_IM_IW_MIN, OSC_IM_IW_MAX, "IhOriAngio", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->CArmAnglesAngioGiven, 0, 1, "CArmAnglesAngioGiven", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->ShuttersRoiPosGiven, 0, 1, "ShuttersRoiPosGiven", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->angioProcessed, 0, 1, "angioProcessed", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->ShuttersRoiFullScale.XMin, 0, ExtractProc->IwOriAngio, "ShuttersRoiFullScale.XMin", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->ShuttersRoiFullScale.XMax, 0, ExtractProc->IwOriAngio, "ShuttersRoiFullScale.XMax", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->ShuttersRoiFullScale.YMin, 0, ExtractProc->IhOriAngio, "ShuttersRoiFullScale.YMin", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->ShuttersRoiFullScale.YMax, 0, ExtractProc->IhOriAngio, "ShuttersRoiFullScale.YMax", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->ShuttersRoiSafeFullScale.XMin, 0, ExtractProc->IwOriAngio, "ShuttersRoiSafeFullScale.XMin", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->ShuttersRoiSafeFullScale.XMax, 0, ExtractProc->IwOriAngio, "ShuttersRoiSafeFullScale.XMax", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->ShuttersRoiSafeFullScale.YMin, 0, ExtractProc->IhOriAngio, "ShuttersRoiSafeFullScale.YMin", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->ShuttersRoiSafeFullScale.YMax, 0, ExtractProc->IhOriAngio, "ShuttersRoiSafeFullScale.YMax", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->ShuttersRoi.XMin, 0, ExtractProc->IwOriAngio, "ShuttersRoi.XMin", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->ShuttersRoi.XMax, 0, ExtractProc->IwOriAngio, "ShuttersRoi.XMax", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->ShuttersRoi.YMin, 0, ExtractProc->IhOriAngio, "ShuttersRoi.YMin", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->ShuttersRoi.YMax, 0, ExtractProc->IhOriAngio, "ShuttersRoi.YMax", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->ShuttersRoiSafe.XMin, 0, ExtractProc->IwOriAngio, "ShuttersRoiSafe.XMin", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->ShuttersRoiSafe.XMax, 0, ExtractProc->IwOriAngio, "ShuttersRoiSafe.XMax", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->ShuttersRoiSafe.YMin, 0, ExtractProc->IhOriAngio, "ShuttersRoiSafe.YMin", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->ShuttersRoiSafe.YMax, 0, ExtractProc->IhOriAngio, "ShuttersRoiSafe.YMax", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->NbProc, 1, OSC_MAX_NB_PROC, "NbProc", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->SSFactor, 1, OSC_MAX_SSFACTOR, "SSFactor", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->IwProcAngio, OSC_IM_PROC_IW_MIN, OSC_IM_IW_MAX, "IwProcAngio", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->IhProcAngio, OSC_IM_PROC_IW_MIN, OSC_IM_IW_MAX, "IhProcAngio", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->detectedDiaphragm, 0, 1, "detectedDiaphragm", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->detectedCatheter, 0, 3, "detectedCatheter", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractSequence->nExtractPictures, 0, OSC_MAX_NB_ANGIO_IMAGES, "nExtractPictures", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractSequence->CardiacCycleStart, 0, OSC_MAX_NB_ANGIO_IMAGES, "CardiacCycleStart", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractSequence->CardiacCycleEnd, 0, OSC_MAX_NB_ANGIO_IMAGES, "CardiacCycleEnd", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractSequence->PyrNbBands, 1, OSC_MAX_PYR_LEVEL, "PyrNbBands", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->tableInfoFromtheStart, -1, 1, "tableInfoFromtheStart", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->t, -1, OSC_MAX_NB_ANGIO_IMAGES, "t", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->ShuttersRoiFullScaleRef.XMin, 0, ExtractProc->IwOriAngio, "ShuttersRoiFullScaleRef.XMin", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->ShuttersRoiFullScaleRef.XMax, 0, ExtractProc->IwOriAngio, "ShuttersRoiFullScaleRef.XMax", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->ShuttersRoiFullScaleRef.YMin, 0, ExtractProc->IhOriAngio, "ShuttersRoiFullScaleRef.YMin", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->ShuttersRoiFullScaleRef.YMax, 0, ExtractProc->IhOriAngio, "ShuttersRoiFullScaleRef.YMax", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->CArmAnglesAngioGivenRef, 0, 1, "CArmAnglesAngioGivenRef", "OscLoadFromMemory");

        if ( (ExtractProc->maxAngioLengthAlloc > 0) && (ExtractProc->maxAngioLengthAlloc != scalarValI[2]) ){
            char Reason[128];
            sprintf_s(Reason, 128, "Memory allocated incoherent with load values (maxAngioLength %d allocated, %d needed)", 
				ExtractProc->maxAngioLengthAlloc, scalarValI[2]);
            ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, Reason);
            return OSC_ERR_MEM_ALLOC;
        }
        ExtractProc->maxAngioLengthAlloc = scalarValI[2];
        ExtractProc->OrigTablePosXPix = scalarValI[39]; ExtractProc->OrigTablePosYPix = scalarValI[40];
        CHECK_READINGERROR(ExtractProc->OrigTablePosXPix, -OSC_MAX_TABPOS_PIX, OSC_MAX_TABPOS_PIX, "OrigTablePosXPix", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->OrigTablePosYPix, -OSC_MAX_TABPOS_PIX, OSC_MAX_TABPOS_PIX, "OrigTablePosYPix", "OscLoadFromMemory");
        
        ExtractProc->lastDeltaXSubAngio = scalarValI[41]; ExtractProc->lastDeltaYSubAngio = scalarValI[42];
        CHECK_READINGERROR(ExtractProc->lastDeltaXSubAngio, -OSC_MAX_TABPOS_PIX, OSC_MAX_TABPOS_PIX, "lastDeltaXSubAngio", "OscLoadFromMemory");
        CHECK_READINGERROR(ExtractProc->lastDeltaYSubAngio, -OSC_MAX_TABPOS_PIX, OSC_MAX_TABPOS_PIX, "lastDeltaYSubAngio", "OscLoadFromMemory");

        IwProc = ExtractProc->IwProcAngio; IhProc = ExtractProc->IhProcAngio;
        IwOri = ExtractProc->IwOriAngio; IhOri = ExtractProc->IhOriAngio;
		
		READ_VALARRAY_FROM_STREAM_AND_PROGRESS(ptrUC, 10, float, scalarValF);
        ExtractProc->frameRateAngio = scalarValF[0]; ExtractProc->frameRateFluoro = scalarValF[1];
		ExtractProc->isoCenterPixSize = scalarValF[2]; ExtractProc->RotAngio = scalarValF[3]; ExtractProc->AngAngio = scalarValF[4];
		ExtractProc->isoCenterPixSizeRef = scalarValF[5]; ExtractProc->RotAngioRef = scalarValF[6]; ExtractProc->AngAngioRef = scalarValF[7];
		ExtractProc->OrigTablePosXMMPlane = scalarValF[8]; ExtractProc->OrigTablePosYMMPlane = scalarValF[9];
        
        CHECK_READINGERROR_F(ExtractProc->frameRateAngio, OSC_MIN_FRAME_RATE, OSC_MAX_FRAME_RATE, "frameRateAngio", "OscLoadFromMemory");
        CHECK_READINGERROR_F(ExtractProc->isoCenterPixSize, OSC_MIN_ISO_PIXSIZE, OSC_MAX_ISO_PIXSIZE, "isoCenterPixSize", "OscLoadFromMemory");
        CHECK_READINGERROR_F(ExtractProc->RotAngio, -180.f, 180.f, "RotAngio", "OscLoadFromMemory");
        CHECK_READINGERROR_F(ExtractProc->AngAngio, -180.f, 180.f, "AngAngio", "OscLoadFromMemory");
        CHECK_READINGERROR_F(ExtractProc->isoCenterPixSizeRef, OSC_MIN_ISO_PIXSIZE, OSC_MAX_ISO_PIXSIZE, "isoCenterPixSizeRef", "OscLoadFromMemory");
        CHECK_READINGERROR_F(ExtractProc->RotAngioRef, -180.f, 180.f, "RotAngioRef", "OscLoadFromMemory");
        CHECK_READINGERROR_F(ExtractProc->AngAngioRef, -180.f, 180.f, "AngAngioRef", "OscLoadFromMemory");
        
		if (ExtractProc->detectedDiaphragm){
			READ_VALARRAY_FROM_STREAM_AND_PROGRESS(ptrUC, 6, float, scalarValF);
			ExtractProc->DiaphragmParams[0] = scalarValF[0]; ExtractProc->DiaphragmParams[1] = scalarValF[1]; ExtractProc->DiaphragmParams[2] = scalarValF[2];
            ExtractProc->DiaphragmParamsRefAngio[0] = scalarValF[3]; 
            ExtractProc->DiaphragmParamsRefAngio[1] = scalarValF[4]; 
            ExtractProc->DiaphragmParamsRefAngio[2] = scalarValF[5]; 
		}
		for (int i=0; i<=ExtractSequence->PyrNbBands; i++){
			READ_VALARRAY_FROM_STREAM_AND_PROGRESS(ptrUC, 2, int, scalarValI);
			ExtractSequence->PyrW[i] = scalarValI[0]; ExtractSequence->PyrH[i] = scalarValI[1];
		}

		for (int frame = ExtractSequence->CardiacCycleStart; frame <= ExtractSequence->CardiacCycleEnd; frame ++){
			COscExtractPicture *ExtractPic = &ExtractSequence->ExtractPictures[frame];

            READ_VALARRAY_FROM_STREAM_AND_PROGRESS(ptrUC, 2, int, scalarValI);
			ExtractPic->deltaTabXSub = scalarValI[0]; ExtractPic->deltaTabYSub = scalarValI[1];

            READ_VALARRAY_FROM_STREAM_AND_PROGRESS(ptrUC, ExtractProc->maxAngioLengthAlloc, int, ExtractSequence->dXAngioToAngio[frame]);
            READ_VALARRAY_FROM_STREAM_AND_PROGRESS(ptrUC, ExtractProc->maxAngioLengthAlloc, int, ExtractSequence->dYAngioToAngio[frame]);
            
			ExtractPic->ImOverlayMask = ExtractProc->BuffOverlayMask[frame-ExtractSequence->CardiacCycleStart];
            READ_VALARRAY_FROM_STREAM_AND_PROGRESS(ptrUC, IwProc*IhProc, unsigned char, ExtractPic->ImOverlayMask);

			ExtractPic->ImPyrDir1D = ExtractProc->BuffPyrDir1D[frame-ExtractSequence->CardiacCycleStart];
			for (int i=0; i<= ExtractSequence->PyrNbBands; i++){
                memcpy((void*)(ExtractPic->ImPyrDir1D[i]), ptrUC
                                , ExtractSequence->PyrW[i]*ExtractSequence->PyrH[i]*sizeof(ExtractPic->ImPyrDir1D[i][0]));\
                ptrUC += ExtractSequence->PyrW[i]*ExtractSequence->PyrH[i] * sizeof(ExtractPic->ImPyrDir1D[i][0]) / sizeof(unsigned char);    
            }
		}

		// Sternal wires
        READ_VALARRAY_FROM_STREAM_AND_PROGRESS(ptrUC, 1, int, scalarValI);
        ExtractProc->SWRes.nbPixMask = scalarValI[0];
        ExtractProc->SwNbPixStored = ExtractProc->SWRes.nbPixMask;
        CHECK_READINGERROR(ExtractProc->SWRes.nbPixMask, 0, IwOri*IhOri, "SWRes.nbPixMask", "OscLoadFromMemory");

        if (ExtractProc->SWRes.nbPixMask > 0){
            READ_VALARRAY_FROM_STREAM_AND_PROGRESS(ptrUC, IwOri*IhOri, unsigned char, ExtractProc->MaskSWStored);
            int ErrorCode = SWRestoreStatusForTracking(&ExtractProc->SWHdl, IwOri, IhOri, ExtractProc->MaskSWStored);

            if (ErrorCode == -1){
		        sprintf_s(Reason, 128,"New mask size not compatible with allocated image sizes in SWLib");
		        ErrStartErrorReportFromLeaf(This->ErrHdl, "SWRestoreStatusForTracking", OSC_ERR_SWLIB, Reason);
		        return OSC_ERR_SWLIB;
            }
        }
	}
	ExtractProc->RefAngioResStored = 1;

	return 0;
}



long OscComputeMemorySizeForSave(COscHdl OscHdl){

	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);
	COscExtractSequence *ExtractSequence = &(This->ExtractSequence);

    int                 i;
    long                sizeMem = 0;
    int                 sizePyr1Ds;
    int					CardiacCycleLength	= (ExtractSequence->CardiacCycleEnd-ExtractSequence->CardiacCycleStart+1);

    sizeMem += 5*sizeof(int);   // Version + angioProcessFlag

    // Then, in case angio has been processed

    sizeMem += 43*sizeof(int);   // Int info in handle:
    // frameRate, IwOri, ExtractProc->IhOri, ExtractProc->Id, (int)CArmAnglesAngioGiven, (int)ShuttersRoiPosGiven, (int)angioProcessed
    // ShuttersRoiFullScale.XMin, ShuttersRoiFullScale.XMax,ShuttersRoiFullScale.YMin,ShuttersRoiFullScale.YMax
	// ShuttersRoiSafeFullScale.XMin, ShuttersRoiSafeFullScale.XMax, ExtractProc->ShuttersRoiSafeFullScale.YMin, ExtractProc->ShuttersRoiSafeFullScale.YMax;
	//  ShuttersRoi.XMin, ShuttersRoi.XMax, ShuttersRoi.YMin, ShuttersRoi.YMax, ShuttersRoiSafe.XMin, ShuttersRoiSafe.XMax;
	// ShuttersRoiSafe.YMin, ShuttersRoiSafe.YMax, OrigTablePosX, OrigTablePosY, NbProc, SSFactor,IwProc, IhProc
	// detectedDiaphragm, detectedCatheter, nExtractPictures, CardiacCycleStart, CardiacCycleEnd, PyrNbBands
	// tableInfoFromtheStart, ExtractProc->t, ShuttersRoiFullScaleRef.XMin, ShuttersRoiFullScaleRef.XMax
	// ShuttersRoiFullScaleRef.YMin, ExtractProc->ShuttersRoiFullScaleRef.YMax, (int)CArmAnglesAngioGivenRef;

    sizeMem += 10*sizeof(float);   // float info in handle:
    // isoCenterPixSize, RotAngio, AngAngio, isoCenterPixSizeRef, RotAngioRef, AngAngioRef;
        
    // If a diaphragm has been detected
    if (ExtractProc->detectedDiaphragm)
        sizeMem += 6*sizeof(float);

    // Pyramid
    sizeMem += (ExtractSequence->PyrNbBands+1)*2*sizeof(int);

    // Heart cycle
    sizePyr1Ds = 0;
    for (i=0; i<= ExtractSequence->PyrNbBands; i++)
        sizePyr1Ds += ExtractSequence->PyrW[i]*ExtractSequence->PyrH[i] 
                        * sizeof(ExtractSequence->ExtractPictures[ExtractSequence->CardiacCycleStart].ImPyrDir1D[i][0]);

    sizeMem += CardiacCycleLength *                                     // Heart cycle length
        ( 2*sizeof(int)
            + ExtractProc->maxAngioLengthAlloc * 2 * sizeof(int)          // dXAngioToAngio and dYAngioToAngio
            + ExtractProc->IwProcAngio*ExtractProc->IhProcAngio * sizeof(unsigned char)             // ImOverlayMask
            + sizePyr1Ds);                                      // ExtractPic.ImPyrDir1D

    // Sternal wires
    sizeMem += sizeof(int);     // Flag SW
    if (ExtractProc->SWRes.nbPixMask > 0)
        sizeMem += ExtractProc->IwOriAngio*ExtractProc->IhOriAngio*sizeof(unsigned char);


	return sizeMem;
}

int OscStoreRefAngioStatus(COscHdl OscHdl){

	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);
	int                  ErrorCodeOut = 0;//ErrorCode, 

	ExtractProc->RefAngioResStored = 1;

	// Diaphragm
	if (ExtractProc->detectedDiaphragm){
		ExtractProc->DiaphragmParamsRefAngio[0] = ExtractProc->DiaphragmParams[0]; 
		ExtractProc->DiaphragmParamsRefAngio[1] = ExtractProc->DiaphragmParams[1]; 
		ExtractProc->DiaphragmParamsRefAngio[2] = ExtractProc->DiaphragmParams[2];
	}

    // SW
    ExtractProc->SwNbPixStored = ExtractProc->SWRes.nbPixMask;
    if (ExtractProc->SWRes.nbPixMask > 0){
        memcpy(ExtractProc->MaskSWStored, ExtractProc->SWRes.SWMask, ExtractProc->IwOriAngio*ExtractProc->IhOriAngio*sizeof(unsigned char));
    }
    
	return ErrorCodeOut;
}

int OscRestoreAngioStatus(COscHdl OscHdl){

	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);

	if (ExtractProc->RefAngioResStored == 0){
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscRestoreAngioStatus", OSC_ERR_INCORRECT_TIME, "Asked to load a reference angio status that has not been stored!");
		return OSC_ERR_INCORRECT_TIME;
	}

	// Diaphragm
	if (ExtractProc->detectedDiaphragm){
		ExtractProc->DiaphragmParams[0] = ExtractProc->DiaphragmParamsRefAngio[0]; 
		ExtractProc->DiaphragmParams[1] = ExtractProc->DiaphragmParamsRefAngio[1]; 
		ExtractProc->DiaphragmParams[2] = ExtractProc->DiaphragmParamsRefAngio[2];
	}

    ExtractProc->SWRes.nbPixMask = ExtractProc->SwNbPixStored;
    if (ExtractProc->SWRes.nbPixMask){
        int ErrorCode = SWRestoreStatusForTracking(&ExtractProc->SWHdl, ExtractProc->IwOriAngio, ExtractProc->IhOriAngio, ExtractProc->MaskSWStored);
        if (ErrorCode == -1){
            char Reason[128];
		    sprintf_s(Reason, 128,"New mask size not compatible with allocated image sizes in SWLib");
		    ErrStartErrorReportFromLeaf(This->ErrHdl, "SWRestoreStatusForTracking", OSC_ERR_SWLIB, Reason);
		    return OSC_ERR_SWLIB;
        }
    }

	ExtractProc->t = -1;

	return 0;
}


int OscMemoryDesallocation(COscHdl OscHdl){
	
	COsc                *This = (COsc*)OscHdl.Pv;

	COscExtractProc     *ExtractProc = &(This->ExtractProc);
	COscExtractSequence *ExtractSequence = &(This->ExtractSequence);
	int					Err = 0, k, ErrorCodeOut = 0;
	
    if (ExtractProc->ImOriWorking != NULL)		{free(ExtractProc->ImOriWorking);			ExtractProc->ImOriWorking = NULL;}

	if (ExtractProc->ImSub != NULL)		{free(ExtractProc->ImSub);			ExtractProc->ImSub = NULL;}
	if (ExtractProc->ImRdg != NULL)		{free(ExtractProc->ImRdg);			ExtractProc->ImRdg = NULL;}
	if (ExtractProc->ImDir != NULL)		{free(ExtractProc->ImDir);			ExtractProc->ImDir = NULL;}

	if (ExtractProc->BufsF != NULL){
		int nbBufs = OSC_MAX(5, 5*ExtractProc->nbProcMaxAlloc);
		for (k=0; k<nbBufs; k++){
            if (ExtractProc->BufsF[k] != NULL)		{free(ExtractProc->BufsF[k]);			ExtractProc->BufsF[k] = NULL;}
        }
	}
	free(ExtractProc->BufsF); ExtractProc->BufsF = NULL;
	if (ExtractProc->BufsS != NULL){
		int nbBufs = OSC_MAX(6, 5*ExtractProc->nbProcMaxAlloc);
		for (k=0; k<nbBufs; k++){
            if (ExtractProc->BufsS[k] != NULL)		{free(ExtractProc->BufsS[k]);			ExtractProc->BufsS[k] = NULL;}
        }
	}
	free(ExtractProc->BufsS); ExtractProc->BufsS = NULL;
	if (ExtractProc->BufsI != NULL){
		int nbBufs = OSC_MAX(2, 2*ExtractProc->nbProcMaxAlloc);
		for (k=0; k<nbBufs; k++){
            if (ExtractProc->BufsI[k] != NULL)		{free(ExtractProc->BufsI[k]);			ExtractProc->BufsI[k] = NULL;}
        }
	}
	free(ExtractProc->BufsI); ExtractProc->BufsI = NULL;
	if (ExtractProc->BufsUC != NULL){
		int nbBufs = OSC_MAX(1, 4*ExtractProc->nbProcMaxAlloc+1);
		for (k=0; k<nbBufs; k++){
            if (ExtractProc->BufsUC[k] != NULL)		{free(ExtractProc->BufsUC[k]);			ExtractProc->BufsUC[k] = NULL;}
        }
	}
	free(ExtractProc->BufsUC); ExtractProc->BufsUC = NULL;

	if (ExtractProc->DiaHdlCreatedFlag){Err = DiaDelete(ExtractProc->DiaHdl); ExtractProc->DiaHdlCreatedFlag = 0;}
	if (Err){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrPrintErrorReport(ExtractProc->DiaHdl.ErrHdl);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "DiaDelete", OSC_ERR_DIALIB, "Error in DiaDelete");
		return OSC_ERR_DIALIB;
	}

	if (ExtractProc->CBDLibCreatedFlag){Err = CBDDelete(ExtractProc->CBDHdl); ExtractProc->CBDLibCreatedFlag = 0;}
	if (Err){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrPrintErrorReport(ExtractProc->CBDHdl.ErrHdl);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryDesallocation", OSC_ERR_CBDLIB, "Error in CBDHdl");
		return OSC_ERR_CBDLIB;
	}
	if(ExtractProc->ImCathPotential != NULL) {free(ExtractProc->ImCathPotential); ExtractProc->ImCathPotential = NULL;}
    
    if (ExtractProc->SWHdlCreatedFlag == 1){
        SWDelete(&ExtractProc->SWHdl);
		ExtractProc->SWHdlCreatedFlag = 0;
    }
	if(ExtractProc->MaskSWStored != NULL) {free(ExtractProc->MaskSWStored); ExtractProc->MaskSWStored = NULL;}

	if(ExtractProc->ImBkg != NULL) {free(ExtractProc->ImBkg); ExtractProc->ImBkg = NULL;}
	if(ExtractProc->ImFrg != NULL) {free(ExtractProc->ImFrg); ExtractProc->ImFrg = NULL;}
	if(ExtractProc->ImSee != NULL) {free(ExtractProc->ImSee); ExtractProc->ImSee = NULL;}
	if(ExtractProc->ImThr != NULL) {free(ExtractProc->ImThr); ExtractProc->ImThr = NULL;}
	if(ExtractProc->ImMask != NULL) {free(ExtractProc->ImMask); ExtractProc->ImMask = NULL;}
	if(ExtractProc->ImLab != NULL) {free(ExtractProc->ImLab); ExtractProc->ImLab = NULL;}

    if (This->ExtractSequence.ExtractPictures != NULL){
        for (k=0; k<ExtractProc->maxAngioLengthAlloc; k++){
		    COscExtractPicture  *ExtractPic = &This->ExtractSequence.ExtractPictures[k];

		    if(ExtractPic->ImFrgUC != NULL) {free(ExtractPic->ImFrgUC); ExtractPic->ImFrgUC = NULL;}
		    if(ExtractPic->ImDirUC != NULL) {free(ExtractPic->ImDirUC); ExtractPic->ImDirUC = NULL;}
		    if(ExtractPic->ImLabS != NULL) {free(ExtractPic->ImLabS); ExtractPic->ImLabS = NULL;}
		    if (ExtractPic->ImPyr1D != NULL){
			    Err = OscFreePyramid(This, ExtractPic->ImPyr1D, ExtractProc->pyrLevelMaxAlloc);
                OSC_ERR_REPORT_FROM_CALLER(Err, ErrorCodeOut, "OscFreePyramid", "OscMemoryDesallocation");
			    ExtractPic->ImPyr1D = NULL;
		    }
	    }
        free(This->ExtractSequence.ExtractPictures); This->ExtractSequence.ExtractPictures = NULL;
    }
	if (ExtractProc->PyrWFluoro != NULL){
		Err = OscFreePyramidDim(This, ExtractProc->PyrWFluoro, ExtractProc->PyrHFluoro);
        OSC_ERR_REPORT_FROM_CALLER(Err, ErrorCodeOut, "OscFreePyramidDim", "OscMemoryDesallocation");
		ExtractProc->PyrWFluoro = NULL; ExtractProc->PyrHFluoro = NULL;
	}
	if (ExtractSequence->PyrW != NULL){
		Err = OscFreePyramidDim(This, ExtractSequence->PyrW, ExtractSequence->PyrH);
        OSC_ERR_REPORT_FROM_CALLER(Err, ErrorCodeOut, "OscFreePyramidDim", "OscMemoryDesallocation");
		ExtractSequence->PyrW = NULL; ExtractSequence->PyrH = NULL;
	}
    if (ExtractSequence->MatchingScores != NULL){
        for (k=0; k<ExtractProc->maxAngioLengthAlloc; k++){
            if (ExtractSequence->MatchingScores[k] != NULL){free(ExtractSequence->MatchingScores[k]); ExtractSequence->MatchingScores[k] = NULL;}
        }
        free(ExtractSequence->MatchingScores); ExtractSequence->MatchingScores = NULL;
    }
    if (ExtractSequence->dXAngioToAngio != NULL){
        for (k=0; k<ExtractProc->maxAngioLengthAlloc; k++){
            if (ExtractSequence->dXAngioToAngio[k] != NULL){free(ExtractSequence->dXAngioToAngio[k]); ExtractSequence->dXAngioToAngio[k] = NULL;}
        }
        free(ExtractSequence->dXAngioToAngio); ExtractSequence->dXAngioToAngio = NULL;
    }
    if (ExtractSequence->dYAngioToAngio != NULL){
        for (k=0; k<ExtractProc->maxAngioLengthAlloc; k++){
                if (ExtractSequence->dYAngioToAngio[k] != NULL){free(ExtractSequence->dYAngioToAngio[k]); ExtractSequence->dYAngioToAngio[k] = NULL;}
            }
        free(ExtractSequence->dYAngioToAngio); ExtractSequence->dYAngioToAngio = NULL;
    }

    if (ExtractProc->ViterbiStateScore != NULL){
        for (k=0; k<ExtractProc->maxAngioLengthAlloc; k++){
            if (ExtractProc->ViterbiStateScore[k] != NULL){free(ExtractProc->ViterbiStateScore[k]); ExtractProc->ViterbiStateScore[k] = NULL;}
        }
        free(ExtractProc->ViterbiStateScore); ExtractProc->ViterbiStateScore = NULL;
    }
    if (ExtractProc->FluoroToAngioScore != NULL){free(ExtractProc->FluoroToAngioScore); ExtractProc->FluoroToAngioScore = NULL;}
    for (k=0; k<OSC_VITERBI_FLUORO_DEPTH; k++){
        if (ExtractProc->FluoroToAngioDX[k] != NULL){free(ExtractProc->FluoroToAngioDX[k]); ExtractProc->FluoroToAngioDX[k] = NULL;}
        if (ExtractProc->FluoroToAngioDY[k] != NULL){free(ExtractProc->FluoroToAngioDY[k]); ExtractProc->FluoroToAngioDY[k] = NULL;}
    }

	if (ExtractProc->ImCircBuffer != NULL){
        int maxCircBufferSize = (int)ceil(OSC_INTEGRATION_TIME_CIRC_BKG * ExtractProc->maxFrameRateAlloc);
		for (k=0; k<maxCircBufferSize; k++)
			if (ExtractProc->ImCircBuffer[k] != NULL){
                free(ExtractProc->ImCircBuffer[k]); ExtractProc->ImCircBuffer[k] = NULL;
            }
		free(ExtractProc->ImCircBuffer); ExtractProc->ImCircBuffer = NULL;
	}

	// Desallocating Initial memory
	if (ExtractProc->BuffOverlayMask != NULL){
		for (k=0; k<ExtractProc->CardiacCycleMaxFrames; k++){
			if (ExtractProc->BuffOverlayMask[k] != NULL){ 
				free(ExtractProc->BuffOverlayMask[k]); ExtractProc->BuffOverlayMask[k] = NULL;
			}
		}
		free(ExtractProc->BuffOverlayMask); ExtractProc->BuffOverlayMask = NULL;
	}
	if (ExtractProc->BuffPyrDir1D != NULL){
		for (k=0; k<ExtractProc->CardiacCycleMaxFrames; k++){
			if(ExtractProc->BuffPyrDir1D[k] != NULL) {
				Err = OscFreePyramidGeneric(This, (void **)ExtractProc->BuffPyrDir1D[k], ExtractProc->pyrLevelMaxAlloc);
                OSC_ERR_REPORT_FROM_CALLER(Err, ErrorCodeOut, "OscFreePyramidGeneric", "OscMemoryDesallocation");
				ExtractProc->BuffPyrDir1D[k] = NULL;
			}
		}
		free(ExtractProc->BuffPyrDir1D); ExtractProc->BuffPyrDir1D = NULL;
	}

	if (ExtractProc->ImFluoroDirPyr1D != NULL){
		Err = OscFreePyramidGeneric(This, (void **)ExtractProc->ImFluoroDirPyr1D, ExtractProc->pyrLevelMaxAlloc);
        OSC_ERR_REPORT_FROM_CALLER(Err, ErrorCodeOut, "OscFreePyramidGeneric", "OscMemoryDesallocation");
		ExtractProc->ImFluoroDirPyr1D = NULL;
	}
    
	if (ExtractProc->BufVectF1 != NULL){ free(ExtractProc->BufVectF1); ExtractProc->BufVectF1 = NULL;}
	if (ExtractProc->BufVectF2 != NULL){ free(ExtractProc->BufVectF2); ExtractProc->BufVectF2 = NULL;}
	if (ExtractProc->BufStack1 != NULL){ free(ExtractProc->BufStack1); ExtractProc->BufStack1 = NULL;}
	if (ExtractProc->BufStack2 != NULL){ free(ExtractProc->BufStack2); ExtractProc->BufStack2 = NULL;}
	if (ExtractProc->BufVectUC != NULL){ free(ExtractProc->BufVectUC); ExtractProc->BufVectUC = NULL;}
	if (ExtractProc->BufVectI1 != NULL){ free(ExtractProc->BufVectI1); ExtractProc->BufVectI1 = NULL;}
	if (ExtractProc->BufVectI2 != NULL){ free(ExtractProc->BufVectI2); ExtractProc->BufVectI2 = NULL;}
    for (k=0; k<NB_EXTRACTED_CATHINJROI_MAX; k++){
        if (ExtractProc->cumRdgRegions[k] != NULL){ free(ExtractProc->cumRdgRegions[k]); ExtractProc->cumRdgRegions[k] = NULL;}
        if (ExtractProc->cumRdgRegionsFiltered[k] != NULL){ free(ExtractProc->cumRdgRegionsFiltered[k]); ExtractProc->cumRdgRegionsFiltered[k] = NULL;}
    }
    
    if (ExtractProc->storedThresholdsBuf1 != NULL){free(ExtractProc->storedThresholdsBuf1); ExtractProc->storedThresholdsBuf1 = NULL;}
    if (ExtractProc->storedThresholdsBuf2 != NULL){free(ExtractProc->storedThresholdsBuf2); ExtractProc->storedThresholdsBuf2 = NULL;}

#ifdef OSC_CLEAN_DEBUG
    for (int i=0; i<ExtractSequence->maxImageNbCycle; i++){
        if (ExtractSequence->ImLabOverSeq[i] != NULL){free(ExtractSequence->ImLabOverSeq[i]); ExtractSequence->ImLabOverSeq[i] = NULL;}
        if (ExtractSequence->ImLabOverSeqAfterRdgThres[i] != NULL){free(ExtractSequence->ImLabOverSeqAfterRdgThres[i]); ExtractSequence->ImLabOverSeqAfterRdgThres[i] = NULL;}
        if (ExtractSequence->ImLabLargestRegions[i] != NULL){free(ExtractSequence->ImLabLargestRegions[i]); ExtractSequence->ImLabLargestRegions[i] = NULL;}
        if (ExtractSequence->ImLabAfterClustering[i] != NULL){free(ExtractSequence->ImLabAfterClustering[i]); ExtractSequence->ImLabAfterClustering[i] = NULL;}
    }
    if (ExtractSequence->ImLabOverSeq != NULL){free(ExtractSequence->ImLabOverSeq); ExtractSequence->ImLabOverSeq = NULL;}
    if (ExtractSequence->ImLabOverSeqAfterRdgThres != NULL){free(ExtractSequence->ImLabOverSeqAfterRdgThres); ExtractSequence->ImLabOverSeqAfterRdgThres = NULL;}
    if (ExtractSequence->ImLabLargestRegions != NULL){free(ExtractSequence->ImLabLargestRegions); ExtractSequence->ImLabLargestRegions = NULL;}
    if (ExtractSequence->ImLabAfterClustering != NULL){free(ExtractSequence->ImLabAfterClustering); ExtractSequence->ImLabAfterClustering = NULL;}
#endif  
    if (ExtractSequence->storeCycleShort != NULL){
        for (int i=0; i<ExtractSequence->maxImageNbCycle; i++){
            if (ExtractSequence->storeCycleShort[i] != NULL){free(ExtractSequence->storeCycleShort[i]); ExtractSequence->storeCycleShort[i] = NULL;}
        }
        free(ExtractSequence->storeCycleShort); ExtractSequence->storeCycleShort = NULL;
        ExtractSequence->maxImageNbCycle = 0;
    }

	return 0;
}

int OscMemoryAllocationGivenSize(COscHdl OscHdl, int IwFull, int IhFull, int IwSub, int IhSub, float maxFrameRate
                                 , float maxAngioDuration, int nbProcMax, int nbPyrLevelsMax){
	
	COsc                *This = (COsc*)OscHdl.Pv;

	COscExtractProc     *ExtractProc = &(This->ExtractProc);
	COscExtractSequence *ExtractSequence = &(This->ExtractSequence);
	int					ErrOut = 0, Err, k, sizeForMedian;
	char				Reason[128];
  
    ExtractProc->IwOriRawAlloc = IwFull; ExtractProc->IhOriRawAlloc = IhFull;
    ExtractProc->IwProcAlloc = IwSub; ExtractProc->IhProcAlloc = IhSub;
    ExtractProc->nbProcMaxAlloc = nbProcMax; ExtractProc->pyrLevelMaxAlloc = nbPyrLevelsMax;
    ExtractProc->maxFrameRateAlloc = maxFrameRate;
    ExtractProc->maxAngioLengthAlloc = (int)ceil(maxFrameRate*maxAngioDuration);
    
	OSC_CALLOC(ExtractProc->ImOriWorking, IwFull*IhFull);
	if (ExtractProc->ImOriWorking == NULL){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, "Allocation of ImOriWorking impossible");
		return OSC_ERR_MEM_ALLOC;
	}

	OSC_CALLOC(ExtractProc->ImSub, IwSub*IhSub);
	if (ExtractProc->ImSub == NULL){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, "Allocation of ImSub impossible");
		return OSC_ERR_MEM_ALLOC;
	}
	OSC_CALLOC(ExtractProc->ImRdg, IwSub*IhSub);
	if (ExtractProc->ImSub == NULL){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, "Allocation of ImRdg impossible");
		return OSC_ERR_MEM_ALLOC;
	}
	OSC_CALLOC(ExtractProc->ImDir, IwSub*IhSub);
	if (ExtractProc->ImSub == NULL){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, "Allocation of ImDir impossible");
		return OSC_ERR_MEM_ALLOC;
	}
	{
		int nbBufs = OSC_MAX(5, 5*nbProcMax);
		OSC_CALLOC(ExtractProc->BufsF, nbBufs);
		if (ExtractProc->BufsF == NULL){
            ExtractProc->FatalErrorOnPreviousImage = 1;
			ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, "Allocation of BufsF impossible");
			return OSC_ERR_MEM_ALLOC;
		}
        for (k=0; k<nbBufs; k++)
            ExtractProc->BufsF[k] = NULL;
		for (k=0; k<nbBufs; k++){
			OSC_CALLOC(ExtractProc->BufsF[k], IwSub*IhSub);
				if (ExtractProc->BufsF[k] == NULL){
                    ExtractProc->FatalErrorOnPreviousImage = 1;
					sprintf_s(Reason, 128, "Allocation of BufsF[%d] impossible", k);
					ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, Reason);
					return OSC_ERR_MEM_ALLOC;
				}
		}
	}
	{
		int nbBufs = OSC_MAX(6, 5*nbProcMax);
		OSC_CALLOC(ExtractProc->BufsS, nbBufs);
		if (ExtractProc->BufsS == NULL){
            ExtractProc->FatalErrorOnPreviousImage = 1;
			ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, "Allocation of BufsS impossible");
			return OSC_ERR_MEM_ALLOC;
		}
        for (k=0; k<nbBufs; k++)
            ExtractProc->BufsS[k] = NULL;
		for (k=0; k<nbBufs; k++){
			OSC_CALLOC(ExtractProc->BufsS[k], IwSub*IhSub);
				if (ExtractProc->BufsS[k] == NULL){
                    ExtractProc->FatalErrorOnPreviousImage = 1;
					sprintf_s(Reason, 128, "Allocation of BufsS[%d] impossible", k);
					ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, Reason);
					return OSC_ERR_MEM_ALLOC;
				}
		}
	}
    {
		int nbBufs = OSC_MAX(2, 2*nbProcMax);
		OSC_CALLOC(ExtractProc->BufsI, nbBufs);
		if (ExtractProc->BufsI == NULL){
            ExtractProc->FatalErrorOnPreviousImage = 1;
			ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, "Allocation of BufsI impossible");
			return OSC_ERR_MEM_ALLOC;
		}
        for (k=0; k<nbBufs; k++)
            ExtractProc->BufsI[k] = NULL;
		for (k=0; k<nbBufs; k++){
			OSC_CALLOC(ExtractProc->BufsI[k], IwSub*IhSub);
				if (ExtractProc->BufsI[k] == NULL){
                    ExtractProc->FatalErrorOnPreviousImage = 1;
					sprintf_s(Reason, 128, "Allocation of BufsI[%d] impossible", k);
					ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, Reason);
					return OSC_ERR_MEM_ALLOC;
				}
		}
	}
    {
		int nbBufs = OSC_MAX(1, 4*nbProcMax+1);
		OSC_CALLOC(ExtractProc->BufsUC, nbBufs);
		if (ExtractProc->BufsUC == NULL){
            ExtractProc->FatalErrorOnPreviousImage = 1;
			ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, "Allocation of BufsUC impossible");
			return OSC_ERR_MEM_ALLOC;
		}
        for (k=0; k<nbBufs; k++)
            ExtractProc->BufsUC[k] = NULL;
		for (k=0; k<nbBufs; k++){
			OSC_CALLOC(ExtractProc->BufsUC[k], IwSub*IhSub);
				if (ExtractProc->BufsUC[k] == NULL){
                    ExtractProc->FatalErrorOnPreviousImage = 1;
					sprintf_s(Reason, 128, "Allocation of BufsUC[%d] impossible", k);
					ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, Reason);
					return OSC_ERR_MEM_ALLOC;
				}
		}
	}

	Err = DiaCreateGivenWorkingImSize(&This->ExtractProc.DiaHdl, IwFull, IhFull, IwSub, IhSub);
	if (Err){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrPrintErrorReport(ExtractProc->DiaHdl.ErrHdl);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_DIALIB, "Error in DiaCreateGivenWorkingImSize");
		return OSC_ERR_DIALIB;
	}
	This->ExtractProc.DiaHdlCreatedFlag = 1;

	Err = CBDCreate(&This->ExtractProc.CBDHdl);
	if (Err){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrPrintErrorReport(ExtractProc->CBDHdl.ErrHdl);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_CBDLIB, "Error in CBDHdl");
		return OSC_ERR_CBDLIB;
	}
	Err = CDBDefaultParameters(&This->ExtractProc.CBDParams, IwSub, CBD_ANGIO);
	if (Err){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrPrintErrorReport(ExtractProc->CBDHdl.ErrHdl);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_CBDLIB, "Error in CBDHdl");
		return OSC_ERR_CBDLIB;
	}
	Err = CBDMemoryAllocation(ExtractProc->CBDHdl, IwSub, IhSub, 1, This->ExtractProc.CBDParams.nbSeedsPerSide);
	if (Err){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrPrintErrorReport(ExtractProc->CBDHdl.ErrHdl);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_CBDLIB, "Error in CBDHdl");
		return OSC_ERR_CBDLIB;
	}
	OSC_CALLOC(ExtractProc->ImCathPotential, IwSub*IhSub);
	if (ExtractProc->ImCathPotential == NULL){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, "Allocation of ImCathPotential impossible");
		return OSC_ERR_MEM_ALLOC;
	}
	// CBDHdl creation continued...
	{
		int ShutterPosInit[4]; 
		ShutterPosInit[0] = 0; ShutterPosInit[1] = IwSub-1; ShutterPosInit[2] = 0; ShutterPosInit[3] = IhSub-1;
		CBDTrackerInitialization(ExtractProc->ImCathPotential, IwSub, IhSub, ShutterPosInit
								, &ExtractProc->CBDParams, &ExtractProc->CBDHdl);
		ExtractProc->CBDLibCreatedFlag = 1;
	}

	// SW
    SWCreate(&ExtractProc->SWHdl, IwFull, IhFull, nbProcMax);
    ExtractProc->SWHdlCreatedFlag = 1;
    OSC_CALLOC(ExtractProc->MaskSWStored, IwFull*IhFull);
	if (ExtractProc->MaskSWStored == NULL){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, "Allocation of MaskSWStored impossible");
		return OSC_ERR_MEM_ALLOC;
	}

	
    OSC_CALLOC(ExtractProc->ImBkg, IwSub*IhSub);
	if (ExtractProc->ImBkg == NULL){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, "Allocation of ImBkg impossible");
		return OSC_ERR_MEM_ALLOC;
	}
    OSC_CALLOC(ExtractProc->ImFrg, IwSub*IhSub);
	if (ExtractProc->ImFrg == NULL){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, "Allocation of ImFrg impossible");
		return OSC_ERR_MEM_ALLOC;
	}
	
    OSC_CALLOC(ExtractProc->ImSee, IwSub*IhSub);
	if (ExtractProc->ImSee == NULL){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, "Allocation of ImSee impossible");
		return OSC_ERR_MEM_ALLOC;
	}
    OSC_CALLOC(ExtractProc->ImThr, IwSub*IhSub);
	if (ExtractProc->ImThr == NULL){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, "Allocation of ImThr impossible");
		return OSC_ERR_MEM_ALLOC;
	}
    OSC_CALLOC(ExtractProc->ImMask, IwSub*IhSub);
	if (ExtractProc->ImMask == NULL){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, "Allocation of ImMask impossible");
		return OSC_ERR_MEM_ALLOC;
	}
    OSC_CALLOC(ExtractProc->ImLab, IwSub*IhSub);
	if (ExtractProc->ImLab == NULL){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, "Allocation of ImLab impossible");
		return OSC_ERR_MEM_ALLOC;
	}

    // ExtractSequence allocation
    ExtractSequence->ExtractPictures = (COscExtractPicture*)malloc(ExtractProc->maxAngioLengthAlloc*sizeof(COscExtractPicture));
    if (ExtractSequence->ExtractPictures == NULL){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, "Allocation of ExtractSequence->ExtractPictures impossible");
		return OSC_ERR_MEM_ALLOC;
    }
    for (k=0; k<ExtractProc->maxAngioLengthAlloc; k++){
        ExtractSequence->ExtractPictures[k].ImFrgUC = NULL;
        ExtractSequence->ExtractPictures[k].ImDirUC = NULL;
        ExtractSequence->ExtractPictures[k].ImLabS = NULL;
        ExtractSequence->ExtractPictures[k].ImOverlayMask = NULL;
        ExtractSequence->ExtractPictures[k].ImPyr1D = NULL;
    }
    ExtractSequence->MatchingScores = (float**)malloc(ExtractProc->maxAngioLengthAlloc*sizeof(float*));
    if (ExtractSequence->MatchingScores == NULL){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, "Allocation of MatchingScores impossible");
		return OSC_ERR_MEM_ALLOC;
    }
    for (k=0; k<ExtractProc->maxAngioLengthAlloc; k++)
        ExtractSequence->MatchingScores[k] = NULL;
    ExtractSequence->dXAngioToAngio = (int**)malloc(ExtractProc->maxAngioLengthAlloc*sizeof(int*));
    if (ExtractSequence->dXAngioToAngio == NULL){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, "Allocation of dXAngioToAngio impossible");
		return OSC_ERR_MEM_ALLOC;
    }
    for (k=0; k<ExtractProc->maxAngioLengthAlloc; k++)
        ExtractSequence->dXAngioToAngio[k] = NULL;
    ExtractSequence->dYAngioToAngio = (int**)malloc(ExtractProc->maxAngioLengthAlloc*sizeof(int*));
    if (ExtractSequence->dYAngioToAngio == NULL){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, "Allocation of dYAngioToAngio impossible");
		return OSC_ERR_MEM_ALLOC;
    }
    for (k=0; k<ExtractProc->maxAngioLengthAlloc; k++)
        ExtractSequence->dYAngioToAngio[k] = NULL;
    for (k=0; k<ExtractProc->maxAngioLengthAlloc; k++){
        ExtractSequence->MatchingScores[k] = (float*)malloc(ExtractProc->maxAngioLengthAlloc*sizeof(float));
        if (ExtractSequence->MatchingScores[k] == NULL){
            ExtractProc->FatalErrorOnPreviousImage = 1;
            sprintf_s(Reason, 128, "Allocation of MatchingScores[%d] impossible", k);
		    ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, Reason);
		    return OSC_ERR_MEM_ALLOC;
        }
        ExtractSequence->dXAngioToAngio[k] = (int*)malloc(ExtractProc->maxAngioLengthAlloc*sizeof(int));
        if (ExtractSequence->dXAngioToAngio[k] == NULL){
            ExtractProc->FatalErrorOnPreviousImage = 1;
            sprintf_s(Reason, 128, "Allocation of dXAngioToAngio[%d] impossible", k);
		    ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, Reason);
		    return OSC_ERR_MEM_ALLOC;
        }
        ExtractSequence->dYAngioToAngio[k] = (int*)malloc(ExtractProc->maxAngioLengthAlloc*sizeof(int));
        if (ExtractSequence->dYAngioToAngio[k] == NULL){
            ExtractProc->FatalErrorOnPreviousImage = 1;
            sprintf_s(Reason, 128, "Allocation of dXAngioToAngio[%d] impossible", k);
		    ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, Reason);
		    return OSC_ERR_MEM_ALLOC;
        }
    }


	// COscExtractPicture allocation
	Err = OscGetPyramidDim(This, nbPyrLevelsMax, IwSub, IhSub, &ExtractProc->PyrWFluoro, &ExtractProc->PyrHFluoro);
    OSC_ERR_REPORT_FROM_CALLER(Err, ErrOut, "OscGetPyramidDim", "OscMemoryAllocationGivenSize");
	Err = OscGetPyramidDim(This, nbPyrLevelsMax, IwSub, IhSub, &ExtractSequence->PyrW, &ExtractSequence->PyrH);
    OSC_ERR_REPORT_FROM_CALLER(Err, ErrOut, "OscGetPyramidDim", "OscMemoryAllocationGivenSize");
	for (k=0; k<ExtractProc->maxAngioLengthAlloc; k++){
		COscExtractPicture  *ExtractPic = &ExtractSequence->ExtractPictures[k];

		OSC_CALLOC(ExtractPic->ImFrgUC, IwSub*IhSub);
		if (ExtractPic->ImFrgUC == NULL){
            ExtractProc->FatalErrorOnPreviousImage = 1;
			sprintf_s(Reason, 128, "Allocation of ExtractPic->ImFrgUC %d (/%d) impossible", k, ExtractProc->maxAngioLengthAlloc);
			ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, Reason);
			return OSC_ERR_MEM_ALLOC;
		}
		OSC_CALLOC(ExtractPic->ImDirUC, IwSub*IhSub);
		if (ExtractPic->ImDirUC == NULL){
            ExtractProc->FatalErrorOnPreviousImage = 1;
			sprintf_s(Reason, 128, "Allocation of ExtractPic->ImDirUC %d (/%d) impossible", k, ExtractProc->maxAngioLengthAlloc);
			ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, Reason);
			return OSC_ERR_MEM_ALLOC;
		}
		OSC_CALLOC(ExtractPic->ImLabS, IwSub*IhSub);
		if (ExtractPic->ImLabS == NULL){
            ExtractProc->FatalErrorOnPreviousImage = 1;
			sprintf_s(Reason, 128, "Allocation of ExtractPic->ImLabS %d (/%d) impossible", k, ExtractProc->maxAngioLengthAlloc);
			ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, Reason);
			return OSC_ERR_MEM_ALLOC;
		}

		Err = OscAllocatePyramid(This, &(ExtractPic->ImPyr1D), ExtractSequence->PyrW, ExtractSequence->PyrH, nbPyrLevelsMax);
		OSC_ERR_REPORT_FROM_CALLER(Err, ErrOut, "OscAllocatePyramid", "OscMemoryAllocationGivenSize");
	}

    ExtractProc->FluoroToAngioScore = (float*)malloc(ExtractProc->maxAngioLengthAlloc*sizeof(float));
    if (ExtractProc->FluoroToAngioScore == NULL){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, "Allocation of FluoroToAngioScore impossible");
		return OSC_ERR_MEM_ALLOC;
    }
    ExtractProc->ViterbiStateScore = (float**)malloc(ExtractProc->maxAngioLengthAlloc*sizeof(float*));
    if (ExtractProc->ViterbiStateScore == NULL){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, "Allocation of ViterbiStateScore impossible");
		return OSC_ERR_MEM_ALLOC;
    }
    for (k=0; k<ExtractProc->maxAngioLengthAlloc; k++)
        ExtractProc->ViterbiStateScore[k] = NULL;
    for (k=0; k<ExtractProc->maxAngioLengthAlloc; k++){
        ExtractProc->ViterbiStateScore[k] = (float*)malloc(ExtractProc->maxAngioLengthAlloc*sizeof(float));
        if (ExtractProc->ViterbiStateScore[k] == NULL){
            ExtractProc->FatalErrorOnPreviousImage = 1;
            sprintf_s(Reason, 128, "Allocation of ViterbiStateScore[%d] impossible", k);
		    ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, Reason);
		    return OSC_ERR_MEM_ALLOC;
        }
    }
    for (k=0; k<OSC_VITERBI_FLUORO_DEPTH; k++){
        ExtractProc->FluoroToAngioDX[k] = (int*)malloc(ExtractProc->maxAngioLengthAlloc*sizeof(int));
        if (ExtractProc->FluoroToAngioDX[k] == NULL){
            ExtractProc->FatalErrorOnPreviousImage = 1;
            sprintf_s(Reason, 128, "Allocation of FluoroToAngioDX[%d] impossible", k);
		    ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, Reason);
		    return OSC_ERR_MEM_ALLOC;
        }
        ExtractProc->FluoroToAngioDY[k] = (int*)malloc(ExtractProc->maxAngioLengthAlloc*sizeof(int));
        if (ExtractProc->FluoroToAngioDY[k] == NULL){
            ExtractProc->FatalErrorOnPreviousImage = 1;
            sprintf_s(Reason, 128, "Allocation of FluoroToAngioDY[%d] impossible", k);
		    ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, Reason);
		    return OSC_ERR_MEM_ALLOC;
        }
    }


	// Initial memory allocation
	ExtractProc->CardiacCycleMaxFrames = (int)ceil(maxFrameRate*60.f / OSC_MIN_CARDIACFREQ);

	OSC_MALLOC(ExtractProc->BuffOverlayMask, ExtractProc->CardiacCycleMaxFrames);
	if (ExtractProc->BuffOverlayMask == NULL){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		sprintf_s(Reason, 128, "Allocation of ExtractProc->BuffOverlayMask impossible");
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, Reason);
		return OSC_ERR_MEM_ALLOC;
	}
    for (k=0; k<ExtractProc->CardiacCycleMaxFrames; k++)
        ExtractProc->BuffOverlayMask[k] = NULL;
	for (k=0; k<ExtractProc->CardiacCycleMaxFrames; k++){
		OSC_MALLOC(ExtractProc->BuffOverlayMask[k], IwFull*IhFull);
		if (ExtractProc->BuffOverlayMask[k] == NULL){
            ExtractProc->FatalErrorOnPreviousImage = 1;
			sprintf_s(Reason, 128, "Allocation of ExtractProc->BuffOverlayMask %d impossible", k);
			ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, Reason);
			return OSC_ERR_MEM_ALLOC;
		}
	}
	
	OSC_MALLOC(ExtractProc->BuffPyrDir1D, ExtractProc->CardiacCycleMaxFrames);
	if (ExtractProc->BuffPyrDir1D == NULL){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		sprintf_s(Reason, 128, "Allocation of ExtractProc->BuffPyrDir1D impossible");
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, Reason);
		return OSC_ERR_MEM_ALLOC;
	}
    for (k=0; k<ExtractProc->CardiacCycleMaxFrames; k++)
        ExtractProc->BuffPyrDir1D[k] = NULL;
	for (k=0; k<ExtractProc->CardiacCycleMaxFrames; k++){

		Err |= OscAllocatePyramidGeneric(This, (void ***)&(ExtractProc->BuffPyrDir1D[k]), sizeof(ExtractProc->BuffPyrDir1D[k][0])
										, ExtractSequence->PyrW, ExtractSequence->PyrH, nbPyrLevelsMax);
		OSC_ERR_REPORT_FROM_CALLER(Err, ErrOut, "OscAllocatePyramidGeneric", "OscMemoryAllocationGivenSize");
	}

    {
        int maxCircBufferSize = (int)ceil(OSC_INTEGRATION_TIME_CIRC_BKG * maxFrameRate);
	    OSC_MALLOC(ExtractProc->ImCircBuffer, maxCircBufferSize);
	    if (ExtractProc->ImCircBuffer == NULL){
            ExtractProc->FatalErrorOnPreviousImage = 1;
		    sprintf_s(Reason, 128, "Allocation of ExtractProc->ImCircBuffer impossible");
		    ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, Reason);
		    return OSC_ERR_MEM_ALLOC;
	    }
        for (k=0; k<maxCircBufferSize; k++)
            ExtractProc->ImCircBuffer[k] = NULL;
	    for (k=0; k<maxCircBufferSize; k++){
		    OSC_MALLOC(ExtractProc->ImCircBuffer[k], IwSub*IhSub);
		    if (ExtractProc->ImCircBuffer[k] == NULL){
                ExtractProc->FatalErrorOnPreviousImage = 1;
			    sprintf_s(Reason, 128, "Allocation of ExtractProc->ImCircBuffer %d impossible", k);
			    ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, Reason);
			    return OSC_ERR_MEM_ALLOC;
		    }
	    }
    }

	Err = OscAllocatePyramidGeneric(This, (void ***)&(ExtractProc->ImFluoroDirPyr1D), sizeof(ExtractProc->ImFluoroDirPyr1D[0])
                                    , ExtractProc->PyrWFluoro, ExtractProc->PyrHFluoro, nbPyrLevelsMax);
	OSC_ERR_REPORT_FROM_CALLER(Err, ErrOut, "OscAllocatePyramidGeneric", "OscMemoryAllocationGivenSize");

    // Cath inj ROI detection
    OSC_CALLOC(ExtractProc->BufVectF1, ExtractProc->maxAngioLengthAlloc);
	if (ExtractProc->BufVectF1 == NULL){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, "Allocation of BufVectF1 impossible");
		return OSC_ERR_MEM_ALLOC;
	}
    OSC_CALLOC(ExtractProc->BufVectF2, ExtractProc->maxAngioLengthAlloc);
	if (ExtractProc->BufVectF2 == NULL){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, "Allocation of BufVectF2 impossible");
		return OSC_ERR_MEM_ALLOC;
	}
    sizeForMedian = 2*(int)floor(0.5*OSC_MAX(OSC_CATHINJ_MEDIANSIZE_INJINDFILTERING, OSC_CATHINJROI_MEDIANSIZE_CUMRDG))+1;
    OSC_CALLOC(ExtractProc->BufStack1, sizeForMedian);
	if (ExtractProc->BufStack1 == NULL){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, "Allocation of BufStack1 impossible");
		return OSC_ERR_MEM_ALLOC;
	}
    OSC_CALLOC(ExtractProc->BufStack2, sizeForMedian);
	if (ExtractProc->BufStack2 == NULL){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, "Allocation of BufStack2 impossible");
		return OSC_ERR_MEM_ALLOC;
	}
    OSC_CALLOC(ExtractProc->BufVectUC, ExtractProc->maxAngioLengthAlloc);
	if (ExtractProc->BufVectUC == NULL){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, "Allocation of BufVectUC impossible");
		return OSC_ERR_MEM_ALLOC;
	}
    OSC_CALLOC(ExtractProc->BufVectI1, ExtractProc->maxAngioLengthAlloc);
	if (ExtractProc->BufVectI1 == NULL){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, "Allocation of BufVectI1 impossible");
		return OSC_ERR_MEM_ALLOC;
	}
    OSC_CALLOC(ExtractProc->BufVectI2, ExtractProc->maxAngioLengthAlloc);
	if (ExtractProc->BufVectI2 == NULL){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, "Allocation of BufVectI2 impossible");
		return OSC_ERR_MEM_ALLOC;
	}
    for (k=0; k<NB_EXTRACTED_CATHINJROI_MAX; k++){
        OSC_CALLOC(ExtractProc->cumRdgRegions[k], ExtractProc->maxAngioLengthAlloc);
	    if (ExtractProc->cumRdgRegions[k] == NULL){
            ExtractProc->FatalErrorOnPreviousImage = 1;
			sprintf_s(Reason, 128, "Allocation of cumRdgRegions[%d] impossible", k);
			ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, Reason);
			return OSC_ERR_MEM_ALLOC;
	    }
        OSC_CALLOC(ExtractProc->cumRdgRegionsFiltered[k], ExtractProc->maxAngioLengthAlloc);
	    if (ExtractProc->cumRdgRegionsFiltered[k] == NULL){
            ExtractProc->FatalErrorOnPreviousImage = 1;
			sprintf_s(Reason, 128, "Allocation of cumRdgRegionsFiltered[%d] impossible", k);
			ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, Reason);
			return OSC_ERR_MEM_ALLOC;
	    }
    }

 //   OSC_CALLOC(ExtractSequence->LabNbElts, ExtractProc->CardiacCycleMaxFrames);
	//if (ExtractSequence->LabNbElts == NULL){
 //       ExtractProc->FatalErrorOnPreviousImage = 1;
	//	ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, "Allocation of LabNbElts impossible");
	//	return OSC_ERR_MEM_ALLOC;
	//}
 //   for (int k=0; k<ExtractProc->CardiacCycleMaxFrames; k++){
 //       OSC_CALLOC(ExtractSequence->LabNbElts[k], OSC_MAXLABELSNB+1);
	//    if (ExtractSequence->LabNbElts[k] == NULL){
 //           ExtractProc->FatalErrorOnPreviousImage = 1;
	//		sprintf_s(Reason, 128, "Allocation of LabNbElts[%d] impossible", k);
	//		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocation", OSC_ERR_MEM_ALLOC, Reason);
	//		return OSC_ERR_MEM_ALLOC;
	//    }
 //   }

    ExtractProc->nbThresholdsForMedianComputation = (int)ceil(maxFrameRate * OSC_CLEAN_IMAGE_WISE_CAUSAL_THRESHOLD_CHECK_DURATION);
    OSC_CALLOC(ExtractProc->storedThresholdsBuf1, ExtractProc->nbThresholdsForMedianComputation);
    OSC_CALLOC(ExtractProc->storedThresholdsBuf2, ExtractProc->nbThresholdsForMedianComputation);

    ExtractSequence->maxImageNbCycle = (int)ceil(maxFrameRate*60.f / OSC_MIN_CARDIACFREQ);
    OSC_CALLOC(ExtractSequence->storeCycleShort, ExtractSequence->maxImageNbCycle);
    if (ExtractSequence->storeCycleShort == NULL){
        ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMemoryAllocationGivenSize", OSC_ERR_MEM_ALLOC, "Cannot allocate storeCycleShort");
		return OSC_ERR_MEM_ALLOC;
    }

    for (int i=0; i<ExtractSequence->maxImageNbCycle; i++){
        OSC_CALLOC(ExtractSequence->storeCycleShort[i], IwSub*IhSub);
    }

#ifdef OSC_CLEAN_DEBUG
    OSC_CALLOC(ExtractSequence->ImLabOverSeq, ExtractSequence->maxImageNbCycle);
    OSC_CALLOC(ExtractSequence->ImLabOverSeqAfterRdgThres, ExtractSequence->maxImageNbCycle);
    OSC_CALLOC(ExtractSequence->ImLabLargestRegions, ExtractSequence->maxImageNbCycle);
    OSC_CALLOC(ExtractSequence->ImLabAfterClustering, ExtractSequence->maxImageNbCycle);
    for (int i=0; i<ExtractSequence->maxImageNbCycle; i++){
        OSC_CALLOC(ExtractSequence->ImLabOverSeq[i], IwSub*IhSub);
        OSC_CALLOC(ExtractSequence->ImLabOverSeqAfterRdgThres[i], IwSub*IhSub);
        OSC_CALLOC(ExtractSequence->ImLabLargestRegions[i], IwSub*IhSub);
        OSC_CALLOC(ExtractSequence->ImLabAfterClustering[i], IwSub*IhSub);
    }
#endif  
	return ErrOut;
}
