#pragma once


#include "LtComputeFeature.h"


namespace Lt{

	/**	\class LtComputeFeatureLength
	*	\brief Computes the length of the curve
	*
	*/
	template<typename TTree>
	class LtComputeFeatureLength : public LtComputeFeature<TTree>{
	public:

		LtComputeFeatureLength(){}

		virtual void Compute(int trackind){
			TrackType* track = m_Tree->Track(trackind);
			NodeType* endpoint = m_Tree->Node(track->m_EndPoint);

			track->m_Length += endpoint->m_Length;
		}
	};

}

