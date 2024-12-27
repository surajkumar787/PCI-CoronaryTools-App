// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "util.h"

#include <mc3msg.h>
#include <diction.h>
#include <stdint.h>

namespace CommonPlatform { namespace Dicom { namespace Merge
{
	class PrivateBlock
	{
	public:
		PrivateBlock(): m_code(), m_group(0) {}
		explicit PrivateBlock(const std::string & code, uint16_t group) : m_code(code), m_group(group) {};

		inline const char * code()  const { return m_code.c_str();  };
		inline uint16_t     group() const { return m_group; };

	private:
		std::string m_code;
		uint16_t m_group;
	};

	class Item;
	class MessageBase
	{
		friend class Association;

	public:
		struct Attribute
		{
			Attribute() : tag(0), vr(UNKNOWN_VR), numValues(0) {}

			unsigned long   tag;
			MC_VR           vr;
			int             numValues;
		};       

	protected:
		MessageBase();
		virtual ~MessageBase();

	public:
		int                     id                      () const;

		bool                    addAttribute            (unsigned long tag);
		bool                    addAttribute            (unsigned long tag, MC_VR vr);
		bool                    addAttribute            (const PrivateBlock & block, uint8_t element, MC_VR vr);

		bool                    deleteAttribute         (unsigned long tag);
		bool                    deletePrivateAttribute  (const PrivateBlock & block, uint8_t element);

		std::vector<Attribute>  attributes() const;

		PrivateBlock            addPrivateBlock         (const std::string & privateCode, uint16_t group);
		bool                    deletePrivateBlock      (const PrivateBlock & block);

		bool                    get                     (unsigned long tag, std::string & value) const;
		bool                    get                     (unsigned long tag, std::wstring & value) const;
		bool                    get                     (unsigned long tag,       int8_t & value) const;
		bool                    get                     (unsigned long tag,      uint8_t & value) const;
		bool                    get                     (unsigned long tag,      int16_t & value) const;
		bool                    get                     (unsigned long tag,     uint16_t & value) const;
		bool                    get                     (unsigned long tag,      int32_t & value) const;
		bool                    get                     (unsigned long tag,     uint32_t & value) const;
		bool                    get                     (unsigned long tag,      int64_t & value) const;
		bool                    get                     (unsigned long tag,     uint64_t & value) const;
		bool                    get                     (unsigned long tag,        float & value) const;
		bool                    get                     (unsigned long tag,       double & value) const;
		bool                    get                     (unsigned long tag,         bool & value) const;

		bool                    get                     (const PrivateBlock & pBlock, uint8_t element, std::string  & value) const;
		bool                    get                     (const PrivateBlock & pBlock, uint8_t element, std::wstring & value) const;
		bool                    get                     (const PrivateBlock & pBlock, uint8_t element,        int8_t  & value) const;
		bool                    get                     (const PrivateBlock & pBlock, uint8_t element,       uint8_t  & value) const;
		bool                    get                     (const PrivateBlock & pBlock, uint8_t element,       int16_t  & value) const;
		bool                    get                     (const PrivateBlock & pBlock, uint8_t element,      uint16_t  & value) const;
		bool                    get                     (const PrivateBlock & pBlock, uint8_t element,       int32_t  & value) const;
		bool                    get                     (const PrivateBlock & pBlock, uint8_t element,      uint32_t  & value) const;
		bool                    get                     (const PrivateBlock & pBlock, uint8_t element,       int64_t  & value) const;
		bool                    get                     (const PrivateBlock & pBlock, uint8_t element,      uint64_t  & value) const;
		bool                    get                     (const PrivateBlock & pBlock, uint8_t element,         float  & value) const;
		bool                    get                     (const PrivateBlock & pBlock, uint8_t element,        double  & value) const;
		bool                    get                     (const PrivateBlock & pBlock, uint8_t element,          bool  & value) const;

