// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "messagebase.h"

#include <mc3msg.h>

namespace CommonPlatform { namespace Dicom { namespace Merge
{
	class OutputFile;
	class Service;

	class Message : public MessageBase
	{
		friend class Association;
		friend class FileBase;

	public:
		Message(const Service & service, MC_COMMAND command);
		Message(const std::string& serviceName, MC_COMMAND command);

	private:
		explicit Message(int id);

	public:
		std::string service() const;
		int         command() const;
		OutputFile  toFile() const;
	};

}}}