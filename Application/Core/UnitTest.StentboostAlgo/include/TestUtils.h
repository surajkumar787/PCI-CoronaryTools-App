// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "StbFrangi.h"
#include "XraySinkFxd.h"

namespace Pci {	namespace Test {
using namespace StentBoostAlgo;

void loadExpectedImage(StbFrangiImage<float> &expected,  std::string fileName)
{
	FILE *file;
	fopen_s(&file, fileName.c_str(), "rb");
	fpos_t pos;
	pos = 0;
	fsetpos(file, &pos);
	fread_s(expected.pointer, expected.width * expected.height * sizeof(float), sizeof(float), expected.height*expected.width, file);

	fclose(file);
}

void saveNewReferenceResult(std::shared_ptr<StbFrangiImage<float>>& output, std::string fileName)
{
	FILE *file;
	fopen_s(&file, fileName.c_str(), "wb+");
	fwrite(output->pointer, sizeof(float), output->height*output->width, file);

	fclose(file);
}

}}