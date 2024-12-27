#include "LtTracker.h"

#include <algorithm>
#include <sstream>
#include "float.h"

#undef MIN
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#undef MAX
#define MAX(X,Y) ((X) < (Y) ? (Y) : (X))

#undef DEBUG_INFO
#undef DISPLAY_INT

#ifndef NDEBUG
	#define DEBUG_INFO
	//#define DISPLAY_INT
#endif

#ifdef DISPLAY_INT
	#define MX_USE_DLL_AS_STATIC 1
	#include "MxLab.h"
#endif


namespace Lt{

	template<typename TTree>
	LtTracker<TTree>::LtTracker(){
		#ifdef DISPLAY_INT
			MxOpen();
			MxSetVisible(1);
			MxCommand("NumFig=1;");
		#endif
		m_Tree = new TreeType((int) 1E5 ,(int) 1E3);

		m_MaxNbIter = 50;
		
		m_NbIterations = 0;

	}

	template<typename TTree>
	LtTracker<TTree>::~LtTracker(){
		for(unsigned int i=0 ; i< m_ExtendFuncs.size() ; ++i){	m_ExtendFuncs[i]->Reset(); delete m_ExtendFuncs[i];}
		for(unsigned int i=0 ; i< m_FeatureFuncs.size() ; ++i){	m_FeatureFuncs[i]->Reset(); delete m_FeatureFuncs[i];}
		for(unsigned int i=0 ; i< m_PruningFuncs.size() ; ++i){	m_PruningFuncs[i]->Reset(); delete m_PruningFuncs[i];}
		//for(unsigned int i=0 ; i< m_StopFuncs.size() ; ++i){	m_StopFuncs[i]->Reset(); delete m_StopFuncs[i];}
		delete m_Tree;
	}

	template<typename TTree>
	int LtTracker<TTree>::Reset(){
		m_Tree->Reset();
		for(unsigned int i=0 ; i< m_ExtendFuncs.size() ; ++i)	m_ExtendFuncs[i]->Reset();
		for(unsigned int i=0 ; i< m_FeatureFuncs.size() ; ++i)	m_FeatureFuncs[i]->Reset();
		for(unsigned int i=0 ; i< m_PruningFuncs.size() ; ++i)	m_PruningFuncs[i]->Reset();
		for(unsigned int i=0 ; i< m_StopFuncs.size() ; ++i)		m_StopFuncs[i]->Reset();

		return 0;
	}

	template<typename TTree>
	int LtTracker<TTree>::RemoveStopFuncs(){
		while (!m_StopFuncs.empty())
			m_StopFuncs.pop_back();

		return 0;
	}

	template<typename TTree>
	int LtTracker<TTree>::AddPruningFunc(LtPruningTracks<TreeType>* pf){
		m_PruningFuncs.push_back(pf);
		pf->SetTree(m_Tree);
		return 0;
	}

	template<typename TTree>
	int LtTracker<TTree>::AddStopFunc(LtStopTracks<TreeType>* sf){
		m_StopFuncs.push_back(sf);
		sf->SetTree(m_Tree);
		return 0;
	}

	template<typename TTree>
	int LtTracker<TTree>::AddExtensionFunc(LtExtendTrack<TreeType>* ef){
		m_ExtendFuncs.push_back(ef);
		ef->SetTree(m_Tree);
		return 0;
	}

	template<typename TTree>
	int LtTracker<TTree>::AddFeatureFunc(LtComputeFeature<TreeType>* ff){
		m_FeatureFuncs.push_back(ff);
		ff->SetTree(m_Tree);
		return 0;
	}

	template<typename TTree>
	int LtTracker<TTree>::AddFilterCompletedFunc(LtFilterCompletedTrack<TreeType>* ff){
		m_FilterCompletedFuncs.push_back(ff);
		ff->SetTree(m_Tree);
		return 0;
	}

