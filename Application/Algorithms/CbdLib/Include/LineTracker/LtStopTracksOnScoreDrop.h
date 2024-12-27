#pragma once

#include "LtStopTracks.h"


namespace Lt{

	/** \class LtStopTracksOnScoreDrop
	*	\brief Stop a track when the score decreases several times in a row
	*/
	template<typename TTree> 
	class LtStopTracksOnScoreDrop : public LtStopTracks<TTree>{
	public:
		LtStopTracksOnScoreDrop(){
			m_RatioA = 0.4f; 
			m_RatioB = 0.7f;
			m_AccuTH = 3.f;
			m_ForgetFactor = 0.8f;
		}

		virtual int StopTracks(std::vector<int>& trackinds){
			for(std::set<int>::iterator it=m_Tree->m_RunningTracks.begin() ; it != m_Tree->m_RunningTracks.end() ; ++it){
				TreeType::TrackType* track = m_Tree->Track(*it);
				
				float pen = (float)MAX(0,MIN(1,(track->m_DeltaScore/track->m_Score - m_RatioB)/(m_RatioA - m_RatioB)));

				track->m_ScoreDrop = m_ForgetFactor*track->m_ScoreDrop + pen;
				if(track->m_ScoreDrop > m_AccuTH) trackinds.push_back(*it);
			}

			return 0;
		}
		
		float m_RatioA;
		float m_RatioB;
		float m_AccuTH;
		float m_ForgetFactor;
	};
}


