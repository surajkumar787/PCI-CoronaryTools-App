// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <Sense/Graphics/TextureInput.h>
#include <Sense/Graphics/IRenderer.h>
#include <IpSimplicity/Simplicity/Interface.h>

namespace Sense
{
	
class TextureSimplicity : public TextureInput
{
public:
	explicit TextureSimplicity(IGpu &gpu);
	virtual ~TextureSimplicity();

	void invalidate();

	void set(const Simplicity::Image<short>			&image);
	void set(const Simplicity::Image<unsigned char> &image);
	void set(const Simplicity::Image<unsigned>		&image);
	
	void render(const Space &space, IRenderer &renderer, const Simplicity::Image<short>			&image, const Color &color, const Rect &rect, const Matrix &dst, const Matrix &src = Matrix::identity(), bool cull = true);
	void render(const Space &space, IRenderer &renderer, const Simplicity::Image<unsigned char>	&image, const Color &color, const Rect &rect, const Matrix &dst, const Matrix &src = Matrix::identity(), bool cull = true);
	void render(const Space &space, IRenderer &renderer, const Simplicity::Image<unsigned>		&image, const Color &color, const Rect &rect, const Matrix &dst, const Matrix &src = Matrix::identity(), bool cull = true);

	void renderImpl(const Matrix & dst, const Rect &rect, const Color & color, const Matrix & src, IRenderer &renderer, const Space & space, bool cull);

private:

	bool valid;

	TextureSimplicity(const TextureSimplicity&);
	TextureSimplicity& operator=(const TextureSimplicity&);
};

}
