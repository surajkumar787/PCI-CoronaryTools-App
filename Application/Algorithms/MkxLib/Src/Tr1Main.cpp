// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#include <stdio.h> 
#include <stdlib.h> 
#include <math.h> 

#include <Tr1.h> 

#define MKX_DEBUG_TRHIGH 0
#define PRINTREASONFORSKIP 0
#define PRINTAL     0    // Yes/No printing of the incoming alarms
#define PRINTERR    0    // Yes/No printing of the Error matrix
#define PRINTTRACK  0    // Yes/No printing of the tracks

#define PRFS(T, C, A) if (A) printf("Tr at %d: Best track t-1 (" #A ") %s\n", (T), (C));
#define PRFS2(T, val1, val2, A) if (A) printf("Tr at %d: (" #A ") %8.3f > %8.3f\n", (T), (val1), (val2));

//====================================================================
// Deep copy for CTr1Pred
void Tr1PredCopy(CTr1Pred* OutPred, CTr1Pred* InPred, int NghbN)
{

  OutPred->Cx = InPred->Cx;
  OutPred->Cy = InPred->Cy;
  OutPred->Angle = InPred->Angle;
  OutPred->Breadth = InPred->Breadth;
  OutPred->X1 = InPred->X1;
  OutPred->Y1 = InPred->Y1;
  OutPred->X2 = InPred->X2;
  OutPred->Y2 = InPred->Y2;
}

//====================================================================
// Calculate integration factor Alpha (on the past)
void Tr1CalcAlpha(int Time, float *Alpha, float AlphaMax)
{
  float AlphaPrev = *Alpha, AlphaNew;

  if (Time == 0) AlphaNew = (float)0; // This alpha value is normally not used
  else AlphaNew = AlphaMax / ((float)1 + AlphaMax - AlphaPrev);

  *Alpha = AlphaNew;
}

//====================================================================
// Calculate merit from strength and prediction error
float Tr1Merit(float Strength, float PredErr, float StrengthImpact)
{
  return (StrengthImpact*Strength + (1 - StrengthImpact)*(1 - PredErr));
}


//====================================================================
// Tracking process instantiation
CTr1Tracking* Tr1Create(CTr1Param *Param)
{
  CTr1Tracking      *This;
  CTr1AlarmStack    *AlStack;
  CTr1TrackStack    *TrStack;
  CTr1Track         *Track;
  //CTr1Alarm         *Alarm;
  CTr1PredErrFields DummyPredErrFields;
  int              t, kTr;// , kAl;

  This = (CTr1Tracking*)malloc(sizeof(CTr1Tracking));
  This->Time = -1;
  This->TrAlpha = 0.f; //PL_070426

  // Parameter copying (from external to internal values)
  This->Param = *Param; // Must be a deep copy

  // Alarm stack allocation
  AlStack = &(This->AlarmStack);
  AlStack->Depth = This->Param.StackDepth;
  AlStack->MaxNbAlarmEver = This->Param.MaxNbAlarmEver;
  AlStack->Planes = (CTr1AlarmPlane*)calloc(sizeof(CTr1AlarmPlane), AlStack->Depth);

  // Alarm planes allocation
  for (t = 0; t < AlStack->Depth; t++)
  {
    AlStack->Planes[t].NbAlarm = 0;
    AlStack->Planes[t].Alarms = (CTr1Alarm*)calloc(sizeof(CTr1Alarm), AlStack->MaxNbAlarmEver);
    //for (kAl = 0; kAl < AlStack->MaxNbAlarmEver; kAl++)
    //{
    //  Alarm = &(AlStack->Planes[t].Alarms[kAl]);
    //}
  }
  // Track stack allocation
  TrStack = &(This->TrackStack);
  TrStack->Depth = This->Param.StackDepth;
  TrStack->MaxNbTrack = This->Param.MaxNbAlarmEver * 2; // At most 1 virtual per alarm
  TrStack->Planes = (CTr1TrackPlane*)calloc(sizeof(CTr1TrackPlane), TrStack->Depth);

  // For each track plane
  for (t = 0; t < TrStack->Depth; t++)
  {
    TrStack->Planes[t].NbTrack = 0;
    TrStack->Planes[t].Tracks = (CTr1Track*)calloc(sizeof(CTr1Track), TrStack->MaxNbTrack);

    // Within each track plane : track-related allocation
    for (kTr = 0; kTr < TrStack->MaxNbTrack; kTr++)
    {
      Track = &(TrStack->Planes[t].Tracks[kTr]);
      Track->CurPredErrors = (CTr1PredErr*)calloc(sizeof(CTr1PredErr), AlStack->MaxNbAlarmEver);
      Track->AvrPredErrors = (CTr1PredErr*)calloc(sizeof(CTr1PredErr), AlStack->MaxNbAlarmEver);
    }
  }

  // Allocating room for resulting ranked indices
  This->RankedIdx = (int*)malloc(This->TrackStack.MaxNbTrack*sizeof(int));

  // Initialize dummy fields
  DummyPredErrFields.Angle = -1;
  DummyPredErrFields.Breadth = -1;
  DummyPredErrFields.Centroid = -1;
  DummyPredErrFields.Composite = -1;
  This->DummyErr.Raw = DummyPredErrFields;
  This->DummyErr.Fuz = DummyPredErrFields;

  return This;
}

//====================================================================
// Destruction of a tracking process
void Tr1Delete(CTr1Tracking* This)
{
  CTr1AlarmStack *AlStack = &(This->AlarmStack);
  CTr1TrackStack *TrStack = &(This->TrackStack);
  CTr1Track      *Track;
  //CTr1Alarm      *Alarm;
  int           t, kTr;// , kAl;


  /////////////////////////////////////////////////////////
  // Delete output-related material
  free(This->RankedIdx);

  /////////////////////////////////////////////////////////
  // Delete track-related material

  // For each track plane 
  for (t = 0; t < TrStack->Depth; t++)
  {
    // For each track : free track-internal-related material
    for (kTr = 0; kTr < TrStack->MaxNbTrack; kTr++)
    {
      Track = &(TrStack->Planes[t].Tracks[kTr]);
      free(Track->AvrPredErrors);
      free(Track->CurPredErrors);
    }
    // Free all tracks in each track plane
    free(TrStack->Planes[t].Tracks);
  }

  // Free all track planes from track stack
  free(TrStack->Planes);


  /////////////////////////////////////////////////////////
  // Delete alarm-related material

  // For each alarm plane
  for (t = 0; t < AlStack->Depth; t++)
  {
    //for (kAl = 0; kAl < AlStack->MaxNbAlarmEver; kAl++)
    //{
    //  Alarm = &(AlStack->Planes[t].Alarms[kAl]);
    //}
    free(AlStack->Planes[t].Alarms);
  }

  // Free all alarm planes from alarm stack
  free(AlStack->Planes);

  /////////////////////////////////////////////////////////
  // Delete the tracking process itself

  free(This);
}


