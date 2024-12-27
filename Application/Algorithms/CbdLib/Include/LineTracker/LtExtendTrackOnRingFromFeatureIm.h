#pragma once

#include "LtExtendTrack.h"

namespace Lt{

	/**
	* \class LtExtendTrackOnRingFromFeatureIm
	* \brief Extend tracks from a feature image.
	*
	* That function relies on a feature image as a rigde map. 
	* It extracts samples on a circle around the track endpoint. 
	* Then local extrema are extracted and a track is generated for each one. 
	*/
	template<typename TTree, typename TRingSampler>
	class LtExtendTrackOnRingFromFeatureIm : public LtExtendTrack<TTree>{
	public:
		LtExtendTrackOnRingFromFeatureIm();
		~LtExtendTrackOnRingFromFeatureIm();

		virtual void ExtendTrack(int trackind);
		virtual void ExtendTrackWOInitialDir(int trackind);

		void CreateRingSampler(typename TRingSampler::ImagePixelType* featim, int featiminc, int featimsize[2], float radius, int nbsamples);
		#ifdef VIRTUAL_VINCENT
			void CreateRingSampler(typename TRingSampler::ImagePixelType* featim, int featiminc
									, int featimsize[2], float radius, int nbsamples, float angleLimitForVirtual);
		#endif
		void SetRingSampler(TRingSampler*);

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


		int Init();


	///////////////////////////////////////////////////////////////////////////
	// Parameters
	///////////////////////////////////////////////////////////////////////////
	public:	
		float m_IrregLocScaleA,m_IrregLocScaleB;

	///////////////////////////////////////////////////////////////////////////
	// Inputs
	///////////////////////////////////////////////////////////////////////////
	protected:
		// Use to get samples of featim on a circle centered on a given point
		TRingSampler* m_RingSampler;

	///////////////////////////////////////////////////////////////////////////
	// Internal buffers 
	///////////////////////////////////////////////////////////////////////////
		// Internal buffer used for local maxima extraction
		bool* m_LocMaxExtBuf;
		// Internal buffer that contains curvature penalty
		float* m_LocalCurvatureBuffer;
		// Internal buffer to hold the combination of sampling and curvature penalty
		float* m_SamplesBuffer;
	};

}


#include "LtExtendTrackOnRingFromFeatureIm.txx"
