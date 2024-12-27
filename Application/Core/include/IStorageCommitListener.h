// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <string>
#include <functional>
#include <vector>

namespace Pci { namespace Core { namespace PACS
{
	struct StorageItem
	{
		StorageItem(const std::wstring& sopClass, const std::wstring& sopInstance) :
			SopClass(sopClass), SopInstance(sopInstance)
		{}
		std::wstring SopClass;
		std::wstring SopInstance;
	};

	struct StorageReport
	{
		std::wstring TransactionUid;
		std::vector<StorageItem> Items;
	};

	class IStorageCommitListener
	{
	public:
		virtual ~IStorageCommitListener() {}

		std::function<void(const StorageReport& report)> onStoreSuccess;
		std::function<void(const StorageReport& report)> onStoreFailure;
	};

}}}