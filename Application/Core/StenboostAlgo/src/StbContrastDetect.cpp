// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StbContrastDetect.h"
#include "StbFrangiImage.h"
#include "sense.h"
#include "XrayImage.h"
#include <fstream>
#include <array>
#include <tuple>

namespace Pci {	namespace StentBoostAlgo {

static const int StentTrancinalMidPoint             = 4;
static const int NumberOfFramesForInit              = 10;
static const int contrastFrangiDownSample	        = 4;	//for efficiency, the input is downsampled before frangi is applied. this is the scale factor
static const float contrastFrangiStrength	        = 8.0f;	//the strength of the frangi filter
static const float contrastFrangiBlobbiness         = 2.5f;	//the blobbiness of the frangi filter
static const float contrastFrangiScale		        = 2.0f;	//the filter scale of the frangi filter
static const double MaxStentWidth			        = 10.0; // max diameter of a coronare vessel is 8 mm with some margio => 10 mm. 
static const float ThresholdFractionStdevMeasurment  = 4.0f;
static const float ThresholdFractionStdevBackGround  = 2.0f;
static const float ThresholdFractionAvargeMeasurment = 1.0f;

using Vector2d = Simplicity::NodeAffineTransform::Vector;
void saveFrangiImage(std::string filename, StbFrangiImage<float> &image);

float sqr(float a) { return a*a; }

StbContrastDetect::StbContrastDetect():
	m_image(),
	m_refereceImage(),
	m_frangi(),
	m_maxGrayValue(4096.0),
	m_threshold(0.0f)
{
}

StbContrastDetect::~StbContrastDetect()
{	
	m_image.destroy();
	m_frangi.destroy();
}

void StbContrastDetect::init(int height, int width, int pixelBitDeph, const Sense::Point &mmPerPixel)
{
	m_mmPerPixel = mmPerPixel;

	//calculate the dimensions of the downsized images
	int widthDownSampled  = static_cast<int>(width / contrastFrangiDownSample);
	int heightDownSampled = static_cast<int>(height / contrastFrangiDownSample);

	m_frangi.init(widthDownSampled, heightDownSampled, contrastFrangiScale);
	m_image.resize( widthDownSampled, heightDownSampled);
	m_maxGrayValue = static_cast<float>((1 << pixelBitDeph) - 1);

	m_refereceImage = nullptr;
}

void StbContrastDetect::preProcess( const CommonPlatform::Xray::XrayImage& input, StbContrastDesc &contrast, int index )
{
	//re-initialize the destination image that will be filled with the frangi image
	contrast.image->resize(m_image.width, m_image.height);

	std::memset(m_image.pointer, 0, m_image.height*m_image.stride * sizeof(float));
	std::memset(contrast.image->pointer, 0, contrast.image->height*contrast.image->stride * sizeof(float));

	// factor for conversion from short to float
	imageDownScale( m_maxGrayValue, input, m_image);

	//perform the frangi operation
	m_frangi.filter(m_image, contrast.image, contrastFrangiBlobbiness, contrastFrangiStrength);

	if (m_refereceImage == nullptr)
	{
		m_refereceImage = std::make_unique<StbFrangiImage<float>>();
		m_refereceImage->copyFrom( *contrast.image);
	}
}

void StbContrastDetect::process(std::vector<StbContrastDesc> &contrast, int index)
{
	calcStentContrast(contrast, index);

	if (index == NumberOfFramesForInit)
	{
		m_threshold = calcThreshold(contrast);
	}

	//and finally determine whether the frame contains contrast or not, based on the intensity
	contrast[index].hasContrast = false;
	if (index <= NumberOfFramesForInit || contrast[index].intensityMeasured < m_threshold)
	{
		if ( !contrast[index].markers.valid && index > NumberOfFramesForInit )
		{
			contrast[index].hasContrast = contrast[index - 1].hasContrast;
		}
	}
	if (index > NumberOfFramesForInit && contrast[index].intensityMeasured > m_threshold)
	{
		contrast[index].hasContrast = true;
	}
}

void StbContrastDetect::calcStentContrast( std::vector<StbContrastDesc> &contrast, int index)
{
	if (!contrast[index].markers.valid) return;
	
	auto searchRanges = determentSearchDistance(contrast[index].markers);

	float subSample = (1.0f / contrastFrangiDownSample);
	auto p1 = contrast[index].markers.p1 * subSample;
	auto p2 = contrast[index].markers.p2 * subSample;
	Vector2d subSampledSearchRange( ceil( searchRanges.x *subSample) , ceil(searchRanges.y *subSample));

	if( !contrast[0].valid )
	{
		for (int i = 0; i < index; i++)
		{
			analyseVesselNessAroundStent(p1, p2, subSampledSearchRange, i, contrast[i]);
		}
	}

	analyseVesselNessAroundStent(p1, p2, subSampledSearchRange, index, contrast[index]);
}

std::tuple<float, float> StbContrastDetect::determentSymmetricVesselNess(std::array<VesselNessData, StentTrancinal> &line)
{
	float sumCurrent = 0.0f;
	float sumBackground = 0.0f;

	for (int i = 0; i < StentTrancinalMidPoint; i++)
	{
		float currentLeft = line[StentTrancinalMidPoint - 1 - i].current / std::max(1, line[StentTrancinalMidPoint - 1 - i].count);
		float backLeft = line[StentTrancinalMidPoint - 1 - i].background / std::max(1, line[StentTrancinalMidPoint - 1 - i].count);

		float curentRight = line[i + StentTrancinalMidPoint].current / std::max(1, line[i + StentTrancinalMidPoint].count);
		float backRight = line[i + StentTrancinalMidPoint].background / std::max(1, line[i + StentTrancinalMidPoint].count);

		sumCurrent += std::fmin(curentRight, currentLeft);
		sumBackground += std::fmin(backRight, backLeft);
	}

	return std::tuple<float, float>(std::fmaxf(sumCurrent - sumBackground, 0.0f) , sumBackground);
}

void StbContrastDetect::analyseVesselNessAroundStent(Vector2d &p1, Vector2d &p2,const Vector2d &searchRanges, int index,  StbContrastDesc &contrast)
{
	float count = 0.0f;
	bool hasValues = false;
	auto stent = p2 - p1;
	auto direction = stent.normal();
	Vector2d dir2(-direction.y, direction.x);
	auto start = p1 - direction * searchRanges.y;
	auto scanLength = stent.length() + searchRanges.y * 2.0f;
	auto maxbounds = std::max( 2.0f * searchRanges.x, searchRanges.y);

	float lateralFraction =  2.0f * searchRanges.x / StentTrancinal;
	float longitudionalFraction = scanLength / StentLongitude;
	std::array<std::array<VesselNessData, StentTrancinal>, StentLongitude> checkerBoard;
	
	int x_max = static_cast<int>(std::min((std::max(p1.x, p2.x) + maxbounds), static_cast<float>(contrast.image->width)));
	int x_min = static_cast<int>(std::max((std::min(p1.x, p2.x) - maxbounds), 0.0f));
	int y_max = static_cast<int>(std::min((std::max(p1.y, p2.y) + maxbounds), static_cast<float>(contrast.image->height)));
	int y_min = static_cast<int>(std::max((std::min(p1.y, p2.y) - maxbounds), 0.0f));

	for (int y = y_min; y < y_max; y++)
	{
		int offSet = y * contrast.image->stride;
		for (int x = x_min; x < x_max; x++)
		{
			auto point = Vector2d(x - start.x, y - start.y);
			auto longitudional = point.x * direction.x + point.y * direction.y;
			auto lateral = point.x * dir2.x + point.y * dir2.y;

			int checkerBoardX = static_cast<int>( std::lroundf((longitudional / longitudionalFraction) - 0.5f));
			int checkerBoardY = static_cast<int>(StentTrancinalMidPoint - 1 + std::lroundf(lateral / lateralFraction));
			int checkerBoardNeighborY = static_cast<int>( std::lroundf(StentTrancinalMidPoint - 0.5f + (lateral / lateralFraction)));

			if ((checkerBoardX >= 0 && checkerBoardX < StentLongitude) &&
				(checkerBoardNeighborY >= 0 && checkerBoardY < StentTrancinal))
			{
				hasValues = true;
				if (checkerBoardY >= 0)
				{
					checkerBoard[checkerBoardX][checkerBoardY].current += contrast.image->pointer[offSet + x];
					checkerBoard[checkerBoardX][checkerBoardY].background += m_refereceImage->pointer[offSet + x];
					checkerBoard[checkerBoardX][checkerBoardY].count++;
				}
				if (checkerBoardNeighborY < StentTrancinal)
				{
					checkerBoard[checkerBoardX][checkerBoardNeighborY].current += contrast.image->pointer[offSet + x];
					checkerBoard[checkerBoardX][checkerBoardNeighborY].background += m_refereceImage->pointer[offSet + x];
					checkerBoard[checkerBoardX][checkerBoardNeighborY].count++;
				}
			}
		}
	}

	contrast.intensityBackground = 0.0f;
	contrast.intensityMeasured = 0.0f;
	contrast.valid = hasValues;

	for ( auto& line: checkerBoard)
	{
		auto lineData = determentSymmetricVesselNess(line);
		contrast.intensityMeasured += std::get<0>(lineData);
		contrast.intensityBackground += std::get<1>(lineData);
	}
}


Vector2d StbContrastDetect::determentSearchDistance(const StentBoostAlgo::StentMarkers &markers)
{
	Simplicity::NodeAffineTransform::Vector size;
	double StentLength = std::sqrt(sqr(markers.p1.x - markers.p2.x) + sqr(markers.p1.y - markers.p2.y));
	double searchDistance = StentLength * 0.10;
	if (m_mmPerPixel.x > 0.01)
	{
		searchDistance = 0.5 * MaxStentWidth / (m_mmPerPixel.x);
	}

	size.x = static_cast<float>(searchDistance);
	size.y = static_cast<float>(StentLength * 0.15);

	return size;
}


void StbContrastDetect::imageDownScale(float maxGrayValue, const CommonPlatform::Xray::XrayImage &input, StbFrangiImage<float> &output)
{
	float pixelCorrection = 1.0f / (maxGrayValue * (contrastFrangiDownSample*contrastFrangiDownSample));
	for (int y = 0; y < output.height; y++)
	{
		int yScale = y * contrastFrangiDownSample;
		int yDestOffset = y * output.stride;
		for (int v = 0; v < contrastFrangiDownSample; v++)
		{
			int yImageOffset = std::min((yScale + v), input.height - 1);
			yImageOffset *= input.stride;
			for (int x = 0; x < output.width; x++)
			{
				int xScale = x * contrastFrangiDownSample;
				for (int u = 0; u < contrastFrangiDownSample; u++)
				{
					output.pointer[yDestOffset + x] += static_cast<float>(input.pointer[std::min(xScale + u,input.width - 1) + yImageOffset]);
				}
			}
		}

		for (int x = 0; x < output.width; x++)
		{
			output.pointer[yDestOffset + x] *= pixelCorrection;
		}
	}
}

float StbContrastDetect::calcThreshold(std::vector<StbContrastDesc> &contrast)
{
	if (contrast.size() <= NumberOfFramesForInit) return FLT_MAX;

	float average = 0.0f;
	float bckAverage = 0.0f;
	float stdef = 0.0f;
	float stdefbck = 0.0f;
	int count = 0;

	for (int i = 0; i <= NumberOfFramesForInit; i++)
	{
		average += contrast[i].intensityMeasured;
		bckAverage += contrast[i].intensityBackground;
		count += (contrast[i].valid) ? 1 : 0;
	}

	if (count > 1)
	{
		average /= count;
		bckAverage /= count;

		for (int i = 0; i <= NumberOfFramesForInit; i++)
		{
			stdef += sqr(contrast[i].intensityMeasured - average);
			stdefbck += sqr(contrast[i].intensityBackground - bckAverage);
		}

		stdef = std::sqrtf(stdef / (count - 1));
		stdefbck = std::sqrt(stdefbck / (count - 1));
	}

	return stdef * ThresholdFractionStdevMeasurment + stdefbck * ThresholdFractionStdevBackGround + bckAverage * ThresholdFractionAvargeMeasurment;
}

void saveFrangiImage(std::string filename, StbFrangiImage<float> &image)
{
	std::ofstream stream;
	stream.open(filename, std::ofstream::trunc | std::ofstream::binary);

	float min = image.pointer[0];
	float max = image.pointer[0];
	for (int i = 0; i < image.height * image.stride; i++)
	{
		max = std::max(max, image.pointer[i]);
		min = std::min(min, image.pointer[i]);
	}
	for (int i = 0; i < image.stride * image.height; i++)
	{
		short value = static_cast<short>(image.pointer[i] * 16256.0f / (max - min));
		stream.write(reinterpret_cast<char*>(&value), sizeof(short));
	}

	stream.flush();
	stream.close();
}

void StbContrastDetect::storevalues(std::vector<StbContrastDesc> &contrast)
{
	FILE *f;
	std::string filename = "LocalContrast.txt";
	fopen_s(&f, filename.c_str(), "w+");
	float trashhold = calcThreshold(contrast);
	
	for (int i = 0; i < contrast.size(); i++)
	{
		std::string state = contrast[i].hasContrast ? "contrast" : "Boost" ;
		if( i ==0 )
		{
			fprintf_s(f, "%d\t%f\t%f\t%s\t%f\n ", i, contrast[i].intensityBackground, contrast[i].intensityMeasured, state.c_str(), trashhold);
		}
		else
		{
			fprintf_s(f, "%d\t%f\t%f\t%s\n ", i, contrast[i].intensityBackground, contrast[i].intensityMeasured, state.c_str());
		}
	}

	fclose(f);
}

void StbContrastDetect::storeArray(std::array<std::array<StbContrastDetect::VesselNessData, StentTrancinal>, StentLongitude> data)
{
	FILE *f;
	std::string filename = "arraydata.txt";

	fopen_s(&f, filename.c_str(), "w+");

	for (auto& items : data)
	{
		for (StbContrastDetect::VesselNessData& value : items)
		{
			fprintf_s(f, "%f\t ", value.current);
		}
		fprintf_s(f, "\n ");
	}

	fclose(f);
}

}}