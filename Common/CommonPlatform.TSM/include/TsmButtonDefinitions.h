// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <string>

namespace CommonPlatform { namespace TSM 
{ 
	namespace ButtonType
	{
		extern const std::wstring MainLevelFunction;
		extern const std::wstring MainLevelFunctionB;
		extern const std::wstring MainLevelFunctionTouchPad;
		extern const std::wstring MainLevelSetting;
		extern const std::wstring SubLevelFunction;
		extern const std::wstring SubLevelSetting;
		extern const std::wstring Menu;
		extern const std::wstring Up;
		extern const std::wstring Down;
		extern const std::wstring Right;
		extern const std::wstring Left;
		extern const std::wstring Decrease;
		extern const std::wstring HDecrease;
		extern const std::wstring Increase;
		extern const std::wstring HIncrease;
		extern const std::wstring Help;

		// simulator extension
		extern const std::wstring FolderUp;
		extern const std::wstring Tools;
		extern const std::wstring Acquisition;
	}

	namespace ButtonIcon
	{
		extern const std::wstring MeasurementChart;
		extern const std::wstring NextImage;
		extern const std::wstring NextRun;
		extern const std::wstring PreviousImage;
		extern const std::wstring PreviousRun;
		extern const std::wstring Cbe; // ???
		extern const std::wstring RotateVolume;
		extern const std::wstring Snapshot;
		extern const std::wstring TranslateVolume;
		extern const std::wstring StoreRun;
		extern const std::wstring Zoom;
		extern const std::wstring PanZoom;
		extern const std::wstring MoviePlay;
		extern const std::wstring MovieStop;
		extern const std::wstring MouseCursor;
		extern const std::wstring VideoInvert;
		extern const std::wstring PreviousMenu;
		extern const std::wstring NextMenu;
		extern const std::wstring Pan;
		extern const std::wstring ToolPosition;
		extern const std::wstring OrientationAxialFeet;
		extern const std::wstring OrientationAxialHead;
		extern const std::wstring OrientationCoronalFront;
		extern const std::wstring OrientationCoronalRear;
		extern const std::wstring OrientationSagittalLeft;
		extern const std::wstring OrientationSagittalRight;
		extern const std::wstring OMA; // ???
	}
}}