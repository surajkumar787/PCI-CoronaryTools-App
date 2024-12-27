// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "jpegencoder.h"

#include <jpegenc.h>
#include <jpegdec.h>
#include <membuffout.h>
#include <membuffin.h>
#include <decqtbl.h>
#include <assert.h>



namespace CommonPlatform { namespace Dicom
{

	static const int CompressionQuality = 75; // can be 1..100
	
	std::vector<char> encodeJpeg(const char* pixels,
	                             int     width,
	                             int     height,
								 int     bitsAllocated,
								 int     highBit,
								 int     samplesPerPixel,
	                             bool    lossless)
	{
		std::vector<char> result;
		CMemBuffInput  inputBuffer;
		inputBuffer.Open(reinterpret_cast<const uint8_t*>(pixels),width * height * samplesPerPixel);
		CMemBuffOutput outputBuffer;
		int bytesPerPixel = (bitsAllocated + 7) / 8;

		IppiSize imageSize;
		imageSize.height = height;
		imageSize.width = width;
		JCOLOR color = JC_UNKNOWN;
		switch (samplesPerPixel)
		{
		case 3:
			color = JC_RGB;
			break;
		case 4:
			color = JC_RGBA;
			break;
		default:
			assert(false);
			break;
		}


		auto sampling = JSS::JS_444;
		CJPEGEncoder encoder;
		if (bitsAllocated == 8)
		{
			auto bitsStored = highBit + 1;	
			int step =  width * bytesPerPixel * samplesPerPixel;
			encoder.SetSource(const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(pixels)), step, imageSize, samplesPerPixel, color, sampling, bitsStored);
		}
		else
		{
			return {};
		}
		// Set the source with size of the input buffer so that the worse case it can handle
		// Also add a margin for very small images in case the jpeg header is bigger than the actual image. 
		result.resize(width * height * samplesPerPixel * bytesPerPixel + 1024);
		outputBuffer.Open(reinterpret_cast<uint8_t*>(result.data()), static_cast<int>(result.size()));
		encoder.SetDestination(&outputBuffer);
	
		int nRestartInterval = 0; // Restart interval is not significant for 2D data.
		int nHuffTableGenerate = 1; // Generate optimal table

		auto err = JPEG_OK;
		if (lossless)
		{
			int nPointTransform = 0; // tranformation point is needed by huffman coding
			int nPredictor = 1; //  Since we using JPEG Lossless first order prediction.

			err = encoder.SetParams(JPEG_LOSSLESS, JC_RGB, sampling, nRestartInterval, nHuffTableGenerate, nPointTransform, nPredictor);
		}
		else
		{
			err = encoder.SetParams(JPEG_BASELINE, JC_YCBCR, JS_422, nRestartInterval, nHuffTableGenerate, CompressionQuality);
		}

		if (err != JPEG_OK) 
		{
			return {};
		} 
		// Write the header to output buffer.
		err = encoder.WriteHeader();
		if (err != JPEG_OK) 
		{
			return {};
		} 	
		// Write the Data to output buffer.
		err = encoder.WriteData();
		if (err != JPEG_OK) 
		{
			return {};
		}

		auto outSize = encoder.NumOfBytes();
		result.resize(outSize);
		return result;
	}

}}