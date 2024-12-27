// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <IpSimplicity.h>

namespace Pci {	namespace StentBoostAlgo {

template < typename T>
class StbFrangiImage : public Simplicity::Image<T>
{
public:
	StbFrangiImage<T>() : ImageTemplate<T>() { };
	StbFrangiImage<T>(int imageWidth, int imageHeight)
		: Image<T>()
	{
		resize(imageWidth, imageHeight);
	};

	virtual ~StbFrangiImage<T>() { destroy(); };

	void resize(int imageWidth, int imageHeight)
	{
		if ((imageWidth * imageHeight) > 0)
		{	
			destroy();
			this->width   = imageWidth;
			this->height  = imageHeight;
			this->stride  = imageWidth;
			this->pointer = new T[imageWidth * imageHeight];
		}
	};

	void copyFrom(const StbFrangiImage<T> &from) 
	{
		if (&from == this) return;
		resize(from.width, from.height);
		std::memcpy(this->pointer, from.pointer, from.height*from.stride * sizeof(T));
	};

	void destroy()
	{
		if (pointer != nullptr)
		{
			delete[] pointer;
		}
		pointer = nullptr;
	};

	StbFrangiImage<T>(const StbFrangiImage<T>&) = delete;
	StbFrangiImage& operator=(const StbFrangiImage<T>&) = delete;
};

}}