		bool                    get                     (unsigned long tag, std::vector<std::string> & value) const;
		bool                    get                     (unsigned long tag, std::vector<std::wstring> & value) const;
		bool                    get                     (unsigned long tag, std::vector<       int8_t > & value) const;
		bool                    get                     (unsigned long tag, std::vector<      uint8_t > & value) const;
		bool                    get                     (unsigned long tag, std::vector<      int16_t > & value) const;
		bool                    get                     (unsigned long tag, std::vector<     uint16_t > & value) const;
		bool                    get                     (unsigned long tag, std::vector<      int32_t > & value) const;
		bool                    get                     (unsigned long tag, std::vector<     uint32_t > & value) const;
		bool                    get                     (unsigned long tag, std::vector<      int64_t > & value) const;
		bool                    get                     (unsigned long tag, std::vector<     uint64_t > & value) const;
		bool                    get                     (unsigned long tag, std::vector<        float > & value) const;
		bool                    get                     (unsigned long tag, std::vector<       double > & value) const;
		bool                    get                     (unsigned long tag, std::vector<         Item > & value) const;

		bool                    get                     (const PrivateBlock & block, uint8_t element, std::vector<  std::string > & value) const;
		bool                    get                     (const PrivateBlock & block, uint8_t element, std::vector< std::wstring > & value) const;
		bool                    get                     (const PrivateBlock & block, uint8_t element, std::vector<       int8_t > & value) const;
		bool                    get                     (const PrivateBlock & block, uint8_t element, std::vector<      uint8_t > & value) const;
		bool                    get                     (const PrivateBlock & block, uint8_t element, std::vector<      int16_t > & value) const;
		bool                    get                     (const PrivateBlock & block, uint8_t element, std::vector<     uint16_t > & value) const;
		bool                    get                     (const PrivateBlock & block, uint8_t element, std::vector<      int32_t > & value) const;
		bool                    get                     (const PrivateBlock & block, uint8_t element, std::vector<     uint32_t > & value) const;
		bool                    get                     (const PrivateBlock & block, uint8_t element, std::vector<      int64_t > & value) const;
		bool                    get                     (const PrivateBlock & block, uint8_t element, std::vector<     uint64_t > & value) const;
		bool                    get                     (const PrivateBlock & block, uint8_t element, std::vector<        float > & value) const;
		bool                    get                     (const PrivateBlock & block, uint8_t element, std::vector<       double > & value) const;
		bool                    get                     (const PrivateBlock & block, uint8_t element, std::vector<         Item > & value) const;



		bool                    set                     (unsigned long tag, const std::wstring & value) const;
		bool                    set                     (unsigned long tag, const  std::string & value) const;
		bool                    set                     (unsigned long tag,             int8_t   value) const;
		bool                    set                     (unsigned long tag,            uint8_t   value) const;
		bool                    set                     (unsigned long tag,            int16_t   value) const;
		bool                    set                     (unsigned long tag,           uint16_t   value) const;
		bool                    set                     (unsigned long tag,            int32_t   value) const;
		bool                    set                     (unsigned long tag,           uint32_t   value) const;
		bool                    set                     (unsigned long tag,            int64_t   value) const;
		bool                    set                     (unsigned long tag,           uint64_t   value) const;
		bool                    set                     (unsigned long tag,              float   value) const;
		bool                    set                     (unsigned long tag,             double   value) const;

		bool                    set                     (const PrivateBlock & block, uint8_t element, const std::wstring & value) const;
		bool                    set                     (const PrivateBlock & block, uint8_t element, const  std::string & value) const;
		bool                    set                     (const PrivateBlock & block, uint8_t element,             int8_t   value) const;
		bool                    set                     (const PrivateBlock & block, uint8_t element,            uint8_t   value) const;
		bool                    set                     (const PrivateBlock & block, uint8_t element,            int16_t   value) const;
		bool                    set                     (const PrivateBlock & block, uint8_t element,           uint16_t   value) const;
		bool                    set                     (const PrivateBlock & block, uint8_t element,            int32_t   value) const;
		bool                    set                     (const PrivateBlock & block, uint8_t element,           uint32_t   value) const;
		bool                    set                     (const PrivateBlock & block, uint8_t element,            int64_t   value) const;
		bool                    set                     (const PrivateBlock & block, uint8_t element,           uint64_t   value) const;
		bool                    set                     (const PrivateBlock & block, uint8_t element,              float   value) const;
		bool                    set                     (const PrivateBlock & block, uint8_t element,             double   value) const;

