// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

namespace CommonPlatform { namespace TSM
{
	class TsmPage;

	class ITsmSource
	{
	public:
        virtual ~ITsmSource() {}

		virtual void setPage(TsmPage* page) = 0;
	};

}}