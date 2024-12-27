
#undef LTEFI_DEBUG_INFO
#undef LTEFI_DISPLAY_INT
#ifndef NDEBUG
	//#define LTEFI_DEBUG_INFO
	//#define LTEFI_DISPLAY_INT
#endif

#ifdef LTEFI_DISPLAY_INT
	#define MX_USE_DLL_AS_STATIC 1
	#include "MxLab.h"
#endif


#ifndef _PI
	#define _PI (4.f * atan(1.f))
#endif

namespace Lt{

	template<typename TTree, typename TRingSampler>
	LtExtendTrackOnRingFromFeatureIm<TTree,TRingSampler>::LtExtendTrackOnRingFromFeatureIm(){
		m_RingSampler = 0;

		m_IrregLocScaleA = _PI/2.f ; 
		m_IrregLocScaleB = _PI*2/3.f;

		m_LocMaxExtBuf = 0;
		m_LocalCurvatureBuffer = 0;
		m_SamplesBuffer = 0;
	}

	template<typename TTree, typename TRingSampler>
	LtExtendTrackOnRingFromFeatureIm<TTree,TRingSampler>::~LtExtendTrackOnRingFromFeatureIm(){
		delete []m_LocMaxExtBuf;
		delete []m_LocalCurvatureBuffer;
		delete []m_SamplesBuffer;
		if(m_RingSampler != 0){
			delete m_RingSampler;
			m_RingSampler = 0;
		}
	}

	template<typename TTree, typename TRingSampler>
	void LtExtendTrackOnRingFromFeatureIm<TTree,TRingSampler>::CreateRingSampler(typename TRingSampler::ImagePixelType* featim, int featiminc, int featimsize[2], float radius, int nbsamples){
		if(m_RingSampler != 0) delete m_RingSampler;
		m_RingSampler = new TRingSampler(featim,featiminc,featimsize,radius,nbsamples);

		this->Init();
	}

	#ifdef VIRTUAL_VINCENT
		template<typename TTree, typename TRingSampler>
		void LtExtendTrackOnRingFromFeatureIm<TTree,TRingSampler>::CreateRingSampler(typename TRingSampler::ImagePixelType* featim, int featiminc, int featimsize[2], float radius, int nbsamples, float angleLimitForVirtual){
			if(m_RingSampler != 0) delete m_RingSampler;
			m_RingSampler = new TRingSampler(featim,featiminc,featimsize,radius,nbsamples, angleLimitForVirtual);

			this->Init();
		}
	#endif

	template<typename TTree, typename TRingSampler>
	void LtExtendTrackOnRingFromFeatureIm<TTree,TRingSampler>::SetRingSampler(TRingSampler* rs){
		if(m_RingSampler != 0) delete m_RingSampler;
		m_RingSampler = rs;

		this->Init();
	}

	template<typename TTree, typename TRingSampler>
	int LtExtendTrackOnRingFromFeatureIm<TTree,TRingSampler>::Init(){
		if(m_LocMaxExtBuf) delete []m_LocMaxExtBuf;
		if(m_LocalCurvatureBuffer) delete []m_LocalCurvatureBuffer;
		if(m_SamplesBuffer) delete []m_SamplesBuffer;

		int nbsamples = m_RingSampler->m_NbSamples;
		m_LocMaxExtBuf = new bool[2*nbsamples];
		m_LocalCurvatureBuffer = new float[nbsamples];
		m_SamplesBuffer = new float[nbsamples];

		if (m_IrregLocScaleA != m_IrregLocScaleB)
			for(int i=0 ; i<nbsamples ; ++i){
				m_LocalCurvatureBuffer[i] = MIN(1.f,MAX(0.f,(fabs(i*2.f*_PI/nbsamples-_PI) - m_IrregLocScaleB)/(m_IrregLocScaleA-m_IrregLocScaleB)));
			}
		else
			for(int i=0 ; i<nbsamples ; ++i){
				if (fabs(i*2.f*_PI/nbsamples-_PI) < m_IrregLocScaleA)
					m_LocalCurvatureBuffer[i] = 0;
				else
					m_LocalCurvatureBuffer[i] = 1;
			}

		return 0;
	}


