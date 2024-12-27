
#ifndef NDEBUG
	//#define THD_DEBUG_INFO
	//#define THD_DISPLAY_INT
#endif

namespace Lt{

	template<typename TTree>
	LtPruningTracksHeadDistance<TTree>::LtPruningTracksHeadDistance(){
		m_DistMat = 0;
		m_DistMatInds = 0;

		m_PenDistA = -5;
		m_PenDistB = 20;
	}

	template<typename TTree>
	void LtPruningTracksHeadDistance<TTree>::Reset(){
		if(m_DistMatInds!=0){
			delete m_DistMatInds;
			m_DistMatInds = 0;
		}
		if(m_DistMat!=0){
			delete []m_DistMat[0];
			delete []m_DistMat;
			m_DistMat=0;
		}
	}

	template<typename TTree>
	void LtPruningTracksHeadDistance<TTree>::Prune(){
		int nbmaxtracks = m_MaxTracks;

		std::vector<int> tracksinds = m_Tree->GetTracks();
		//if(nbmaxtracks > tracksinds.size()) return;
		if(tracksinds.size()==0) return;

		std::vector<TIndScore> trackswtscore(tracksinds.size());
		for(unsigned int i=0 ; i<tracksinds.size() ; ++i){
			trackswtscore[i].ind = tracksinds[i];
			trackswtscore[i].score = m_Tree->Track(tracksinds[i])->m_Score;
			trackswtscore[i].remove = true;
			trackswtscore[i].indnode = m_Tree->Track(tracksinds[i])->m_EndPoint;
			trackswtscore[i].indprevnode = m_Tree->Node(trackswtscore[i].indnode)->m_Father;
			trackswtscore[i].pos[0] = m_Tree->Node(trackswtscore[i].indnode)->m_Pos[0];
			trackswtscore[i].pos[1] = m_Tree->Node(trackswtscore[i].indnode)->m_Pos[1];
		}

		bool notfirst = true;
		if(m_DistMatInds==0)	notfirst=false;

		int nbtracks = 0;
		
		float penA = m_PenDistA;
		float penB = m_PenDistB;
		

		while(true){
			//select best track
			std::vector<TIndScore>::iterator curtrack = std::min_element(trackswtscore.begin() , trackswtscore.end() ,indScoreOrder);
			//compute distance and update scores
			curtrack->remove = false;
			curtrack->score = -FLT_MAX;
			float px = curtrack->pos[0];
			float py = curtrack->pos[1];
			int prevind;
			if(notfirst) prevind = (*m_DistMatInds)[curtrack->indprevnode];

			for(unsigned int i=0 ; i<tracksinds.size() ; ++i){
				int oind;
				if(notfirst) oind = (*m_DistMatInds)[trackswtscore[i].indprevnode];
			
				float prevdist;
				if(notfirst && prevind>=0 && oind>=0)	prevdist = m_DistMat[prevind][oind];
				else						prevdist = 0;

				float dist = ((trackswtscore[i].pos[0]-px)*(trackswtscore[i].pos[0]-px) + (trackswtscore[i].pos[1]-py)*(trackswtscore[i].pos[1]-py));
				float penalty = MAX(0,MIN(1,(MAX(dist,prevdist) - penA)/(penB-penA)));

				trackswtscore[i].score -= fabs(trackswtscore[i].score)*(1-penalty);
			}

			nbtracks++;
			double curscore = curtrack->score;
		//	if(curscore <= 0) break;
			if(nbtracks>=nbmaxtracks) break;

		}

		for(std::vector<TIndScore>::iterator ittracks = trackswtscore.begin() ; ittracks != trackswtscore.end() ; ++ittracks){
			if(ittracks->remove){
				m_Tree->RemoveNode(m_Tree->Track(ittracks->ind)->m_EndPoint);
				m_Tree->RemoveTrack(ittracks->ind);
			}
		}
		
		//build maxdist matrix
		float** newdistmat;
		std::map<int,int>* newdistmatinds = new std::map<int,int>();
		int count=0;
		for(std::vector<TIndScore>::iterator ittracks = trackswtscore.begin() ; ittracks != trackswtscore.end() ; ++ittracks){
			if(!ittracks->remove){
				newdistmatinds->insert(std::make_pair<int,int>(m_Tree->Track(ittracks->ind)->m_EndPoint, count++));
			}
		}

		if(count>0){
			newdistmat = new float*[count];
			newdistmat[0] = new float[count*count];
			for(int i=1;i<count;++i) newdistmat[i] = newdistmat[i-1]+count;
		}
		
		for(std::vector<TIndScore>::iterator ittracks1 = trackswtscore.begin() ; ittracks1 != trackswtscore.end() ; ++ittracks1){
			if(!ittracks1->remove){

				float px = ittracks1->pos[0];
				float py = ittracks1->pos[1];
				int newind = (*newdistmatinds)[ittracks1->indnode];
				int prevind;
				if(notfirst) prevind = (*m_DistMatInds)[ittracks1->indprevnode];

				for(std::vector<TIndScore>::iterator ittracks2 = ittracks1 ; ittracks2 != trackswtscore.end() ; ++ittracks2){
					if(!ittracks2->remove){
						int onewind = (*newdistmatinds)[ittracks2->indnode];
						int oprevind;
						if(notfirst) oprevind = (*m_DistMatInds)[ittracks2->indprevnode];
					
						float prevdist;
						if(notfirst && prevind>=0 && oprevind>=0)	prevdist = m_DistMat[prevind][oprevind];
						else						prevdist = 0;

						float dist = ((ittracks2->pos[0]-px)*(ittracks2->pos[0]-px) + (ittracks2->pos[1]-py)*(ittracks2->pos[1]-py));
						dist = MAX(prevdist,dist);
						newdistmat[newind][onewind]=dist;
						newdistmat[onewind][newind]=dist;
					}
				}
			}
		}

		if(m_DistMatInds!=0) delete m_DistMatInds;
		if(m_DistMat!=0){
			delete []m_DistMat[0];
			delete []m_DistMat;
			m_DistMat=0;
		}

		m_DistMat = newdistmat;
		m_DistMatInds = newdistmatinds;


		#ifdef THD_DEBUG_INFO
			std::cout<<"Dist max mat"<<std::endl;
			for(int i=0 ; i<count ; ++i){
				for(int j=0 ; j<count ; ++j){
					std::cout<<m_DistMat[i][j]<<" ";
				}
				std::cout<<std::endl;
			}
			std::cout<<std::endl;
		#endif
	}

}