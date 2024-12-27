// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "StbMarkers.h"
#include <IpSimplicity.h>


namespace Pci {	namespace Core
{
	class StbMask
	{
	public:
		StbMask();
		~StbMask();

		void mask(const StbMarkers& markers, const Simplicity::OmpImage<short>& input, Simplicity::OmpImage<short>& output);

	private:
		bool isMaskValues(const StbMarkers &markers) const;
		void createMask(const StbMarkers& markers, const Simplicity::OmpImage<short> &input, const Simplicity::NodeAffineTransform::Vector& inner, const Simplicity::NodeAffineTransform::Vector& outer);
		void applyMask(const Simplicity::OmpImage<short>& input, Simplicity::OmpImage<short>& output);
        bool sizeEquals(const Simplicity::OmpImage<short>&  input);

		Simplicity::ThreadPool						m_pool;
		std::unique_ptr<Simplicity::OmpImage<float>>	m_mask;
		StbMarkers									m_markers;
	};
}}