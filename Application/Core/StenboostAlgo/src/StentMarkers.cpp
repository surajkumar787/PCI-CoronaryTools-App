// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StentMarkers.h"

namespace Pci {	namespace StentBoostAlgo {

using namespace Simplicity;

StentMarkers::StentMarkers()
:
	valid		(false),
	confidence	(0.0),
	stitchDetected(false),
	p1			(NodeAffineTransform::Vector()),
	p2			(NodeAffineTransform::Vector())
{
}

StentMarkers::StentMarkers(bool	validMarkers, bool skipped, bool stitchDetected, float confidence, float alpha, const NodeAffineTransform::Vector &p1, const NodeAffineTransform::Vector &p2)
:
	valid				(validMarkers),
	confidence			(confidence),
	stitchDetected		(stitchDetected),
	p1					(p1),
	p2					(p2)
{
}

NodeAffineTransform::Matrix StentMarkers::getTransform(const StentMarkers &to, float ortogonalScaleFactor) const
{
	auto fromDelta = p2 - p1;
	auto toDelta = to.p2 - to.p1;

	if (fromDelta.length() < 0.001f) return NodeAffineTransform::Matrix::scale(NodeAffineTransform::Vector(1, 1));
	if (toDelta.length() < 0.001f) return NodeAffineTransform::Matrix::scale(NodeAffineTransform::Vector(1, 1));

	auto fromDeltaS = fromDelta.normal()    * (fromDelta.length() * (1.0f - ortogonalScaleFactor) + toDelta.length()* ortogonalScaleFactor);

	auto f = NodeAffineTransform::Matrix::translate(p1) *   NodeAffineTransform::Matrix(fromDelta.x, -fromDeltaS.y, 0,
		fromDelta.y, fromDeltaS.x, 0);

	auto t = NodeAffineTransform::Matrix::translate(to.p1)   *   NodeAffineTransform::Matrix(toDelta.x, -toDelta.y, 0,
		toDelta.y, toDelta.x, 0);

	return t * f.inverse();
}

}}