//====================================================================
// Get a reference on alarm plane corresponding to Time 
CTr1AlarmPlane* Tr1GetAlarmPlane(CTr1Tracking* This, int Time)
{
  CTr1AlarmStack *AlStack;
  AlStack = &(This->AlarmStack);
  return &(AlStack->Planes[Time%AlStack->Depth]);
}

//====================================================================
// Get a reference on track plane corresponding to Time 
CTr1TrackPlane* Tr1GetTrackPlane(CTr1Tracking* This, int Time)
{
  CTr1TrackStack *TrStack;
  TrStack = &(This->TrackStack);
  return &(TrStack->Planes[Time%TrStack->Depth]);
}


//====================================================================
// Manage the no-alarm case : copy track plane from t-1 to t and set skipping mode
void Tr1ManageNoAlarm(CTr1Tracking* This, CTr1TrackPlane *TrPlaneTm1, CTr1TrackPlane *TrPlaneT00, CTr1Output *Output)
{
  CTr1Track *TrTm1, *TrT00;
  int      kTr;

  // Every track at time t-1 is copied in a track a time t
  TrPlaneT00->NbTrack = TrPlaneTm1->NbTrack;
  TrPlaneT00->SkipFlag = TR1_RES_SKIP;

  // For each track at time t-1
  for (kTr = 0; kTr < TrPlaneTm1->NbTrack; kTr++)
  {
    TrTm1 = &(TrPlaneTm1->Tracks[kTr]); // Current track at t-1
    TrT00 = &(TrPlaneT00->Tracks[kTr]); // Current track at t

    // Everything copied (but Pred Error matrices)
    Tr1AlarmCopy(&(TrT00->Alarm), &(TrTm1->Alarm));

    TrT00->AlarmIndex = TrTm1->AlarmIndex;
    TrT00->AvrMerit = TrTm1->AvrMerit;
    TrT00->AvrPredErr = TrTm1->AvrPredErr;
    TrT00->AvrStrength = TrTm1->AvrStrength;
    TrT00->CurMerit = TrTm1->CurMerit;
    TrT00->CurPredErr = TrTm1->CurPredErr;
    TrT00->CurStrength = TrTm1->CurStrength;

    Tr1PredCopy(&(TrT00->Pred), &(TrTm1->Pred), 0);

    TrT00->PrevTrIndex = TrTm1->PrevTrIndex;
    TrT00->CTr1TrackStatus = TrTm1->CTr1TrackStatus;
  }

  // Setting results
  Output->CTr1OutputStatus = TR1_RES_SKIP;
  Output->X1 = -1;
  Output->Y1 = -1;
  Output->X2 = -1;
  Output->Y2 = -1;
  Output->NbCouple = 0;
}


//====================================================================
// Compute various correlation measures on the Alarm and its Predictor
void Tr1CalcMarkersMotion(CTr1Alarm *Alarm, CTr1Pred  *Pred, float *d1, float *d2, float *motion_modulus, float *motion_modulus_correlation, float *motion_correlation)
{
#define TR1_SIGNIFICANT_MOTION 3 // pixels */
  float d11, d22, d12, d21, dd, dx;

  d11 = (float)sqrt((float)((Alarm->X1 - Pred->X1)*(Alarm->X1 - Pred->X1) + (Alarm->Y1 - Pred->Y1)*(Alarm->Y1 - Pred->Y1)));
  d22 = (float)sqrt((float)((Alarm->X2 - Pred->X2)*(Alarm->X2 - Pred->X2) + (Alarm->Y2 - Pred->Y2)*(Alarm->Y2 - Pred->Y2)));
  d12 = (float)sqrt((float)((Alarm->X2 - Pred->X1)*(Alarm->X2 - Pred->X1) + (Alarm->Y2 - Pred->Y1)*(Alarm->Y2 - Pred->Y1)));
  d21 = (float)sqrt((float)((Alarm->X1 - Pred->X2)*(Alarm->X1 - Pred->X2) + (Alarm->Y1 - Pred->Y2)*(Alarm->Y1 - Pred->Y2)));
  dd = TR1_MAX(d11, d22);
  dx = TR1_MAX(d12, d21);

  if (dd <= dx) // direct association 1-1 2-2
  {
    *d1 = d11; // motion from predictor 1
    *d2 = d22; // motion from predictor 2
    // sum of absolute displacements between Pred and Alarm
    *motion_modulus = d11 + d22;
    // correlation between absolute displacements
    if (TR1_MAX(d11, d22) >= TR1_SIGNIFICANT_MOTION)
      *motion_modulus_correlation = TR1_MIN(d11, d22) / TR1_MAX(d11, d22);
    else
      *motion_modulus_correlation = 1;
    // normalized correlation between motion vectors
    if (TR1_MIN(d11, d22) != 0)
      *motion_correlation = ((Alarm->X1 - Pred->X1)*(Alarm->X2 - Pred->X2) + (Alarm->Y1 - Pred->Y1)*(Alarm->Y2 - Pred->Y2)) / (d11*d22);
    else
      *motion_correlation = 1;
  }
  else // cross association 1-2 2-1
  {
    *d1 = d12; // motion from predictor 1
    *d2 = d21; // motion from predictor 2
    // sum of absolute displacements between Pred and Alarm
    *motion_modulus = d12 + d21;
    // correlation between absolute displacements
    if (TR1_MAX(d12, d21) >= TR1_SIGNIFICANT_MOTION)
      *motion_modulus_correlation = TR1_MIN(d12, d21) / TR1_MAX(d12, d21);
    else
      *motion_modulus_correlation = 1;
    // normalized correlation between motion vectors
    if (TR1_MIN(d12, d21) != 0)
      *motion_correlation = ((Alarm->X1 - Pred->X2)*(Alarm->X2 - Pred->X1) + (Alarm->Y1 - Pred->Y2)*(Alarm->Y2 - Pred->Y1)) / (d12*d21);
    else
      *motion_correlation = 1;
  }
}

