// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "PciXrayPositionViewer.h"

namespace Pci { namespace Core {
class PciGuidanceModel;

class PciEpxSelectionView : public PciXrayPositionViewer
{
public:
	PciEpxSelectionView(Sense::Control &parent, ViewType type);

	void setGuidanceModel(PciGuidanceModel &model);

	PciEpxSelectionView	&operator=(const PciEpxSelectionView&) = delete;
	PciEpxSelectionView(const PciEpxSelectionView&) = delete;

private:
	std::unique_ptr<Control> view;
	PciGuidanceModel* _model;
};

}}