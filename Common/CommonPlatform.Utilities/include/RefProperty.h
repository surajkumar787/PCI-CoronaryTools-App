// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <type_traits>

namespace CommonPlatform { namespace Utilities
{
	template<typename T>
	class RefProperty
	{
		static_assert(!std::is_reference<T>::value, "RefProperty Cannot be instantiated with T == reference");
	public:
		explicit RefProperty(T& value) :
			m_value(value)
		{}

		operator const T&() const
		{
			return m_value;
		}

		T* operator-> () const
		{
			return &m_value;
		}

		void operator = (const T& other)
		{
			m_value = other;
		}

		RefProperty(const RefProperty& other) = delete;
		RefProperty& operator = (const RefProperty& other) = delete;

	private:
		RefProperty();

		T& m_value;
	};
}}