//====================================================================
// Analyse motion of alarm
void Tr1AnalyseMotion(CTr1Tracking* This, CTr1Track *Track, int Time)
{
#define TR1_MOTION_ANALYSIS_N_FRAMES 7 // frames */
  // go through the past of best track and determine the displacements of markers along the track

  int CurTime;
  int n_values;
  CTr1Alarm *Alarm = &Track->Alarm;
  CTr1Pred  *Pred = (This->Time > 0) ? &(Tr1GetTrackPlane(This, This->Time - 1)->Tracks[Track->PrevTrIndex].Pred) : &Track->Pred;

  float d1, d2, mean_d1, mean_d2, max_d1, max_d2;
  float motion_modulus, motion_modulus_correlation, motion_correlation;
  float mean_motion_modulus = 0, mean_motion_modulus_correlation = 0, mean_motion_correlation = 0;
  float max_motion_modulus = 0, max_motion_modulus_correlation = 0, max_motion_correlation = 0;

  Tr1CalcMarkersMotion(Alarm, Pred, &d1, &d2, &motion_modulus, &motion_modulus_correlation, &motion_correlation);


  mean_d1 = d1;
  mean_d2 = d2;
  mean_motion_modulus = motion_modulus;
  mean_motion_modulus_correlation = motion_modulus_correlation;
  mean_motion_correlation = motion_correlation;
  n_values = 1;

  max_d1 = d1;
  max_d2 = d2;
  max_motion_modulus = motion_modulus;
  max_motion_modulus_correlation = motion_modulus_correlation;
  max_motion_correlation = motion_correlation;

  for (CurTime = This->Time - 1; CurTime > 0 && CurTime >= Time - TR1_MOTION_ANALYSIS_N_FRAMES; CurTime--)
  {
    CTr1TrackPlane *TrPlane;

    TrPlane = Tr1GetTrackPlane(This, CurTime);        // all tracks at CurTime
    Track = &TrPlane->Tracks[Track->PrevTrIndex];     // track at CurTime coming from CurTime+1
    Alarm = &Track->Alarm;                            // alarm at CurTime
    TrPlane = Tr1GetTrackPlane(This, CurTime - 1);      // all tracks at CurTime-1
    Pred = &TrPlane->Tracks[Track->PrevTrIndex].Pred; // predictor of alarm at CurTime

    Tr1CalcMarkersMotion(Alarm, Pred, &d1, &d2, &motion_modulus, &motion_modulus_correlation, &motion_correlation);

    // accumulate to get mean value
    mean_d1 += d1;
    mean_d2 += d2;
    mean_motion_modulus += motion_modulus;
    mean_motion_modulus_correlation += motion_modulus_correlation;
    mean_motion_correlation += motion_correlation;
    n_values++;

    // get max value
    if (max_d1 < d1)
      max_d1 = d1;
    if (max_d2 < d2)
      max_d2 = d2;
    if (max_motion_modulus < motion_modulus)
      max_motion_modulus = motion_modulus;
    if (max_motion_modulus_correlation < motion_modulus_correlation)
      max_motion_modulus_correlation = motion_modulus_correlation;
    if (max_motion_correlation < motion_correlation)
      max_motion_correlation = motion_correlation;
  }
  mean_d1 /= n_values;
  mean_d2 /= n_values;
  mean_motion_modulus /= n_values;
  mean_motion_modulus_correlation /= n_values;
  mean_motion_correlation /= n_values;

}


//====================================================================
// Track evaluation w.r.t Avrerage Fuzzy Merit 
void Tr1Evaluate(CTr1Tracking* This, CTr1TrackPlane *TrPlane, const CTr1Input *Input, CTr1Output *Output)
{
  CTr1Track *Tr;
  int      kTr, i;
  int      MinAvrFuzPredErrIdx, MaxAvrStrengthIdx, MaxAvrFuzMeritIdx;
  float    MinAvrFuzPredErrVal, MaxAvrStrengthVal, MaxAvrFuzMeritVal;

  // Calculate each track status 
  MinAvrFuzPredErrIdx = MaxAvrStrengthIdx = MaxAvrFuzMeritIdx = 0;
  Tr = &(TrPlane->Tracks[0]);
  MinAvrFuzPredErrVal = Tr->AvrPredErr.Fuz.Composite;
  MaxAvrStrengthVal = Tr->AvrStrength;
  MaxAvrFuzMeritVal = Tr->AvrMerit;

  for (kTr = 1; kTr < TrPlane->NbTrack; kTr++)
  {
    Tr = &(TrPlane->Tracks[kTr]);

    // Finding track with minimun prediction error
    if (Tr->AvrPredErr.Fuz.Composite < MinAvrFuzPredErrVal)
    {
      MinAvrFuzPredErrIdx = kTr;
      MinAvrFuzPredErrVal = Tr->AvrPredErr.Fuz.Composite;
    }

    // Finding track with maximum strength
    if (Tr->AvrStrength > MaxAvrStrengthVal)
    {
      MaxAvrStrengthIdx = kTr;
      MaxAvrStrengthVal = Tr->AvrStrength;
    }

    // Finding track with maximum overall merit
    if (Tr->AvrMerit > MaxAvrFuzMeritVal)
    {
      MaxAvrFuzMeritIdx = kTr;
      MaxAvrFuzMeritVal = Tr->AvrMerit;
    }
  }

  // Status affectation
  for (kTr = 0; kTr < TrPlane->NbTrack; kTr++)
  {
    Tr = &(TrPlane->Tracks[kTr]);
    Tr->CTr1TrackStatus = TR1_COMMON;
  }

  TrPlane->Tracks[MinAvrFuzPredErrIdx].CTr1TrackStatus = TR1_PREDICTABLE; // Maximum Predictability (minimum pred error)
  TrPlane->Tracks[MaxAvrStrengthIdx].CTr1TrackStatus = TR1_STRONG;      // Maximum Strength
  TrPlane->Tracks[MaxAvrFuzMeritIdx].CTr1TrackStatus = TR1_MERITING;    // Most meriting (both predictable and strong)

  Tr1AnalyseMotion(This, &TrPlane->Tracks[MaxAvrFuzMeritIdx], This->Time);

  Tr = &(TrPlane->Tracks[MaxAvrFuzMeritIdx]);
  Output->CTr1OutputStatus = ((TrPlane->SkipFlag) ? TR1_RES_SKIP : TR1_RES_OK);

  Output->X1 = Tr->Alarm.X1;
  Output->Y1 = Tr->Alarm.Y1;
  Output->X2 = Tr->Alarm.X2;
  Output->Y2 = Tr->Alarm.Y2;

  Output->CurStrength = Tr->CurStrength;
  Output->AvrStrength = Tr->AvrStrength;
  Output->CurMerit = Tr->CurMerit;
  Output->AvrMerit = Tr->AvrMerit;
  Output->Tr1Status = Tr->CTr1TrackStatus;

  // Copying ranked couples from input to output
  Tr1GetRankedAlarmIdx(TrPlane, This->RankedIdx, &(This->NbRanked));

  Output->NbCouple = This->NbRanked;

  //PL_060721//////////////////////////////////////////////////////////////////
  //if CloseAlarm create new output couple
  for (i = 0; i < This->NbRanked; i++)
  {
    if (This->RankedIdx[i] < Input->NbCouple)
    {
      Tr1CoupleCopy(&(Output->Couples[i]), &(Input->Couples[This->RankedIdx[i]]));
    }
    else    //if NbRanked >  Input->NbCouple    create new output couple
    {
      Output->Couples[i].X1 = Tr->Alarm.X1;
      Output->Couples[i].Y1 = Tr->Alarm.Y1;
      Output->Couples[i].X2 = Tr->Alarm.X2;
      Output->Couples[i].Y2 = Tr->Alarm.Y2;
      Output->Couples[i].Strength = Tr->Alarm.Strength;
    }
  }
}

