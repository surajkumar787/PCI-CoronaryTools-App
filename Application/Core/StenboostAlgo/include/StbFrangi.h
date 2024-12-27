// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "StbFrangiImage.h"

//implements the frangi-filter algorithm for StbImages. The frangi filter is a vesselness filter
//for pixels that are likely to belong to a contrast-filled vessel, the filter will produce a white 
//pixel. Other pixels will become black. 
//This filter is used to detect contrast in X-ray sequences. If a filtered image contains a lot of white
//pixels then the image is likely to contain contrast. 

//basically, the algorithm first calculates the second-order derratives of an input image in XX, XY and YY direction
//(horizontal, diagonal and vertical). Then, based on these derratives, it estimates whether a pixel is likely 
//to be a vessel or not. 

namespace Pci {	namespace StentBoostAlgo {

class StbFrangi
{
public:
	StbFrangi();
	~StbFrangi();

	//initializes the filter for images of the given width and height. Scale is the sigma 
	//and is an indication for the size of the expected vessels
	bool init(int width, int height, float scale);

	bool isValidImageSize(int height, int width, float scale);

	void destroy();

	//writes the frangi-filtered input image to output. beta_Rb and beta_S are parameters
	//to control the frangi filter algorithm. See frangi-filter documentation for more info on them
	void filter(const StbFrangiImage<float> &input, std::shared_ptr<StbFrangiImage<float>> &output, float beta_Rb, float beta_S);

private:
	
	//this function calculates the second-order derratives of the input image and stores them in flDxx, flDxy, flDyy
	void fill  (const StbFrangiImage<float> &input);
	//performs convolution on the indicated image. If dir=0, convolution is in X, if dir=1, convolution is in Y
	void filter(const StbFrangiImage<float> &src, StbFrangiImage<float> &dst, const StbFrangiImage<float> &kernel, int dir);

	//the convolution kernels that generate the second-order derrative values
	StbFrangiImage<float> kernelXX;
	StbFrangiImage<float> kernelXY;
	StbFrangiImage<float> kernelYY;

	//a temporary buffer used in the convolution processing
	StbFrangiImage<float> tmp;
	
	//the images that will hold the second order derratives in xx, xy and yy directions
	StbFrangiImage<float> flDxx;
	StbFrangiImage<float> flDxy;
	StbFrangiImage<float> flDyy;

};
}}