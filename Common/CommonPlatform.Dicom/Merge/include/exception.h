// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <mcstatus.h>
#include <exception>
#include <string>

namespace CommonPlatform { namespace Dicom { namespace Merge
{

	void mcCheck(MC_STATUS status);
	void mcCheck(MC_STATUS status, std::string info);

	class Exception : public std::exception
	{
	public:
		explicit Exception(MC_STATUS status);
		Exception(MC_STATUS status, std::string info);
		const char* what() const override;
		MC_STATUS status;

	private:
		std::string info;
	};

	class TimeoutException : public Exception
	{
	public:
		explicit TimeoutException(MC_STATUS status);
		TimeoutException(MC_STATUS status, std::string info);
	};

	class AssociationClosedException : public Exception
	{ 
	public:
		explicit AssociationClosedException(MC_STATUS status);
		AssociationClosedException(MC_STATUS status, std::string info);
	};

	class AssociationAbortedException : public Exception
	{
	public:
		explicit AssociationAbortedException(MC_STATUS status);
		AssociationAbortedException(MC_STATUS status, std::string info);
	};

}}}