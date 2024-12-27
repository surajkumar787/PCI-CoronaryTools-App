// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <IpSimplicity.h>
#include <stdio.h>
#include <string>

namespace Pci{ namespace StentBoostAlgo{

class FxdSink
{
public:
	FxdSink(const std::wstring &filename);
	~FxdSink();

	void add(Simplicity::Image<short> &image);

	bool valid;

private:

	bool writeHeader();

	int count;
	int width;
	int height;

	FILE *file;
};

}}