	template<typename TTree, typename TRingSampler>
	void LtExtendTrackOnRingFromFeatureIm<TTree,TRingSampler>::ExtendTrack(int trackid){
		std::vector<short> locmaxinds;
	
		TreeType::TrackType* track = m_Tree->Track(trackid);
		TreeType::NodeType* endpoint = m_Tree->Node(track->m_EndPoint);
		// extract samples on a ring centered on pos. If sample i is outside the image, aresamplesin[i] is set to false
		m_RingSampler->Eval<true>(endpoint->m_Pos);
		
		int nbsamples = m_RingSampler->m_NbSamples;

		// weight the samples according the local direction and rotate samples so that the begin of the buffer may not be a local minima
		int offset = (int)floor((_PI - endpoint->m_Dir)/(2.f*_PI)*nbsamples+0.5);
		for(int i=MAX(0,offset) ; i<MIN(nbsamples,nbsamples+offset) ; ++i)	m_SamplesBuffer[i] = m_LocalCurvatureBuffer[i] * m_RingSampler->m_Samples[i-offset];
		for(int i=0 ; i<MAX(0,offset); ++i)												m_SamplesBuffer[i] = m_LocalCurvatureBuffer[i] * m_RingSampler->m_Samples[nbsamples + i-offset];
		for(int i=MIN(nbsamples,nbsamples+offset) ; i<nbsamples ; ++i)m_SamplesBuffer[i] = m_LocalCurvatureBuffer[i] * m_RingSampler->m_Samples[i-offset - nbsamples];

		locmaxinds.clear();
		ExtractLocMaxFast(m_SamplesBuffer,nbsamples,locmaxinds);
		for(unsigned int i=0 ; i<locmaxinds.size() ; ++i){
			locmaxinds[i] -= offset;
			if(locmaxinds[i]<0) locmaxinds[i] += nbsamples;
			if(locmaxinds[i]>=nbsamples) locmaxinds[i] -= nbsamples;
		}

		#ifdef LTEFI_DISPLAY_INT
			MxCommand("figure(19); title('extend ring profile'); ");

			MxPutVectorFloat(m_SamplesBuffer,nbsamples,"ringprof");
			MxCommand("plot(ringprof,'color','blue');");
		
		#endif

		// loop over local extrema
		#ifdef VIRTUAL_VINCENT
			if (endpoint->m_Depth >= 3)
				endpoint->m_possiblyExtendVirtual = 1;
			else
				endpoint->m_possiblyExtendVirtual = 0;
		#endif
		for(unsigned int i=0 ; i<locmaxinds.size() ; ++i){
			unsigned short ind = locmaxinds[i];
			//create new line
			int newendpointind;
			if(m_Tree->AddNode(newendpointind)){endpoint = m_Tree->Node(track->m_EndPoint);}
			m_Tree->LinkNode(newendpointind, track->m_EndPoint);

			TreeType::NodeType* newendpoint = m_Tree->Node(newendpointind);
			newendpoint->m_Pos[0] = endpoint->m_Pos[0] + m_RingSampler->m_Dx[ind];
			newendpoint->m_Pos[1] = endpoint->m_Pos[1] + m_RingSampler->m_Dy[ind];
			newendpoint->m_Dir = m_RingSampler->m_Angle[ind];
			newendpoint->m_Length = m_RingSampler->m_Radius;
			newendpoint->m_Depth = endpoint->m_Depth + 1;

			#ifdef VIRTUAL_VINCENT
				if (endpoint->m_Depth >= 3){
					#ifdef FILTER_ANGLE_VINCENT
						float dTheta = newendpoint->m_Dir - endpoint->m_FilteredDir;
					#else
						float dTheta = m_RingSampler->m_Angle[ind];
					#endif

					if(dTheta<0)	dTheta = MIN( fabs(dTheta) , fabs(dTheta+2*_PI) ); 
					else		dTheta = MIN( fabs(dTheta) , fabs(dTheta-2*_PI) ); 

					if (dTheta < m_RingSampler->m_AngleLimitForVirtual)
						endpoint->m_possiblyExtendVirtual = 0;

					#ifdef VA_DEBUG
						printf("Entension %.2f %.2f %.2f %d\n", endpoint->m_FilteredDir, newendpoint->m_Dir
								, m_RingSampler->m_AngleLimitForVirtual, endpoint->m_possiblyExtendVirtual);
					#endif
				}
			#endif
			
			int newtrackind;
			if(m_Tree->AddTrack(newtrackind)){track = m_Tree->Track(trackid);}
			TreeType::TrackType* newtrack = m_Tree->Track(newtrackind);
			*newtrack = *track;
			newtrack->m_EndPoint = newendpointind;
		}
	}