//====================================================================
// Track initialisation
void Tr1InitTrack(CTr1Tracking* This)
{
  CTr1AlarmPlane *AlPlane;
  CTr1TrackPlane *TrPlane;
  CTr1Alarm      *Alarm;
  CTr1Track      *Track;
  int            k;

#if TR1_PRINT_FOR_DEBUG
  if (This->Time != 0)
  {
    printf("Error in Tr1InitTrack : current time %d : should be 0\n", This->Time);
    TR1_MAJOR_ERR_HANDLER();
  }
#endif

  AlPlane = Tr1GetAlarmPlane(This, 0);
  TrPlane = Tr1GetTrackPlane(This, 0);

  TrPlane->NbTrack = AlPlane->NbAlarm;
  TrPlane->SkipFlag = TR1_RES_OK;
  TrPlane->Locked = TR1_RES_UNLOCKED;

  for (k = 0; k < TrPlane->NbTrack; k++)
  {
    Alarm = &(AlPlane->Alarms[k]);
    Track = &(TrPlane->Tracks[k]);

    // Initialisation of matching alarm
    Track->AlarmIndex = k;
    Tr1AlarmCopy(&(Track->Alarm), Alarm);

    // Initialisation of prediction
    Track->PrevTrIndex = -1;
    Track->Pred.Cx = Alarm->Cx;
    Track->Pred.Cy = Alarm->Cy;
    Track->Pred.Angle = Alarm->Angle;
    Track->Pred.Breadth = Alarm->Breadth;
    Track->Pred.X1 = Alarm->X1;
    Track->Pred.Y1 = Alarm->Y1;
    Track->Pred.X2 = Alarm->X2;
    Track->Pred.Y2 = Alarm->Y2;

    // Initialisation of average prediction errors to zero (for correct recursivity)
    Track->AvrPredErr.Raw.Angle = 0;
    Track->AvrPredErr.Raw.Breadth = 0;
    Track->AvrPredErr.Raw.Centroid = 0;
    Track->AvrPredErr.Raw.Composite = 0;

    Track->AvrPredErr.Fuz.Angle = 0;
    Track->AvrPredErr.Fuz.Breadth = 0;
    Track->AvrPredErr.Fuz.Centroid = 0;
    Track->AvrPredErr.Fuz.Composite = 0;

    // Initialisation of measurements
    Track->AvrStrength = Track->CurStrength = Alarm->Strength;
    Track->AvrMerit = Track->CurMerit = Alarm->Strength;
  }
}

//====================================================================
// Alarm-to-track matching 
void Tr1Match(CTr1Tracking* This, CTr1AlarmPlane *AlPlane, CTr1TrackPlane *TrPlaneTm1, CTr1TrackPlane *TrPlaneT00)
{
  CTr1Track   *TrTm1, *TrT00;
  CTr1PredErr *AvrPredErr;
  int        kAl, kTr, kTrMinArg;
  float      MinAvrFuzErr, TrAlpha = (float)This->TrAlpha;

  // No virtual indication yet
  TrPlaneT00->SkipFlag = TR1_RES_OK;

  //At this stage, each alarm creates a track head at time t
  TrPlaneT00->NbTrack = AlPlane->NbAlarm;

  // For each alarm at time t
  for (kAl = 0; kAl < AlPlane->NbAlarm; kAl++)
  {
    // For each track at time t-1 : find best alarm-track match (w.r.t average fuzzy composite error)
    kTrMinArg = 0;
    MinAvrFuzErr = TrPlaneTm1->Tracks[kTrMinArg].AvrPredErrors[kAl].Fuz.Composite;
    for (kTr = 1; kTr < TrPlaneTm1->NbTrack; kTr++)
    {
      TrTm1 = &(TrPlaneTm1->Tracks[kTr]);
      AvrPredErr = &(TrTm1->AvrPredErrors[kAl]);

      if (AvrPredErr->Fuz.Composite < MinAvrFuzErr) // Best match condition
      {
        kTrMinArg = kTr;
        MinAvrFuzErr = AvrPredErr->Fuz.Composite;
      }
    }

    TrT00 = &(TrPlaneT00->Tracks[kAl]);
    TrTm1 = &(TrPlaneTm1->Tracks[kTrMinArg]);

    TrT00->AlarmIndex = kAl;
    Tr1AlarmCopy(&(TrT00->Alarm), &(AlPlane->Alarms[kAl]));
    TrT00->PrevTrIndex = kTrMinArg;

    TrT00->CurPredErr = TrTm1->CurPredErrors[kAl]; // Deep copy
    TrT00->AvrPredErr = TrTm1->AvrPredErrors[kAl]; // Deep copy

    TrT00->CurStrength = TrT00->Alarm.Strength;
    TrT00->AvrStrength = (TrTm1->AvrStrength*TrAlpha + TrT00->CurStrength*(1 - TrAlpha));

    TrT00->CurMerit = Tr1Merit(TrT00->CurStrength, TrT00->CurPredErr.Fuz.Composite, This->Param.StrengthImpact);
    TrT00->AvrMerit = Tr1Merit(TrT00->AvrStrength, TrT00->AvrPredErr.Fuz.Composite, This->Param.StrengthImpact);
  }
}

