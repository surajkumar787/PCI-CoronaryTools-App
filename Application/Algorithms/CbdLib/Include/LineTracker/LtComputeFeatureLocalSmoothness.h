#pragma once

#include "LtComputeFeature.h"


namespace Lt{

	/**	\class LtComputeFeatureLocalSmoothness
	*	\brief Computes a local smoothness criterion based on the variation of track direction.
	*
	*	The current track direction (at the endpoint) is compared with the direction at the previous node.
	*	A penalty is then derived from the difference using a step function defined by parameters m_IrregA and m_IrregB:
	*	bellow m_IrregA no penalty, above m_IrregB max penalty, in between linear.
	*
	*	m_IrregLocalScale parameter of the track then holds the cumulated local irregularity along the track.
	*/
	template<typename TTree>
	class LtComputeFeatureLocalSmoothness : public LtComputeFeature<TTree>{
	public:

		LtComputeFeatureLocalSmoothness(){
			m_IrregA = _PI/6.f;
			m_IrregB = _PI/3.f;
		}

		virtual void Compute(int trackind){
			
			TrackType* track = m_Tree->Track(trackind);
			NodeType* endpoint = m_Tree->Node(track->m_EndPoint);
			NodeType* prevpoint = m_Tree->Node(endpoint->m_Father);
			
			float diff = endpoint->m_Dir-prevpoint->m_Dir;
			if(diff<0)	diff = MIN( fabs(diff) , fabs(diff+2*_PI) ); 
			else		diff = MIN( fabs(diff) , fabs(diff-2*_PI) ); 
			//TODO angles in degrees and precomp denom
			float irreglocscale = MIN(1,MAX(0,(m_IrregA - diff)/(m_IrregA-m_IrregB)));
			track->m_IrregLocalScale += irreglocscale;//*irreglocscale;
		}

		float m_IrregA;
		float m_IrregB;
	};

}