	template<typename TTree, typename TRingSampler>
	void LtExtendTrackOnRingFromFeatureIm<TTree,TRingSampler>::ExtendTrackWOInitialDir(int trackind){
		TreeType::TrackType* track = m_Tree->Track(trackind);
		TreeType::NodeType* endpoint = m_Tree->Node(track->m_EndPoint);

		m_RingSampler->Eval<true>(endpoint->m_Pos);

		int nbsamples = m_RingSampler->m_NbSamples;

		std::vector<short> locmaxinds;
		ExtractLocMax(m_RingSampler->m_Samples,nbsamples,locmaxinds);

		for(unsigned int i=0 ; i<locmaxinds.size() ; ++i){
			unsigned short ind = locmaxinds[i];
			//create new track
			int newtrackind;
			if(m_Tree->AddTrack(newtrackind)){track = m_Tree->Track(trackind);}
			int newendpointind;
			if(m_Tree->AddNode(newendpointind)){endpoint = m_Tree->Node(track->m_EndPoint);}
			int newsourcepointind;
			if(m_Tree->AddNode(newsourcepointind)){endpoint = m_Tree->Node(track->m_EndPoint);}
			m_Tree->LinkNode(newendpointind, newsourcepointind);

			TreeType::NodeType* newsourcepoint = m_Tree->Node(newsourcepointind);
			*newsourcepoint = *endpoint;
			newsourcepoint->m_Dir = m_RingSampler->m_Angle[ind];
			#ifdef FILTER_ANGLE_VINCENT
				newsourcepoint->m_FilteredDir = m_RingSampler->m_Angle[ind];
			#endif

			TreeType::NodeType* newendpoint = m_Tree->Node(newendpointind);
			newendpoint->m_Pos[0] = endpoint->m_Pos[0] + m_RingSampler->m_Dx[ind];
			newendpoint->m_Pos[1] = endpoint->m_Pos[1] + m_RingSampler->m_Dy[ind];
			newendpoint->m_Dir = m_RingSampler->m_Angle[ind];
			#ifdef FILTER_ANGLE_VINCENT
				newendpoint->m_FilteredDir = m_RingSampler->m_Angle[ind];
			#endif
			newendpoint->m_Length = m_RingSampler->m_Radius;
			newendpoint->m_Depth = 1;

			TreeType::TrackType* newtrack = m_Tree->Track(newtrackind);
			*newtrack = *track;
			newtrack->m_EndPoint = newendpointind;
		}
	}

	template<typename TTree, typename TRingSampler>
	int LtExtendTrackOnRingFromFeatureIm<TTree,TRingSampler>::ExtractLocMax(float* samples,int nb,std::vector<short> &locmaxinds){
		bool* maybe = m_LocMaxExtBuf;
		bool* maybe2 = m_LocMaxExtBuf+nb;

		maybe[0] = false;
		for(int i=1 ; i<nb; ++i)
			if(samples[i-1]<samples[i]) maybe[i] = true;
			else if(samples[i-1]==samples[i] && maybe[i-1]) maybe[i] = true;
				 else maybe[i]=false;
		
		if(samples[nb-1]<samples[0]) maybe[0] = true;
		else if(samples[nb-1]==samples[0] && maybe[nb-1]) maybe[0] = true;
			 else maybe[0]=false;

		for(int i=1 ; i<nb ; ++i)
			if(samples[i-1]<samples[i]) maybe[i] = true;
			else if(samples[i-1]==samples[i] && maybe[i-1]) maybe[i] = true;
				 else maybe[i]=false;

		maybe2[nb-1] = false;
		for(int i=nb-2 ; i>=0 ; --i){
			if(samples[i+1]<samples[i]) maybe2[i] = true;
			else if(samples[i+1]==samples[i] && maybe2[i+1]) maybe2[i] = true;
				 else maybe2[i] = false;
		}
	
		if(samples[0]<samples[nb-1]) maybe2[nb-1] = true;
		else if(samples[0]==samples[nb-1] && maybe2[0]) maybe2[nb-1] = true;
			 else maybe2[nb-1] = false;
		
		for(int i=nb-2 ; i>=0 ; --i){
			if(samples[i+1]<samples[i]) maybe2[i] = true;
			else if(samples[i+1]==samples[i] && maybe2[i+1]) maybe2[i] = true;
				 else maybe2[i] = false;
		}

		for(int i=0 ; i<nb-1 ; ++i){
			if(maybe[i]&&maybe2[i]) locmaxinds.push_back(i);
		}
		return 0;
	}

	template<typename TTree, typename TRingSampler>
	int LtExtendTrackOnRingFromFeatureIm<TTree,TRingSampler>::ExtractLocMaxFast(float* samples,int nb,std::vector<short> &locmaxinds){
		bool* maybe = m_LocMaxExtBuf;

		maybe[0] = false;
		for(int i=1 ; i<nb; ++i)
			if(samples[i-1]<samples[i]) maybe[i] = true;
			else if(samples[i-1]==samples[i] && maybe[i-1]) maybe[i] = true;
				 else maybe[i]=false;
		
		maybe[nb-1] = false;
		for(int i=nb-2 ; i>=0 ; --i){
			if(maybe[i]){
				if(samples[i+1]<samples[i]) locmaxinds.push_back(i);
				else if(samples[i+1]==samples[i] && maybe[i+1]) locmaxinds.push_back(i);
					 else maybe[i] = false;
			}
		}

		return 0;
	}
}
