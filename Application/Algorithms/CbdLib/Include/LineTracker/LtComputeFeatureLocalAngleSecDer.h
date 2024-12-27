#pragma once

#include "LtComputeFeature.h"


namespace Lt{

	/**	\class LtComputeFeatureLocalSmoothness
	*	\brief Computes a local smoothness criterion based on the second derivative of track direction.
	*
	*	The second derivative of the track direction is computed from the 3 last nodes.

	*	A penalty is then derived from the difference using a step function defined by parameters m_IrregA and m_IrregB:
	*	bellow m_IrregA no penalty, above m_IrregB max penalty, in between linear.
	*
	*	m_IrregLocalScale parameter of the track then holds the cumulated local irregularity along the track.
	*/
	template<typename TTree>
	class LtComputeFeatureLocalAngleSecDer : public LtComputeFeature<TTree>{
	public:

		LtComputeFeatureLocalAngleSecDer(){
			m_IrregA = _PI/200.f;
			m_IrregB = _PI/100.f;
		}

		virtual void Compute(int trackind){
			
			TrackType* track = m_Tree->Track(trackind);
			NodeType* p1 = m_Tree->Node(track->m_EndPoint);
			if(p1->m_Father<0) return;
			NodeType* p2 = m_Tree->Node(p1->m_Father);
			if(p2->m_Father<0) return;
			NodeType* p3 = m_Tree->Node(p2->m_Father);

			float d1 = p1->m_Dir; 
			float d2 = p2->m_Dir; 
			float d3 = p3->m_Dir; 

			if(d2-d1<=-_PI) d2+=2*_PI;
			if(d2-d1>_PI) d2-=2*_PI;
			if(d3-d2<=-_PI) d3+=2*_PI;
			if(d3-d2>_PI) d3-=2*_PI;

			float diff1 = (d1-d2)/ p1->m_Length;
			float diff2 = (d2-d3)/ p2->m_Length;
			float diff = (diff1-diff2) / p1->m_Length;

			//TODO angles in degrees and precomp denom
			float irreglocscale = (diff - m_IrregB)/(m_IrregA-m_IrregB);
			track->m_IrregLocalScale += 1-MIN(1,MAX(0,irreglocscale));
		}

		float m_IrregA;
		float m_IrregB;
	};

}