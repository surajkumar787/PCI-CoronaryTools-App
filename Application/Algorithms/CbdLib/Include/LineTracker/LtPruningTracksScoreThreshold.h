#pragma once

#include "LtPruningTracks.h"

namespace Lt{

	/** \class LtPruningTracksScoreThreshold
	*	\brief Remove all tracks whose score is below a given threshold
	*/
	template<class TTree>
	class LtPruningTracksScoreThreshold : public LtPruningTracks<TTree>{
	public:
		LtPruningTracksScoreThreshold(){
			m_MinTh = 0.f;
		};

		virtual void Prune(){
			std::vector<int> tracksinds = m_Tree->GetTracks();

			for(unsigned int i=0 ; i<tracksinds.size() ; ++i){
				if(m_Tree->Track(tracksinds[i])->m_Score <= m_MinTh){
					m_Tree->RemoveNode(m_Tree->Track(tracksinds[i])->m_EndPoint);
					m_Tree->RemoveTrack(tracksinds[i]);
				}
			}
		}

		float m_MinTh;
	};

}
