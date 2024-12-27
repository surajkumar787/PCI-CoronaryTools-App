// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "file.h"

#include "exception.h"
#include "message.h"
#include "application.h"
#include "service.h"

#include <mc3media.h>
#include <fstream>
#include <array>
#include <assert.h>

#undef min
#undef max

namespace CommonPlatform { namespace Dicom { namespace Merge
{
	const static int ImageSize = 1024;

	FileBase::FileBase() : MessageBase()
	{
	}

	class FileWriter
	{
	public:
		FileWriter(const Merge::OutputFile & msg, const std::string& fileName)
			: m_streamBuffer(ImageSize *ImageSize)
			, m_fileName(fileName)
			, m_file()
		{
			m_file.open(m_fileName, std::ios_base::binary);
			m_file.rdbuf()->pubsetbuf(m_streamBuffer.data(), m_streamBuffer.size());

			auto writeFunc = [](char *, void* thisPtr, int size, void* buffer, int, int) -> MC_STATUS
			{ return reinterpret_cast<FileWriter*>(thisPtr)->writeCallback(size, static_cast<char*>(buffer)); };
			Merge::mcCheck(MC_Write_File(msg.id(), 0, this, writeFunc ));
		}

		MC_STATUS writeCallback(int size, char * buffer)
		{
			m_file.write(buffer, size);
			return MC_NORMAL_COMPLETION;
		}

		FileWriter(const FileWriter &) = delete;
		FileWriter & operator=(const FileWriter &) = delete;

	private:
		std::vector<char>        m_streamBuffer;
		std::string              m_fileName;
		std::ofstream            m_file;

	};

	OutputFile::OutputFile(TRANSFER_SYNTAX ts) : FileBase()
	{
		mcCheck(MC_Create_Empty_File(&m_id, ""));
		int id = m_id;
		m_handle.reset([id]{int tmp(id); MC_Free_File(&tmp);});
		setTransferSyntax(ts);
	}

	OutputFile::OutputFile(int id, TRANSFER_SYNTAX ts, bool owner) : FileBase()
	{
		// This constructor is solely used for saving files which are received via a dicom networking request
		m_id = id;
		if(owner) m_handle.reset([id]{int tmp(id); MC_Free_File(&tmp);});
		// Let Merge convert it to little endian if needed. We only support Little Endian in Dcm2Mil
		if (ts == EXPLICIT_BIG_ENDIAN)
		{
			ts = EXPLICIT_LITTLE_ENDIAN;
		}
		
		if (ts == IMPLICIT_BIG_ENDIAN)
		{
			ts = IMPLICIT_LITTLE_ENDIAN;
		}

		setTransferSyntax(ts);
	}

	void OutputFile::write(const std::wstring& fileName)
	{
		return write(toUtf8(fileName));
	}

	void OutputFile::write(const std::string& fileName)
	{
		FileWriter writer(*this, fileName);
	}

	const static long ExplicitHeaderSize = 12;
	const static long ImplicitHeaderSize = 8;

