// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <IpSimplicity.h>
#include <Sense.h>

namespace Pci {	namespace Core {

using namespace Sense;

struct CrmOverlay : public Simplicity::OmpImage<unsigned char>
{
	CrmOverlay();

	bool load(FILE *f);
	bool save(FILE *f) const;

	Matrix frustumToPixels;
	Rect   shutters;
};

}}