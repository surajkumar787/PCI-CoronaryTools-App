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
#include <string.h> 
#include <windows.h>

#include <ErrLib.h>
#include <ErrLibDev.h>
#include <ErrLibVersion.h>
#include <ErrMessages.h>

int ErrErrorIndexFromCode(CErr *This, int ErrorCode);
int ErrStartErrorReportFromString(CErr *This, char *String);
int ErrAppendErrorReportFromCode(CErr *This, int Code);
int ErrSetVersion(CErr *This);

/*************************************************************************************/
// Library instantiation 
/*************************************************************************************/
int ErrCreate(CErrHdl* PtErrHdl)
{
	CErr    *This;
	int     k, PrivateErrNb;
	int     Err = 0;
	
	This = (CErr*)malloc(sizeof(CErr)); // Library object creation
	PtErrHdl->Pv = (void*)This;         // Filling returned handle
	This->ErrHdl.Pv = (void*)This;      // Copy of the handle within library-object

	PrivateErrNb = ERR_LIB_LAST-ERR_LIB_FIRST-1;
	This->MaxNbCharInVersionInfo = (int)(strlen(ERR_VERSION_HEADER) + strlen(ERR_VERSION_INFO) + strlen(ERR_CONFIG)  + strlen(ERR_PLATFORM) + 16); 

	This->NbError = PrivateErrNb;

	This->ErrorArray = (CErrError*)malloc(This->NbError*sizeof(CErrError));

	for(k=0; k<PrivateErrNb; k++)
		This->ErrorArray[k] = __ErrErrorArray[k]; // Deep copy necessary

	Err = ErrSetVersion(This);

	strcpy_s(This->LastErrorReport, ERR_REPORT_MAXLEN, "No Error so far");

	return 0;
} 

/*************************************************************************************/
// Library deleting 
/*************************************************************************************/
int ErrDelete(CErrHdl ErrHdl)
{
	CErr *This = (CErr*)ErrHdl.Pv;
	free(This->VersionInfo);
	free(This->VersionInfoCopy);
	free(This->ErrorArray);
	free(This);
	return 0;
}

/*************************************************************************************/
// Set version material from version include
/*************************************************************************************/
int ErrSetVersion(CErr *This)
{
	int  NbChar;
	char *Format;

	NbChar = This->MaxNbCharInVersionInfo;
	This->VersionInfo     = (char*)malloc(NbChar*sizeof(char));
	This->VersionInfoCopy = (char*)malloc(NbChar*sizeof(char));
	Format                = (char*)malloc(NbChar*sizeof(char));

	sprintf_s(Format, NbChar, "%s%s", ERR_VERSION_HEADER, ERR_VERSION_INFO);
	sprintf_s(This->VersionInfo, NbChar, Format, ERR_CONFIG, ERR_PLATFORM, ERR_VERSION_MAJOR, ERR_VERSION_MINOR, ERR_VERSION_PATCH);

	free(Format);
	return 0;
}

/*************************************************************************************/
// Returns a pointer on version info 
/*************************************************************************************/
int ErrGetVersionInfo(CErrHdl ErrHdl, char ** VersionInfo)
{
	CErr* This = (CErr*)ErrHdl.Pv;
	strcpy_s(This->VersionInfoCopy, This->MaxNbCharInVersionInfo, This->VersionInfo);
	*VersionInfo = This->VersionInfoCopy;
	return 0;
}

/*************************************************************************************/
// Returns version numbers 
/*************************************************************************************/
int ErrGetVersionNumbers(int* Major, int* Minor, int* Patch)
{
	*Major = ERR_VERSION_MAJOR; *Minor = ERR_VERSION_MINOR; *Patch = ERR_VERSION_PATCH;
	return 0;
}

/*************************************************************************************/
// Add error material to error service 
/*************************************************************************************/
int ErrAdd(CErrHdl ErrHdl, CErrError ErrorArrayToAdd[], int ErrorNbToAdd)
{
	CErr      *This;
	CErrError *BufErrors, ErrorToAdd;
	int        k, j, NewErrorNb;
	char      Reason[128 + 2*ERR_MESSAGE_MAXLEN];
	char      FuncName[] = "ErrAdd";
	
	This = (CErr*)ErrHdl.Pv;

	NewErrorNb = This->NbError + ErrorNbToAdd;

	BufErrors = (CErrError*)malloc(This->NbError*sizeof(CErrError));

	for(k=0; k<This->NbError; k++)
		BufErrors[k] = This->ErrorArray[k];

	free(This->ErrorArray);
	This->ErrorArray = (CErrError*)malloc(NewErrorNb*sizeof(CErrError));

	for(k=0; k<This->NbError; k++)
		This->ErrorArray[k] = BufErrors[k];

	for(k=This->NbError; k<NewErrorNb; k++)
	{
		ErrorToAdd = ErrorArrayToAdd[k-This->NbError];
		for(j=0; j<This->NbError; j++)
		{
			if(ErrorToAdd.Code == This->ErrorArray[j].Code)
			{
				sprintf_s(Reason, 128+2*ERR_MESSAGE_MAXLEN, "Trying to add error-code %d (%s): code already exist as:\n %s", 
					ErrorToAdd.Code, ErrorToAdd.Message, This->ErrorArray[j].Message);
				
				ErrStartErrorReportFromLeaf(ErrHdl, FuncName, ERR_ERR_CODE_CLASH, Reason);
                free(BufErrors);
				return ERR_ERR_CODE_CLASH;
			}
		}
		This->ErrorArray[k] = ErrorToAdd;
	}

	This->NbError = NewErrorNb;

	free(BufErrors);
	return(0);
} 

