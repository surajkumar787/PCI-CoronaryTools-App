// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


//====================================================================
//Protection against multiple decalaration
#ifndef		_RF_TR1
#define		_RF_TR1

//====================================================================


#define TR1_PRINT_FOR_DEBUG 0


#define TR1_MAJOR_ERR_HANDLER()  printf("Major Error : press sth to exit\n"); \
	getchar();                                   \
	exit(1)

#define TR1_SQ(a)       ((a)*(a))
#define TR1_CUBE(a)     ((a)*(a)*(a))
#define TR1_SIGN(a)     (((a) >= 0)   ? (1): (-(1)))
#define TR1_ABS(a)      (((a) >= 0)   ? (a): (-(a)))
#define TR1_MIN(a,b)    (((a) <= (b)) ? (a):   (b) )
#define TR1_MAX(a,b)    (((a) >= (b)) ? (a):   (b) )
#define TR1_CLIP(a,b,c) TR1_MIN(TR1_MAX((a), (b)), (c))
#define TR1_RND(a)      (((a) >= 0.0) ? (int)((a)+0.5) : (int)((a)-0.5))
#define TR1_EUCLI_DIST(x1,y1,x2,y2) (float)sqrt((double)(TR1_SQ((x1)-(x2)) + TR1_SQ((y1)-(y2))))
#define TR1_UC          unsigned char

// Enum for printing correct fields of error matrices
enum{TR1_RAW, TR1_FUZ};
enum{TR1_CUR, TR1_AVR};

// Client-provided clue's nature values
enum {
	TR1_CLUE_NOTAVAILABLE = 0,   // No clue provided by the client application
	TR1_CLUE_DEBUG,          // Clues provided for debug purposes only
	TR1_CLUE_STRONG          // Strong clues -> change some parameters and processing
};

// Best track status : returned to client
enum {
	TR1_RES_OK = 0,  // Best track ok
	TR1_RES_SKIP,    // Best track's head is virtual (no matching alarm)
}; 

// Locked track information : returned to client
enum {
	TR1_RES_UNLOCKED = 0,    // not locked
	TR1_RES_LOCKED,          // locked
};

// Track evaluation status
enum{
	TR1_COMMON = 0,      
	TR1_PREDICTABLE,
	TR1_STRONG,
	TR1_MERITING
};


//====================================================================
// Structure definition

typedef void* CTr1Hdl;

// Tracking parameters 
typedef	struct
{
	int   WarningOnFlag;       // Verbose flag for warning-like comment printing
	int   MaxNbAlarm;          // Maximum number of alarms to be tracked per frame (can change over time)
	int   MaxNbAlarmEver;      // Maximum number of alarms as defined at tracker init time 
	int   StackDepth;          // Stack depth for memorized alarms and tracks (along time)
	int   InitTime;            // Tracker's init time (beyond which results are trustworthy)
	int   VirtualPenalize;     // Penalization (in %) for consecutive virtual tracks (cumulative)
	int   LockCount;           // For locking: Nb of consecutive times when best alarm is tracked by same track
	int   LockMaxConSkip;      // For locking: maximum number of consecutive skips within the locking period
	int   VirtualWhileInit;    // If 1 virtual tracks might be created durint the tracker init phase
	int   Iw, Ih;              // Image size
	float TrAlpha;             // Integration factor
	float LowAngleDist;        // Value below which angle dist are considered low
	float SkipAngleDist;       // Value above which angle dist might lead to skipping
	float HighAngleDist;       // Value above which angle dist are considered high 
	float LowBreadthDist;      // Value below which breadth dist are considered low
	float SkipBreadthDist;     // Value above which breadth dist might lead to skipping
	float HighBreadthDist;     // Value above which breadth dist are considered high 
	float LowCentroidDist;     // Value below which centroid dist are considered low
	float SkipCentroidDist;    // Value above which centroid dist might lead to skipping
	float HighCentroidDist;    // Value above which centroid dist are considered high 
	float CoefAngle;           // Angle weighting coef for composite
	float CoefBreadth;         // Breadth weighting coef for composite
	float CoefCentroid;        // Centroid weighting coef for composite
	float StrengthImpact;      // Fraction of couple strength (VS predictability) for track evaluation
}
CTr1Param;