	class FileReader
	{
	public:
		FileReader(const Merge::Application & app, const Merge::InputFile & msg, const std::string& fileName)
			: m_app            (app)
			, m_file           (fileName, std::ios_base::binary)
			, m_buffer         (ImageSize *ImageSize)
			, m_uncompressedPixelData(false)
			, m_transferSyntax(INVALID_TRANSFER_SYNTAX)
			, m_pixelDataOffset(0)
			, m_pixelDataSize  (0)
		{
			uint64_t fileSize(0);
			long pixeldataTagOffset(0);
			long tagHeaderSize(0);

			auto readCallbackFunc = [](char *, void * thisPtr, int * size, void ** buffer, int, int * last) -> MC_STATUS
			{ return reinterpret_cast<FileReader*>(thisPtr)->readCallback(size, buffer, last); };
			Merge::mcCheck(MC_Open_File_Upto_Tag(m_app.id(), msg.id(), this, MC_ATT_PIXEL_DATA-1, &pixeldataTagOffset, readCallbackFunc));
			m_file.clear();

			std::string tsString;
			msg.get(MC_ATT_TRANSFER_SYNTAX_UID, tsString);
			MC_Get_Enum_From_Transfer_Syntax(tsString.c_str(), &m_transferSyntax);
			switch(m_transferSyntax)
			{
			case EXPLICIT_BIG_ENDIAN:
			case EXPLICIT_LITTLE_ENDIAN:
				m_uncompressedPixelData = true;
				tagHeaderSize = ExplicitHeaderSize;
				break;

			default: // Explicity little endian is enforced for compression schemes
				m_uncompressedPixelData = false;
				tagHeaderSize = ExplicitHeaderSize;
				break;
			case IMPLICIT_BIG_ENDIAN:
			case IMPLICIT_LITTLE_ENDIAN:
				m_uncompressedPixelData = true;
				tagHeaderSize = ImplicitHeaderSize;
				break;
			case INVALID_TRANSFER_SYNTAX:
				throw "Invalid transfer syntax";
			}

			m_file.seekg(0, m_file.end);
			fileSize = m_file.tellg();

			if((pixeldataTagOffset + tagHeaderSize) < fileSize)
			{
				m_file.seekg(pixeldataTagOffset);
				std::array<uint8_t, 12> element;
				m_file.read(reinterpret_cast<char*>(&element[0]), element.size());
				if (m_transferSyntax == IMPLICIT_LITTLE_ENDIAN || m_transferSyntax == EXPLICIT_LITTLE_ENDIAN || m_transferSyntax == JPEG_LOSSLESS_HIER_14)
				{
					std::swap(reinterpret_cast<uint16_t &>(element[0]), reinterpret_cast<uint16_t &>(element[2]));
					unsigned long tag = reinterpret_cast<uint32_t &>(element[0]);
					if(tag != MC_ATT_PIXEL_DATA) throw Merge::Exception(MC_SYSTEM_ERROR, std::string("Not implemented: Cannot read pixel data tag..."));

					int sizeIndex = 8;
					if (m_transferSyntax == IMPLICIT_LITTLE_ENDIAN) 
						sizeIndex -= 4;

					m_pixelDataOffset = pixeldataTagOffset + tagHeaderSize;
					uint32_t tagSize(reinterpret_cast<uint32_t &>(element[sizeIndex]));
					if (tagSize == 0xFFFFFFFF)
					{
						// Bulkdata is encapsulated
						m_pixelDataSize = tagSize;
					}
					else
					{                    
						m_pixelDataSize = std::min(uint32_t(tagSize), uint32_t(fileSize - m_pixelDataOffset));
					}
				} 
				else
				{
					throw Merge::Exception(MC_SYSTEM_ERROR, std::string("Not implemented: Trying to read an unsupported transfer syntax..."));
				}
			}

		}

		~FileReader()
		{
			m_file.close();
		}

		MC_STATUS readCallback(int * size, void ** buffer, int * last)
		{
			if(m_file)
			{
				m_file.read(&m_buffer[0], m_buffer.size());
				*buffer = &m_buffer[0];
				auto bytesRead(m_file.gcount());
				*size = int(bytesRead + bytesRead % 2); // Buffer must be of even size
				*last = !m_file;
				return MC_NORMAL_COMPLETION;
			} else {
				return MC_CANNOT_COMPLY;
			}
		}

		int64_t getPixelDataOffset() const
		{
			return m_pixelDataOffset;
		}

		uint32_t getPixelDataSize() const
		{
			return m_pixelDataSize;
		}

		TRANSFER_SYNTAX getTransferSystax() const
		{
			return m_transferSyntax;
		}

		bool getUncompressedPixelData() const
		{
			return m_uncompressedPixelData;
		}

		FileReader(const FileReader &) = delete;
		FileReader & operator=(const FileReader &) = delete;

	private:
		Merge::Application    m_app;
		std::ifstream         m_file;
		std::vector<char>     m_buffer;
		bool                  m_uncompressedPixelData;
		TRANSFER_SYNTAX       m_transferSyntax;
		int64_t               m_pixelDataOffset;
		uint32_t              m_pixelDataSize;
	};

