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

#include <Tr1.h> 

//====================================================================
// Print alarm plane at time t
void Tr1PrintAlarmPlane(CTr1Tracking* This, int Time)
{
	CTr1AlarmPlane *AlPlane;
	CTr1Ref        *Ref;
	int           k;

	AlPlane = Tr1GetAlarmPlane(This, Time);
	Ref     = &(AlPlane->Ref);

	printf("\n\n");
	printf("TrPrintAlarmPlane START ****************   ");
	printf("Time = %d : %d alarms\n", Time, AlPlane->NbAlarm);
	if(Ref->Available) 
	{				
		printf("Reference : (%5.1f, %5.1f) (%3d, %3d), (%3d, %3d), Angle=%3d, Breadth=%4.1f\n", 
						Ref->Cx, Ref->Cy, Ref->X1, Ref->Y1, Ref->X2, Ref->Y2, (int)Ref->Angle, Ref->Breadth);
	}
	else               
		printf("Refefence : not available\n");

	printf(" Alarm  :(X1, Y1) :(X2, Y2) : Angle  : Breadth:Strength:   Rank :MatchRef:\n");
	printf("        :         :         :        :        :        :        :        :\n");	
	for(k=0; k<AlPlane->NbAlarm; k++)
	{
		CTr1Alarm  *Al = &(AlPlane->Alarms[k]);
		printf("    %3d : %3d %3d : %3d %3d :    %3d :%7.2f :%7.2f :    %3d :      %1d :\n",
			k, Al->X1, Al->Y1, Al->X2, Al->Y2, (int)Al->Angle, Al->Breadth, Al->Strength, Al->Rank, Al->MatchingRef);
	}	
	printf("TrPrintAlarmPlane END ******************\n");
}

//====================================================================
// Print alarm plane at time t
void Tr1PrintTracks(CTr1TrackPlane* TrPlaneTm1, int Time)
{
	printf("\n\n");
	printf("Tr1PrintTracks START ****************   ");
	printf("Time = %d : %d tracks\n", Time, TrPlaneTm1->NbTrack);

	printf(" Track  :(X1, Y1) :(X2, Y2) : Angle  : Breadth:Strength:\n");
	printf("        :         :         :        :        :        :\n");	
	for(int k=0; k<TrPlaneTm1->NbTrack; k++)
	{
		CTr1Track *Tr = &(TrPlaneTm1->Tracks[k]);
		printf("    %3d : %3d %3d : %3d %3d :    %3d :%7.2f :%7.2f :\n",
			k, Tr->Pred.X1, Tr->Pred.Y1, Tr->Pred.X2, Tr->Pred.Y2, (int)Tr->Pred.Angle, Tr->Pred.Breadth, Tr->CurStrength);
	}	
	printf("Tr1PrintTracks END ******************\n");
}

//====================================================================
// Print track plane at time t
void Tr1PrintTrackPlane(CTr1Tracking* This, int Time)
{
	CTr1TrackPlane *TrPlane;
	int           kTr;
	char          StatusConv[4];

	StatusConv[TR1_COMMON]      = ' ';
	StatusConv[TR1_PREDICTABLE] = 'P';
	StatusConv[TR1_STRONG]      = 'S';
	StatusConv[TR1_MERITING]    = 'M';

	TrPlane = Tr1GetTrackPlane(This, Time);

	printf("TrPrintTrackPlane START ****************  ");
	printf("Time = %d : %d tracks\n", Time, TrPlane->NbTrack);

	printf(" Track  :AlarmInd:PrevTrIn: CurRaw : CurFuz : AvrRaw : AvrFuz :CurStren:AvrStren:CurMerit:AvrMerit: Status :MatchRef:\n");
	printf("        :        :        :        :        :        :        :        :        :        :        :        :        :\n");	
	for(kTr=0; kTr<TrPlane->NbTrack; kTr++)
	{
		CTr1Track *Tr = &(TrPlane->Tracks[kTr]);
		CTr1Alarm *Al = &(Tr->Alarm);

		printf("    %3d :    %3d :    %3d :%7.2f :%7.2f :%7.2f :%7.2f :  %5.1f : %6.2f :  %5.1f : %6.2f :      %c :     %2d : \n",
						kTr, 
						Tr->AlarmIndex, 
						Tr->PrevTrIndex, 
						Tr->CurPredErr.Raw.Composite,
						Tr->CurPredErr.Fuz.Composite,
						Tr->AvrPredErr.Raw.Composite,
						Tr->AvrPredErr.Fuz.Composite,
						Tr->CurStrength,
						Tr->AvrStrength,
						Tr->CurMerit,
						Tr->AvrMerit,
						StatusConv[Tr->CTr1TrackStatus],
						Al->MatchingRef 
					);
	}	
	printf("TrPrintTrackPlane END ******************\n"); 
}

