// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <vector>

namespace Sense
{
	class Drawing;
	struct Matrix;
	struct Point;
}

namespace Pci { namespace Core {

	class StbUIHelpers
	{
	public:
		static void drawMarkersEllipse(Sense::Drawing& drawing, const Sense::Matrix& matrix, const Sense::Point& marker1, const Sense::Point& marker2);
		static void drawMarkersPlus(Sense::Drawing& drawing, const Sense::Matrix& matrix, const std::vector<Sense::Point>& markers);
	};

}}