	bool InputFile::isDicom(const std::wstring  & filename) { return isDicom(toUtf8(filename)); }
	bool InputFile::isDicom(const std::string  & filename)
	{
		std::ifstream file(filename, std::ios::binary);
		if(file)
		{
			int preableSize(128);
			int headerSize(4);
			std::vector<char> buf(preableSize + headerSize, 0);
			file.read(buf.data(), buf.size());
			return std::string(&buf[preableSize], headerSize) == "DICM";
		}
		return false;
	}

	InputFile InputFile::open(const Application & application, const std::wstring & filepath) { return open(application, toUtf8(filepath)); }

	InputFile InputFile::open(const Application & application, const std::string & filepath)
	{
		if(!pathExists(filepath)) throw Exception(MC_SYSTEM_ERROR, "File " + filepath + " not found");

		int id;
		mcCheck
			(
			MC_Create_Empty_File(&id, to_charp(filepath)),
			std::string("Error creating empty file from: ") + filepath
			);

		InputFile file(id);
		FileReader fr(application, file, filepath);
		file.m_filepath        = filepath;
		file.m_uncompressedPixelData = fr.getUncompressedPixelData();
		file.m_transferSyntax  = fr.getTransferSystax();
		file.m_pixelDataSize   = fr.getPixelDataSize();
		file.m_pixelDataOffset = fr.getPixelDataOffset();
		return file;
	}

	InputFile::InputFile()
		: InputFile( 0 , false)	
	{
	}

	InputFile::InputFile(int id, bool owner) : 
		FileBase      (),
		m_filepath(),
		m_uncompressedPixelData(false),
		m_transferSyntax (INVALID_TRANSFER_SYNTAX),
		m_pixelDataOffset(0),
		m_pixelDataSize  (0)
	{
		m_id = id;
		if(owner) m_handle.reset([id]{int tmp(id); MC_Free_File(&tmp);});
	}

	struct FragmentHeader
	{
		uint32_t tag;
		uint32_t length;
		void read(std::ifstream& stream)
		{
			static_assert(sizeof(FragmentHeader) == 8, "May not be padded");
			stream.read(reinterpret_cast<char *>(this), sizeof(FragmentHeader));
			for (size_t i = 0; i < 4; i+=4)
			{
				std::swap(reinterpret_cast<uint8_t*>(this)[i + 1], reinterpret_cast<uint8_t*>(this)[i + 3]);
				std::swap(reinterpret_cast<uint8_t*>(this)[i + 0], reinterpret_cast<uint8_t*>(this)[i + 2]);
			}            
		}
		bool isValid() const
		{
			return(tag == 0xFFFEE000);
		}
	};