//====================================================================
// Return most meriting track index at time t
int Tr1GetBestTrackIdx(CTr1Tracking* This, int Time)
{
	CTr1TrackPlane *TrPlane;
	int             kTr, kTrBest=-1;

	TrPlane = Tr1GetTrackPlane(This, Time);

	for(kTr=0; kTr<TrPlane->NbTrack; kTr++)
	{
		CTr1Track  *Tr = &(TrPlane->Tracks[kTr]);

		if(Tr->CTr1TrackStatus == TR1_MERITING) 
		{
			kTrBest = kTr;
			break;
		}
	}	
	return kTrBest;
}

//====================================================================
// Display alarm at t of the track defined by (Track-head's time and idx)
void Tr1DisplayTrack(CTr1Tracking* This, int TrHeadTime, int TrHeadIdx, int Time)
{
	int           CurTime, CurTrIdx;

	if(Time>TrHeadTime) return;       // Beyond track's head
	if(TrHeadTime>This->Time) return; // Track not completely built

	for(CurTime=TrHeadTime, CurTrIdx=TrHeadIdx; CurTime>=Time; CurTime--)
	{
		CTr1TrackPlane *TrPlane = Tr1GetTrackPlane(This, CurTime);
		CTr1Track      *Track = &(TrPlane->Tracks[CurTrIdx]);
		CTr1Alarm      *Alarm = &(Track->Alarm);
		CurTrIdx = Track->PrevTrIndex;
	}
}

//====================================================================
// Check if all the tracks preceding TrHeadIdx between TrHeadTime and Time are TR1_MERITING
void Tr1Lock(CTr1Tracking* This, int TrHeadTime, int TrHeadIdx, int Time, int TrLockMaxConSkip, int* pLocked)
{
	int            CurTime, CurTrIdx;
	CTr1TrackPlane *TrPlane;
	CTr1Track      *Track;
	//CTr1Alarm      *Alarm;
	CTr1TrackPlane *TrPlaneT00, *TrPlaneTm1;
	int Flag1, Flag2, Flag3;
	int MaxConSkip;
	char StatusConv[4];
//	CTr1Track* Virtual;


	StatusConv[TR1_COMMON]      = '-';
	StatusConv[TR1_PREDICTABLE] = 'P';
	StatusConv[TR1_STRONG]      = 'S';
	StatusConv[TR1_MERITING]    = 'M';

	if(Time>TrHeadTime) 
	{
		return;       // Beyond track's head
	}
	if(TrHeadTime> This->Time)
	{
		return; // Track not completely built
	}
	//Condition 1:all previous tracks MERITING
	Flag1 = 1;
	for(CurTime=TrHeadTime, CurTrIdx=TrHeadIdx; CurTime>=Time; CurTime--)
	{
		TrPlane  = Tr1GetTrackPlane(This, CurTime);
		Track    = &(TrPlane->Tracks[CurTrIdx]);

		Flag1 &= (Track->CTr1TrackStatus==TR1_MERITING);
		if(Track->PrevTrIndex < 0)
		{
			Flag1 = 0;
			break;
		}
		CurTrIdx = Track->PrevTrIndex;
	}

	//Condition 2: current not skipped or previous locked

	TrPlaneT00  = Tr1GetTrackPlane(This, TrHeadTime);
	TrPlaneTm1  = Tr1GetTrackPlane(This, TrHeadTime-1);
	Flag2 = ((TrPlaneT00->SkipFlag == TR1_RES_OK)||(TrPlaneTm1->Locked == TR1_RES_LOCKED));

	//Condition 3: not more consecutive skips than TrLockMaxConSkip in all previous planes
	MaxConSkip = 0;
	for(CurTime=TrHeadTime; CurTime>=Time; CurTime--)
	{
		int nConSkip = 0;
		int t = CurTime;
		TrPlane = Tr1GetTrackPlane(This, t);

		while(TrPlane->SkipFlag == TR1_RES_SKIP && t >= Time)
		{
			nConSkip++;
			t--;
			if(t>=Time)
			{
				TrPlane  = Tr1GetTrackPlane(This, t);
			}
		}
		MaxConSkip = TR1_MAX(MaxConSkip, nConSkip);
	}

	Flag3 = (MaxConSkip <= TrLockMaxConSkip);

	if(Flag1 & Flag2 & Flag3)
		TrPlaneT00->Locked = TR1_RES_LOCKED;
	else
		TrPlaneT00->Locked = TR1_RES_UNLOCKED;

	*pLocked = TrPlaneT00->Locked;

}

