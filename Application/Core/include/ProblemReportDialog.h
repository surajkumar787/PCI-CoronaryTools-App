// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "problemreportdialog.ui.h"

namespace Pci { namespace Core {

using namespace Sense;
using namespace Sense::ExperienceIdentity;

class ProblemReportDialog : public ProblemReportDialogUi
{
public:
    explicit ProblemReportDialog(Control &parent);

    void show();
    void hide();

};

}}