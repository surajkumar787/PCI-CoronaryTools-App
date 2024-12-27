// Copyright Koninklijke Philips N.V. 2017
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "windows.h"
#include "windef.h"

namespace Pci {	namespace PciApp {
	class WindowKeyHandler
	{
	public:
		explicit WindowKeyHandler(bool disableAltTab);
		virtual ~WindowKeyHandler();

	private:
		void registerKeyEventHook();
	};
}}