//====================================================================
// Prediction of track plane t+1 (stored in track plane t) 
void Tr1Predict(CTr1Tracking* This, CTr1TrackPlane *TrPlaneTm1, CTr1TrackPlane *TrPlaneT00)
{
  CTr1Alarm *Alarm;
  CTr1Track *TrTm1, *TrT00;
  CTr1Pred  *PredTm1, *PredT00;
  int      kTr;

  // for each track at time t
  for (kTr = 0; kTr < TrPlaneT00->NbTrack; kTr++)
  {
    TrT00 = &(TrPlaneT00->Tracks[kTr]);                // Current track at t
    Alarm = &(TrT00->Alarm);                           // Matching alarm at t
    TrTm1 = &(TrPlaneTm1->Tracks[TrT00->PrevTrIndex]); // Matching track at t-1
    PredT00 = &(TrT00->Pred);                            // Current prediction at t
    PredTm1 = &(TrTm1->Pred);                            // Previous prediction (t-1)

    // Zero-order prediction
    if (TrT00->AlarmIndex >= 0) // not a virtual track : prediction based on alarm
    {
      PredT00->Angle = Alarm->Angle;
      PredT00->Breadth = Alarm->Breadth;

      // Test of other predictors 
      //	PredT00->Angle   =   Alpha * TrTm1->Alarm.Angle   + ((float)1 - Alpha) * Alarm->Angle;
      //	PredT00->Breadth =   Alpha * TrTm1->Alarm.Breadth + ((float)1 - Alpha) * Alarm->Breadth;
      //	PredT00->Angle =  Alarm->Angle     + (float)1 * (Alarm->Angle - TrTm1->Alarm.Angle);
      //	PredT00->Breadth =  Alarm->Breadth + (float)1 *(Alarm->Breadth - TrTm1->Alarm.Breadth);
      //	PredT00->Cx =  Alarm->Cx + (float)1 * (Alarm->Cx - TrTm1->Alarm.Cx);
      //	PredT00->Cy =  Alarm->Cy + (float)1 * (Alarm->Cy - TrTm1->Alarm.Cy);

      PredT00->Cx = Alarm->Cx;
      PredT00->Cy = Alarm->Cy;
      PredT00->X1 = Alarm->X1;
      PredT00->Y1 = Alarm->Y1;
      PredT00->X2 = Alarm->X2;
      PredT00->Y2 = Alarm->Y2;
    }
    else // virtual track : prediction from past only
    {
      Tr1PredCopy(PredT00, PredTm1, 0);
    }
  }
}

//====================================================================
// When several tracks are associated to the same track at t-1, penalize bad ones
void Tr1PenalizeContender(CTr1Tracking* This, CTr1TrackPlane *TrPlane)
{
  CTr1Track   *Tr, *Tr2;
  int        kTr, kTr2, *Penalized;
  float      TrAlpha = (float)This->TrAlpha;

  Penalized = (int*)calloc((TrPlane->NbTrack + 1), sizeof(int));

  // for each track at time t : find if it should be penalized
  for (kTr = 0; kTr < TrPlane->NbTrack; kTr++)
  {
    Tr = &(TrPlane->Tracks[kTr]);

    for (kTr2 = 0; kTr2<TrPlane->NbTrack; kTr2++)
    {
      Tr2 = &(TrPlane->Tracks[kTr2]);

      if (kTr != kTr2 && Tr->PrevTrIndex == Tr2->PrevTrIndex)
      {
        // If found another track sharing the same past and more meriting
        if (Tr2->AvrMerit > Tr->AvrMerit)
        {
          Penalized[kTr] = 1;
          break;
        }
      }
    }
  }
  // for each track at time t : apply penality if requested
  for (kTr = 0; kTr < TrPlane->NbTrack; kTr++)
  {
    if (Penalized[kTr])
    {
      Tr = &(TrPlane->Tracks[kTr]);

      Tr->AvrPredErr.Raw.Angle = Tr->AvrPredErr.Raw.Angle     * (2 - TrAlpha);
      Tr->AvrPredErr.Raw.Breadth = Tr->AvrPredErr.Raw.Breadth   * (2 - TrAlpha);
      Tr->AvrPredErr.Raw.Centroid = Tr->AvrPredErr.Raw.Centroid  * (2 - TrAlpha);
      Tr->AvrPredErr.Raw.Composite = Tr->AvrPredErr.Raw.Composite * (2 - TrAlpha);

      Tr->AvrPredErr.Fuz.Angle = Tr->AvrPredErr.Fuz.Angle     * (2 - TrAlpha);
      Tr->AvrPredErr.Fuz.Breadth = Tr->AvrPredErr.Fuz.Breadth   * (2 - TrAlpha);
      Tr->AvrPredErr.Fuz.Centroid = Tr->AvrPredErr.Fuz.Centroid  * (2 - TrAlpha);
      Tr->AvrPredErr.Fuz.Composite = Tr->AvrPredErr.Fuz.Composite * (2 - TrAlpha);

      Tr->AvrStrength = Tr->AvrStrength * TrAlpha;

      Tr->AvrMerit = Tr1Merit(Tr->AvrStrength, Tr->AvrPredErr.Fuz.Composite, This->Param.StrengthImpact);
    }
  }

  free(Penalized);
}
//====================================================================
//PL_060720 
void Tr1FindClosestAlarm(CTr1TrackPlane *TrPlaneTm1, int NbAlarm, int kTrTm1, int* PtkAl)
{
  CTr1Track *TrTm1;
  int kAl, kAlMinArg;
  float MinAvrFuzErr, CurAvrFuzErr;

  TrTm1 = &(TrPlaneTm1->Tracks[kTrTm1]); // Provided track at t-1

  // Find the alarm-t closest to provided track t-1
  kAlMinArg = 0;
  MinAvrFuzErr = TrTm1->AvrPredErrors[0].Fuz.Composite;

  for (kAl = 1; kAl < NbAlarm; kAl++)
  {
    CurAvrFuzErr = TrTm1->AvrPredErrors[kAl].Fuz.Composite;
    if (CurAvrFuzErr < MinAvrFuzErr)
    {
      kAlMinArg = kAl;
      MinAvrFuzErr = CurAvrFuzErr;
    }
  }

  *PtkAl = kAlMinArg;
}

void Tr1PrintTrack(CTr1Track  *t)
{
  printf("AlarmIndex: %d\n", t->AlarmIndex);
  printf("PrevTrIndex: %d\n", t->PrevTrIndex);
  printf("CurStrength: %6.2f\n", t->CurStrength);
  printf("AvrStrength: %6.2f\n", t->AvrStrength);
  printf("CurMerit: %6.2f\n", t->CurMerit);
  printf("AvrMerit: %6.2f\n", t->AvrMerit);
  printf("Status: %d\n", t->CTr1TrackStatus);
}

