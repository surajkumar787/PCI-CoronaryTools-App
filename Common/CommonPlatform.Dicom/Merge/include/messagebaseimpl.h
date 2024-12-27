// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "messagebase.h"

namespace CommonPlatform { namespace Dicom { namespace Merge
{
	class Item;

	bool getImpl(int msg, const PrivateBlock * block, unsigned long tagOrElm,       std::string  & value, int valNr);
	bool getImpl(int msg, const PrivateBlock * block, unsigned long tagOrElm,       std::wstring & value, int valNr);
	bool getImpl(int msg, const PrivateBlock * block, unsigned long tagOrElm,             int8_t & value, int valNr);
	bool getImpl(int msg, const PrivateBlock * block, unsigned long tagOrElm,            uint8_t & value, int valNr);
	bool getImpl(int msg, const PrivateBlock * block, unsigned long tagOrElm,            int16_t & value, int valNr);
	bool getImpl(int msg, const PrivateBlock * block, unsigned long tagOrElm,           uint16_t & value, int valNr);
	bool getImpl(int msg, const PrivateBlock * block, unsigned long tagOrElm,            int32_t & value, int valNr);
	bool getImpl(int msg, const PrivateBlock * block, unsigned long tagOrElm,           uint32_t & value, int valNr);
	bool getImpl(int msg, const PrivateBlock * block, unsigned long tagOrElm,            int64_t & value, int valNr);
	bool getImpl(int msg, const PrivateBlock * block, unsigned long tagOrElm,           uint64_t & value, int valNr);
	bool getImpl(int msg, const PrivateBlock * block, unsigned long tagOrElm,              float & value, int valNr);
	bool getImpl(int msg, const PrivateBlock * block, unsigned long tagOrElm,             double & value, int valNr);
	bool getImpl(int msg, const PrivateBlock * block, unsigned long tagOrElm,               bool & value, int valNr);

	bool setImpl(int msg, const PrivateBlock * block, unsigned long tagOrElm, const std::string  & value, int valNr);
	bool setImpl(int msg, const PrivateBlock * block, unsigned long tagOrElm, const std::wstring & value, int valNr);
	bool setImpl(int msg, const PrivateBlock * block, unsigned long tagOrElm, const       int8_t   value, int valNr);
	bool setImpl(int msg, const PrivateBlock * block, unsigned long tagOrElm, const      uint8_t   value, int valNr);
	bool setImpl(int msg, const PrivateBlock * block, unsigned long tagOrElm, const      int16_t   value, int valNr);
	bool setImpl(int msg, const PrivateBlock * block, unsigned long tagOrElm, const     uint16_t   value, int valNr);
	bool setImpl(int msg, const PrivateBlock * block, unsigned long tagOrElm, const      int32_t   value, int valNr);
	bool setImpl(int msg, const PrivateBlock * block, unsigned long tagOrElm, const     uint32_t   value, int valNr);
	bool setImpl(int msg, const PrivateBlock * block, unsigned long tagOrElm, const      int64_t   value, int valNr);
	bool setImpl(int msg, const PrivateBlock * block, unsigned long tagOrElm, const     uint64_t   value, int valNr);
	bool setImpl(int msg, const PrivateBlock * block, unsigned long tagOrElm, const        float   value, int valNr);
	bool setImpl(int msg, const PrivateBlock * block, unsigned long tagOrElm, const       double   value, int valNr);
	bool setImpl(int msg, const PrivateBlock * block, unsigned long tagOrElm, const         Item   value, int valNr);

}}}