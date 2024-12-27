#pragma once

#include "LtStopTracks.h"

#include "LtSamplerProfile.h"

#undef DEBUG_INFO
#undef DISPLAY_INT
#undef DISPLAY_INT2
#ifndef NDEBUG
	#define DEBUG_INFO
	//#define DISPLAY_INT
#endif

#ifdef DISPLAY_INT
	#define MX_USE_DLL_AS_STATIC 1
	#include "MxLab.h"
#endif


namespace Lt{

	/** \class LtStopTracksWhenProfileChange
	*	\brief Stop a track when the profile on endpoint differs from the mean profile
	*
	*	To use that class, the track type of the tree must inherit from LtTreeTrackAttribswtProfile.
	*
	*	TODO accumulate along current dir
	*	TODO change similarity measurement
	*/
	template<typename TTree, class PT=float> 
	class LtStopTracksOnProfileChange : public LtStopTracks<TTree>{
	public:
		LtStopTracksOnProfileChange(PT* im, int iminc, int imsize[2]){
			int halfnbsamples = TreeType::TrackType::ProfileSize /2;
			m_Sampler = new LtSamplerProfile<PT>(im, iminc, imsize, halfnbsamples);
		}

		virtual int StopTracks(std::vector<int>& trackinds){
			std::vector<int> tracksinds = m_Tree->GetRunningTracks();
			for(int i=0 ; i<tracksinds.size() ; ++i){
				int ind = tracksinds[i];
				TreeType::TrackType* track = m_Tree->Track(ind);
				TreeType::NodeType* endpoint = m_Tree->Node(track->m_EndPoint);
				float px = endpoint->m_Pos[0];
				float py = endpoint->m_Pos[1];

				// Compute profile
				m_Sampler->EvalNearest<true>(endpoint->m_Pos,endpoint->m_Dir + _PI/2.f);

				// Decide if we stop the track
				bool end = false;

				// Compute correl
				if(track->m_ProfileInitialized){
					float correl = ComputeCorrel(m_Sampler->m_Samples, track->m_MeanProfile, m_Sampler->m_NbSamples);
					//std::cout<<correl<<std::endl;
					if(correl < 0.5 && track->m_Length > 30) 
						end = true;
				}

				#ifdef DISPLAY_INT
					MxCommand("figure(20); title('profiles'); ");

					MxPutVectorFloat(m_Sampler->m_Samples,m_Sampler->m_NbSamples,"curprofile");
					MxPutVectorFloat(track->m_MeanProfile,m_Sampler->m_NbSamples,"meanprofile");

					MxCommand("plot(curprofile,'color','blue');hold on;");
					MxCommand("plot(meanprofile,'color','red');");
					MxCommand("hold off");
					
				#endif

				if(end) trackinds.push_back(ind);

				// Update mean profile
				if(track->m_ProfileInitialized){
					for(float *pti = m_Sampler->m_Samples, *pto=track->m_MeanProfile ; pti != m_Sampler->m_Samples+m_Sampler->m_NbSamples ; ++pti , ++pto){
						*pto = *pto * 0.8 + *pti * 0.2;
					}
				}else{
					std::copy(m_Sampler->m_Samples, m_Sampler->m_Samples+m_Sampler->m_NbSamples , track->m_MeanProfile);
					track->m_ProfileInitialized = true;
				}

			}
			return 0;
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


		LtSamplerProfile<PT>* m_Sampler;
	};
}


