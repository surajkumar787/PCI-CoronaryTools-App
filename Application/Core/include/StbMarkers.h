// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "StentMarkers.h"

namespace Pci{ namespace Core{

struct StbMarkers : public Pci::StentBoostAlgo::StentMarkers
{
	StbMarkers();
	StbMarkers(const Pci::StentBoostAlgo::StentMarkers &markers);

	bool load(FILE *f);
	bool save(FILE *f) const;

};


}}