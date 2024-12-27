// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "messagebase.h"

namespace CommonPlatform { namespace Dicom { namespace Merge
{

	class Item : public MessageBase
	{
	public:
		Item();
		explicit Item(const std::wstring& itemName);
		Item(int messageId);
		Item(int messageId, bool freeOnDestruction);
	};

}}}
 