//====================================================================
// Adding virtual track if good but non-correctly-matching track at t-1
void Tr1AddVirtual(CTr1Tracking* This, CTr1TrackPlane *TrPlaneTm1, CTr1TrackPlane *TrPlaneT00, CTr1AlarmPlane *AlPlane, int InitTime, int VirtualWhileInit, CTr1AdvancedOutput *advOutput)
{
  CTr1Track   *TrTm1, *TrT00, *Virtual;
  CTr1PredErr *CurPredErr;
  //CTr1Alarm   *Alarm;
  int        Time, kTrTm1, kTrT00, kAl, BestMatchFoundForMeriting, kTrT00Found = 0, NbTrackAtT00BeforeVirtual;
  int        TrackTm1AssociatedToSth, BestMatchTooFar = 0;
  float      TrAlpha, MaxMerit, MeritThresh;
  int        NoCloseAlarm;
  float      TrVirtualPenalize;
  int        PrintReasonForSkip = PRINTREASONFORSKIP;
  int        PrintTrack = PRINTTRACK; // Yes/No printing of the tracks
  int        MKX_Debug_TRHIGH = MKX_DEBUG_TRHIGH;

  Time = This->Time;
  MeritThresh = (float)0.75;
  MaxMerit = (float)0;

  // Early return if too early for virtual track creation
  if (VirtualWhileInit == 0 && Time < InitTime)
  {
    return;
  }

  NbTrackAtT00BeforeVirtual = TrPlaneT00->NbTrack;
  TrAlpha = This->TrAlpha;
  TrVirtualPenalize = (float)(100 - This->Param.VirtualPenalize) / (float)100;

  // For each track at time t-1
  for (kTrTm1 = 0; kTrTm1 < TrPlaneTm1->NbTrack; kTrTm1++)
  {
    TrTm1 = &(TrPlaneTm1->Tracks[kTrTm1]); // Current track at t-1
    TrackTm1AssociatedToSth = 0; // No association to time t found or created yet

    // If track remarkable (both strong and predicatable)
    if (TrTm1->CTr1TrackStatus == TR1_MERITING || (VirtualWhileInit && Time < InitTime && TrTm1->AvrMerit >= MeritThresh))
    {
      // Find if that t-1 track is linked at least to one track at t
      // And if so, find the best matching t track
      for (BestMatchFoundForMeriting = 0, kTrT00 = 0; kTrT00 < NbTrackAtT00BeforeVirtual; kTrT00++)
      {
        TrT00 = &(TrPlaneT00->Tracks[kTrT00]); // Current track at t
        if (TrT00->PrevTrIndex == kTrTm1)
        {
          if (BestMatchFoundForMeriting == 0 || TrT00->AvrMerit > MaxMerit)
          {
            MaxMerit = TrT00->AvrMerit;
            kTrT00Found = kTrT00;
          }
          BestMatchFoundForMeriting = 1;
        }
      }

      // if most meriting track at t-1 is not link to a track at t
      // Find if alarm within acceptable range. If Yes, create supporting track
      if (!BestMatchFoundForMeriting && TrTm1->CTr1TrackStatus == TR1_MERITING)
      {
        //Alarm = NULL;
        Tr1FindClosestAlarm(TrPlaneTm1, AlPlane->NbAlarm, kTrTm1, &kAl);
        //printf("==================> Closest alarm to most meriting Tr[%d] at t-1 = %d\n", kTrTm1, kAl);

        CurPredErr = &(TrPlaneTm1->Tracks[kTrTm1].CurPredErrors[kAl]);
        NoCloseAlarm = 0;
        if (CurPredErr->Raw.Angle > This->Param.SkipAngleDist) NoCloseAlarm++;
        if (CurPredErr->Raw.Angle > 2 * This->Param.SkipAngleDist) NoCloseAlarm++;
        if (CurPredErr->Raw.Breadth > This->Param.SkipBreadthDist) NoCloseAlarm++;
        if (CurPredErr->Raw.Breadth > 2 * This->Param.SkipBreadthDist) NoCloseAlarm++;
        if (CurPredErr->Raw.Centroid > This->Param.SkipCentroidDist) NoCloseAlarm++;
        if (CurPredErr->Raw.Centroid > 2 * This->Param.SkipCentroidDist) NoCloseAlarm++;
        NoCloseAlarm = (NoCloseAlarm > 1);

        if (PrintReasonForSkip)
        {
          PRFS(Time, "For closest alarm at t", CurPredErr->Raw.Angle > This->Param.SkipAngleDist);
          PRFS(Time, "For closest alarm at t", CurPredErr->Raw.Angle > 2 * This->Param.SkipAngleDist);
          PRFS(Time, "For closest alarm at t", CurPredErr->Raw.Breadth > This->Param.SkipBreadthDist);
          PRFS(Time, "For closest alarm at t", CurPredErr->Raw.Breadth > 2 * This->Param.SkipBreadthDist);
          PRFS(Time, "For closest alarm at t", CurPredErr->Raw.Centroid > This->Param.SkipCentroidDist);
          PRFS(Time, "For closest alarm at t", CurPredErr->Raw.Centroid > 2 * This->Param.SkipCentroidDist);
        }

        if (!NoCloseAlarm)
        {
          TrT00 = &(TrPlaneT00->Tracks[TrPlaneT00->NbTrack]);
          TrTm1 = &(TrPlaneTm1->Tracks[kTrTm1]);
          TrT00->AlarmIndex = kAl;
          Tr1AlarmCopy(&(TrT00->Alarm), &(AlPlane->Alarms[kAl]));

          TrT00->PrevTrIndex = kTrTm1;
          TrT00->CurPredErr = TrTm1->CurPredErrors[kAl]; // Deep copy
          TrT00->AvrPredErr = TrTm1->AvrPredErrors[kAl]; // Deep copy
          TrT00->CurStrength = TrT00->Alarm.Strength;
          TrT00->AvrStrength = (TrTm1->AvrStrength*TrAlpha + TrT00->CurStrength*(1 - TrAlpha));
          TrT00->CurMerit = Tr1Merit(TrT00->CurStrength, TrT00->CurPredErr.Fuz.Composite, This->Param.StrengthImpact);
          TrT00->AvrMerit = Tr1Merit(TrT00->AvrStrength, TrT00->AvrPredErr.Fuz.Composite, This->Param.StrengthImpact);
          TrPlaneT00->NbTrack++;
        }

        TrackTm1AssociatedToSth |= !NoCloseAlarm;
      }

      if (BestMatchFoundForMeriting) // If matching track, find if too far
      {
        TrT00 = &(TrPlaneT00->Tracks[kTrT00Found]); // best matching t track
        CurPredErr = &(TrT00->CurPredErr);
        BestMatchTooFar = 0;
        if (CurPredErr->Raw.Angle > This->Param.SkipAngleDist) BestMatchTooFar++;
        if (CurPredErr->Raw.Angle > 2 * This->Param.SkipAngleDist) BestMatchTooFar++;
        if (CurPredErr->Raw.Breadth > This->Param.SkipBreadthDist) BestMatchTooFar++;
        if (CurPredErr->Raw.Breadth > 2 * This->Param.SkipBreadthDist) BestMatchTooFar++;
        if (CurPredErr->Raw.Centroid > This->Param.SkipCentroidDist) BestMatchTooFar++;
        if (CurPredErr->Raw.Centroid > 2 * This->Param.SkipCentroidDist) BestMatchTooFar++;
        BestMatchTooFar = (BestMatchTooFar > 1);
        if (PrintReasonForSkip)
        {
          PRFS2(Time, CurPredErr->Raw.Angle, This->Param.SkipAngleDist, CurPredErr->Raw.Angle > This->Param.SkipAngleDist);
          PRFS2(Time, CurPredErr->Raw.Angle, 2 * This->Param.SkipAngleDist, CurPredErr->Raw.Angle > 2 * This->Param.SkipAngleDist);
          PRFS2(Time, CurPredErr->Raw.Breadth, This->Param.SkipBreadthDist, CurPredErr->Raw.Breadth > This->Param.SkipBreadthDist);
          PRFS2(Time, CurPredErr->Raw.Breadth, 2 * This->Param.SkipBreadthDist, CurPredErr->Raw.Breadth > 2 * This->Param.SkipBreadthDist);
          PRFS2(Time, CurPredErr->Raw.Centroid, This->Param.SkipCentroidDist, CurPredErr->Raw.Centroid > This->Param.SkipCentroidDist);
          PRFS2(Time, CurPredErr->Raw.Centroid, 2 * This->Param.SkipCentroidDist, CurPredErr->Raw.Centroid > 2 * This->Param.SkipCentroidDist);
        }
        if (MKX_Debug_TRHIGH)
        {
          if (TrT00->CurPredErr.Raw.Angle > This->Param.HighAngleDist)       printf("Angle   =%8.3f > %8.3f\n", TrT00->CurPredErr.Raw.Angle, This->Param.HighAngleDist);
          if (TrT00->CurPredErr.Raw.Breadth > This->Param.HighBreadthDist)   printf("Breadth =%8.3f > %8.3f\n", TrT00->CurPredErr.Raw.Breadth, This->Param.HighBreadthDist);
          if (TrT00->CurPredErr.Raw.Centroid > This->Param.HighCentroidDist) printf("Centroid=%8.3f > %8.3f\n", TrT00->CurPredErr.Raw.Centroid, This->Param.HighCentroidDist);
          //if(TrT00->CurPredErr.Raw.Correl>This->Param.HighCorrelDist)     printf("Correl  =%8.3f > %8.3f\n", TrT00->CurPredErr.Raw.Correl, This->Param.HighCorrelDist);
        }
        advOutput->CurRaw[Tr1Angle] = TrT00->CurPredErr.Raw.Angle;
        advOutput->CurRaw[Tr1Breadth] = TrT00->CurPredErr.Raw.Breadth;
        advOutput->CurRaw[Tr1Centroid] = TrT00->CurPredErr.Raw.Centroid;
        advOutput->CurRaw[Tr1Composite] = TrT00->CurPredErr.Raw.Composite;

        advOutput->CurFuz[Tr1Angle] = TrT00->CurPredErr.Fuz.Angle;
        advOutput->CurFuz[Tr1Breadth] = TrT00->CurPredErr.Fuz.Breadth;
        advOutput->CurFuz[Tr1Centroid] = TrT00->CurPredErr.Fuz.Centroid;
        advOutput->CurFuz[Tr1Composite] = TrT00->CurPredErr.Fuz.Composite;

        advOutput->AvrRaw[Tr1Angle] = TrT00->AvrPredErr.Raw.Angle;
        advOutput->AvrRaw[Tr1Breadth] = TrT00->AvrPredErr.Raw.Breadth;
        advOutput->AvrRaw[Tr1Centroid] = TrT00->AvrPredErr.Raw.Centroid;
        advOutput->AvrRaw[Tr1Composite] = TrT00->AvrPredErr.Raw.Composite;

        advOutput->AvrFuz[Tr1Angle] = TrT00->AvrPredErr.Fuz.Angle;
        advOutput->AvrFuz[Tr1Breadth] = TrT00->AvrPredErr.Fuz.Breadth;
        advOutput->AvrFuz[Tr1Centroid] = TrT00->AvrPredErr.Fuz.Centroid;
        advOutput->AvrFuz[Tr1Composite] = TrT00->AvrPredErr.Fuz.Composite;

        advOutput->High[Tr1Angle] = This->Param.HighAngleDist;
        advOutput->High[Tr1Breadth] = This->Param.HighBreadthDist;
        advOutput->High[Tr1Centroid] = This->Param.HighCentroidDist;

        advOutput->Skip[Tr1Angle] = This->Param.SkipAngleDist;
        advOutput->Skip[Tr1Breadth] = This->Param.SkipBreadthDist;
        advOutput->Skip[Tr1Centroid] = This->Param.SkipCentroidDist;

        TrackTm1AssociatedToSth |= !BestMatchTooFar;
      }
      // If no good matching track at t and no close alarm to meriting: virtual track creation
      if (TrackTm1AssociatedToSth == 0)
      {
        if (TrPlaneT00->NbTrack >= This->TrackStack.MaxNbTrack)
        {
#if TR1_PRINT_FOR_DEBUG
          printf("Error in Tr1AddVirtual: No room for virtual track creation\n");
          TR1_MAJOR_ERR_HANDLER();
#endif
        }

        // Indicating that the current track plane is to be skipped
        TrPlaneT00->SkipFlag = TR1_RES_SKIP;

        Virtual = &(TrPlaneT00->Tracks[TrPlaneT00->NbTrack]);
        Virtual->AlarmIndex = -1;
        Virtual->PrevTrIndex = kTrTm1;
        Tr1AlarmCopy(&(Virtual->Alarm), &(TrTm1->Alarm));

        Virtual->CurPredErr = This->DummyErr;
        Virtual->AvrPredErr = TrTm1->AvrPredErr;

        Virtual->CurStrength = (float)-1;
        Virtual->AvrStrength = TrTm1->AvrStrength;

        // Apply virtual penalizing
        VirtualPenalizing(Virtual, TrVirtualPenalize);

        // Compute Merit from Strength and Predictability
        Virtual->CurMerit = (float)-1;
        Virtual->AvrMerit = Tr1Merit(Virtual->AvrStrength, Virtual->AvrPredErr.Fuz.Composite, This->Param.StrengthImpact);

        TrPlaneT00->NbTrack++;

        if (PrintTrack) Tr1PrintTrack(Virtual);
      }
    }
  }
}

