// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StbAlgoTransform.h"
#include "XrayImage.h"
#include "StbImage.h"
#include "StbParameters.h"

using namespace Simplicity;

namespace Pci {	namespace Core	{

StbAlgoTransform::StbAlgoTransform(float maxZoomFactor) :
	_refMarkers(),
	_width(0),
	_height(0),
	_lostMarkerCount(0),
	_state(State::Idle),
	_maxZoomFactor(maxZoomFactor)
{}

StbAlgoTransform::~StbAlgoTransform()
{
}

bool StbAlgoTransform::process(const StbParameters &params, const std::shared_ptr<const CommonPlatform::Xray::XrayImage> &image,const std::shared_ptr<StbMarkers> &markers, Transform &trans, bool skip)
{
	if (!image || !markers || skip)	return false;

	if (_state == State::Idle || _width != image->width || _height != image->height)
	{
		_width = image->width;
		_height = image->height;
		_state = State::Idle;
	}

	switch (_state)
	{
	default:
	case State::Idle:
	{
		_state = State::NoLock;
		_lostMarkerCount = 0;
		// Indeed, there is no 'break' statement here
	}
	case State::NoLock:
		if (markers->valid)
		{
			setRefMarkers(*markers);
			processLock(params, *markers, trans);
			_state = State::Locked;
		}
		else
		{
			processNoLock(params, trans);
		}
		break;
	case State::Locked:
		if (markers->valid)
		{
			processLock(params, *markers, trans);
		}
		else
		{
			processLostLock(params, trans);
			_state = State::Unlocked;
		}
		break;
	case State::Unlocked:
		if (markers->valid)
		{
			processLock(params, *markers, trans);
			_state = State::Locked;
		}
		else
		{
			processLostLock(params, trans);
			if (_lostMarkerCount > params.markerParams->maxLostMarkerCount) _state = State::LostLock;
		}
		break;
	case State::LostLock:
		if (markers->valid)
		{
			setRefMarkers(*markers);
			_lostMarkerCount = 0;
			processLock(params, *markers, trans);
			_state = State::Locked;
		}
		else
		{
			_refMarkers.valid = false;
			processLostLock(params, trans);
		}
		break;
	}

	trans.refMarkers = _refMarkers;
	return true;
}

void StbAlgoTransform::setRefMarkers(const StentMarkers& markers)
{
	auto centrePoint = 0.5f * (markers.p1 + markers.p2);
	float tx = (_width / 2.0f) - centrePoint.x;
	float ty = (_height / 2.0f) - centrePoint.y;
	auto t = NodeAffineTransform::Matrix::translate(NodeAffineTransform::Vector(tx, ty));
	_refMarkers.p1 = t * markers.p1;
	_refMarkers.p2 = t * markers.p2;
	_refMarkers.valid = true;
}

bool StbAlgoTransform::processLock(const StbParameters& params, const StbMarkers &markers, Transform &trans)
{
	auto pprev = (_refMarkers.p1 - _refMarkers.p2);
	auto pnew = (markers.p1 - markers.p2);
	auto localMarkers( markers);

	// Check whether the markers have swapped position, if so swap to prevent 180 rotation.
	if (((pprev.x * pnew.x) + (pprev.y * pnew.y)) < 0.0f)
	{
		std::swap(localMarkers.p1, localMarkers.p2);
	}

	auto transfrom = localMarkers.getTransform(_refMarkers, params.ortogonalScaleFactor);

	// calculate zoom factor, either dynamic or fixed
	float factor = params.boostZoomFactor;
	if (factor < 0)
	{
		auto p = _refMarkers.p1 - _refMarkers.p2;
		float scalex = std::numeric_limits<float>::max();
		float scaley = std::numeric_limits<float>::max();
		if (abs(p.x) > 0.0f) scalex = static_cast<float>(_width) / abs(p.x);
		if (abs(p.y) > 0.0f) scaley = static_cast<float>(_height) / abs(p.y);
		factor = -factor * std::min(scalex, scaley);
		factor = std::max(std::min(factor, _maxZoomFactor), 1.0f);
	}

	auto  centrePoint = 0.5f*(_refMarkers.p1 + _refMarkers.p2);

	trans.trans = NodeAffineTransform::Matrix::translate(NodeAffineTransform::Vector(_width / 2.0f, _height / 2.0f)) *
		NodeAffineTransform::Matrix::scale(NodeAffineTransform::Vector(factor, factor)) *
		NodeAffineTransform::Matrix::translate(-centrePoint) *
		transfrom;

	trans.hasMarkers = true;
	trans.showWarning = false;
	trans.refMarkers = _refMarkers;
	_lostMarkerCount = 0;
	return true;
}

bool StbAlgoTransform::processNoLock(const StbParameters &params, Transform &trans)
{
	_lostMarkerCount++;
	trans.showWarning = (_lostMarkerCount > params.markerParams->lostMarkerWarningThreshold);

	trans.refMarkers = _refMarkers;
	trans.hasMarkers = false;
	return true;
}

bool StbAlgoTransform::processLostLock(const StbParameters& params, Transform &trans)
{
	return processNoLock( params, trans);
}

bool  StbAlgoTransform::end()
{
	_state = State::Idle;
	_refMarkers.valid = false;
	return true;
}

}}