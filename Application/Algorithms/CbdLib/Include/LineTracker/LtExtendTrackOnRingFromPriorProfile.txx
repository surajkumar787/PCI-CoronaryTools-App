
#undef LTEPF_DEBUG_INFO
#undef LTEPF_DISPLAY_INT
#ifndef NDEBUG
	//#define LTEPF_DEBUG_INFO
	//#define LTEPF_DISPLAY_INT
#endif

#ifdef LTEPF_DISPLAY_INT
	#define MX_USE_DLL_AS_STATIC 1
	#include "MxLab.h"
#endif


namespace Lt{

	template<typename TTree, typename T>
	LtExtendTrackOnRingFromPriorProfile<TTree,T>::LtExtendTrackOnRingFromPriorProfile(){
		m_RingSampler = 0;

		m_Im = 0;
		m_ImInc = 0;
		m_ImSize[0] = 0;
		m_ImSize[1] = 0;

		m_PriorProfile = 0;
		m_PriorProfileSize = 0;
		m_PriorProfileSp = 1.f;

		m_RingRadius = 10;
		m_RingNbSamples = 30; 
		
		m_IrregLocScaleA = _PI/5.f ; 
		m_IrregLocScaleB = _PI/3.f;

		m_LocMaxExtBuf = new bool[2*m_RingNbSamples];
		m_LocalCurvatureBuffer = new float[m_RingNbSamples];
		m_SamplesBuffer = new float[m_RingNbSamples];
	}

	template<typename TTree, typename T>
	LtExtendTrackOnRingFromPriorProfile<TTree,T>::~LtExtendTrackOnRingFromPriorProfile(){
		delete []m_LocMaxExtBuf;
		delete []m_LocalCurvatureBuffer;
		delete []m_SamplesBuffer;
	}

	template<typename TTree, typename T>
	int LtExtendTrackOnRingFromPriorProfile<TTree,T>::SetInputIm(T* im, int iminc, int imsize[2]){
		m_Im = im;
		m_ImInc = iminc;
		m_ImSize[0] = imsize[0];
		m_ImSize[1] = imsize[1];

		return 0;
	}

	template<typename TTree, typename T>
	int LtExtendTrackOnRingFromPriorProfile<TTree,T>::SetPriorProfile(float* profilebuf, int profilesize, float profilesp){
		m_PriorProfile = profilebuf;
		m_PriorProfileSize = profilesize;
		m_PriorProfileSp = profilesp;

		return 0;
	}

	template<typename TTree, typename T>
	int LtExtendTrackOnRingFromPriorProfile<TTree,T>::Init(){
		m_RingNbSamples = 2*_PI*m_RingRadius / m_PriorProfileSp; 

		if(m_LocMaxExtBuf) delete []m_LocMaxExtBuf;
		if(m_LocalCurvatureBuffer) delete []m_LocalCurvatureBuffer;
		if(m_SamplesBuffer) delete []m_SamplesBuffer;

		m_LocMaxExtBuf = new bool[2*m_RingNbSamples];
		m_LocalCurvatureBuffer = new float[m_RingNbSamples];
		m_SamplesBuffer = new float[m_RingNbSamples];

		if(m_RingSampler != 0) delete m_RingSampler;
		m_RingSampler = new LtSamplerRingLinear<float>(m_Im,m_ImInc,m_ImSize,m_RingRadius,m_RingNbSamples);

		for(int i=0 ; i<m_RingNbSamples ; ++i){
			m_LocalCurvatureBuffer[i] = MIN(1.f,MAX(0.f,(fabs(i*2.f*_PI/m_RingNbSamples-_PI) - m_IrregLocScaleB)/(m_IrregLocScaleA-m_IrregLocScaleB)));
		}
		return 0;
	}


