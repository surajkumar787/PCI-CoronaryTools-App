// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <functional> 

namespace Pci { namespace Core
{

template<typename T>
class PropertyNotifier
{
public:
	PropertyNotifier() :
		_data()
	{}

	PropertyNotifier &operator=(const PropertyNotifier &) = delete;
	PropertyNotifier(const PropertyNotifier&) = delete;

	PropertyNotifier &operator=(const T& newvalue)
	{
		// todo enable this check when there is a one to one relation between model end view.
		//	if(_data == newvalue)return *this;
		_data = newvalue;

		if (notifier)
		{
			notifier(newvalue);
		}
		return *this;
	}

	operator const T&() const { return _data; }

	bool operator==(const T& other) const
	{
		return other == _data;
	}

	void setNotifier(std::function<void(const T&)>  newNotifier)
	{
		notifier = newNotifier;

		if (notifier)
		{
			notifier(_data);
		}
	}

private:
	std::function<void(const T&)> notifier;

protected:
	T _data;
};
}}