//====================================================================
// Analyse ref-track back to time 0
void Tr1AnalyseRefTrack(CTr1Tracking* This, int Time, int Iw, int Ih)
{
	int            CurTime;
	CTr1Ref        *Ref, *PrevRef;
	float          CentroidDist, AngleDist, BreadthDist;
	float          MaxCDist, MoyCDist, MaxADist, MoyADist, MaxBDist, MoyBDist; 

	PrevRef = &(Tr1GetAlarmPlane(This, Time)->Ref);
	if(!PrevRef->Available) 
	{
#if TR1_PRINT_FOR_DEBUG
		printf("Cannot analyse ref track : ref[%d] not available\n", Time);
#endif
		return;
	}

	MaxCDist = MoyCDist = MaxADist = MoyADist = MaxBDist = MoyBDist = (float)0;

	for(CurTime=Time; CurTime>=0; CurTime--)
	{
		CTr1AlarmPlane *AlPlane = Tr1GetAlarmPlane(This, CurTime);
		Ref     = &(AlPlane->Ref);
		if(!Ref->Available) 
		{
#if TR1_PRINT_FOR_DEBUG
			printf("Cannot analyse ref track : ref[%d] not available\n", Time);
#endif
			return;
		}

		AngleDist    = Tr1AngleErr(PrevRef->Angle, Ref->Angle);
		BreadthDist  = Tr1BreadthErr(PrevRef->Breadth, Ref->Breadth);
		CentroidDist = Tr1CentroidErr(PrevRef->Cx, PrevRef->Cy, Ref->Cx, Ref->Cy, Iw, Ih);

		MoyADist += AngleDist;
		MoyBDist += BreadthDist;
		MoyCDist += CentroidDist;

		MaxADist = TR1_MAX(MaxADist, AngleDist);
		MaxBDist = TR1_MAX(MaxBDist, BreadthDist);
		MaxCDist = TR1_MAX(MaxCDist, CentroidDist);

#if TR1_PRINT_FOR_DEBUG
		printf("Ref(%3d) : (%3d %3d %3d %3d) (%5.1f, %5.1f) Angle=%5.1f Breadth=%5.1f Delta : C=%5.1f  A=%5.1f B=%5.1f \n", 
			      CurTime, Ref->X1, Ref->Y1, Ref->X2, Ref->Y2, Ref->Cx, Ref->Cy, Ref->Angle, Ref->Breadth, CentroidDist, AngleDist, BreadthDist);
#endif
		PrevRef = Ref;
	}

	if(Time>=1)
	{
		MoyADist /= Time;
		MoyBDist /= Time;
		MoyCDist /= Time;

#if TR1_PRINT_FOR_DEBUG
		printf("NbIma=%3d : Angle=(%5.1f %5.1f)  Breadth=(%5.1f %5.1f)  Centroid=(%5.1f %5.1f)\n",
					Time, MoyADist, MaxADist, MoyBDist, MaxBDist, MoyCDist, MaxCDist);
#endif
	}
}

