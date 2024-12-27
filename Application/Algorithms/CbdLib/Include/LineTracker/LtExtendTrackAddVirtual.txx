
namespace Lt{

	template<typename TTree, typename TFeatureFunc>
	LtExtendTrackAddVirtual<TTree, TFeatureFunc>::LtExtendTrackAddVirtual(){
		#ifdef VIRTUAL_VINCENT
			maxNbTracks = 10;
		#endif
	}

	#ifdef VIRTUAL_VINCENT
		template<typename TTree, typename TFeatureFunc>
		LtExtendTrackAddVirtual<TTree, TFeatureFunc>::LtExtendTrackAddVirtual(int maxNbVirtual){
			maxNbTracks = maxNbVirtual;
		}
	#endif

	template<typename TTree, typename TFeatureFunc>
	LtExtendTrackAddVirtual<TTree, TFeatureFunc>::~LtExtendTrackAddVirtual(){

	}

	template<typename TTree, typename TFeatureFunc>
	void LtExtendTrackAddVirtual<TTree, TFeatureFunc>::ExtendTrack(int trackind){
		
		TreeType::TrackType* track = m_Tree->Track(trackind);
		TreeType::NodeType* endpoint = m_Tree->Node(track->m_EndPoint);

		#ifdef VIRTUAL_VINCENT
			if (endpoint->m_ExtendVirtual == 0) return;
		#endif

		float px;
		float py;
		float dir;
		float length;
		float depth;
		if(endpoint->m_Father<0) return;

		TreeType::NodeType* p1 = m_Tree->Node(endpoint->m_Father);
	
		#ifdef FILTER_ANGLE_VINCENT
			//R = sqrt((endpoint->m_Pos[0]-p1->m_Pos[0]) * (endpoint->m_Pos[0]-p1->m_Pos[0])
			//			+ (endpoint->m_Pos[1]-p1->m_Pos[1]) * (endpoint->m_Pos[1]-p1->m_Pos[1]) );
			px = endpoint->m_Pos[0] + endpoint->m_Length * cos(p1->m_FilteredDir);
			py = endpoint->m_Pos[1] + endpoint->m_Length * sin(-p1->m_FilteredDir);
		#else
			px = endpoint->m_Pos[0] + endpoint->m_Pos[0] - p1->m_Pos[0];
			py = endpoint->m_Pos[1] + endpoint->m_Pos[1] - p1->m_Pos[1];
		#endif

		#ifdef VA_DEBUG
			printf("Virtual (%.1f %.1f) -> (%.1f %.1f) \n", endpoint->m_Pos[0], endpoint->m_Pos[1]
															, px, py);
		#endif


		dir = endpoint->m_Dir;
		length = endpoint->m_Length;
		depth = endpoint->m_Depth;

		// create new track
		int newtrackind;
		if(m_Tree->AddTrack(newtrackind)){track = m_Tree->Track(trackind);}
		int newendpointind;
		if(m_Tree->AddNode(newendpointind)){endpoint = m_Tree->Node(track->m_EndPoint);}
		m_Tree->LinkNode(newendpointind, track->m_EndPoint);

		TreeType::NodeType* newendpoint = m_Tree->Node(newendpointind);
		newendpoint->m_Pos[0] = px;
		newendpoint->m_Pos[1] = py;
		newendpoint->m_Dir = dir;
		newendpoint->m_Length = length;
		newendpoint->m_Depth = depth;

		TreeType::TrackType* newtrack = m_Tree->Track(newtrackind);
		*newtrack = *track;
		newtrack->m_EndPoint = newendpointind;	
	}
/*
	template<typename TTree, typename TFeatureFunc>
	void LtExtendTrackAddVirtual<TTree, TFeatureFunc>::ExtendTrack(int * trackind, int nbTracks){

		int nbVirtualExtended = 0;
		float alreadySelectedX[m_maxVirtual], alreadySelectedY[m_maxVirtual];
		int virtualAddedAtPreviousIteration = 1;

		while ((nbVirtualExtended < m_maxVirtual) && (virtualAddedAtPreviousIteration == 1)){

			float maxScoreGoodSon = -FLT_MAX;
			int iSelected = -1;
			TreeType::NodeType* selectedPoint;
			TreeType::NodeType* selectedEndpoint;

			virtualAddedAtPreviousIteration = 0;

			for (int i=0; i<nbTracks; i++){
				TreeType::TrackType* track = m_Tree->Track(trackind[i]);

				// Best score
				if (track->m_Score < maxScoreGoodSon) break;

				TreeType::NodeType* endpoint = m_Tree->Node(track->m_EndPoint);

				if(endpoint->m_Father<0) break;
				TreeType::NodeType* p1 = m_Tree->Node(endpoint->m_Father);

				// Good son - does the father have sons in the proper angular arch?
				int GoodSon = 0;
				for (int j=0; j<nbTracks; j++){

					TreeType::TrackType* track2 = m_Tree->Track(trackind[j]);
					TreeType::NodeType* endpoint2 = m_Tree->Node(track2->m_EndPoint);

					if(endpoint2->m_Father<0){
						TreeType::NodeType* p2 = m_Tree->Node(endpoint2->m_Father);

						if ((p2->m_Pos[0] - p1->m_Pos[0]) * (p2->m_Pos[0] - p1->m_Pos[0])
							+ (p2->m_Pos[1] - p1->m_Pos[1]) * (p2->m_Pos[1] - p1->m_Pos[1]) < 0.5){	
								
								// Same father
								float dTheta = endpoint2->m_Dir - p1->m_Dir;
								if(dTheta<0)	dTheta = MIN( fabs(dTheta) , fabs(dTheta+2*_PI) ); 
								else		dTheta = MIN( fabs(dTheta) , fabs(dTheta-2*_PI) ); 

								if (dTheta < m_AngleLimitForVirtual)
									GoodSon = 1;

						}
					}
				}

				if (GoodSon == 1) break;

				// Check that it was not already extended
				bool alreadyExtended = false;
				for (int j=0; j<nbVirtualExtended; j++)
					if ( (p1->m_Pos[0] - alreadySelectedX[j]) * (p1->m_Pos[0] - alreadySelectedX[j])
						+ (p1->m_Pos[1] - alreadySelectedY[j]) * (p1->m_Pos[1] - alreadySelectedY[j]) < 1)
						alreadyExtended = true;
				if (alreadyExtended) break;

				// Best candidate so far
				maxScoreGoodSon = track->m_Score;
				selectedPoint = m_Tree->Node(p1);
				selectedEndpoint = m_Tree->Node(endpoint);
				iSelected = i;
			}

			if (iSelected > 0){
				float px;
				float py;
				float dir;
				float length;

				alreadySelectedX = selectedEndpoint->m_Pos[0];
				alreadySelectedY = selectedEndpoint->m_Pos[1];

				// estimate new pos
				px = selectedEndpoint->m_Pos[0] + selectedEndpoint->m_Pos[0] - selectedPoint->m_Pos[0];
				py = selectedEndpoint->m_Pos[1] + selectedEndpoint->m_Pos[1] - selectedPoint->m_Pos[1];
				dir = selectedEndpoint->m_Dir;
				length = selectedEndpoint->m_Length;

				// create new track
				int newtrackind;
				if(m_Tree->AddTrack(newtrackind)){track = m_Tree->Track(iSelected);}
				int newendpointind;
				if(m_Tree->AddNode(newendpointind)){endpoint = m_Tree->Node(track->m_EndPoint);}
				m_Tree->LinkNode(newendpointind, track->m_EndPoint);

				TreeType::NodeType* newendpoint = m_Tree->Node(newendpointind);
				newendpoint->m_Pos[0] = px;
				newendpoint->m_Pos[1] = py;
				newendpoint->m_Dir = dir;
				newendpoint->m_Length = length;

				TreeType::TrackType* newtrack = m_Tree->Track(newtrackind);
				*newtrack = *track;
				newtrack->m_EndPoint = newendpointind;

				nbVirtualExtended++;
				virtualAddedAtPreviousIteration = 1;
			}
		}
	}*/


	template<typename TTree, typename TFeatureFunc>
	void LtExtendTrackAddVirtual<TTree, TFeatureFunc>::ExtendTrackWOInitialDir(int trackind){
		return;
	}

}
