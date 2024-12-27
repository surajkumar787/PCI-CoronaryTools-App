// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

namespace Pci {	namespace Core
{
	class IReplayControl
	{
	public:

		virtual ~IReplayControl() {}

		virtual void startXRayReplay(bool loop) = 0;
		virtual void stopXRayReplay() = 0;
		virtual bool isActive() = 0;
	};
}}