/*************************************************************************************/
// Print all the API error messages 
/*************************************************************************************/
int ErrPrintErrorMessages(CErrHdl ErrHdl)
{
	CErr *This = (CErr*)ErrHdl.Pv;
	int  k;
	
	printf("\nERRLIB: PRINTING ERROR MESSAGES\n");
	
	for(k=0; k<This->NbError; k++)
		printf("Code %-9d: %s\n", This->ErrorArray[k].Code, This->ErrorArray[k].Message);

	printf("ERRLIB: END OF ERROR PRINTING\n\n");

	return(0);
} 

/*************************************************************************************/
// Get error index from error code 
/*************************************************************************************/
int ErrErrorIndexFromCode(CErr *This, int ErrorCode)
{
	int k, ErrorIndex=-1;
	
	for(k=0; k<This->NbError; k++)
	{
		if(This->ErrorArray[k].Code == ErrorCode)
		{
			ErrorIndex = k;
			break;
		}
	}

	return ErrorIndex; // -1 if not found
} 

/*************************************************************************************/
// Get error message from error code (into a callee-allocated string) 
/*************************************************************************************/
int ErrGetErrorMessage(CErrHdl ErrHdl, int ErrorCode, char **ErrorMessage)
{
	CErr *This = (CErr*)ErrHdl.Pv;
	int  ErrorIndex;
	
	ErrorIndex = ErrErrorIndexFromCode(This, ErrorCode);

	if(ErrorIndex<0)
	{
		ErrorIndex = ErrErrorIndexFromCode(This, ERR_WRN_WRONG_ERRCODE);
		strcpy_s(This->ErrorMessageCopy, ERR_MESSAGE_MAXLEN, This->ErrorArray[ErrorIndex].Message);
		*ErrorMessage = This->ErrorMessageCopy;
		sprintf_s(*ErrorMessage, ERR_MESSAGE_MAXLEN, "Error in ErrGetErrorMessage : Error code %d does not exist", ErrorCode);
		ErrStartErrorReportFromString(This, *ErrorMessage);
		return ERR_WRN_WRONG_ERRCODE;
	}
	else
	{
		strcpy_s(This->ErrorMessageCopy, ERR_MESSAGE_MAXLEN, This->ErrorArray[ErrorIndex].Message);
		*ErrorMessage = This->ErrorMessageCopy;
		return(0);
	}
} 

/*************************************************************************************/
// To know whether or not an error is fatal
/*************************************************************************************/
int ErrIsFatal(CErrHdl ErrHdl, int ErrorCode, int *PtIsFatal)
{
	CErr *This = (CErr*)ErrHdl.Pv;
	char ErrorMessage[ERR_MESSAGE_MAXLEN];
	int  ErrorIndex;

	*PtIsFatal    = 1;  // return fatal in case of wrong usage
	
	ErrorIndex = ErrErrorIndexFromCode(This, ErrorCode);

	if(ErrorIndex<0)
	{
		sprintf_s(ErrorMessage, ERR_MESSAGE_MAXLEN, "Error in ErrGetErrorMessage : Error code %d does not exist", ErrorCode);
		ErrStartErrorReportFromString(This, ErrorMessage);
		return ERR_WRN_WRONG_ERRCODE;
	}

	*PtIsFatal = This->ErrorArray[ErrorIndex].IsFatal;
	return 0;
}

/*************************************************************************************/
// Get last error report : full contextual error stack 
/*************************************************************************************/
int ErrGetLastErrorReport(CErrHdl ErrHdl, char **ErrorReport)
{
	CErr *This = (CErr*)ErrHdl.Pv;

	*ErrorReport = (char*)malloc(ERR_REPORT_MAXLEN*sizeof(char));
	
	strcpy_s(This->ErrorReportCopy, ERR_REPORT_MAXLEN, This->LastErrorReport);

	*ErrorReport = This->ErrorReportCopy;

	return 0;
} 