	std::vector<std::vector<uint8_t>> InputFile::getPixelData(uint32_t frameIndex, uint32_t bytesPerImage) const
	{
		std::vector<std::vector<uint8_t>> result;
		if (m_pixelDataOffset == 0)
		{
			return result;
		}

		if (m_pixelDataSize != 0xFFFFFFFF)
		{
			// Non-encapsulated data
			uint64_t bytesToRead;
			if (m_uncompressedPixelData)
			{
				// Uncompressed single or multi frame
				bytesToRead = bytesPerImage;
			}
			else
			{
				// Compressed single frame. Read all pixels
				assert(frameIndex == 0);
				bytesToRead = m_pixelDataSize;
			}
			result.emplace_back(bytesToRead);
			auto& buffer = result[0];
			std::ifstream file(m_filepath, std::ios_base::binary);
			file.seekg(m_pixelDataOffset + static_cast<int64_t>(frameIndex) * bytesPerImage);
			file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
			auto readCount = file.gcount();
			if ((readCount <= 0) || (static_cast<uint64_t>(readCount) != buffer.size()))
			{
				assert(false);
				// Do allow the half-read bulk to pass in case of partially corrupted data
			}
			return result;
		}
		else
		{
			// Encapsulated data
			std::ifstream file(m_filepath, std::ios_base::binary);
			file.seekg(0, file.end);
			auto fileSize = file.tellg();    
			file.seekg(m_pixelDataOffset);
			FragmentHeader fragmentHeader;
			fragmentHeader.read(file);
			assert(fragmentHeader.isValid());            
			uint64_t startOfFrame = 0;
			uint64_t startOfNextFrame;
			bool sizeDetermined;
			if (fragmentHeader.length == 0)
			{
				// Single fragment of undetermined size (when compressed), read all data
				assert(frameIndex == 0);
				startOfNextFrame = std::numeric_limits<uint64_t>::max();
				sizeDetermined = false;
			}
			else
			{
				// Parse the offsets of the various frames in the dicom-file
				// The offsets are 4-byte unsigned integers
				assert(fragmentHeader.length % sizeof(uint32_t) == 0);
				if (fragmentHeader.length % sizeof(uint32_t) != 0)
				{
					return result; // Invalid dicom data
				}

				std::vector<uint32_t> frameOffsets(fragmentHeader.length / sizeof(uint32_t));
				file.read(reinterpret_cast<char *>(frameOffsets.data()), frameOffsets.size() * sizeof(uint32_t));
				startOfFrame = frameOffsets[frameIndex];
				if (frameIndex + 1 < frameOffsets.size())
				{
					assert(frameOffsets[frameIndex] <= frameOffsets[frameIndex + 1]);
					startOfNextFrame = frameOffsets[frameIndex + 1];
					sizeDetermined = true;
				}
				else
				{                
					// Frame index cannot be found. In case there is no table (single frame message) it is ok
					// Otherwise requested index in invalid
					if (frameIndex > 0)
					{
						return result;
					}

					startOfNextFrame = std::numeric_limits<uint64_t>::max();
					sizeDetermined = false;                    

				}
			}
			
			file.seekg(startOfFrame, std::ios_base::cur);
			uint64_t bytesRead = startOfFrame;
			while (bytesRead < startOfNextFrame)
			{
				fragmentHeader.read(file);
				bytesRead += sizeof(fragmentHeader);
				if (!fragmentHeader.isValid() || file.eof())
				{
					assert(!sizeDetermined);
					return result;
				}
										   
				result.emplace_back(fragmentHeader.length); // Create new buffer for the fragment
				auto& buffer = result[result.size() - 1];
				file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
				bytesRead += buffer.size();
				auto readCount = file.gcount();
				if ((readCount <= 0) ||  (static_cast<uint64_t>(readCount) != buffer.size()))
				{
					assert(false);
					return result;
				}                
			}

			return result;
		}
	}

	std::string InputFile::getFileName() const
	{
		return m_filepath;
	}

	bool InputFile::getBlobSize(const PrivateBlock * pBlock, unsigned long tagOrElm, unsigned long& dataSize) const 
	{
		// Use for any raw (uncompressed) blob attribute (so not for pixel-data)
		if (tagOrElm != MC_ATT_PIXEL_DATA)
		{
			return MessageBase::getBlobSize(pBlock, tagOrElm, dataSize);
		}

		assert(m_pixelDataSize != 0xFFFFFFFF);
		assert(m_uncompressedPixelData);
		if (m_pixelDataOffset != 0xFFFFFFFF && m_uncompressedPixelData)
		{
			dataSize = m_pixelDataSize;
			return true;
		}
		else
		{
			dataSize = m_pixelDataSize; // Do set the special return value
			return false;
		}
	}

