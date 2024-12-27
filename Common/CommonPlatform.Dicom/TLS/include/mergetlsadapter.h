// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <string>
#include <mergecom.h>

#include "applicationcontext.h"

namespace CommonPlatform { namespace Dicom { namespace TLS
{
	/*!
	\brief  This class implements the functions that will be passed as callbacks to MergeCom.
	Mergecom will call the functions of this class to perform secure communications.
	*/
	class MergeTlsAdapter
	{
	public:
		/*!
		\brief  This function is called by MergeCom when it wants to negotiate the secure connection. 
		MergeCom makes a connection with the remote application on a socket and passes that socket to this function.
		See MergeCom reference manual.
		*/
		static SS_STATUS SS_Session_Start(MC_SOCKET SocketToUse, CONN_TYPE ConnectionType, void* ApplicationContext, void** SecurityContext);

		/*!
		\brief  The responsibility of this function is to decrypt input data given by MergeCom and save it in the buffer provided. 
		It is called only if SESSION_START was successful.
		See MergeCom reference manual.
		*/
		static SS_STATUS SS_Read(void*         SScontext,
								 void*         ApplicationContext,
								 char*         Buffer,
								 unsigned int  BytesToRead,
								 unsigned int* BytesRead,
								 int           Timeout);

		/*!
		\brief  The responsibility of this function is to encrypt input data given by MergeCom and transmit it over the network.
		It is called only if SESSION_START was successful. 
		See MergeCom reference manual.
		*/
		static SS_STATUS SS_Write(void*         SScontext,
								  void*         ApplicationContext,
								  char*         Buffer,
								  unsigned int  BytesToWrite,
								  unsigned int* BytesWritten,
								  int           Timeout);


		/*!
		\brief   This function is called by Mergecom when it wants to terminate the secure connection. 
		See MergeCom reference manual.
		*/
		static void SS_Session_Shutdown(void* SScontext, void* ApplicationContext);
	};

	/*!
	\brief This function should be called by SCUs to open a secure association with an SCP. It opens a secure connection with MC_Open_Secure_Association.
	*/
	MC_STATUS openSecureAssociation(const std::unique_ptr<ApplicationContext>& applicationContext,
									int                applicationID,
									int*               associationID,
									const std::string& remoteApplicationTitle,
									int                remoteHostPortNumber,
									const std::string& remoteHostTCPIPName,
									const std::string& serviceList);

	/*! 
	\brief This function should be called by an SCP to negotiate a secure connection after a connection is accepted with MC_Wait_For_Connection.
	*/
	MC_STATUS processSecureAssociationRequest(const MC_SOCKET& socketToUse,
											  const std::string& ServiceList,
											  int& applicationID,
											  int& associationID,
											  const std::unique_ptr<ApplicationContext>& applicationContext);

}}}