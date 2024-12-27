// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
// **********************************************************************


#include "StbFrangi.h"
#include "StbContrastDetect.h"
#include "Math.h"
#include "windows.h"


namespace Pci {	namespace StentBoostAlgo {


#define PI 3.1415926535f

StbFrangi::StbFrangi():
	kernelXX(),
	kernelXY(),
	kernelYY(),
	tmp(),
	flDxx(),
	flDxy(),
	flDyy()
{
}

StbFrangi::~StbFrangi()
{
}

static const float SigmaToKernalSize = 3.0f;

bool StbFrangi::init(int width, int height, float scale)
{
	if (!isValidImageSize(height, width, scale)) return false;

	tmp.resize(width, height);

	flDxx.resize(width, height);
	flDxy.resize(width, height);
	flDyy.resize(width, height);

	float gamma = 1.0f;
	float sigma = scale;

	//calculate the required size of the convolution kernels based on the given sigma
	int kernelSide = static_cast<int>(SigmaToKernalSize * sigma);
	int kernelLength = (2 * kernelSide) + 1;

	//the convolution tables have a width of 2; the first column contains the values
	//for filtering in X, the second column contains values for filtering in Y
	kernelXX.resize(kernelLength, 2);
	kernelXY.resize(kernelLength, 2);
	kernelYY.resize(kernelLength, 2);

	float sigma2 = sigma*sigma;

	float S = 1.0f / (2.0f * sigma2);
	float fractionXX = S / (PI * sigma2) *  powf(sigma2, gamma);
	float fractionXY = fractionXX / sigma2;

	//initialize the convolution kernels. For an exact explanation, please refer to Marc Schrijver...
	for (int i = -kernelSide; i <= kernelSide; ++i)
	{
		int x = i + kernelSide;
		float fraction = expf(-S*(i*i));
		float iiDevSigma = (static_cast<float>(i*i) / sigma2) - 1.0f;
		kernelYY.pointer[x + 0] = fraction * fractionXX * iiDevSigma;
		kernelXY.pointer[x + 0] = fraction * fractionXY * static_cast<float>(i);
		kernelXX.pointer[x + 0] = fraction * fractionXX;

		kernelXX.pointer[x + kernelLength] = fraction * iiDevSigma;
		kernelXY.pointer[x + kernelLength] = fraction * static_cast<float>(i);
		kernelYY.pointer[x + kernelLength] = fraction;
	}

	return true;
}

void StbFrangi::destroy()
{
	tmp.destroy();

	flDxx.destroy();
	flDxy.destroy();
	flDyy.destroy();

	kernelXX.destroy();
	kernelXY.destroy();
	kernelYY.destroy();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	Main function to call frangi processing </summary>
///
/// <remarks>	PvdHouten, 2010-08-20. </remarks>
///
/// <param name="input">	The input. </param>
/// <param name="output">	[in,out] The output. </param>
/// <param name="beta_Rb">	The beta rb. </param>
/// <param name="beta_S">	The beta s. </param>
////////////////////////////////////////////////////////////////////////////////////////////////////

static const float BetaSRangeCorrectionFactor = 0.0001220703125; // 8.0 / ( 256 *256)


void StbFrangi::filter(const StbFrangiImage<float> &input, std::shared_ptr<StbFrangiImage<float>> &output, float beta_Rb, float beta_S)
{
	// Make the parameters beta_Rb and beta_S ready to be used
	beta_Rb = 2.0f * beta_Rb * beta_Rb;

	//Marc Schrijvers original algorithm expected a data range of [0..256]. Because we operate on the range
	//[0..1], modifiy beta_S so that parameter compatibility is preserved

	beta_S = BetaSRangeCorrectionFactor * beta_S  * beta_S;

	//first, calculate the second-order derratives
	fill(input);

	//then, based on the derratives, give an estimation whether a pixel is likely to be a vessel or not
	for (int y = 0; y < input.height; y++)
	{
		int offsetY = y * input.width;
		for (int x = 0; x < input.width; x++)
		{
			int imageOffset = x + offsetY;
			float apc = flDxx.pointer[imageOffset] + flDyy.pointer[imageOffset];	//a - plus  - c

																					//this is a direct implementation of the frangi-algorithm. 
																					//For a more detailed explanation, please refer to on-line documentation on the Frangi Filter
			if (apc >= 0.0f)
			{
				flDxy.pointer[imageOffset] *= 2.0f;
				float amc = flDxx.pointer[imageOffset] - flDyy.pointer[imageOffset];	//a - minus - c
				float Det = sqrtf((amc*amc) + (flDxy.pointer[imageOffset] * flDxy.pointer[imageOffset]));
				float lambda1 = apc + Det;
				float lambda2 = apc - Det;

				float S2 = expf(((lambda1*lambda1) + (lambda2*lambda2)) / -beta_S);

				float Rb2 = 0;
				if (lambda1 > 0) Rb2 = lambda2 / lambda1;

				Rb2 *= Rb2;

				// Note that, since apc(r,c) > 0.0, we know that abs(lambda1) > abs(lambda2), AND that lambda1 > 0.0
				output->pointer[imageOffset] = std::fmax(output->pointer[imageOffset], expf(-Rb2 / beta_Rb) * (1.0f - S2));
			}
		}
	}
}

void StbFrangi::fill(const StbFrangiImage<float> &input)
{

	filter(input, tmp, kernelXX, 0);	//perform filtering in x, and store in the temporary buffer
	filter(tmp, flDxx, kernelXX, 1);	//then perform filtering in y, and store in the destination
	filter(input, tmp, kernelXY, 0);
	filter(tmp, flDxy, kernelXY, 1);
	filter(input, tmp, kernelYY, 0);
	filter(tmp, flDyy, kernelYY, 1);
}

void StbFrangi::filter(const StbFrangiImage<float> &src, StbFrangiImage<float> &dst, const StbFrangiImage<float> &kernel, int dir)
{
	//perform convolution on the source image and write to dst.
	int kernelSide = kernel.width / 2;
	int kernalOffset = dir * kernel.width;
	std::memset(dst.pointer, 0, static_cast<unsigned int>(dst.height*dst.stride) * sizeof(float));
	
	for (int i = -kernelSide; i <= kernelSide; ++i)
	{
		float kernelValue = kernel.pointer[i + kernelSide + kernalOffset];
		int dirI = i * dir;

		for (int y = 0; y < src.height; y++)
		{
			int py = y + dirI;
			if (py >= 0 && py < src.height)
			{
				int yOffset = py * src.stride;
				int yDestOffset = y * src.stride;

				for (int x = 0; x < src.width; x++)
				{
					int px = x + i - dirI;
					if (px >= 0 && px < src.width)
					{
						int imageOffset = x + yDestOffset;
						//if the coordinate is valid, add it to the convolution calculation
						dst.pointer[imageOffset] += src.pointer[px + yOffset] * kernelValue;
						if (dst.pointer[imageOffset] > LONG_MAX)
						{
							dst.pointer[imageOffset] = 0.0;
						}
					}
				}
			}
		}
	}
}

bool StbFrangi::isValidImageSize(int height, int width, float scale)
{
	return height > 0 && width > 0 && scale < static_cast<float>(width) && scale >= 1.0f;
}

}}