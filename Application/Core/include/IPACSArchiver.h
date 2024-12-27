// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <memory>

#include "Snapshot.h"
#include "ApplicationType.h"

namespace Pci { namespace Core { namespace PACS
{
class IPACSArchiver
{
public:
	virtual ~IPACSArchiver() {};

	virtual void sendSnapshot(std::shared_ptr<Snapshot> frame, int seriesNumber,RunType runType) = 0;
	virtual void sendMovie(std::vector<std::shared_ptr<Snapshot>> frame, int framesPerSecond, int seriesNumber,RunType runType) = 0;

};
}}}