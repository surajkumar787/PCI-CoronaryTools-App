// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "item.h"

#include "exception.h"

#include <mergecom.h>

namespace CommonPlatform { namespace Dicom { namespace Merge
{

	Item::Item()
	{
		mcCheck(MC_Open_Empty_Item(&m_id), "Unable to create new sequence item");
		int id(m_id);
		m_handle.reset([id]{int tmp(id); MC_Free_Item(&tmp);});
	}

	Item::Item(const std::wstring& itemName)
	{
		mcCheck(MC_Open_Item(&m_id, toUtf8(itemName).c_str()));
		int id(m_id);
		m_handle.reset([id]{int tmp(id); MC_Free_Item(&tmp);});
	}

	Item::Item(int id):Item (id,true)
	{
	}

	Item::Item(int id, bool freeOnDestruction)
	{
		m_id = id;
		if (freeOnDestruction)
		{
			m_handle.reset([id] {int tmp(id); MC_Free_Item(&tmp); });
		}
	}

}}}