#pragma once

#include "LtComputeFeature.h"

namespace Lt{

	/**	\class LtComputeFeatureGlobalSmoothness
	*	\brief Compute a smoothness criterion at a large scale.
	*
	*	The scale is defined by the parameter m_Scale. The direction from the current node with a previous node 
	*	at a distance m_Scale in the track is computed. That direction is then compared to the (large scale) direction of that node.
	*	A penalty is then derived from the difference using a step function defined by parameters m_IrregA and m_IrregB:
	*	bellow m_IrregA no penalty, above m_IrregB max penalty, in between linear.
	*
	*	m_IrregLargeScale parameter of the track then holds the cumulated large scale irregularity along the track.
	*/
	template<typename TTree> 
	class LtComputeFeatureGlobalSmoothness : public LtComputeFeature<TTree>{
	public:
		LtComputeFeatureGlobalSmoothness(){
			m_IrregA = _PI/4.f;
			m_IrregB = _PI/3.f;
			m_Scale = 20;
		}

		virtual void Compute(int trackind){
			TrackType* track = m_Tree->Track(trackind);
			NodeType* endpoint = m_Tree->Node(track->m_EndPoint);
			float dist = 0;
			NodeType* p = endpoint;
			int pindprev = track->m_EndPoint;
			while(dist<m_Scale){
				if(p->m_Father<0)	break;

				dist+=p->m_Length;
				pindprev = p->m_Father;
				p = m_Tree->Node(pindprev);
			}

			if(p->m_Father<0 || pindprev==track->m_EndPoint){
				endpoint->m_Forbear = -1;
				endpoint->m_FarDir = 0;
				return;
			}

			endpoint->m_Forbear = pindprev;
			p = m_Tree->Node(pindprev);

			
			endpoint->m_FarDir = endpoint->m_FarDir = atan2(endpoint->m_Pos[1]-p->m_Pos[1],endpoint->m_Pos[0]-p->m_Pos[0]);
			
			if(p->m_Forbear<0) return;

			float diff = endpoint->m_FarDir-p->m_FarDir;
			if(diff<0)	diff = MIN( fabs(diff) , fabs(diff+2*_PI) ); 
			else		diff = MIN( fabs(diff) , fabs(diff-2*_PI) ); 
			//TODO angles in degrees and precomp denom
			float irreg = (m_IrregA - diff)/(m_IrregA-m_IrregB);
			track->m_IrregLargeScale += MIN(1,MAX(0,irreg));
		}

		float m_IrregA;
		float m_IrregB;
		float m_Scale;
	};

}