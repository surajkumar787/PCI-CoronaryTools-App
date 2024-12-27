#pragma once

#include "LtPruningTracks.h"

namespace Lt{

	/** \class LtPruningTracksScore
	*	\brief Select the N best tracks according to their score. 
	*/
	template<class TTree>
	class LtPruningTracksScore : public LtPruningTracks<TTree>{
	public:
		virtual void Prune(){
			unsigned int nbmaxtracks = m_MaxTracks;

			std::vector<int> tracksinds = m_Tree->GetTracks();
			if(nbmaxtracks < tracksinds.size()){
				typedef struct{
					double score;
					int ind;
				} TIndScore;

				struct IndScoreOrder {
					bool operator()(TIndScore i,TIndScore j) { return (j.score<i.score); }
				} indScoreOrder;

				std::vector<TIndScore> trackswtscore(tracksinds.size());
				for(unsigned int i=0 ; i<tracksinds.size() ; ++i){
					trackswtscore[i].ind = tracksinds[i];
					trackswtscore[i].score = m_Tree->Track(tracksinds[i])->m_Score;
				}

				std::partial_sort(trackswtscore.begin() , trackswtscore.begin()+nbmaxtracks, trackswtscore.end() , indScoreOrder);
			
				for(std::vector<TIndScore>::iterator itbadtracks = trackswtscore.begin()+nbmaxtracks ; itbadtracks != trackswtscore.end() ; ++itbadtracks){
					m_Tree->RemoveNode(m_Tree->Track(itbadtracks->ind)->m_EndPoint);
					m_Tree->RemoveTrack(itbadtracks->ind);
				}
			}
		}
	};

}