	bool InputFile::getBlob(const PrivateBlock * pBlock, unsigned long tagOrElm, void* buffer, unsigned long bufferSize) const
	{
		// Use for any raw (uncompressed) blob attribute (so not for pixel-data)
		if (tagOrElm != MC_ATT_PIXEL_DATA)
		{
			return MessageBase::getBlob(pBlock, tagOrElm, buffer, bufferSize);
		}

		assert(m_pixelDataSize != 0xFFFFFFFF);
		assert(m_uncompressedPixelData);
		if (m_pixelDataOffset != 0xFFFFFFFF && m_uncompressedPixelData)
		{
			auto copySize = std::min<unsigned long>(bufferSize, m_pixelDataSize);

			std::ifstream file(m_filepath, std::ios_base::binary);
			file.seekg(m_pixelDataOffset);
			file.read(reinterpret_cast<char *>(buffer), copySize);
			auto readCount = file.gcount();
			if ((readCount <= 0) || (static_cast<uint64_t>(readCount) != copySize))
			{
				assert(false);
				return false;
			}
			return true;
		}
		return false;
	} 

	TRANSFER_SYNTAX InputFile::getTransferSyntax() const
	{
		return m_transferSyntax;
	}    

	bool InputFile::hasUncompressedPixelData() const
	{
		return m_uncompressedPixelData;
	}

	namespace 
	{
		struct CompressedDataWriter
		{
			CompressedDataWriter(const std::vector<std::vector<uint8_t>>& data) :
				m_data(data),
				m_index(0)
			{
			}

			static MC_STATUS writeData(int /*CBMsgFileItemID*/, unsigned long /*CBtag*/, int /*CBisFirst*/, 
				void* CBuserInfo, 
				int* CBdataSizePtr,
				void** CBdataBufferPtr, 
				int* CBisLastPtr)
			{
				auto* writer = reinterpret_cast<CompressedDataWriter*>(CBuserInfo);
				auto& m_data = writer->m_data;
				auto& m_index = writer->m_index;
				*CBdataSizePtr = static_cast<int>(m_data[m_index].size());
				*CBdataBufferPtr = const_cast<uint8_t*>(m_data[m_index].data());
				m_index++;
				if (m_index == m_data.size())
				{
					*CBisLastPtr = 1;
				}
				return MC_NORMAL_COMPLETION;
			}

			const std::vector<std::vector<uint8_t>>& m_data;
			size_t m_index;

			CompressedDataWriter(const CompressedDataWriter&) = delete;
			CompressedDataWriter& operator= (const CompressedDataWriter&) = delete;
		};
	}

	Message FileBase::toMessage(const Service & service, MC_COMMAND command)
	{
		mcCheck(MC_File_To_Message(m_id));
		mcCheck(MC_Set_Service_Command(m_id, service.mergeServiceName().c_str(), command));
		std::vector<uint8_t> pixels;

		// Try loading and setting regular non-embadded bulk
		bool result = get(MC_ATT_PIXEL_DATA, pixels);
		if (result)
		{
			addAttribute(MC_ATT_PIXEL_DATA);
			set(MC_ATT_PIXEL_DATA, pixels);
		}
		else
		{
			// Try encapsulated bulk instead  
			result = true;
			bool firstImage = true;
			bool imageWritten = false;
			int index = 0;
			while(index < std::numeric_limits<int>::max())
			{
				auto pixeldata = getPixelData(index, 0); // Bytes per image does not need to be filled in, in case the data is encapsulated
				if (pixeldata.size() > 0 && pixeldata[0].size() > 0)
				{
					CompressedDataWriter writer(pixeldata);
					if (firstImage)
					{
						addAttribute(MC_ATT_PIXEL_DATA);
						result &= MC_Set_Encapsulated_Value_From_Function(m_id, MC_ATT_PIXEL_DATA, &writer, &CompressedDataWriter::writeData) == MC_NORMAL_COMPLETION;
						firstImage = false;
						imageWritten = true;
					}
					else
					{
						result &= MC_Set_Next_Encapsulated_Value_From_Function(m_id, MC_ATT_PIXEL_DATA, &writer, &CompressedDataWriter::writeData) == MC_NORMAL_COMPLETION;
					}
				}
				else
				{
					break;
				}
				index++;
			}

			if (imageWritten)
				result &= MC_Close_Encapsulated_Value(m_id, MC_ATT_PIXEL_DATA) == MC_NORMAL_COMPLETION;
		}

		Message m(m_id);
		m_id = 0;
		m_handle.detach();
		return m;
	}

}}}