// Alarm couple
typedef struct
{
	int    X1, Y1;       // First marker's coordinates
	int    X2, Y2;       // Second marker's coordinates
	float  Strength;     // Global couple strength value
}
CTr1Couple;

// Tracking result structure
typedef	struct
{
	int       X1, Y1;    // 1st marker's coordinates
	int       X2, Y2;    // 2nd marker's coordinates
	int       CTr1OutputStatus;    // Status describing the tracking results
	int       Locked;    // Indicates whether tracking is locked on a reliable track
	CTr1Couple *Couples; // Alarm couples after tracking (ranked)
	int       NbCouple;  // Number of ranked alarm couples after tracking
	float        CurStrength;    // Strength of the track-head's alarm
	float        AvrStrength;    // Average strength of the alarms on which that track is built
	float        CurMerit;       // Overall current merit of the track's head (cur fuzzy composite strength + pred)
	float        AvrMerit;       // Average overall merit of the total track (avr fuzzy composite strenght + pred)
	int          Tr1Status;         // Track status : internally used to characterise that track
}
CTr1Output;

// Input clues regarding real markers (not mandatory)
typedef struct
{
	int   X1, Y1;     // First marker's coordinates provided by the client
	int   X2, Y2;     // Second marker's coordinates provided by the client
	int   Nature;     // Nature of the clue (no clue, strong clue, for debug...)
	//short *Nghb1;     // Pixel Neighbourhood of marker 1 (1D pointer) : client's allocation
	//short *Nghb2;     // Pixel Neighbourhood of marker 2 (1D pointer) : client's allocation

}
CTr1Clue;

// All the input information to tracker at each iteration
typedef struct
{
	int       NbCouple;  // Number of candidate couples
	CTr1Couple *Couples; // Array of candidate couples
	CTr1Clue   Clue;     // User's input clues

}
CTr1Input;

// Prediction description
typedef	struct
{
	int     X1, Y1;         // First marker in couple : coordinates (not prediction - previous alarm)
	int     X2, Y2;         // Second marker in couple : coordinates
	float   Cx, Cy;         // Prediction of alarm's centroid (coordinates)
	float   Angle;          // Prediction of alarm's angle 
	float   Breadth;        // Prediction of alarm's breadth (distance between points)
	//short   *Nghb1;        // Pixel Neighbourhood of marker 1 (1D pointer) 
	//short   *Nghb2;        // Pixel Neighbourhood of marker 2 (1D pointer) 
}
CTr1Pred;

// Alarm-to-track prediction error field description
typedef	struct
{
	float   Angle;          // Prediction error on angle 
	float   Breadth;        // Prediction error on breadth
	float   Centroid;       // Prediction error on centroid
	float   Composite;      // Composite prediction error
}
CTr1PredErrFields;


// Alarm-to-track prediction error description : raw and fuzzy components
typedef	struct
{
	CTr1PredErrFields   Raw;    // Raw prediction error 
	CTr1PredErrFields   Fuz;    // Fuzzy prediction error (after non-linearity)
}
CTr1PredErr;

// Alarm description
typedef	struct
{
	int     X1, Y1;         // First marker in couple : coordinates
	int     X2, Y2;         // Second marker in couple : coordinates
	float   Cx, Cy;         // Couple centroid's coordinates
	float   Angle;          // Couple angle in degrees (in (y,x) direct referential)
	float   Breadth;        // Breadth (distance between markers)
	float   Strength;       // Global merit value (marker couple evaluation enabling rank determination)
	int     Rank;           // Rank among all the alarms of the current alarm plane
	int     MatchingRef;    // Matching marker-reference variable (1=match, 0=no-match, -2=no ref file, -1=no ref image)
	//short   *Nghb1;         // Pixel Neighbourhood of marker 1 (1D pointer)
	//short   *Nghb2;         // Pixel Neighbourhood of marker 2 (1D pointer)
}
CTr1Alarm;

