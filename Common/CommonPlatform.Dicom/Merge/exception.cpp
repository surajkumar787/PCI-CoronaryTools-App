// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "exception.h"

namespace CommonPlatform { namespace Dicom { namespace Merge
{

	void mcCheck(MC_STATUS status)
	{
		mcCheck(status, "");
	}

	void mcCheck(MC_STATUS status, std::string info)
	{
		switch(status)
		{
		case MC_NORMAL_COMPLETION:
			//Ok
			break;
		case MC_ASSOCIATION_ABORTED:
			throw AssociationAbortedException(status, info);
		case MC_ASSOCIATION_CLOSED:
			throw AssociationClosedException(status, info);
		case MC_TIMEOUT:
			throw TimeoutException(status, info);
		default:
			throw Exception(status, info);
		}
	}

	Exception::Exception(MC_STATUS status, std::string info) :
		status(status),
		info(info + "; " + MC_Error_Message(status) + " [" + std::to_string(status)+ "]")
	{
	}

	const char* Exception::what() const
	{
		return info.c_str();
	}

	Exception::Exception									(MC_STATUS status) : Exception(status, "") {};
	TimeoutException::TimeoutException                      (MC_STATUS status) : Exception(status) {}
	TimeoutException::TimeoutException						(MC_STATUS status, std::string info) : Exception(status, info) {}
	AssociationClosedException::AssociationClosedException  (MC_STATUS status) : Exception(status) {}
	AssociationClosedException::AssociationClosedException  (MC_STATUS status, std::string info) : Exception(status, info) {}
	AssociationAbortedException::AssociationAbortedException(MC_STATUS status) : Exception(status) {}
	AssociationAbortedException::AssociationAbortedException(MC_STATUS status, std::string info) : Exception(status, info) {}

}}}