	template<typename TTree>
	int LtTracker<TTree>::AddFilterTreeFunc(LtFilterTree<TreeType>* ftf){
		m_FilterTreeFuncs.push_back(ftf);
		ftf->SetTree(m_Tree);
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////
// Inputs 
/////////////////////////////////////////////////////////////////////////////////

	template<typename TTree>
	int	LtTracker<TTree>::AddSeed(float xSeed, float ySeed, float dir){
		int newtrackind;
		m_Tree->AddTrack(newtrackind);

		int newnodeind;
		m_Tree->AddNode(newnodeind);
		m_Tree->LinkNode(newnodeind, -1);

		TreeType::NodeType* newnode = m_Tree->Node(newnodeind);
		newnode->m_Dir = dir;
		newnode->m_Length = 0;
		newnode->m_Pos[0] = xSeed;
		newnode->m_Pos[1] = ySeed;
		newnode->m_Depth = 0;
		newnode->m_FilteredDir = dir;

		TreeType::TrackType* newtrack= m_Tree->Track(newtrackind);
		newtrack->Reset();
		newtrack->m_EndPoint = newnodeind;

		return 0;
	}

	template<typename TTree>
	int	LtTracker<TTree>::AddSeed(float xSeed, float ySeed){
		int newnodeind;
		m_Tree->AddNode(newnodeind);
		m_Tree->LinkNode(newnodeind, -1);

		TreeType::NodeType* newnode = m_Tree->Node(newnodeind);
		newnode->m_Pos[0] = xSeed;
		newnode->m_Pos[1] = ySeed;
		newnode->m_Length = 0;
		newnode->m_Depth = 0;

		int newtrackind;
		m_Tree->AddTrack(newtrackind);

		TreeType::TrackType* newtrack= m_Tree->Track(newtrackind);
		newtrack->Reset();
		newtrack->m_EndPoint = newnodeind;
		
		// As we don't know the direction of that track we extract the local maxima 
		// on a ring and create a track for each one
		for(unsigned int i=0 ; i< m_ExtendFuncs.size() ; ++i)
			m_ExtendFuncs[i]->ExtendTrackWOInitialDir(newtrackind);

		m_Tree->RemoveTrack(newtrackind);

		return 0;
	}


/////////////////////////////////////////////////////////////////////////////////
// Outputs
/////////////////////////////////////////////////////////////////////////////////

	template<typename TTree>
	int LtTracker<TTree>::GetBestTrack(LtTrack* besttrack){
		int ind;
		if(GetBestTrackInd(ind)<0) return -1;
		m_Tree->GetTrack(ind,besttrack);
		return 0;
	}

	template<typename TTree>
	int LtTracker<TTree>::GetBestTrackAndScores(LtTrack* besttrack, float *score, float *length){
		int ind;
		if(GetBestTrackInd(ind)<0) return -1;
		m_Tree->GetTrack(ind,besttrack);

		*score				= (float)m_Tree->Track(ind)->m_Score;
		*length				= (float)m_Tree->Track(ind)->m_Length;

		return 0;
	}

	template<typename TTree>
	int LtTracker<TTree>::GetBestTrackV(LtTrack* besttrack){
		int ind;
		if(GetBestTrackInd(ind)<0) return -1;
		m_Tree->GetTrack(ind,besttrack);
		return 0;
	}

	template<typename TTree>
	int LtTracker<TTree>::GetBestTrackInd(int &ind){
		std::vector<int> tracksinds = m_Tree->GetTracks();
		if(tracksinds.size()==0) return -1;

		double maxscore = -FLT_MAX;
		for(unsigned int i=0 ; i<tracksinds.size() ; ++i){
	/*
			{
				LtTrack * currentTrack = new Lt::LtTrack();
				m_Tree->GetTrack(tracksinds[i],currentTrack);

				printf("Score %d %d : %.2f (%d %d)\n", i, tracksinds[i], m_Tree->Track(tracksinds[i])->m_Score
					, (int)currentTrack->m_Pts[0].X
					, (int)currentTrack->m_Pts[0].Y);
			}
			*/
			if(m_Tree->Track(tracksinds[i])->m_Score > maxscore){
				maxscore = m_Tree->Track(tracksinds[i])->m_Score;
				ind = tracksinds[i];
			}
		}

		return 0;
	}

	template<typename TTree>
	std::vector<LtTrack*> LtTracker<TTree>::GetTracks(){
		std::vector<LtTrack*> result;

		std::vector<int> tracksinds = m_Tree->GetTracks();

		for(unsigned int i=0 ; i<tracksinds.size() ; ++i){
			LtTrack* t = new LtTrack();
			m_Tree->GetTrack(tracksinds[i],t);
			result.push_back(t);
		}
		return result;
	}

	template<typename TTree>
	int	LtTracker<TTree>::ClearTracks(){

		m_Tree->Reset();
		//delete m_Tree;
		//m_Tree = new TreeType((int) 1E5 ,(int) 1E3);

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////
// Main 
/////////////////////////////////////////////////////////////////////////////////
	template<typename TTree>
	int LtTracker<TTree>::Track(){
//#ifdef DISPLAY_INT
//	MxCommand("aviobj = avifile('D:\\temp\\tracking.avi','compression','None');");
//#endif

		//MxDisplayTracks(20);
		for(m_NbIterations  = 0; m_NbIterations  < m_MaxNbIter; m_NbIterations++){
			int endFlag = Iter();
			//TxtDisplayCurrentState();
			//MxDisplayTracks(20);
		//#ifdef DISPLAY_INT
		//	MxCommand("F = getframe(fig);");
		//	MxCommand("aviobj = addframe(aviobj,F);");
		//#endif
			if(endFlag)	break;
		}

//#ifdef DISPLAY_INT
//	MxCommand("aviobj = close(aviobj);");
//#endif
		return 0;
	} 

	template<typename TTree>
	int LtTracker<TTree>::Iter(){

		#ifdef VA_DEBUG
			printf("\n********************************\n");
		#endif

	////////////////////////////////////////////////////////////////
	// Extend tracks
	////////////////////////////////////////////////////////////////
		std::vector<int> tracksinds = m_Tree->GetRunningTracks();
		if(tracksinds.size()==0)	return 1;

		for(unsigned int i=0 ; i< m_ExtendFuncs.size() ; ++i){
			// Different extensions (practice: image-base extensions, then potentially virtual)
			for(std::vector<int>::iterator ittracks=tracksinds.begin() ; ittracks != tracksinds.end(); ++ittracks){
				#ifdef VA_DEBUG
					TreeType::TrackType* track = m_Tree->Track(*ittracks);
					TreeType::NodeType* endpoint = m_Tree->Node(track->m_EndPoint);
					printf("*** Extend track %d : (%.1f %.1f) (%.2f %.2f)\n", *ittracks
											, endpoint->m_Pos[0], endpoint->m_Pos[1] 
											, endpoint->m_Dir, endpoint->m_FilteredDir);
				#endif
				m_ExtendFuncs[i]->ExtendTrack(*ittracks);
			}

			#ifdef VIRTUAL_VINCENT
				// Select which tracks are to be extended by the next ExtendFuncs (if practice, virtual)
				if (i<m_ExtendFuncs.size()-1)
					TracksToExtendIdentification(tracksinds, m_ExtendFuncs[i+1]->maxNbTracks);
			#endif
		}

		for(std::vector<int>::iterator ittracks=tracksinds.begin() ; ittracks != tracksinds.end(); ++ittracks)
			// Remove previous tree
			m_Tree->RemoveTrack(*ittracks);

	////////////////////////////////////////////////////////////////
	// Compute features on tracks
	////////////////////////////////////////////////////////////////
		for(std::set<int>::iterator ittracks=m_Tree->m_RunningTracks.begin() ; ittracks != m_Tree->m_RunningTracks.end(); ++ittracks){
			#ifdef FILTER_ANGLE_VINCENT
				ComputeFilteredAngle(*ittracks);
			#endif
			// Irregularity term computation
			for(unsigned int i=0 ; i< m_FeatureFuncs.size() ; ++i)	m_FeatureFuncs[i]->Compute(*ittracks);
			// Global score computation
			typename TreeType::TrackType* t = m_Tree->Track(*ittracks);
			t->ComputeScore();
			#ifdef VA_DEBUG
				TreeType::TrackType* track = m_Tree->Track(*ittracks);
				TreeType::NodeType* endpoint = m_Tree->Node(track->m_EndPoint);
				printf("*** Scores: track %d (%.1f %.1f) score %.2f\n", *ittracks, endpoint->m_Pos[0]
						, endpoint->m_Pos[1], track->m_Score);
			#endif
			t->CopyInfoToEndPoint(m_Tree->Node(t->m_EndPoint));
		}

	////////////////////////////////////////////////////////////////
	// Apply filtering functions that operate on whole tree
	////////////////////////////////////////////////////////////////
		for(unsigned int i=0 ; i< m_FilterTreeFuncs.size() ; ++i)	
			m_FilterTreeFuncs[i]->Filter();

	////////////////////////////////////////////////////////////////
	// Prune the tracks to extract best ones
	////////////////////////////////////////////////////////////////
		for(unsigned int i=0 ; i< m_PruningFuncs.size() ; ++i)
			m_PruningFuncs[i]->Prune();

	////////////////////////////////////////////////////////////////
	// Complete some tracks (for instance if it has reached the image border
	////////////////////////////////////////////////////////////////
		std::vector<int> trackstostop;
		for(unsigned int i=0 ; i< m_StopFuncs.size() ; ++i)
			m_StopFuncs[i]->StopTracks(trackstostop);

		for(std::vector<int>::iterator it=trackstostop.begin() ; it != trackstostop.end() ; ++it){
			m_Tree->SendTrackToSleep(*it);
			for(unsigned int i=0 ; i< m_FilterCompletedFuncs.size() ; ++i)
				m_FilterCompletedFuncs[i]->Filter(*it);
		}

		return 0;

	}

	#ifdef VIRTUAL_VINCENT
		// *** Labels Points to virtually extend
		template<typename TTree>
		void LtTracker<TTree>::TracksToExtendIdentification(std::vector<int> tracksinds, int maxNbVirtual){

			if(tracksinds.size()==0)	return;

			// Initialized to "not to extend" - and compute filtered angle
			for(std::vector<int>::iterator ittracks=tracksinds.begin() ; ittracks != tracksinds.end(); ++ittracks){
				TreeType::TrackType* track = m_Tree->Track(*ittracks);
				TreeType::NodeType* endpoint = m_Tree->Node(track->m_EndPoint);

				// Label "not to extend"
				endpoint->m_ExtendVirtual = 0;
			}

			bool foundExtension = true;
			float maxScore = -FLT_MAX;
			int nbExtendedTracks = 0, indiceMaxTrack;

			// Loop to find the best (ie, with higher score) extendable track
			while ((foundExtension == true)&&(nbExtendedTracks < maxNbVirtual)){

				foundExtension = false;

				// Find extendable track with highest score
				for(std::vector<int>::iterator ittracks=tracksinds.begin() ; ittracks != tracksinds.end(); ++ittracks){

					TreeType::TrackType* track = m_Tree->Track(*ittracks);
					TreeType::NodeType* endpoint = m_Tree->Node(track->m_EndPoint);

					if ( (endpoint->m_possiblyExtendVirtual == 1) && (track->m_Score > maxScore)){
							maxScore = (float)track->m_Score;
							indiceMaxTrack = *ittracks;
							foundExtension = true;
						}
				}

				// One to extend - change label and loop
				if (foundExtension == true){
					TreeType::TrackType* track = m_Tree->Track(indiceMaxTrack);
					TreeType::NodeType* endpoint = m_Tree->Node(track->m_EndPoint);
					endpoint->m_ExtendVirtual = 1;
					nbExtendedTracks++;

					#ifdef VA_DEBUG
						printf("Virtual choice %d %.1f %.1f\n", nbExtendedTracks, endpoint->m_Pos[0], endpoint->m_Pos[1]);
					#endif
				}
			}
		}
	#endif

	#ifdef FILTER_ANGLE_VINCENT
		// *** Labels Points to virtually extend
		template<typename TTree>
		void LtTracker<TTree>::ComputeFilteredAngle(int trackid){
		

				int m_Scale = 5;
				float *VectForMedian = new float [(int)m_Scale+1];
				float *StackForMedian = new float[(int)m_Scale+1];

				// Compute filtered angle
				TreeType::TrackType* track = m_Tree->Track(trackid);
				TreeType::NodeType* endpoint = m_Tree->Node(track->m_EndPoint);

				// Compute filtered angle
				TreeType::NodeType* p = endpoint; int pindprev = track->m_EndPoint;
				int sizeMedian = (int)p->m_Depth;
				if (sizeMedian > m_Scale)
					sizeMedian = m_Scale;

				for (int i=0; i<sizeMedian; i++){
					pindprev = p->m_Father; p = m_Tree->Node(pindprev);
					VectForMedian[i] = p->m_Dir;
				}

				//for (int i=0; i<sizeMedian; i++)
				//	printf("%.2f ", VectForMedian[i]);

				endpoint->m_FilteredDir = median(VectForMedian, StackForMedian, 0
													, (int)(sizeMedian-1)/2, sizeMedian-1);

				//printf("\nMedian %.2f\n", endpoint->m_FilteredDir);

				delete[]VectForMedian; delete[]StackForMedian;
		}
	#endif
/*
	float LtMedian(float * stack1, float * stack2, int x0, int pos, int xEnd)
	{

		int xMinus, xPlus, i;
		float pivot;

		xMinus = x0;
		xPlus = xEnd;
		pivot = stack1[x0];

		for (i=x0+1; i<=xEnd;i++)
			if (stack1[i]<pivot){
				stack2[xMinus] = stack1[i];
				xMinus ++;
			} else if (stack1[i]>pivot) {
				stack2[xPlus] = stack1[i];
				xPlus --;
			}
		
		xMinus--; xPlus++;

		if ( (xMinus < pos) && (xPlus > pos) )
			return pivot;
		else if (xMinus >= pos)
			return LtMedian(stack2,stack1,x0,pos,xMinus);
		else 
			return LtMedian(stack2,stack1,xPlus,pos,xEnd);
	}
	*/


/////////////////////////////////////////////////////////////////////////////////
// Display and debug functions
/////////////////////////////////////////////////////////////////////////////////
	template<typename TTree>
	void LtTracker<TTree>::MxDisPlaySetBackgroundImage(unsigned short* im, int iminc, int imsize[2]){
	#ifdef DISPLAY_INT
		float* newim = new float[imsize[0]*imsize[1]];
		IppiSize roi = {imsize[0],imsize[1]};
		ippiConvert_16u32f_C1R(im,iminc*sizeof(unsigned short), newim, imsize[0]*sizeof(float), roi);

		MxPutMatrixFloat(newim,imsize[0],imsize[1],"trackdispbg");
		delete []newim;
	#endif
	}

	template<typename TTree>
	void LtTracker<TTree>::MxDisPlaySetBackgroundImage(float* im, int iminc, int imsize[2]){
	#ifdef DISPLAY_INT
		float* newim = new float[imsize[0]*imsize[1]];
		float* ptim = newim;
		for(int i=0; i<imsize[1] ; ++i){
			float* pt = im + i*iminc;
			for(int j=0 ; j<imsize[0] ; j++){
				*(ptim++) = *(pt++);
			}
		}

		MxPutMatrixFloat(newim,imsize[0],imsize[1],"trackdispbg");
		delete []newim;
	#endif
	}


	template<typename TTree>
	void LtTracker<TTree>::MxDisplayTrack(int numfig, int trackind){
	#ifdef DISPLAY_INT
		std::ostringstream ss;
		ss<<"fig = figure("<<numfig<<");";
		ss<<"imagesc(trackdispbg);colormap gray; title('Tracking track "<<trackind<<"'); axis image;hold on;";
		MxCommand((char*)ss.str().c_str());

		LtTrackV* track = new LtTrackV();
		m_Tree->GetTrack(trackind,track);

		for(unsigned int i=0 ; i<track->m_X.size() ; ++i){
			track->m_X[i]++;
			track->m_Y[i]++;
		}
		MxPutVectorFloat(&track->m_X[0],track->m_X.size(),"trackx");
		MxPutVectorFloat(&track->m_Y[0],track->m_Y.size(),"tracky");

		MxCommand("plot(trackx,tracky);");
		MxCommand("hold off");
		
		delete track;
	#endif
	}

	template<typename TTree>
	void LtTracker<TTree>::MxDisplayTracks(int numfig){
	#ifdef DISPLAY_INT
		std::ostringstream ss;
		ss<<"fig = figure("<<numfig<<");";
		ss<<"imagesc(trackdispbg);colormap gray; title('Tracking'); axis image;hold on;";
		MxCommand((char*)ss.str().c_str());

		std::vector<int> tracksinds = m_Tree->GetTracks();

		double maxscore = 0;
		for(unsigned int i=0 ; i<tracksinds.size() ; ++i){
			double score = m_Tree->Track(tracksinds[i])->m_Score;
			if(score>maxscore) maxscore = score;
		}
		maxscore+=0.001;

		LtTrackV* track = new LtTrackV();
		for(unsigned int i=0 ; i<tracksinds.size() ; ++i){
			m_Tree->GetTrack(tracksinds[i],track);
	
			for(unsigned int j=0 ; j<track->m_X.size() ; ++j){
				track->m_X[j]++;
				track->m_Y[j]++;
			}

			MxPutVectorFloat(&track->m_X[0],track->m_X.size(),"trackx");
			MxPutVectorFloat(&track->m_Y[0],track->m_Y.size(),"tracky");
		
			float cr,cg,cb;
			double H = (1.f-m_Tree->Track(tracksinds[i])->m_Score/maxscore) * 3.f;
			float X = (float)(1-fabs(H-((int)H/2)*2 - 1));
			if(H<1){
				cr=1 ; cg=X; cb=0;
			}else if(H<2){
				cr=X ; cg=1; cb=0;
			}else if(H<3){
				cr=0 ; cg=1; cb=X;
			}else{
				cr=0 ; cg=X; cb=1;
			}

			std::ostringstream trss;
			trss<<"plot(trackx,tracky,'color',["<<cr<<","<<cg<<","<<cb<<"],'linewidth',2);";
			MxCommand((char*)trss.str().c_str());

		}
		MxCommand("hold off");
		delete track;
	#endif
	}

	template<typename TTree>
	void LtTracker<TTree>::TxtDisplayCurrentState(){
//	#ifdef DEBUG_INFO
		std::cout<<std::endl;
		std::cout<<"Tracker iteration: "<<m_NbIterations<<std::endl;
		std::cout<<"Number of tracks: "<<m_Tree->GetNumberOfTracks()<<std::endl;
		std::cout<<"Number of nodes in tree: "<<m_Tree->m_AllocatedNodes.size()-m_Tree->m_FreeNodes.size()<<std::endl;
		std::cout<<std::endl;
//	#endif
	}
}

