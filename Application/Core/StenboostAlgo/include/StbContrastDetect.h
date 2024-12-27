// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "StbFrangiImage.h"
#include "StbFrangi.h"
#include "StentMarkers.h"
#include "StbContrastSelect.h"
#include "Sense.h"

namespace CommonPlatform { namespace Xray {
	struct XrayImage;
}}

namespace Pci{ namespace StentBoostAlgo{
using Vector2d = Simplicity::NodeAffineTransform::Vector;

struct StbContrastDesc
{
	std::shared_ptr<StbFrangiImage<float>>	image;				//the vesselness-filtered image. No real need to store it, but might be usefull for debugging
	bool					hasContrast;				//does the frame contains contrast or not

	float					intensityMeasured;	
	float					intensityBackground;
	bool					valid;
	StbSelectDesc			stbSelect;
	StentMarkers			markers;
};


class StbContrastDetect
{
public:
	StbContrastDetect();
	~StbContrastDetect();

	void init(int height, int width , int pixelBitDeph, const Sense::Point &mmPerPixel);

	//index indicates the index of the given frame in the sequence
	void preProcess(const CommonPlatform::Xray::XrayImage& image, StbContrastDesc &contrast, int index);
	void process(std::vector<StbContrastDesc> &contrast,int index);
	void storevalues(std::vector<StbContrastDesc> &contrast);

	static void imageDownScale( float maxGrayValue, const CommonPlatform::Xray::XrayImage &input, StbFrangiImage<float> &output);

	struct VesselNessData
	{
		VesselNessData() { background = current = 0.0f; count = 0; }
		float background;
		float current;	  
		int count;
	};
private:
	static const int StentLongitude = 12;
	static const int StentTrancinal = 8;

	Vector2d determentSearchDistance(const StentBoostAlgo::StentMarkers &markers);

	void analyseVesselNessAroundStent(Vector2d &p1, Vector2d &p2,const Vector2d &searchRanges, int index,  StbContrastDesc &contrast);
	void calcStentContrast(std::vector<StbContrastDesc> &contrast, int index);
	float calcThreshold(std::vector<StbContrastDesc> &contrast);
	std::tuple<float, float> determentSymmetricVesselNess(std::array<VesselNessData, StentTrancinal> &line);

	void storeArray(std::array<std::array<VesselNessData, StentTrancinal>, StentLongitude> data);

	StbFrangiImage<float>					m_image;			// A temporary image that holds the downsampled version of the current input image

	std::unique_ptr<StbFrangiImage<float>>	m_refereceImage;
	StbFrangi								m_frangi;			// the frangi filter
	float									m_maxGrayValue;
	Sense::Point							m_mmPerPixel;
	float									m_threshold;
};									       

}}