	template<typename TTree, typename T>
	void LtExtendTrackOnRingFromPriorProfile<TTree,T>::ExtendTrack(int trackid){
		std::vector<short> locmaxinds;
	
		TreeType::TrackType* track = m_Tree->Track(trackid);
		TreeType::NodeType* endpoint = m_Tree->Node(track->m_EndPoint);
		// extract samples on a ring centered on pos. If sample i is outside the image, aresamplesin[i] is set to false
		//m_RingSampler->EvalNearest<true>(endpoint->m_Pos);
		m_RingSampler->EvalInterp<true>(endpoint->m_Pos);
		
		// Compute cyclic correlation of prior profile within extracted samples
		int profilehs = m_PriorProfileSize/2;
		int extsamplessize = m_RingNbSamples + 2*profilehs;
		float* extsamples = new float[extsamplessize];

		for(int i=0 ; i<profilehs ; ++i)	extsamples[i] = m_RingSampler->m_Samples[m_RingNbSamples-profilehs+i];
		for(int i=0 ; i<m_RingNbSamples ; ++i) extsamples[i+profilehs] = m_RingSampler->m_Samples[i];
		for(int i=0 ; i<profilehs ; ++i)	extsamples[i+profilehs+m_RingNbSamples] = m_RingSampler->m_Samples[i];

		float* correl = new float[m_RingNbSamples];
//		ippsCrossCorr_32f(m_PriorProfile, m_PriorProfileSize, extsamples, extsamplessize, correl,m_RingNbSamples,0);

		float* cumulatedsamp = new float[extsamplessize+1];
		cumulatedsamp[0] = 0;
		for(int i=1 ; i<extsamplessize+1 ; ++i)	cumulatedsamp[i] = cumulatedsamp[i-1]+extsamples[i-1];

/*		for(int i=0 ; i<m_RingNbSamples ; ++i){
			correl[i] = 0;
			for(int j=0 ; j<m_PriorProfileSize ; ++j){
				correl[i] += extsamples[i+j]*m_PriorProfile[j];
			}
			correl[i] -= (cumulatedsamp[i+m_PriorProfileSize] - cumulatedsamp[i])/m_PriorProfileSize * 0.5;
		}
		delete []cumulatedsamp;
*/
		float alpha = 40;

		float meanprior = 0;
		for(int j=0 ; j<m_PriorProfileSize ; ++j) meanprior += m_PriorProfile[j];
		meanprior/=m_PriorProfileSize;

		float meansqprior = 0;
		for(int j=0 ; j<m_PriorProfileSize ; ++j) meansqprior += m_PriorProfile[j]*m_PriorProfile[j];
		meansqprior /= m_PriorProfileSize;

		for(int i=0 ; i<m_RingNbSamples ; ++i){
			float meansignal = (cumulatedsamp[i+m_PriorProfileSize] - cumulatedsamp[i])/m_PriorProfileSize;

			float m = alpha * meanprior - meansignal;

			/*correl[i] = 0;
			for(int j=0 ; j<m_PriorProfileSize ; ++j){
				correl[i] += extsamples[i+j]*m_PriorProfile[j];
			}
			correl[i] = -(correl[i]/m_PriorProfileSize - meanprior*meansignal)/(meansqprior - meanprior*meanprior);
			*/
			correl[i] = 0;
			float distl = 0;
			float distr = 0;
			for(int j=0 ; j<profilehs ; ++j){
				distl += fabs(alpha*m_PriorProfile[j]-m - extsamples[i+j]);
				distr += fabs(alpha*m_PriorProfile[m_PriorProfileSize-1-j]-m - extsamples[i+m_PriorProfileSize-1-j]);
			}
			correl[i] = - MAX(distl,distr);
		}
		delete []cumulatedsamp;

		float mincorrel = correl[0];
		for(int i=0 ; i<m_RingNbSamples ; ++i) if(correl[i]<mincorrel) mincorrel = correl[i];

		//for(int i=0 ; i<m_RingNbSamples ; ++i) correl[i]-=mincorrel;


		// weight the result according the local direction and rotate samples so that the begin of the buffer may not be a local minima
		int offset = (_PI - endpoint->m_Dir)/(2.f*_PI)*m_RingNbSamples;
		for(int i=MAX(0,offset) ; i<MIN(m_RingNbSamples,m_RingNbSamples+offset) ; ++i)	m_SamplesBuffer[i] = m_LocalCurvatureBuffer[i] * (correl[i-offset]-mincorrel);
		for(int i=0 ; i<MAX(0,offset); ++i)												m_SamplesBuffer[i] = m_LocalCurvatureBuffer[i] * (correl[m_RingNbSamples + i-offset]-mincorrel);
		for(int i=MIN(m_RingNbSamples,m_RingNbSamples+offset) ; i<m_RingNbSamples ; ++i)m_SamplesBuffer[i] = m_LocalCurvatureBuffer[i] * (correl[i-offset - m_RingNbSamples]-mincorrel);

		#ifdef LTEPF_DISPLAY_INT
			MxCommand("figure(17); title('ring profile'); ");
			MxPutVectorFloat(m_RingSampler->m_Samples,m_RingNbSamples,"ringsamples");
			MxCommand("plot(ringsamples,'color','blue');");

			MxCommand("figure(18); title('prior profile'); ");
			MxPutVectorFloat(m_PriorProfile,m_PriorProfileSize,"priorprofile");
			MxCommand("plot(priorprofile,'color','blue');");

			MxCommand("figure(19); title('correl'); ");
			MxPutVectorFloat(correl,m_RingNbSamples,"correl");
			MxCommand("plot(correl,'color','blue');");


			MxCommand("figure(20); title('extend ring profile'); ");
			MxPutVectorFloat(m_SamplesBuffer,m_RingNbSamples,"ringprof");
			MxCommand("plot(ringprof,'color','blue');");
		
		#endif




		locmaxinds.clear();
		ExtractLocMaxFast(m_SamplesBuffer,m_RingNbSamples,locmaxinds);
		for(unsigned int i=0 ; i<locmaxinds.size() ; ++i){
			locmaxinds[i] -= offset;
			if(locmaxinds[i]<0) locmaxinds[i] += m_RingNbSamples;
			if(locmaxinds[i]>=m_RingNbSamples) locmaxinds[i] -= m_RingNbSamples;
		}

		// loop over local extrema
		for(unsigned int i=0 ; i<locmaxinds.size() ; ++i){
			unsigned short ind = locmaxinds[i];
			//create new line
			int newtrackind;
			if(m_Tree->AddTrack(newtrackind)){track = m_Tree->Track(trackid);}
			int newendpointind;
			if(m_Tree->AddNode(newendpointind)){endpoint = m_Tree->Node(track->m_EndPoint);}
			m_Tree->LinkNode(newendpointind, track->m_EndPoint);

			TreeType::NodeType* newendpoint = m_Tree->Node(newendpointind);
			newendpoint->m_Pos[0] = endpoint->m_Pos[0] + m_RingSampler->m_Dx[ind];
			newendpoint->m_Pos[1] = endpoint->m_Pos[1] + m_RingSampler->m_Dy[ind];
			newendpoint->m_Dir = m_RingSampler->m_Angle[ind];
			newendpoint->m_Length = m_RingSampler->m_Radius;

			TreeType::TrackType* newtrack = m_Tree->Track(newtrackind);
			*newtrack = *track;
			newtrack->m_EndPoint = newendpointind;
		}

		delete []correl;
		delete []extsamples;
	}