// Track description
typedef	struct
{
	int          AlarmIndex;     // Index of the last alarm on which that track is built (<0 -> virtual)
	CTr1Alarm    Alarm;          // Content of the last alarm on which that track is built
	int          PrevTrIndex;    // Index of the previous track from which that track stems
	CTr1Pred     Pred;           // Track prediction
	CTr1PredErr *CurPredErrors;  // CurPredErrors[kAl] : current pred error for each alarm-to-that-track-pair 
	CTr1PredErr *AvrPredErrors;  // AvrPredErrors[kAl] : average pred error for each alarm-to-that-track-pair 
	CTr1PredErr  CurPredErr;     // Current prediction error for best match
	CTr1PredErr  AvrPredErr;     // Average prediction error for best matches (integrated along track)
	float        CurStrength;    // Strength of the track-head's alarm
	float        AvrStrength;    // Average strength of the alarms on which that track is built
	float        CurMerit;       // Overall current merit of the track's head (cur fuzzy composite strength + pred)
	float        AvrMerit;       // Average overall merit of the total track (avr fuzzy composite strenght + pred)
	int          CTr1TrackStatus;         // Track status : internally used to characterise that track
}
CTr1Track;

// Reference info description
typedef struct{
	int   Available; // Flag specifying the availability of the target reference
	int   Nature;    // Nature of the target reference information
	int   X1, Y1;    // First marker in ref couple : coordinates
	int   X2, Y2;    // Second marker in ref couple : coordinates
	float Cx, Cy;    // Ref couple centroid
	float Angle;     // Angle of reference couple
	float Breadth;   // Breadth of reference couple
}
CTr1Ref;

// Track list per image : a certain number of tracks
typedef	struct
{
	int        NbTrack;  // Number of tracks in that plane (image)
	int        SkipFlag; // Skip flag: 1 -> to be skipped
	int        Locked;   // Indicating whether the tracker is locked or not
	CTr1Track *Tracks;   // Array of tracks
}
CTr1TrackPlane;

// Alarm list per image : a certain number of alarms + possible reference information
typedef	struct
{
	int       NbAlarm;      // Number of alarms in that plane (image)
	CTr1Ref    Ref;         // Reference target (available or not)
	CTr1Alarm  *Alarms;     // Array of alarms
}
CTr1AlarmPlane;

// Track stack : a certain depth of track planes
typedef	struct 
{
	int           Depth;      // stack's depth
	int           MaxNbTrack; // Maximum number of tracks per track plane (image)
	CTr1TrackPlane *Planes;   // Array of track planes
}
CTr1TrackStack;

// Alarm stack : a certain depth of alarm planes
typedef	struct
{
	int           Depth;          // stack's depth
	int           MaxNbAlarmEver; // Maximum number of alarms per alarm plane (fixed once for all at TrCreate)
	CTr1AlarmPlane *Planes;       // Array of alarm planes
}
CTr1AlarmStack;

// Tracking process object
typedef	struct
{
	int             Time;        // Current time index
	float           TrAlpha;     // Current alpha value
	//int             CorrelNghbN; // Correlation neighbourhood size (Number of pixels)
	CTr1PredErr     DummyErr;    // Dummy pred error (all -1)
	CTr1Param       Param;       // Tracking parameters
	CTr1AlarmStack  AlarmStack;  // Alarm stack
	CTr1TrackStack  TrackStack;  // Track stack
	int            *RankedIdx;   // Current ranked alarm indices after tracking (for output)
	int             NbRanked;    // Number of ranked alarms after tracking (for output)
}
CTr1Tracking;

enum{Tr1Angle=0, Tr1Breadth, Tr1Centroid, Tr1Composite};
typedef struct
{
	float CurRaw[4];
	float CurFuz[4];
	float AvrRaw[4];
	float AvrFuz[4];
	float High[3];
	float Skip[3];
}
CTr1AdvancedOutput;

//====================================================================
// Function declaration

// Main functions
void  Tr1CalcAlpha(int Time, float *Alpha, float AlphaMax);
void  Tr1PredCopy(CTr1Pred* OutPred, CTr1Pred* InPred, int NghbN);
float Tr1Merit(float Strength, float PredErr, float StrengthImpact);