		bool                    set                     (unsigned long tag, const std::vector<  std::string > & value);
		bool                    set                     (unsigned long tag, const std::vector< std::wstring > & value);
		bool                    set                     (unsigned long tag, const std::vector<       int8_t > & value);
		bool                    set                     (unsigned long tag, const std::vector<      uint8_t > & value);
		bool                    set                     (unsigned long tag, const std::vector<      int16_t > & value);
		bool                    set                     (unsigned long tag, const std::vector<     uint16_t > & value);
		bool                    set                     (unsigned long tag, const std::vector<      int32_t > & value);
		bool                    set                     (unsigned long tag, const std::vector<     uint32_t > & value);
		bool                    set                     (unsigned long tag, const std::vector<      int64_t > & value);
		bool                    set                     (unsigned long tag, const std::vector<     uint64_t > & value);
		bool                    set                     (unsigned long tag, const std::vector<        float > & value);
		bool                    set                     (unsigned long tag, const std::vector<       double > & value);
		bool                    set                     (unsigned long tag, const std::vector<         Item > & value);

		bool                    set                     (const PrivateBlock & block, uint8_t element, const std::vector<  std::string > & value);
		bool                    set                     (const PrivateBlock & block, uint8_t element, const std::vector< std::wstring > & value);
		bool                    set                     (const PrivateBlock & block, uint8_t element, const std::vector<       int8_t > & value);
		bool                    set                     (const PrivateBlock & block, uint8_t element, const std::vector<      uint8_t > & value);
		bool                    set                     (const PrivateBlock & block, uint8_t element, const std::vector<      int16_t > & value);
		bool                    set                     (const PrivateBlock & block, uint8_t element, const std::vector<     uint16_t > & value);
		bool                    set                     (const PrivateBlock & block, uint8_t element, const std::vector<      int32_t > & value);
		bool                    set                     (const PrivateBlock & block, uint8_t element, const std::vector<     uint32_t > & value);
		bool                    set                     (const PrivateBlock & block, uint8_t element, const std::vector<      int64_t > & value);
		bool                    set                     (const PrivateBlock & block, uint8_t element, const std::vector<     uint64_t > & value);
		bool                    set                     (const PrivateBlock & block, uint8_t element, const std::vector<        float > & value);
		bool                    set                     (const PrivateBlock & block, uint8_t element, const std::vector<       double > & value);
		bool                    set                     (const PrivateBlock & block, uint8_t element, const std::vector<         Item > & value);

		// Sets the Dicom VR of an attribute. Needed for private attributes that are not in the dictionary
		bool                    setVR                   (const PrivateBlock & block, uint8_t element, MC_VR vr) const;
		
		bool                    setEmpty                (unsigned long tag);
		bool                    setEmpty                (const PrivateBlock & block, uint8_t element);
		bool                    setNull                 (unsigned long tag);
		bool                    setNull                 (const PrivateBlock & block, uint8_t element);
		
		void setTransferSyntax(TRANSFER_SYNTAX ts);
		virtual TRANSFER_SYNTAX getTransferSyntax() const;        
		virtual bool            hasUncompressedPixelData() const;
		bool					hasAttribute(unsigned long tag) const;
		virtual std::vector<std::vector<uint8_t>> getPixelData(uint32_t frameIndex, uint32_t bytesPerImage) const;

	protected:
		virtual bool            getBlobSize             (const PrivateBlock * pBlock, unsigned long tagOrElm, unsigned long& dataSize) const; // Use for any raw (uncompressed) blob attribute (so not for pixel-data)
		virtual bool            getBlob                 (const PrivateBlock * pBlock, unsigned long tagOrElm, void* buffer, unsigned long bufferSize) const; // Use for any raw (uncompressed) blob attribute (so not for pixel-data)

	private:
		template <typename T>
		bool internalGetVector(const PrivateBlock * pBlock, unsigned long tagOrElm, std::vector<T> & value) const;

		template <typename T>
		bool internalSetVector(const PrivateBlock * pBlock, unsigned long tagOrElm, const std::vector<T> & value);

	protected:
		Handle          m_handle;
		int             m_id;
	};

	std::ostream & operator<<(std::ostream & os, const MessageBase & msg);

	std::string to_hstring(unsigned long tag);
	std::string to_hstring(const PrivateBlock & block, uint8_t element);

}}}