//====================================================================
// VirtualPenalizing
void VirtualPenalizing(CTr1Track *Virtual, float Coef)
{
  Virtual->AvrPredErr.Raw.Angle = Virtual->AvrPredErr.Raw.Angle     * (2 - Coef);
  Virtual->AvrPredErr.Raw.Breadth = Virtual->AvrPredErr.Raw.Breadth   * (2 - Coef);
  Virtual->AvrPredErr.Raw.Centroid = Virtual->AvrPredErr.Raw.Centroid  * (2 - Coef);
  Virtual->AvrPredErr.Raw.Composite = Virtual->AvrPredErr.Raw.Composite * (2 - Coef);

  Virtual->AvrPredErr.Fuz.Angle = Virtual->AvrPredErr.Fuz.Angle     * (2 - Coef);
  Virtual->AvrPredErr.Fuz.Breadth = Virtual->AvrPredErr.Fuz.Breadth   * (2 - Coef);
  Virtual->AvrPredErr.Fuz.Centroid = Virtual->AvrPredErr.Fuz.Centroid  * (2 - Coef);
  Virtual->AvrPredErr.Fuz.Composite = Virtual->AvrPredErr.Fuz.Composite * (2 - Coef);

  Virtual->AvrStrength = Virtual->AvrStrength * Coef;
}