CTr1Tracking*   Tr1Create(CTr1Param *Param);
CTr1AlarmPlane* Tr1GetAlarmPlane(CTr1Tracking* Tr, int Time);
CTr1TrackPlane* Tr1GetTrackPlane(CTr1Tracking* This, int Time);
void            Tr1GetParamPt(CTr1Hdl TrackingHdl, CTr1Param **ParamPt);

void Tr1Delete(CTr1Tracking* Tr);
void Tr1Iterate(CTr1Hdl TrackingHdl, int Time, const CTr1Input *Input, CTr1Output *Output, CTr1AdvancedOutput *advOutput);
void Tr1ManageNoAlarm(CTr1Tracking* This, CTr1TrackPlane *TrPlaneTm1, CTr1TrackPlane *TrPlaneT00,  CTr1Output *Output);
void Tr1InitTrack(CTr1Tracking* This);
void Tr1Match(CTr1Tracking* This, CTr1AlarmPlane *AlPlane, CTr1TrackPlane *TrPlaneTm1, CTr1TrackPlane *TrPlaneT00);
void Tr1AddVirtual(CTr1Tracking* This, CTr1TrackPlane *TrPlaneTm1, CTr1TrackPlane *TrPlaneT00, CTr1AlarmPlane *AlPlane, int InitTime, int VirtualWhileInit, CTr1AdvancedOutput *advOutput);
void Tr1Predict(CTr1Tracking* This, CTr1TrackPlane *TrPlaneTm1, CTr1TrackPlane *TrPlaneT00);
void Tr1PenalizeContender(CTr1Tracking* This, CTr1TrackPlane *TrPlane);
void Tr1Evaluate(CTr1Tracking* This, CTr1TrackPlane *TrPlane, CTr1Input *Input, CTr1Output *Output);

// Prediction error related functions TrPredErr.c
float Tr1AngleErr(float Theta1, float Theta2);
float Tr1BreadthErr(float Breadth1, float Breadth2);
float Tr1CentroidErr(float X1, float Y1, float X2, float Y2, int Iw, int Ih);
float Tr1NonLinearity(float X, float Low, float High, float SlopeFac, float FHigh);
void  Tr1CalcPredErr(CTr1Tracking* This, CTr1AlarmPlane *AlPlane, CTr1TrackPlane *TrPlaneTm1);

// Input/Output functions in TrInOut.c
float Tr1PositiveAngle(int Y, int X);
void  Tr1CoupleCopy(CTr1Couple* OutCouple, CTr1Couple* InCouple);
void  Tr1AlarmCopy(CTr1Alarm* OutAlarm, CTr1Alarm* InAlarm);
int   Tr1IsAlarmNextToRef(CTr1Alarm *Al, CTr1Ref *Ref, int Distance);
void  Tr1RankCouples(CTr1Couple *Couples, int NbCouple, CTr1Couple **RankedCouples);
void  Tr1GetRankedAlarmIdx(CTr1TrackPlane *TrPlane, int *RankedIdx, int *NbRanked);
void  Tr1FillAlarmPlane(CTr1Tracking* Tr, int Time, const CTr1Input *Input);
void  Tr1NormalizeAlarmStrength(CTr1Tracking* This, int Time);

// Debug functions in TrDebug.c
void Tr1DisplayTrack(CTr1Tracking* This, int TrHeadTime, int TrHeadIdx, int Time);
void Tr1Lock(CTr1Tracking* This, int TrHeadTime, int TrHeadIdx, int Time, int TrLockMaxConSkip, int* Locked);
int  Tr1GetBestTrackIdx(CTr1Tracking* This, int Time);
void Tr1AnalyseRefTrack(CTr1Tracking* This, int Time, int Iw, int Ih);
void Tr1PrintAlarmPlane(CTr1Tracking* Tr, int Time);
void Tr1PrintTracks(CTr1TrackPlane* TrPlaneTm1, int Time);
void Tr1PrintTrackPlane(CTr1Tracking* This, int Time);
void Tr1PrintPredErr(CTr1Tracking* This, int NbAlarm, CTr1TrackPlane *TrPlaneTm1, int CurAvr, int RawFuz);


void VirtualPenalizing(CTr1Track *Virtual, float Coef);

//====================================================================
// End of protection against multiple declaration
#endif   // End of include protection agains multiple declaration



