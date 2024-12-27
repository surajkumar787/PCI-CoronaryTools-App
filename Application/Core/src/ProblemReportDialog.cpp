// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "ProblemReportDialog.h"
#include "Translation.h"

using namespace Pci::Core;

ProblemReportDialog::ProblemReportDialog(Control &parent)
:   ProblemReportDialogUi(parent)
{
    close.eventClicked          = [&] { hide(); };
    closeButton.eventClicked    = [&] { hide(); };

    this->text          = Localization::Translation::IDS_ProblemReport;
    message.text        = Localization::Translation::IDS_ProblemReportReady;
    contactService.text = Localization::Translation::IDS_ContactServiceProvider;
	closeButton.text    = Localization::Translation::IDS_OK;

    visible     = false;
}

void ProblemReportDialog::show()
{
    auto parentRect = getParentSpace().bounds;
    position = Point( (parentRect.width()-size->width)/2,(parentRect.height()-size->height)/2);
    visible = true;
}

void ProblemReportDialog::hide()
{
    visible = false;
}
