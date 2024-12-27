// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include <gtest/gtest.h>
#include "CrmAlgoFluoroStub.h"
#include "OscStub.h"

//TICS -OLC#009  magic numbers for a test are ok.
namespace Pci {	namespace Test {
using namespace Pci::Core;

class CrmAlgoFluoroTransformationsTest : public ::testing::Test
{
public:

	CrmAlgoFluoroTransformationsTest() :
		m_pool					(false,"CrmTest"),
		m_param					(new CrmParameters(L"crm.ini"))
	{
	}

	~CrmAlgoFluoroTransformationsTest()
	{
		m_xrayImage.reset();
		m_crmRoadmap.reset();
		m_param.reset();
		Simplicity::system().resetHeap();
	}

	void SetUp() override
	{
		m_algoFluoro	= std::make_unique<CrmAlgoFluoroStub>(m_pool, *m_param);

		m_crmRoadmap = std::make_shared<CrmRoadmap>();		
		m_crmRoadmap->data.push_back('t');
		m_crmRoadmap->width  = 11;
		m_crmRoadmap->height = 10;
		m_crmRoadmap->geometry.sourceImageDistance  = 2;
		m_crmRoadmap->geometry.sourceObjectDistance = 1.5;

		m_xrayImage	= std::make_shared<XrayImage>();

		m_xrayGeometry.angulation = 10.5;
		m_xrayGeometry.rotation   = 10.6;

		m_xrayImage->geometry	= m_xrayGeometry;

		reset();

		Test::SetUp();	
	}

	static Matrix4x4 to4d(const Matrix &mat)
	{
		Matrix4x4 result = Matrix4x4::identity();

		result.m[0][0] = mat.m[0][0];	result.m[0][1] = mat.m[0][1]; result.m[0][3] = mat.m[0][2];
		result.m[1][0] = mat.m[1][0];	result.m[1][1] = mat.m[1][1]; result.m[1][3] = mat.m[1][2];
		result.m[3][0] = mat.m[2][0];	result.m[3][1] = mat.m[2][1]; result.m[3][3] = mat.m[2][2];

		return result;
	}

	static Simplicity::NodeAffineTransform::Matrix to3d(const Matrix &mat)
	{
		return Simplicity::NodeAffineTransform::Matrix (	   float(mat.m[0][0]), float(mat.m[0][1]), float(mat.m[0][2]), 
			float(mat.m[1][0]), float(mat.m[1][1]), float(mat.m[1][2]));									
	}

	Matrix getExpectedTransformationMatrix( const std::shared_ptr<const XrayImage> & src, const std::shared_ptr<const CrmRoadmap> & roadmap ) 
	{
		//calculate 3D matrix to transform fluoro image to roadmap
		Matrix4x4 fluoro2angio =	to4d(roadmap->frustumToPixels)		* 
			roadmap	->geometry.frustum.fromRoot * 
			roadmap	->geometry.patient.toRoot	* 
			src		->geometry.patient.fromRoot * 
			src		->geometry.frustum.toRoot	*  
			to4d(src->frustumToPixels).invert();

		//we have a 4D homogenous matrix with perspective; we need a 3D matrix homogenous without perspective. So we estimate the scale at iso and apply 
		//implicitly as scaling in the resulting matrix.

		//calculate axis system of fluoro image in fluoro coordinate space
		Vector3d fluoroOrigin = to4d(src->frustumToPixels) * src->geometry.frustum.fromRoot * src->geometry.iso.toRoot * Vector3d();
		Vector3d fluoroX      = fluoroOrigin + Vector3d(1,0,0);
		Vector3d fluoroY      = fluoroOrigin + Vector3d(0,1,0);

		//calculate axis system of fluoro image in roadmap coordinate space
		fluoroOrigin  = fluoro2angio * fluoroOrigin;
		fluoroX       = fluoro2angio * fluoroX - fluoroOrigin;     
		fluoroY       = fluoro2angio * fluoroY - fluoroOrigin;

		//calculate rotation matrix from x and y normals, containing the perspective scaling
		Matrix rotation = Matrix::identity();
		rotation.m[0][0] = fluoroX.x;	rotation.m[0][1] = fluoroY.x;  
		rotation.m[1][0] = fluoroX.y;	rotation.m[1][1] = fluoroY.y;  

		//calculate full matrix to transform fluoro image to roadmap
		return Matrix::translate(Point(fluoroOrigin.x, fluoroOrigin.y)) * rotation * Matrix::translate(-Point(src->width/2.0, src->height/2.0));
	}

	void  setAlgoFluoro(std::shared_ptr<CrmRoadmap> roadmap) { m_algoFluoro->set(roadmap); };
	Sense::Matrix getTransformationMatrix(std::shared_ptr<XrayImage> image) { return m_algoFluoro->getTransformationMatrix(image); };
	std::shared_ptr<XrayImage>& getImage() { return m_xrayImage; };
	std::shared_ptr<CrmRoadmap>& getRoadmap() { return m_crmRoadmap; };

private:
	Simplicity::ThreadPool				m_pool;
	std::unique_ptr<CrmParameters>		m_param;
	std::unique_ptr<CrmAlgoFluoroStub>	m_algoFluoro;
	std::shared_ptr<CrmRoadmap>			m_crmRoadmap;
	std::shared_ptr<XrayImage>			m_xrayImage;
	XrayGeometry						m_xrayGeometry;
};

TEST_F(CrmAlgoFluoroTransformationsTest, GivenAValidRoadmapWasSet_WhenTransformationMatrixIsCreated_ThenTheTransformationMatrixIsAsExpected)
{
	setAlgoFluoro(getRoadmap());
	ASSERT_TRUE(getExpectedTransformationMatrix(getImage(), getRoadmap()) == getTransformationMatrix(getImage()));
}

TEST_F(CrmAlgoFluoroTransformationsTest, GivenAValidRoadmapWasSet_WhenTableIsPivoted_AndTransformationMatrixIsCreated_ThenTheTransformationMatrixIsAsExpected)
{
	setAlgoFluoro(getRoadmap());
	getImage()->geometry.tablePivotAngle += 3;
	getImage()->geometry.updateSpaces();

	ASSERT_TRUE(getExpectedTransformationMatrix(getImage(), getRoadmap()) == getTransformationMatrix(getImage()));
}

TEST_F(CrmAlgoFluoroTransformationsTest, GivenAValidRoadmapWasSet_WhenTableIsMoved_AndTransformationMatrixIsCreated_ThenTheTransformationMatrixIsAsExpected)
{
	setAlgoFluoro(getRoadmap());
	getImage()->geometry.tableLateralOffset += 3;
	getImage()->geometry.updateSpaces();

	ASSERT_TRUE(getExpectedTransformationMatrix(getImage(), getRoadmap()) == getTransformationMatrix(getImage()));
}
}}
//TICS +OLC#009  magic numbers for a test are ok.