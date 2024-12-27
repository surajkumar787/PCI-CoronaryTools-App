// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
//
// Filename: OscStub.h
#pragma once

#include <Osc.h>

//TICS -ORG#011: cant uses namespace since this is a stub for a external library.
//TICS -STA#002:  cant uses namespace since this is a stub for a external library.
//TICS -INT#021  cant' use arguments by reference since this is a stub for a external library.

int OscProcessPicture_ReturnValue = 0;
int OscAngioProcessSequence_ReturnValue = 0;
int OscSetAngioSequenceParameters_ReturnValue = 0;
int OscSetTableApparentPositionMM_ReturnValue = 0;
int OscSetShuttersRoi_ReturnValue = 0;
int OscSetCurrentGeometryAngio_ReturnValue = 0;
int OscSetTableApparentPositionPix_ReturnValue = 0;
int OscSaveToMemory_ReturnValue = 0;
int OscAngioSeqGetOverlayAssociatedToAngioFrame_indexRequested = 0;
int OscSetFluoroSequenceParameters_ReturnValue = 0;
int OscSetCurrentGeometryFluoro_ReturnValue = 0;
int OscFluoroProcessPicture_ReturnValue = 0;

COscRoiCoord Osc_ShuttersRoi;
float Osc_MmPerPixel;
float Osc_Rotation;
float Osc_Angulation;

int Osc_HeartCycleStart = 0;
int Osc_HeartCycleEnd   = 0;

int Osc_TablePosXPix = 0;
int Osc_TablePosYPix = 0;

float Osc_TablePosXMM = 0;
float Osc_TablePosYMM = 0;
float Osc_TablePosZMM = 0;

int	Osc_FluoroWidth = 0;
int	Osc_FluoroHeight = 0;
float Osc_FluoroFrameRate = 0;

int RETURNVALUE = 0;


int OscCreate(COscHdl* /*PtOscHdl*/, int /*ImaDyn*/, int /*ImaWidthAlloc*/, int /*ImaHeightAlloc*/
                            , int /*ImaWidthSubAlloc*/, int /*ImaHeightSubAlloc*/, float /*maxFrameRate*/, float /*maxAngioDuration*/, int /*maxNbProc*/)
{
	return RETURNVALUE;
}

int OscGetVersionNumbers(int* /*Major*/, int* /*Minor*/, int* /*Patch*/)
{
    return RETURNVALUE;
}

int OscDelete(COscHdl /*OscHdl*/)
{
	return RETURNVALUE;
}

int OscSetAngioSequenceParameters(COscHdl /*OscHdl*/, int /*IwAngio*/, int /*IhAngio*/, float /*frameRateAngio*/, int /*SSFactor*/, IThreadPool * /*pool*/)
{
	return OscSetAngioSequenceParameters_ReturnValue;
}

int OscSetTableApparentPositionMM	(COscHdl /*OscHdl*/, float TablePosXMM, float TablePosYMM, float TablePosZMM)
{
	Osc_TablePosXMM = TablePosXMM;
	Osc_TablePosYMM = TablePosYMM;
	Osc_TablePosZMM = TablePosZMM;

	return OscSetTableApparentPositionMM_ReturnValue;
}

int OscSetShuttersRoi(COscHdl /*OscHdl*/, COscRoiCoord RoiCoord)
{
	Osc_ShuttersRoi = RoiCoord;
	return OscSetShuttersRoi_ReturnValue;
}

int OscSetCurrentGeometryAngio	(COscHdl /*OscHdl*/, float Rot, float Ang, float pixSizeAtIsocenter)
{
	Osc_MmPerPixel = pixSizeAtIsocenter;
	Osc_Rotation = Rot;
	Osc_Angulation = Ang;
	return OscSetCurrentGeometryAngio_ReturnValue;
}

int OscSetTableApparentPositionPix(COscHdl /*OscHdl*/, int TablePosXPix, int TablePosYPix)
{
	Osc_TablePosXPix = TablePosXPix;
	Osc_TablePosYPix = TablePosYPix;
	return OscSetTableApparentPositionPix_ReturnValue;
}

