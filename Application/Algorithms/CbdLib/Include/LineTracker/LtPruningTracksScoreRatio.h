#pragma once

#include "LtPruningTracks.h"

namespace Lt{

	/** \class LtPruningTracksScore
	*	\brief Select the N best tracks according to their score. 
	*/
	template<class TTree>
	class LtPruningTracksScoreRatio : public LtPruningTracks<TTree>{
	public:
		LtPruningTracksScoreRatio(){
			m_MinScoreRatio = 0.4f;
		};

		typedef struct{
			double score;
			int ind;
		} TIndScore;

		struct IndScoreOrder {
			bool operator()(TIndScore i,TIndScore j) { return (j.score<i.score); }
		} indScoreOrder;

		virtual void Prune(){
			int nbmaxtracks = m_MaxTracks;

			std::vector<int> tracksinds = m_Tree->GetTracks();

			std::vector<TIndScore> trackswtscore(tracksinds.size());
			for(unsigned int i=0 ; i<tracksinds.size() ; ++i){
				trackswtscore[i].ind = tracksinds[i];
				trackswtscore[i].score = m_Tree->Track(tracksinds[i])->m_Score;
			}

			std::vector<TIndScore>::iterator curtrack = std::min_element(trackswtscore.begin() , trackswtscore.end() ,indScoreOrder);
			double minscore = curtrack->score * m_MinScoreRatio;
		
			for(std::vector<TIndScore>::iterator ittracks = trackswtscore.begin(); ittracks != trackswtscore.end() ; ++ittracks){
				if(ittracks->score < minscore){
					m_Tree->RemoveNode(m_Tree->Track(ittracks->ind)->m_EndPoint);
					m_Tree->RemoveTrack(ittracks->ind);
				}
			}
		}

		float m_MinScoreRatio;
	};

}
