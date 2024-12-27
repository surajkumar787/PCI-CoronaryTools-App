// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <memory>
#include <vector>
#include "StbContrastDetect.h"
#include "StbContrastSelect.h"
#include "StbFrangiImage.h"


namespace Pci {	namespace StentBoostAlgo 	{
	struct StentMarkers;
}}

namespace CommonPlatform {	namespace Xray {
  struct	XrayImage;
}}

namespace Pci{ namespace Core{

struct StbMarkers;
struct StbImage;
enum class StbImagePhase;

class StbAlgoContrast
{
public:
	StbAlgoContrast();
	virtual ~StbAlgoContrast();

	bool process( const std::shared_ptr<const CommonPlatform::Xray::XrayImage> &image, const StentBoostAlgo::StentMarkers &markers, StbImagePhase& phase);
	const std::vector<std::tuple<int, float>> getContrastScore();
	bool end();

	StbAlgoContrast(const StbAlgoContrast&) = delete;
	StbAlgoContrast& operator=(const StbAlgoContrast&) = delete;

private:
	std::vector<double> getSmoothedContrastValues();
	void fillEmptyValue(std::vector<float> &contrast);

	std::unique_ptr<StentBoostAlgo::StbContrastDetect>	        m_algoContrast;
	std::unique_ptr<StentBoostAlgo::StbContrastSelect>			m_contrastSelect;
	std::vector<StentBoostAlgo::StbContrastDesc>		        m_contrastDesc;
	int															n_count;
	int															m_numberOfContrastImageFound;
};

}}