int OscAngioProcessPicture(COscHdl /*OscHdl*/, short* /*InIma*/, int /*t*/, COscAngioPictureResults*  /*ProcessResults*/)
{
	return OscProcessPicture_ReturnValue;
}

int OscAngioProcessSequence(COscHdl /*OscHdl*/, COscAngioSequenceResults* ProcessResults)
{
	ProcessResults->CardiacCycleStart = Osc_HeartCycleStart;
	ProcessResults->CardiacCycleEnd = Osc_HeartCycleEnd;

	return OscAngioProcessSequence_ReturnValue;
}

long OscComputeMemorySizeForSave(COscHdl /*OscHdl*/)
{
	return RETURNVALUE;
}

int OscSaveToMemory(COscHdl /*OscHdl*/, void* /*memOut*/)
{
	return OscSaveToMemory_ReturnValue;
}

unsigned char* OscAngioSeqGetOverlayAssociatedToAngioFrame(COscHdl /*OscHdl*/, int t)
{
	OscAngioSeqGetOverlayAssociatedToAngioFrame_indexRequested = t;
	unsigned char* test = reinterpret_cast<unsigned char*>("blob");
	return test;
}

void OscAngioGetOvrSize(COscHdl /*OscHdl*/, int *IwOvr, int *IhOvr, int *SSFactor)
{
	*IwOvr = 1;
	*IhOvr = 1;
	*SSFactor = 1;
}

int OscLoadFromMemory(COscHdl /*OscHdl*/, void* /*memIn*/)
{
	return RETURNVALUE;
}

int OscSetFluoroSequenceParameters(COscHdl /*OscHdl*/, int IwFluoro, int IhFluoro, float frameRateFluoro, IThreadPool * /*pool*/)
{
	Osc_FluoroWidth = IwFluoro;
	Osc_FluoroHeight = IhFluoro;
	Osc_FluoroFrameRate = frameRateFluoro;
	return OscSetFluoroSequenceParameters_ReturnValue;
}

int OscSetCurrentGeometryFluoro(COscHdl /*OscHdl*/, float Rot, float Ang, float pixSizeAtIsocenter)
{
	Osc_MmPerPixel = pixSizeAtIsocenter;
	Osc_Rotation = Rot;
	Osc_Angulation = Ang;
	return OscSetCurrentGeometryFluoro_ReturnValue;
}

int OscFluoroProcessPicture(COscHdl /*OscHdl*/, short* /*InIma*/, int /*t*/, int /*FrameStatus*/, COscFluoroPictureResults*  /*ProcessResults*/)
{
	return OscFluoroProcessPicture_ReturnValue;
}

unsigned char * OscFluoroGetOverlay(COscHdl /*OscHdl*/, int * /*IwOvr*/, int * /*IhOvr*/, int * /*SSFactor*/)
{
	unsigned char* test =  reinterpret_cast<unsigned char*>("blob");
	return test;
}

void reset()
{
	OscProcessPicture_ReturnValue = 0;
	OscAngioProcessSequence_ReturnValue = 0;
	OscSetAngioSequenceParameters_ReturnValue = 0;
	OscSetTableApparentPositionMM_ReturnValue = 0;
	OscSetShuttersRoi_ReturnValue = 0;
	OscSetCurrentGeometryAngio_ReturnValue = 0;
	OscSetTableApparentPositionPix_ReturnValue = 0;
	OscSaveToMemory_ReturnValue = 0;
	OscAngioSeqGetOverlayAssociatedToAngioFrame_indexRequested = 0;
	OscSetFluoroSequenceParameters_ReturnValue = 0;
	OscSetCurrentGeometryFluoro_ReturnValue = 0;
	OscFluoroProcessPicture_ReturnValue = 0;

	Osc_HeartCycleStart = 0;
	Osc_HeartCycleEnd   = 0;

	Osc_TablePosXPix = 0;
	Osc_TablePosYPix = 0;

	Osc_TablePosXMM = 0;
	Osc_TablePosYMM = 0;
	Osc_TablePosZMM = 0;

	Osc_FluoroWidth = 0;
	Osc_FluoroHeight = 0;
	Osc_FluoroFrameRate = 0;

	RETURNVALUE = 0;
}

//TICS +ORG#011
//TICS +STA#002
//TICS +INT#021