/*************************************************************************************/
// Start filling error report with string 
/*************************************************************************************/
int ErrStartErrorReportFromString(CErr *This, char *String)
{
	int LengthBeforeTail = (int)(ERR_REPORT_MAXLEN-strlen(ERR_REPORT_TAIL));
	
	if(strlen(String+1)>(unsigned int)LengthBeforeTail)
	{
		strcpy_s(This->LastErrorReport, ERR_REPORT_MAXLEN, ERR_REPORT_TAIL);
		return ERR_WRN_REPORT_OVERFLOW;
	}
	else
	{
		strcpy_s(This->LastErrorReport, ERR_REPORT_MAXLEN, "\n");
		strcat_s(This->LastErrorReport, ERR_REPORT_MAXLEN, String);
		strcat_s(This->LastErrorReport, ERR_REPORT_MAXLEN, "\n");
		return 0;
	}
} 

/*************************************************************************************/
// Start filling error report with string from leaf call 
/*************************************************************************************/
int ErrStartErrorReportFromLeaf(CErrHdl ErrHdl, char* LeafFunc, int ErrorCode, char *Reason)
{
	CErr *This = (CErr*)ErrHdl.Pv;
	char String[ERR_REPORT_MAXLEN];
	int  Error;

	sprintf_s(String, ERR_REPORT_MAXLEN, "Error in %s returning %d : %s", LeafFunc, ErrorCode, Reason);

	ErrStartErrorReportFromString(This, String);

	Error = ErrAppendErrorReportFromCode(This, ErrorCode);
	if(Error)
	{
		ErrAppendErrorReportFromCaller(ErrHdl, "ErrStartErrorReportFromLeaf", "ErrAppendErrorReportFromCode", 0, Error);
		return Error;
	}

	return Error;
} 

/*************************************************************************************/
// Appending error report with a string 
/*************************************************************************************/
int ErrAppendErrorReportFromString(CErr *This, char *String)
{
	char* Report           = This->LastErrorReport;
	int   LengthBeforeTail = (int)(ERR_REPORT_MAXLEN-strlen(ERR_REPORT_TAIL));

	if(strlen(Report)>(unsigned int)LengthBeforeTail)
	{
		return ERR_WRN_REPORT_OVERFLOW;
	}
	else if(strlen(String+1)+strlen(Report)>(unsigned int)LengthBeforeTail)
	{
		strcat_s(Report, ERR_REPORT_MAXLEN, ERR_REPORT_TAIL);
		return ERR_WRN_REPORT_OVERFLOW;
	}
	else
	{
		strcat_s(Report, ERR_REPORT_MAXLEN, String);
		strcat_s(Report, ERR_REPORT_MAXLEN, "\n");
		return 0;
	}
} 

/*************************************************************************************/
// Appending error report from error code 
/*************************************************************************************/
int ErrAppendErrorReportFromCode(CErr *This, int Code)
{
	int  ErrorIndex = ErrErrorIndexFromCode(This, Code);
	char String[ERR_MESSAGE_MAXLEN];

	if(ErrorIndex<0)
	{
		sprintf_s(String, ERR_MESSAGE_MAXLEN, "Error in ErrAppendErrorReportFromCode : Error code %d does not exist", Code);
		ErrAppendErrorReportFromString(This, String);
		return ERR_WRN_WRONG_ERRCODE;
	}
	else
	{
		sprintf_s(String, ERR_MESSAGE_MAXLEN, "Error Code %d=%s", Code, This->ErrorArray[ErrorIndex].Message);
		ErrAppendErrorReportFromString(This, String);
		return 0;
	}
} 

/*************************************************************************************/
// Appending error report from caller with callee's name and code 
/*************************************************************************************/
int ErrAppendErrorReportFromCaller(CErrHdl ErrHdl, char* Caller, char* Callee, char* ArgContext, int CalleeCode)
{
	CErr *This = (CErr*)ErrHdl.Pv;
	char String[ERR_MESSAGE_MAXLEN];
	int  ErrorCode;

	if(ArgContext != NULL)
		sprintf_s(String, ERR_MESSAGE_MAXLEN, "Error in %s calling %s with [%s] which returned %d", Caller, Callee, ArgContext, CalleeCode);
	else
		sprintf_s(String, ERR_MESSAGE_MAXLEN, "Error in %s calling %s which returned %d", Caller, Callee, CalleeCode);

	ErrorCode = ErrAppendErrorReportFromString(This, String);

	return ErrorCode;
}

/*************************************************************************************/
// Print error report, whether or not it has been updated
/*************************************************************************************/
int ErrPrintErrorReport(CErrHdl ErrHdl)
{
	CErr *This = (CErr*)ErrHdl.Pv;
	char FuncName[] = "ErrPrintErrorReport";

	printf("%s:\n", FuncName);
	printf("%s\n", This->LastErrorReport);
	return 0;
}

/*************************************************************************************/
// Print error report, whether or not it has been updated + getchar()
/*************************************************************************************/
int ErrPrintErrorReportAndFreeze(CErrHdl ErrHdl)
{
	CErr *This = (CErr*)ErrHdl.Pv;
	char FuncName[] = "ErrPrintErrorReportAndFreeze";

	printf("%s:\n", FuncName);
	printf("%s\n", This->LastErrorReport);
  printf("Type something to carry on...\n");
	getchar();
	return 0;
}










