// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


// Developer-level first-layer structures.
#ifndef		_ERRLIBDEV_H
#define		_ERRLIBDEV_H

#define ERR_REPORT_MAXLEN 4096
#define ERR_REPORT_TAIL   "\nOverFlow of the Error-report: report truncated...\n"

typedef struct
{
	CErrHdl   ErrHdl;                               // Copy of the library handle
	char      LastErrorReport[ERR_REPORT_MAXLEN];   // Last error report
	char      ErrorReportCopy[ERR_REPORT_MAXLEN];   // When returned, as a pointer, to client
	char      ErrorMessageCopy[ERR_MESSAGE_MAXLEN]; // When returned, as a pointer, to client
	int       NbError;                              // # of all possible errors
	int       MaxNbCharInVersionInfo;               // Maximum  number of characters to code version-info
	CErrError *ErrorArray;                          // Array of all possible errors
	char      *VersionInfo;                         // Version info string
	char      *VersionInfoCopy;                     // When returned, as a pointer, to client
}
CErr;

#endif   //_ERRLIBDEV_H