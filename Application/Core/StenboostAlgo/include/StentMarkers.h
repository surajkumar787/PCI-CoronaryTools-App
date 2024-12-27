// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <IpSimplicity.h>

namespace Pci {	namespace StentBoostAlgo {

struct StentMarkers
{
	StentMarkers();
	StentMarkers(bool validMarkers, bool skipped,bool stitchDetected, float confidence, float alpha, const Simplicity::NodeAffineTransform::Vector &p1, const Simplicity::NodeAffineTransform::Vector	 &p2);
	Simplicity::NodeAffineTransform::Matrix getTransform(const StentMarkers &to, float ortogonalScaleFactor) const;
	
	bool							valid;
	float							confidence;
	bool							stitchDetected;
	Simplicity::NodeAffineTransform::Vector		p1;
	Simplicity::NodeAffineTransform::Vector		p2;
};

}}