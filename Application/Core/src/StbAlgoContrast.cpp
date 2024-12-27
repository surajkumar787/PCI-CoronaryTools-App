// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.


#include "StbAlgoContrast.h"
#include "StbContrastDetect.h"
#include "StbContrastSelect.h"
#include "StbMarkers.h"
#include "StbImage.h"
#include <algorithm> 
#include <array>
#include <limits>


namespace Pci {	namespace Core {

std::array<double, 9> smoothing = { 0.04, 0.08, 0.12, 0.16, 0.2, 0.16, 0.12, 0.08, 0.04 };


StbAlgoContrast::StbAlgoContrast()
	: m_algoContrast(new StentBoostAlgo::StbContrastDetect()),
	m_contrastSelect(new StentBoostAlgo::StbContrastSelect()),
	m_contrastDesc(),
	n_count(0),
	m_numberOfContrastImageFound(0)
{
}

StbAlgoContrast::~StbAlgoContrast()
{
}

bool StbAlgoContrast::process(const std::shared_ptr<const CommonPlatform::Xray::XrayImage> &image, const StentBoostAlgo::StentMarkers &markers, StbImagePhase& phase)
{
	if (n_count == 0)
	{
		int bitDepth = image->bitdepth > 0 ? image->bitdepth : 16;
		m_algoContrast->init(image->height, image->width, bitDepth, image->mmPerPixel);
		m_contrastSelect->init();
	}

	m_contrastDesc.emplace_back();
	m_contrastDesc[n_count].image = std::make_shared<StentBoostAlgo::StbFrangiImage<float>>();
	m_contrastDesc[n_count].markers = markers;

	m_algoContrast->preProcess(*image, m_contrastDesc[n_count], n_count);
	m_algoContrast->process(m_contrastDesc,n_count);

	if (m_contrastDesc[n_count].hasContrast)
	{
		phase = StbImagePhase::transitionPhase;
		m_numberOfContrastImageFound++;
	}

	if (m_numberOfContrastImageFound > 3)
	{
		phase = StbImagePhase::contrastPhase;
	}

	n_count++;
	m_contrastSelect->process(markers, n_count);

	m_contrastSelect->postProcess(markers, m_contrastDesc[n_count - 1], image->imageNumber, phase != StbImagePhase::boostPhase);

	return true;
}

const std::vector<std::tuple<int, float>> StbAlgoContrast::getContrastScore()
{
	auto smoothContrast = getSmoothedContrastValues();

	std::vector<std::tuple<int, float>> score;
	for (int i = 0; i < m_contrastDesc.size(); i++)
	{
		float value = -1.0f;
		if (m_contrastDesc[i].stbSelect.valid && (m_contrastDesc[i].hasContrast))
		{
			value = m_contrastDesc[i].stbSelect.scoreTotal + static_cast<float> (2.0 * smoothContrast[i]);
			score.emplace_back(m_contrastDesc[i].stbSelect.imageNumber, value);
		}
	}
	
	return score;
}

std::vector<double>  StbAlgoContrast::getSmoothedContrastValues()
{
	std::vector<float> contrast;
	std::vector<double> smoothContrast(m_contrastDesc.size());

	for (auto image : m_contrastDesc)
	{
		contrast.push_back(image.intensityMeasured);
	}

	fillEmptyValue(contrast);

	int halfKernalSize = static_cast<int>((smoothing.size() -1) / 2);
	for (int i = 0; i < contrast.size(); i++)
	{
		double iSum = 0.0;
		double value = 0.0;
		for (int j = std::max(0, i - halfKernalSize); j < std::min(static_cast<int>(contrast.size()), i + halfKernalSize); j++)
		{
			iSum += smoothing[j - i + halfKernalSize];
			value += smoothing[j - i + halfKernalSize] * contrast[j];
		}

		value /= iSum;
		smoothContrast[i] = value;
	}

	return smoothContrast;
}

void StbAlgoContrast::fillEmptyValue(std::vector<float> &contrast)
{
	int indexLastValidMarker = -1;
	for (int i = 0; i < m_contrastDesc.size(); i++)
	{
		if (m_contrastDesc[i].markers.valid)
		{
			if (indexLastValidMarker == -1)
			{
				for (int j = i - 1; j >= 0; j--)
				{
					contrast[j] = contrast[i];
				}
			}
			else if (indexLastValidMarker != i - 1)
			{
				int delta = i - indexLastValidMarker;
				for (int j = indexLastValidMarker + 1; j < i; j++)
				{
					contrast[j] = contrast[i] + (contrast[indexLastValidMarker] - contrast[i]) * static_cast<float>((i - j) / delta);
				}
			}

			indexLastValidMarker = i;
		}
	}

	if (static_cast<int>(m_contrastDesc.size() - 1) != indexLastValidMarker)
	{
		for (int i = std::max(indexLastValidMarker, 1); i < m_contrastDesc.size() - 1; i++)
		{
			contrast[i] = contrast[i - 1];
		}
	}
}

bool StbAlgoContrast::end()
{
	m_contrastDesc.clear();
	n_count = 0;
	m_numberOfContrastImageFound = 0;
	return true;
}

}}
