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

//====================================================================
// Calculates the positive angle in degree of vector (x,y)
float Tr1PositiveAngle(int Y, int X)
{
	float Angle;
	Angle = (float)(atan2((double)-Y, (double)X)*57.2958);
	if(Angle<0) Angle += 180;
	return Angle;
}

//====================================================================
// Deep copy for CTr1Couple
void Tr1CoupleCopy(CTr1Couple* OutCouple, CTr1Couple* InCouple)
{

	OutCouple->X1       = InCouple->X1;
	OutCouple->Y1       = InCouple->Y1;
	OutCouple->X2       = InCouple->X2;
	OutCouple->Y2       = InCouple->Y2;
	OutCouple->Strength = InCouple->Strength;
}

//====================================================================
// Deep copy for CTr1Alarm
void Tr1AlarmCopy(CTr1Alarm* OutAlarm, CTr1Alarm* InAlarm)
{
	OutAlarm->X1          = InAlarm->X1;
	OutAlarm->Y1          = InAlarm->Y1;
	OutAlarm->X2          = InAlarm->X2;
	OutAlarm->Y2          = InAlarm->Y2;
	OutAlarm->Cx          = InAlarm->Cx;
	OutAlarm->Cy          = InAlarm->Cy;
	OutAlarm->Angle       = InAlarm->Angle;
	OutAlarm->Breadth     = InAlarm->Breadth;
	OutAlarm->Strength    = InAlarm->Strength;
	OutAlarm->Rank        = InAlarm->Rank;
	OutAlarm->MatchingRef = InAlarm->MatchingRef;
}

//====================================================================
// Determine if an alarm is next to a reference (when available)
int Tr1IsAlarmNextToRef(CTr1Alarm *Al, CTr1Ref *Ref, int Distance)
{
	if(!Ref->Available) return -1;

	return
		( TR1_EUCLI_DIST(Al->X1, Al->Y1, Ref->X1, Ref->Y1) <= Distance &&
		TR1_EUCLI_DIST(Al->X2, Al->Y2, Ref->X2, Ref->Y2) <= Distance) ||
		( TR1_EUCLI_DIST(Al->X1, Al->Y1, Ref->X2, Ref->Y2) <= Distance &&
		TR1_EUCLI_DIST(Al->X2, Al->Y2, Ref->X1, Ref->Y1) <= Distance);
}

//====================================================================
// Ordering candidate couples following their strength
void Tr1RankCouples(CTr1Couple *Couples, int NbCouple, CTr1Couple **RankedCouples)
{
	int i, j;
	CTr1Couple Work;
	int kCpl;

	// Copying input couples into a new array of couples
	// Rem: NbCouple+1 instead of NbCouple to cope with the NbCouple=0 case
	*RankedCouples = (CTr1Couple*)malloc((NbCouple+1)*sizeof(CTr1Couple));
	for(kCpl=0; kCpl<NbCouple; kCpl++)
		Tr1CoupleCopy(&((*RankedCouples)[kCpl]), &(Couples[kCpl]));

	// Ordering
	for (j = 1; j < NbCouple; j++)
	{
		Tr1CoupleCopy( &Work, &((*RankedCouples)[j]));
		i = j - 1;
		while (i >= 0 && (*RankedCouples)[i].Strength < Work.Strength)
		{
			Tr1CoupleCopy(&((*RankedCouples)[i+1]), &((*RankedCouples)[i]));
			i--;
		}
		Tr1CoupleCopy(&((*RankedCouples)[i+1]), &Work);
	}
}

