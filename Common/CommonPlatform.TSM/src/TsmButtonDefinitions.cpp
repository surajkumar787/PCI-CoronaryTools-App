// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "TsmButtonDefinitions.h"

namespace CommonPlatform { namespace TSM
{
	namespace ButtonType
	{
		const std::wstring MainLevelFunction = L"mlf";
		const std::wstring MainLevelFunctionB = L"mlfb";
		const std::wstring MainLevelFunctionTouchPad = L"mlft";
		const std::wstring MainLevelSetting = L"mls";
		const std::wstring SubLevelFunction = L"slf";
		const std::wstring SubLevelSetting = L"sls";
		const std::wstring Menu = L"menu";
		const std::wstring Up = L"up";
		const std::wstring Down = L"down";
		const std::wstring Right = L"right";
		const std::wstring Left = L"left";
		const std::wstring Decrease = L"dec";
		const std::wstring HDecrease = L"hdec";
		const std::wstring Increase = L"inc";
		const std::wstring HIncrease = L"hinc";
		const std::wstring Help = L"help";

		const std::wstring FolderUp = L"Ext_FolderUp";
		const std::wstring Tools = L"Ext_Tools";
		const std::wstring Acquisition = L"Ext_Acquisition";
	}

	namespace ButtonIcon
	{
		const std::wstring MeasurementChart = L"MeasurementChart";
		const std::wstring NextImage = L"NextImage";
		const std::wstring NextRun = L"NextRun";
		const std::wstring PreviousImage = L"PreviousImage";
		const std::wstring PreviousRun = L"PreviousRun";
		const std::wstring Cbe = L"Cbe";
		const std::wstring RotateVolume = L"RotateVolume";
		const std::wstring Snapshot = L"Snapshot";
		const std::wstring TranslateVolume = L"TranslateVolume";
		const std::wstring StoreRun = L"StoreRun";
		const std::wstring Zoom = L"Zoom";
		const std::wstring PanZoom = L"PanZoom";
		const std::wstring MoviePlay = L"MoviePlay";
		const std::wstring MovieStop = L"MovieStop";
		const std::wstring MouseCursor = L"MouseCursor";
		const std::wstring VideoInvert = L"VideoInvert";
		const std::wstring PreviousMenu = L"PreviousMenu";
		const std::wstring NextMenu = L"NextMenu";
		const std::wstring Pan = L"Pan";
		const std::wstring ToolPosition = L"ToolPosition";
		const std::wstring OrientationAxialFeet = L"OrientationAxialFeet";
		const std::wstring OrientationAxialHead = L"OrientationAxialHead";
		const std::wstring OrientationCoronalFront = L"OrientationCoronalFront";
		const std::wstring OrientationCoronalRear = L"OrientationCoronalRear";
		const std::wstring OrientationSagittalLeft = L"OrientationSagittalLeft";
		const std::wstring OrientationSagittalRight = L"OrientationSagittalRight";
		const std::wstring OMA = L"OMA";
	}
}}