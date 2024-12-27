// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "Timing.h"
#include <Windows.h>

namespace CommonPlatform { namespace Timing
{
	double getTimeStamp()
	{
		LARGE_INTEGER count;
		BOOL succes = QueryPerformanceCounter(&count);

		LARGE_INTEGER freq;
		double timestamp = 0;
		if( succes && QueryPerformanceFrequency(&freq))
		{
			timestamp = static_cast<double>(count.QuadPart) / static_cast<double>(freq.QuadPart);
		}

		return timestamp;
	}
}}