//====================================================================
// / Printing Prediction error matrix (alarms at time t, tracks at time t-1)
void Tr1PrintPredErr(CTr1Tracking* This, int NbAlarm, CTr1TrackPlane *TrPlaneTm1, int CurAvr, int RawFuz)
{
	CTr1PredErr *PredErr;
	int        kAl, kTr;
	char       str1[8], str2[8];

	if(CurAvr == TR1_CUR) sprintf(str1, "TR1_CUR");
	else sprintf(str1, "TR1_AVR");
	if(RawFuz == TR1_RAW) sprintf(str2, "TR1_RAW");
	else sprintf(str2, "TR1_FUZ");
	
	// Printing Prediction error matrix (alarms at time t, tracks at time t-1)
	printf("\nTrPrintPredErr START **************** %s %s", str1, str2);
	printf("   Time = %d : Track[%d][*]\n", This->Time, This->Time-1);

	printf("Alarms    ");
	for(kAl=0; kAl<NbAlarm; kAl++) printf("%3d     ", kAl);
	printf("\n");

	PredErr = 0; // Just to make compiler happy

	// For each track at time t-1
	for(kTr=0; kTr<TrPlaneTm1->NbTrack; kTr++)
	{	
		CTr1Track   *TrTm1 = &(TrPlaneTm1->Tracks[kTr]);

		printf("\nT[%03d]  ", kTr);
		for(kAl=0; kAl<NbAlarm; kAl++)
		{
			if(CurAvr == TR1_AVR) PredErr = &(TrTm1->AvrPredErrors[kAl]);
			else                  PredErr = &(TrTm1->CurPredErrors[kAl]);
			if(RawFuz == TR1_RAW) printf("%8.2f", PredErr->Raw.Composite);
			else                  printf("%8.2f", PredErr->Fuz.Composite);			
		}
		printf("\nCentroid");
		for(kAl=0; kAl<NbAlarm; kAl++)
		{
			if(CurAvr == TR1_AVR) PredErr = &(TrTm1->AvrPredErrors[kAl]);
			else                 PredErr = &(TrTm1->CurPredErrors[kAl]);
			if(RawFuz == TR1_RAW) printf("%8.2f", PredErr->Raw.Centroid);
			else                 printf("%8.2f", PredErr->Fuz.Centroid);
		}
		printf("\nBreadth ");
		for(kAl=0; kAl<NbAlarm; kAl++)
		{
			if(CurAvr == TR1_AVR) PredErr = &(TrTm1->AvrPredErrors[kAl]);
			else                 PredErr = &(TrTm1->CurPredErrors[kAl]);
			if(RawFuz == TR1_RAW) printf("%8.2f", PredErr->Raw.Breadth);
			else                 printf("%8.2f", PredErr->Fuz.Breadth);			
		}
		printf("\nAngle   ");
		for(kAl=0; kAl<NbAlarm; kAl++)
		{
			if(CurAvr == TR1_AVR) PredErr = &(TrTm1->AvrPredErrors[kAl]);
			else                 PredErr = &(TrTm1->CurPredErrors[kAl]);
			if(RawFuz == TR1_RAW) printf("%8.2f", PredErr->Raw.Angle);
			else                 printf("%8.2f", PredErr->Fuz.Angle);			
		}

		printf("\n");

		printf("TrPrintPredErr END ****************\n\n");
	}
}


/***************************************************************************/
//         END OF FILE                                                     */
/***************************************************************************/