//====================================================================
// Getting the indices of the ranked alarms after tracking
void  Tr1GetRankedAlarmIdx(CTr1TrackPlane *TrPlane, int *RankedIdx, int *NbRanked)
{
	int i, j;

	// Indices before ranking;
	for(i=0; i<TrPlane->NbTrack; i++) RankedIdx[i] = i;

	// Ordering the tracks
	for (j = 1; j < TrPlane->NbTrack; j++)
	{
		int Work = RankedIdx[j]; 
		i = j - 1;
		while (i >= 0 && TrPlane->Tracks[RankedIdx[i]].AvrMerit < TrPlane->Tracks[Work].AvrMerit)
		{
			RankedIdx[i+1] = RankedIdx[i]; 
			i--;
		}
		RankedIdx[i+1] = Work; 
	}

	// Ordering the alarms by suppressing virtual tracks
	for(i=0, j=0; i<TrPlane->NbTrack; i++)
	{
		if(TrPlane->Tracks[RankedIdx[i]].AlarmIndex >= 0)
		{
			RankedIdx[j] = RankedIdx[i];
			j++;
		}
	}
	*NbRanked = j;
}

//====================================================================
// Fill an alarm plane (this update the internal time counter)
void Tr1FillAlarmPlane(CTr1Tracking* This, int Time, const CTr1Input *Input)
{	
	//CTr1AlarmStack   *AlStack;
	CTr1AlarmPlane   *AlPlane;
	CTr1Alarm        *Alarm;
	CTr1Couple       *Couple, *RankedCouples;
	CTr1Ref          *MyRef;
	const CTr1Clue   *Clue;
	int             kAl, LocX1, LocX2, LocY1, LocY2;
	int             Distance = 4;

	if(Time != This->Time + 1)
	{
#if TR1_PRINT_FOR_DEBUG
		{
			printf("Error in Tr1ReadAlarmPlane: time parameter should start at 0 and increment 1 by 1\n");
			printf("Error in Tr1ReadAlarmPlane: Prev time = %d, New time = %d\n", This->Time, Time);
			TR1_MAJOR_ERR_HANDLER();
		}
#endif
	}
	else This->Time = Time;

	//AlStack = &(This->AlarmStack);
	AlPlane = Tr1GetAlarmPlane(This, This->Time);

	// Filling Ref structure from client's clues
	MyRef            = &(AlPlane->Ref);
	Clue             = &(Input->Clue);
	MyRef->Available = (Clue->Nature!=TR1_CLUE_NOTAVAILABLE);
	MyRef->Nature    = Clue->Nature;
	if(MyRef->Available)
	{
		MyRef->X1 = Clue->X1;
		MyRef->Y1 = Clue->Y1;
		MyRef->X2 = Clue->X2;
		MyRef->Y2 = Clue->Y2;

		MyRef->Cx = (MyRef->X1 + MyRef->X2)*(float)0.5;
		MyRef->Cy = (MyRef->Y1 + MyRef->Y2)*(float)0.5;

		MyRef->Angle   = Tr1PositiveAngle(MyRef->Y1-MyRef->Y2, MyRef->X1-MyRef->X2);
		MyRef->Breadth = TR1_EUCLI_DIST(MyRef->X1, MyRef->Y1, MyRef->X2, MyRef->Y2);
	}
	else
	{
		MyRef->Cx = MyRef->Cy = (float)-1.0;
		MyRef->X1 = MyRef->X2 = MyRef->Y1 = MyRef->Y2 = -1;
		MyRef->Angle = MyRef->Breadth = (float)-1.0;
	}

	// Estimating Ref-trajectory back to time 0
	if(MyRef->Available) Tr1AnalyseRefTrack(This, Time, This->Param.Iw, This->Param.Ih);

	AlPlane->NbAlarm = TR1_MIN(Input->NbCouple, This->Param.MaxNbAlarm);

	// Too many alarms case
	if(Input->NbCouple>This->Param.MaxNbAlarm && This->Param.WarningOnFlag)
	{
#if TR1_PRINT_FOR_DEBUG
		printf("Warning from Tr1ReadAlarmPlane: too many alarms to be read (%d)\n", Input->NbCouple);
printf("Only the first best %d will be taken into account\n", This->Param.MaxNbAlarm);
#endif
	}

	// Too few alarms case
	if(Input->NbCouple==0 && This->Param.WarningOnFlag)
	{
#if TR1_PRINT_FOR_DEBUG
		printf("Warning from Tr1ReadAlarmPlane: no alarm to be read at time %d\n", This->Time);
		#endif

	}

	// Ordering input candidate couples (in case not correctly ordered by the client)
	Tr1RankCouples(Input->Couples, Input->NbCouple, &RankedCouples);

	// Filling alarms from candidate couples 	
	// Managing the strong-clue case : when a "strong clue" is entered, it becomes the most salient alarm
	// The other alarms are also taken into account, but with a reduced strength

	Couple = 0; // Just to make compiler happy

	for(kAl=0; kAl<AlPlane->NbAlarm; kAl++)
	{
		Alarm  = &(AlPlane->Alarms[kAl]);

		// In case of "strong clue", the first alarm will contain the clue, with full strength (1)
		if(kAl == 0 && Clue->Nature == TR1_CLUE_STRONG)
		{
			LocX1 = Clue->X1;
			LocY1 = Clue->Y1;
			LocX2 = Clue->X2;
			LocY2 = Clue->Y2;             				
			Alarm->Strength    = (float)1;
		}
		else
		{
			if(Clue->Nature == TR1_CLUE_STRONG)
			{
				Couple = &(RankedCouples[kAl-1]);                // In case of "strong clue", couple[0] becomes alarm[1], etc...
				Alarm->Strength   = Couple->Strength*(float)0.1; // and the alarm strength of input couples is penalized
			}
			else
			{
				Couple = &(RankedCouples[kAl]);
				Alarm->Strength   = Couple->Strength;
			}
			LocX1 = Couple->X1;
			LocY1 = Couple->Y1;
			LocX2 = Couple->X2;
			LocY2 = Couple->Y2;             			
		}
		// Other fields deriving from previous ones
		// Beware : positive angle calculated in DIRECT referential (Y, X)
		Alarm->X1      = LocX1;
		Alarm->Y1      = LocY1;
		Alarm->X2      = LocX2;
		Alarm->Y2      = LocY2;             
		Alarm->Rank    = kAl; // since couples have been ordered
		Alarm->Cx      = (LocX1 + LocX2)*(float)0.5;
		Alarm->Cy      = (LocY1 + LocY2)*(float)0.5;
		Alarm->Angle   = Tr1PositiveAngle(LocY1 - LocY2, LocX1 - LocX2);
		Alarm->Breadth = TR1_EUCLI_DIST(LocX1, LocY1, LocX2, LocY2);

	}

	// Determining whether or not alarms are next to reference (when available)
	for(kAl=0; kAl<AlPlane->NbAlarm; kAl++)
	{
		Alarm              = &(AlPlane->Alarms[kAl]);
		Alarm->MatchingRef = Tr1IsAlarmNextToRef(Alarm, MyRef, Distance);
	}

	free(RankedCouples);
}

//====================================================================
// Normalisation of the alarm merit for alarm plane t 
void Tr1NormalizeAlarmStrength(CTr1Tracking* This, int Time)
{
	CTr1AlarmPlane *AlPlane;
	CTr1Alarm      *Alarm;
	int           kAl;
	float         MaxStrength;


	AlPlane  = Tr1GetAlarmPlane(This, Time);

	if(AlPlane->NbAlarm==0) return;

	MaxStrength = AlPlane->Alarms[0].Strength;

	// Maximum merit on that alarm plane
	for(kAl=1; kAl<AlPlane->NbAlarm; kAl++)
	{
		Alarm = &(AlPlane->Alarms[kAl]);
		MaxStrength = TR1_MAX(Alarm->Strength, MaxStrength);
	}

	// added by SV 2006-03-15
	if (MaxStrength==0)
		return;

	// Normalisation
	for(kAl=0; kAl<AlPlane->NbAlarm; kAl++)
	{
		Alarm = &(AlPlane->Alarms[kAl]);
		Alarm->Strength = Alarm->Strength/MaxStrength;
	}
}

/***************************************************************************/
//         END OF FILE                                                     */
/***************************************************************************/