	template<typename TTree, typename T>
	void LtExtendTrackOnRingFromPriorProfile<TTree,T>::ExtendTrackWOInitialDir(int trackind){
		TreeType::TrackType* track = m_Tree->Track(trackind);
		TreeType::NodeType* endpoint = m_Tree->Node(track->m_EndPoint);

		m_RingSampler->EvalNearest<true>(endpoint->m_Pos);
		std::vector<short> locmaxinds;
		ExtractLocMax(m_RingSampler->m_Samples,m_RingNbSamples,locmaxinds);

		for(unsigned int i=0 ; i<locmaxinds.size() ; ++i){
			unsigned short ind = locmaxinds[i];
			//create new track
			int newtrackind;
			if(m_Tree->AddTrack(newtrackind)){track = m_Tree->Track(trackind);}
			int newendpointind;
			if(m_Tree->AddNode(newendpointind)){endpoint = m_Tree->Node(track->m_EndPoint);}
			m_Tree->LinkNode(newendpointind, track->m_EndPoint);

			TreeType::NodeType* newendpoint = m_Tree->Node(newendpointind);
			newendpoint->m_Pos[0] = endpoint->m_Pos[0] + m_RingSampler->m_Dx[ind];
			newendpoint->m_Pos[1] = endpoint->m_Pos[1] + m_RingSampler->m_Dy[ind];
			newendpoint->m_Dir = m_RingSampler->m_Angle[ind];
			#ifdef FILTER_ANGLE_VINCENT
				newsourcepoint->m_FilteredDir = m_RingSampler->m_Angle[ind];
			#endif
			newendpoint->m_Length = m_RingSampler->m_Radius;

			TreeType::TrackType* newtrack = m_Tree->Track(newtrackind);
			*newtrack = *track;
			newtrack->m_EndPoint = newendpointind;
		}
	}

	template<typename TTree, typename T>
	int LtExtendTrackOnRingFromPriorProfile<TTree,T>::ExtractLocMax(float* samples,int nb,std::vector<short> &locmaxinds){
		bool* maybe = m_LocMaxExtBuf;
		bool* maybe2 = m_LocMaxExtBuf+m_RingNbSamples;

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

	template<typename TTree, typename T>
	int LtExtendTrackOnRingFromPriorProfile<TTree,T>::ExtractLocMaxFast(float* samples,int nb,std::vector<short> &locmaxinds){
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
