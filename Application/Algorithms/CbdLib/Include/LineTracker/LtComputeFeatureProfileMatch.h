#pragma once

#include "LtComputeFeature.h"

namespace Lt{

	/**	\class LtComputeFeatureProfileMatch
	*	\brief Compute the constancy of the gray-level profile along the track and derive a penalty 
	*
	*/
	template<typename TTree, typename PT> 
	class LtComputeFeatureProfileMatch : public LtComputeFeature<TTree>{
	public:
		LtComputeFeatureProfileMatch(PT* im, int iminc, int imsize[2]){
			int halfnbsamples = TreeType::TrackType::ProfileSize /2;
			m_Sampler = new LtSamplerProfile<PT>(im, iminc, imsize, halfnbsamples);

			m_CorrelA = 0.6f;
			m_CorrelB = 0.9f;
		}

		virtual void Compute(int trackind){
			TreeType::TrackType* track = m_Tree->Track(trackind);
			TreeType::NodeType* endpoint = m_Tree->Node(track->m_EndPoint);
			float px = endpoint->m_Pos[0];
			float py = endpoint->m_Pos[1];

			// Compute profile
			m_Sampler->EvalNearest<true>(endpoint->m_Pos,endpoint->m_Dir + _PI/2.f);

			float correl = 1;
			// Compute correl
			if(track->m_ProfileInitialized){
				correl = ComputeCorrel(m_Sampler->m_Samples, track->m_MeanProfile, m_Sampler->m_NbSamples);
			}

			#ifdef DISPLAY_INT2
				MxCommand("figure(101); title('profiles'); ");

				MxPutVectorFloat(m_Sampler->m_Samples,m_Sampler->m_NbSamples,"curprofile");
				MxPutVectorFloat(track->m_MeanProfile,m_Sampler->m_NbSamples,"meanprofile");

				MxCommand("plot(curprofile,'color','blue');hold on;");
				if(track->m_ProfileInitialized)	MxCommand("plot(meanprofile,'color','red');");
				MxCommand("hold off");
				
			#endif

			// Update mean profile
			if(track->m_ProfileInitialized){
				for(float *pti = m_Sampler->m_Samples, *pto=track->m_MeanProfile ; pti != m_Sampler->m_Samples+m_Sampler->m_NbSamples ; ++pti , ++pto){
					*pto = *pto * 0.8f + *pti * 0.2f;
				}
			}else{
				std::copy(m_Sampler->m_Samples, m_Sampler->m_Samples+m_Sampler->m_NbSamples , track->m_MeanProfile);
				track->m_ProfileInitialized = true;
			}

			float pen = MAX(0,MIN(1,(m_CorrelA-correl)/(m_CorrelB-m_CorrelA)));
			track->m_CumulatedProfilePen += pen*pen;
		}


		float ComputeCorrel(PT* buf1, PT* buf2, int bufsize){
			double mean_xr = 0, mean_patch = 0;
			double std_xr = 0, std_patch = 0, corr = 0;
			
			for (int i = 0; i < bufsize; ++i){
				double f = *(buf1++);
				double m = *(buf2++);
				mean_xr     += f;
				mean_patch  += m;
				std_xr      += f * f;
				std_patch   += m * m;
				corr        += f * m;
			}

			mean_xr /= bufsize;
			mean_patch /= bufsize;
			std_xr = std::sqrt(std_xr / (bufsize-1) - bufsize / (bufsize-1) *mean_xr*mean_xr);
			std_patch = std::sqrt(std_patch / (bufsize-1) - bufsize / (bufsize-1) * mean_patch * mean_patch);
			corr /= bufsize-1;
			corr -= bufsize / (bufsize-1) * mean_xr * mean_patch;
			corr /= std_xr * std_patch;

			return (float)corr;
		}


		float m_CorrelA;
		float m_CorrelB;

		LtSamplerProfile<PT>* m_Sampler;
	};

}