//====================================================================
// Get pointer on tracker parameters 
void Tr1GetParamPt(CTr1Hdl TrackingHdl, CTr1Param **ParamPt)
{
  CTr1Tracking* This = (CTr1Tracking *)TrackingHdl;
  if (TrackingHdl == 0) return; // In case the tracker would not be instantiated
  *ParamPt = &(This->Param);
}

//====================================================================
// Update tracks from previous tracks and current alarms
void Tr1Iterate(
  CTr1Hdl TrackingHdl,
  int Time,
  const CTr1Input *Input,
  CTr1Output *Output, CTr1AdvancedOutput *advOutput
  )
{
  CTr1Tracking* This = (CTr1Tracking *)TrackingHdl;
  CTr1AlarmPlane *AlPlane;
  CTr1TrackPlane *TrPlaneT00, *TrPlaneTm1;
  int           T00, Tm1, NbAlarm;
  int           InitTime = This->Param.InitTime;
  int           VirtualWhileInit = This->Param.VirtualWhileInit;
  int           PrintAl = PRINTAL;    // Yes/No printing of the incoming alarms
  int           PrintErr = PRINTERR;   // Yes/No printing of the Error matrix
  int           PrintTrack = PRINTTRACK; // Yes/No printing of the tracks
  int           BestTrackIdx;
  int           Locked = TR1_RES_UNLOCKED;

  // Read alarms to fill-in a new alarm plane and increment internal time counter
  Tr1FillAlarmPlane(This, Time, Input);

  Tr1NormalizeAlarmStrength(This, This->Time);

  if (PrintAl)	Tr1PrintAlarmPlane(This, This->Time);

  // Time instants after alarm reading
  T00 = This->Time;
  Tm1 = T00 - 1;

  // Evaluate integration factor w.r.t current time
  Tr1CalcAlpha(This->Time, &(This->TrAlpha), This->Param.TrAlpha);

  // Building track plane 0 (initialisation)
  if (T00 == 0)
  {
    Tr1InitTrack(This);
    TrPlaneT00 = Tr1GetTrackPlane(This, T00);
    Tr1Evaluate(This, TrPlaneT00, Input, Output);
    if (PrintTrack) Tr1PrintTrackPlane(This, T00);
    Output->Locked = TR1_RES_UNLOCKED;
    Output->CTr1OutputStatus = TR1_RES_SKIP;

    return;
  }

  // Building track plane t from current alarms and prev tracks 
  AlPlane = Tr1GetAlarmPlane(This, T00);
  NbAlarm = AlPlane->NbAlarm;
  TrPlaneTm1 = Tr1GetTrackPlane(This, Tm1);
  TrPlaneT00 = Tr1GetTrackPlane(This, T00);

  // Managing the zero-alarm case
  if (!NbAlarm)
  {
    Tr1ManageNoAlarm(This, TrPlaneTm1, TrPlaneT00, Output); // Copy t-1 tracks into t track plane, and set skipping mode

    // added 27-05-04
    if (This->Param.LockCount - 1 <= T00)
    {
      BestTrackIdx = Tr1GetBestTrackIdx(This, T00);
      if (BestTrackIdx >= 0)
        Tr1Lock(This, T00, BestTrackIdx, T00 - This->Param.LockCount + 1, This->Param.LockMaxConSkip, &Locked);
      else
        Locked = TR1_RES_UNLOCKED;
    }
    Output->Locked = Locked;

    // forces Status to SKIP at beginning of sequence
    if (Time < InitTime)
      Output->CTr1OutputStatus = TR1_RES_SKIP;

    return;                                                 // Pre-mature returning in that degenerated case
  }

  if (PrintAl)		Tr1PrintTracks(TrPlaneTm1, This->Time);

  // Managing current case
  Tr1CalcPredErr(This, AlPlane, TrPlaneTm1);                // Error matrix between all Al(t) and all Pred(t|t-1)

  if (PrintErr)
  {
    Tr1PrintPredErr(This, NbAlarm, TrPlaneTm1, TR1_CUR, TR1_RAW);    // Printing pred err matrix (current raw)
    Tr1PrintPredErr(This, NbAlarm, TrPlaneTm1, TR1_AVR, TR1_FUZ);    // Printing pred err matrix (averaged fuzzy)
  }

  Tr1Match(This, AlPlane, TrPlaneTm1, TrPlaneT00);          // Tr(t) = Match( Al(t), Pred(t|t-1) ), given error matrix

  Tr1PenalizeContender(This, TrPlaneT00);                   // Favor best track among those matching the same alarm

  Tr1AddVirtual(This, TrPlaneTm1, TrPlaneT00,               // If(Tr(t-1) good and no good match to an Al(t)) -> virtual
    AlPlane, InitTime, VirtualWhileInit, advOutput);             // Virtual might be added during tracker init phase if flag ON

  Tr1Predict(This, TrPlaneTm1, TrPlaneT00);                 // Pred(t+1|t) = Prediction(Tr(t), Tr(t-1))

  Tr1Evaluate(This, TrPlaneT00, Input, Output);    // Track evaluation

  if (PrintTrack)	Tr1PrintTrackPlane(This, T00);             // Print final Tr(t) 

  if (This->Param.LockCount - 1 <= T00)
  {
    BestTrackIdx = Tr1GetBestTrackIdx(This, T00);
    if (BestTrackIdx >= 0)Tr1Lock(This, T00, BestTrackIdx, T00 - This->Param.LockCount + 1, This->Param.LockMaxConSkip, &Locked);
    else			           Locked = TR1_RES_UNLOCKED;
  }
  Output->Locked = Locked;

  // forces Status to SKIP at beginning of sequence
  if (Time < InitTime)
    Output->CTr1OutputStatus = TR1_RES_SKIP;

}


/***************************************************************************/
//         END OF FILE                                                     */
/***************************************************************************/

