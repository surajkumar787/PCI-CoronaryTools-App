// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

namespace Pci { namespace Core
{
	struct PciReviewParameters
	{
		PciReviewParameters(int loops, double loopDelay, size_t loopMinFrames, size_t loopMaxFrames) :
			NrOfLoops(loops),
			LoopDelay(loopDelay),
			LoopMinNrFrames(loopMinFrames),
			LoopMaxNrFrames(loopMaxFrames)
		{}

		int NrOfLoops;
		double LoopDelay;
		size_t LoopMinNrFrames;
		size_t LoopMaxNrFrames;
	};
}}