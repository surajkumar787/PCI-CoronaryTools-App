// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#include "TextureSimplicity.h"

using namespace Sense;

TextureSimplicity::TextureSimplicity(IGpu &gpu)
:
	TextureInput(gpu),
	valid(false)
{
}

TextureSimplicity::~TextureSimplicity()
{
}


void TextureSimplicity::invalidate()
{
	valid = false;
}

void TextureSimplicity::set(const Simplicity::Image<unsigned char> &image)
{
	if (valid)
	{
		return;
	}
	
	TextureInput::set(ITexture::Lock(ITexture::Format::Gray8, reinterpret_cast<char*>(image.pointer), image.width, image.height, image.stride*sizeof(unsigned char)));
	valid = true;
}

void TextureSimplicity::set(const Simplicity::Image<short> &image)
{
	if (valid)
	{
		return;
	}

	TextureInput::set(ITexture::Lock(ITexture::Format::Gray16, reinterpret_cast<char*>(image.pointer), image.width, image.height, image.stride*sizeof(short)));
	valid = true;
}

void TextureSimplicity::set(const Simplicity::Image<unsigned> &image)
{
	if (valid)
	{
		return;
	}

	TextureInput::set(ITexture::Lock(ITexture::Format::Rgba32, reinterpret_cast<char*>(image.pointer), image.width, image.height, image.stride*sizeof(unsigned)));
	valid = true;
}

void TextureSimplicity::render(const Space &space, IRenderer &renderer, const Simplicity::Image<unsigned char> &image, const Color &color, const Rect &rect, const Matrix &dst, const Matrix &src, bool cull)
{
	set(image);
	renderImpl(dst, rect, color, src, renderer, space, cull);
}

void TextureSimplicity::render(const Space &space, IRenderer &renderer, const Simplicity::Image<short> &image, const Color &color, const Rect &rect, const Matrix &dst, const Matrix &src, bool cull)
{
	set(image);
	renderImpl(dst, rect, color, src, renderer, space, cull);
}

void TextureSimplicity::render(const Space &space, IRenderer &renderer, const Simplicity::Image<unsigned> &image, const Color &color, const Rect &rect, const Matrix &dst, const Matrix &src, bool cull)
{
	set(image);
	renderImpl(dst, rect, color, src, renderer, space, cull);
}

void Sense::TextureSimplicity::renderImpl(const Matrix & dst, const Rect &rect, const Color & color, const Matrix & src, IRenderer &renderer, const Space & space, bool cull)
{
	Vertex vertex[] =
	{
		Vertex(dst * rect.bottomLeft(),  color, src * rect.bottomLeft()),
		Vertex(dst * rect.topLeft(),     color, src * rect.topLeft()),
		Vertex(dst * rect.bottomRight(), color, src * rect.bottomRight()),
		Vertex(dst * rect.topRight(),    color, src * rect.topRight()),
		Vertex(dst * rect.topRight(),    color, src * rect.topRight()),
		Vertex(dst * rect.topRight(),    color, src * rect.topRight()),
		Vertex(dst * rect.bottomRight(), color, src * rect.bottomRight()),
		Vertex(dst * rect.topLeft(),     color, src * rect.topLeft()),
		Vertex(dst * rect.bottomLeft(),  color, src * rect.bottomLeft()),
	};

	renderer.render(space, vertex, cull ? 4 : 9, *this, IRenderer::TextureFlags::Xray);
}
