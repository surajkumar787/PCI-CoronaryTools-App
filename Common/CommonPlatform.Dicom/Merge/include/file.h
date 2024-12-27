// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "messagebase.h"

#include <mc3msg.h>

namespace CommonPlatform { namespace Dicom { namespace Merge
{

	class Application;
	class Message;
	class Service;

	class FileBase : public MessageBase
	{
	protected:
		FileBase();
	public:        
		Message         toMessage(const Service & service, MC_COMMAND command);
	};
	
	class OutputFile : public FileBase
	{
		friend class Message;

	public:
		OutputFile(TRANSFER_SYNTAX ts);

	private:
		OutputFile(int id, TRANSFER_SYNTAX ts, bool owner=true); 

	public:                
		void write(const std::wstring& fileName);
		void write(const std::string& fileName);  
	};

	class InputFile : public FileBase
	{
		friend class Message;

	public:      
		InputFile();
		static bool isDicom(const std::string  & filename);
		static bool isDicom(const std::wstring  & filename);
		static InputFile open(const Application & application, const std::string  & filename);
		static InputFile open(const Application & application, const std::wstring & filename);        

	private:        
		InputFile(int id, bool owner=true); 

	public:        
		TRANSFER_SYNTAX getTransferSyntax() const override;
		bool            hasUncompressedPixelData() const override;
		std::vector<std::vector<uint8_t>> getPixelData(uint32_t frameIndex, uint32_t bytesPerImage) const override;
		std::string     getFileName() const;

	protected:
		bool            getBlobSize (const PrivateBlock * pBlock, unsigned long tagOrElm, unsigned long& dataSize) const override; // Use for any raw (uncompressed) blob attribute (so not for pixel-data)
		bool            getBlob     (const PrivateBlock * pBlock, unsigned long tagOrElm, void* buffer, unsigned long bufferSize) const override; // Use for any raw (uncompressed) blob attribute (so not for pixel-data)
		
	private:
		std::string     m_filepath;
		bool            m_uncompressedPixelData;
		TRANSFER_SYNTAX m_transferSyntax;
		int64_t         m_pixelDataOffset; 
		uint32_t        m_pixelDataSize; //0xFFFFFFFF == Encapsulated bulk. 
	};

}}}
