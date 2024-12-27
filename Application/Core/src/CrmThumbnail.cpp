// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#include "CrmThumbnail.h"
#include "Resources.h"

namespace Pci{ namespace Core{

CrmThumbnail::CrmThumbnail(Control &parent)
:
	Control		(parent),
	background	(Color(), [this] { invalidate(); }),
	active		(false,   [this] { invalidate(); }),
	thumb		(false,   [this] { invalidate(); }),
	roi			(Rect(),  [this] { invalidate(); }),
	geometry	(),
	roadmap		(nullptr),
	vessel		(gpu),
	back		(gpu),
	border		(gpu)
{
}

CrmThumbnail::~CrmThumbnail(void)
{
}


void CrmThumbnail::setGeometry(const XrayGeometry &geo)
{
	geometry = geo;
	invalidate();
}

void CrmThumbnail::setRoadmap(const std::shared_ptr<const CrmRoadmap> &Roadmap)
{
	roadmap = Roadmap;
	vessel.invalidate();
	invalidate();
}

Point CrmThumbnail::transform(const Point &pos, const Matrix &frustumToClient) const
{
	const XrayGeometry &src = roadmap->geometry;
	const XrayGeometry &dst = geometry;

	Point    pos2d;
	Vector3d pos3d;

	Vector3d iso = src.frustum.fromRoot * src.iso.toRoot * Vector3d(0,0,0);

	pos3d = src.frustum.toRoot	 * Vector3d(pos.x, pos.y, iso.z);	//get position in the root
	pos3d = src.patient.fromRoot * pos3d;							//get position on the table
	pos3d = dst.patient.toRoot	 * pos3d;							//get position in the root
	pos3d = dst.frustum.fromRoot * pos3d;							//get position in current frustum space
	pos2d = frustumToClient		 * Point(pos3d.x, pos3d.y);			//get position in client space

	return pos2d;
}


static const double TumbnailSize = 1000.0;
static const double TumbnailRadius = 1024.0;
static const double TumbnailBorderOpasaty = 0.85;

void CrmThumbnail::render(IRenderer &renderer) const
{
	Control::render(renderer);

	Space space = getSpace();

	Point dstSize = Point(geometry.detectorFieldSizeX, geometry.detectorFieldSizeY);
	if (geometry.detectorImageRotated) std::swap(dstSize.x, dstSize.y);
	
	Rect dst(-dstSize * 0.5, dstSize * 0.5);
	dst *= Point(1.0, -1.0);
	Rect tgt = roi;
	
	Matrix frustumToClient = Matrix::identity();

	frustumToClient = Matrix::scale(Point(1.0, -1.0))												* frustumToClient;
	frustumToClient = Matrix::scale(std::min(tgt.width()/dst.width(), tgt.height()/-dst.height()))	* frustumToClient;
	frustumToClient = Matrix::translate(tgt.center())												* frustumToClient;
	
	Rect outside = space.bounds;
	Rect inside  = Rect(frustumToClient * dst.topLeft(), frustumToClient * dst.bottomRight())  - Margin(0.5);

	if(!thumb) //render background
	{
		if (!back.isValid()) back.set(PciSuite::Resources::ThumbPreview.data());
		Rect src(0, 0, back.getWidth(), back.getHeight());
		renderer.render(space, inside, Color(1.0), src, back);
	}

	if (roadmap && roadmap->overlay)
	{
		Point srcSize = Point(roadmap->geometry.detectorFieldSizeX, roadmap->geometry.detectorFieldSizeY);
		if (roadmap->geometry.detectorImageRotated) std::swap(srcSize.x, srcSize.y);
		Rect src = Rect(-srcSize * 0.5, srcSize * 0.5) * Point(1, -1);

		Rect  rect  = roadmap->overlay->shutters;
		Color color2 = active ? Color(1,0,0) : Sense::ExperienceIdentity::Palette::Gray140;

		Matrix a = roadmap->overlay->frustumToPixels.inverse();

		Vertex frame[] =
		{
			Vertex(transform(a * rect.bottomLeft(),   frustumToClient), color2, rect.bottomLeft()),  
			Vertex(transform(a * rect.topLeft(),      frustumToClient), color2, rect.topLeft()),
			Vertex(transform(a * rect.bottomRight(),  frustumToClient), color2, rect.bottomRight()),  
			Vertex(transform(a * rect.topRight(),     frustumToClient), color2, rect.topRight()),     
			Vertex(transform(a * rect.topRight(),     frustumToClient), color2, rect.topRight()),     
			Vertex(transform(a * rect.topRight(),     frustumToClient), color2, rect.topRight()),     
			Vertex(transform(a * rect.bottomRight(),  frustumToClient), color2, rect.bottomRight()),  
			Vertex(transform(a * rect.topLeft(),      frustumToClient), color2, rect.topLeft()),
			Vertex(transform(a * rect.bottomLeft(),   frustumToClient), color2, rect.bottomLeft()),  
		};

		//render vessels
		vessel.set(*roadmap->overlay);
		renderer.render(space, frame, 9, vessel, IRenderer::TextureFlags::Mask);
	}
		
	if (!thumb)
	{
		border.bounds		= inside + Margin(TumbnailSize);
		border.edge			= 3.0;
		border.border		= TumbnailSize;
		border.radius		= TumbnailRadius;
		border.color		= Color();
		border.colorEdge	= Sense::ExperienceIdentity::Palette::Gray140;
		border.colorBorder	= Sense::ExperienceIdentity::Palette::Black * TumbnailBorderOpasaty;

		border.render(space, renderer);
	}
}

}}


