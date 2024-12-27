#pragma once

#include "LtFilterCompletedTrack.h"

namespace Lt{

	/** 
	* \class LtFilterCompletedTrackRemoveIfShort 
	* \brief Remove small tracks
	*/
	template<typename TTree>
	class LtFilterCompletedTrackRemoveIfShort : public LtFilterCompletedTrack<TTree>{
	public:
		LtFilterCompletedTrackRemoveIfShort(){
			m_MinLength = 20;
		}

		virtual void Filter(int trackind){
			float length = m_Tree->Track(trackind)->m_Length;
			if(length<m_MinLength){
				m_Tree->RemoveTrack(trackind);
			}
		}
		
		void Reset(){
		
		}

		float m_MinLength;
	};

}