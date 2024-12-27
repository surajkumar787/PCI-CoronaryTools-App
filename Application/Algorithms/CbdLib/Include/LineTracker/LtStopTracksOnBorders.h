#pragma once

#include "LtStopTracks.h"


namespace Lt{

	/** \class LtStopTracksOnBorders
	*	\brief Stop a track when it falls outside a given ROI
	*/
	template<typename TTree> 
	class LtStopTracksOnBorders : public LtStopTracks<TTree>{
	public:
		LtStopTracksOnBorders(float roi[4]){
			std::copy(roi,roi+4,m_Roi) ;
		}

		virtual int StopTracks(std::vector<int>& trackinds){
			
			for(std::set<int>::iterator it=m_Tree->m_RunningTracks.begin() ; it != m_Tree->m_RunningTracks.end() ; ++it){
				TreeType::TrackType* track = m_Tree->Track(*it);
				TreeType::NodeType* endpoint = m_Tree->Node(track->m_EndPoint);
				float px = endpoint->m_Pos[0];
				float py = endpoint->m_Pos[1];

				bool end = false;
				if(px<m_Roi[0] || px>m_Roi[2]) end = true;
				if(py<m_Roi[1] || py>m_Roi[3]) end = true;

				if(end) trackinds.push_back(*it);
			}

			return 0;
		}

		float m_Roi[4];
	};
}


