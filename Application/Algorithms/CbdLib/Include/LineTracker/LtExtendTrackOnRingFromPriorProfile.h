#pragma once

#include "LtSamplerRingLinear.h"

namespace Lt{

	/**
	* \class LtExtendTrackOnRingFromPriorProfile
	* \brief Extend tracks looking on a ring for a gray-level profile provided as input.
	*
	*/
	template<typename TTree, typename T=float>
	class LtExtendTrackOnRingFromPriorProfile : public LtExtendTrack<TTree>{
	public:
		LtExtendTrackOnRingFromPriorProfile();
		~LtExtendTrackOnRingFromPriorProfile();

		/** Set input image
		*	\param[in] im pointer to the buffer
		*	\param[in] iminc line increment 
		*	\param[in] image size */
		int SetInputIm(T* im, int iminc, int imsize[2]);

		/** Set the prior profile
		*	\param[in] profilebuf buffer of size profilesize
		*	\param[in] profilesize 
		*	\param[in] profilesp spacing of the buffer wrt the image i.e. 0.5 means that 2 values of the profile correspond to 1 pixel in the image */
		int SetPriorProfile(float* profilebuf, int profilesize, float profilesp);

		virtual void ExtendTrack(int trackind);
		virtual void ExtendTrackWOInitialDir(int trackind);

		int Init();

	protected:

		/** Extract local maxima in samples. We assume that samples is a circular buffer.
		*	\param[in] samples pointer to the buffer
		*	\param[in] nb buffer size
		*	\param[out] locmaxinds resulting local maxima */
		int ExtractLocMax(float* samples,int nb,std::vector<short> &locmaxinds);

		/* Extract local maxima in samples. We assume here that no loxal maxima is at the begin or at the end of the buffer 
		*	\param[in] samples pointer to the buffer
		*	\param[in] nb buffer size
		*	\param[out] locmaxinds resulting local maxima */
		int ExtractLocMaxFast(float* samples,int nb,std::vector<short> &locmaxinds);

	///////////////////////////////////////////////////////////////////////////
	// Parameters
	///////////////////////////////////////////////////////////////////////////
		// At each iteration we look for candidates on a circle centered on tracks endpoints.
		// Radius of that circle
		float m_RingRadius;
		// Number of samples on that circle. Tuned dynamically wrt the profile spacing. 
		int m_RingNbSamples; 

		float m_IrregLocScaleA,m_IrregLocScaleB;

	///////////////////////////////////////////////////////////////////////////
	// Inputs
	///////////////////////////////////////////////////////////////////////////
		T* m_Im;
		int m_ImInc;
		int m_ImSize[2];

		float* m_PriorProfile;
		int m_PriorProfileSize;
		float m_PriorProfileSp;

	///////////////////////////////////////////////////////////////////////////
	// Internal buffers 
	///////////////////////////////////////////////////////////////////////////
		// Internal buffer used for local maxima extraction
		bool* m_LocMaxExtBuf;
		// Internal buffer that contains curvature penalty
		float* m_LocalCurvatureBuffer;
		// Internal buffer to hold the combination of sampling and curvature penalty
		float* m_SamplesBuffer;

		// Use to get samples of featim on a circle centered on a given point
		LtSamplerRingLinear<T>* m_RingSampler;
	};


}

#include "LtExtendTrackOnRingFromPriorProfile.txx"

