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
	*	m_IrregFilteredLargeScale parameter of the track then holds the cumulated large scale irregularity along the track.
	*/
	template<typename TTree> 
	class LtComputeFeatureFilteredGlobalSmoothness : public LtComputeFeature<TTree>{
	public:
		LtComputeFeatureFilteredGlobalSmoothness(){
			m_IrregA = _PI/9.f;
			m_IrregB = _PI/2.f;
			m_Scale = 5;
		}

		virtual void Compute(int trackind){
			TrackType* track = m_Tree->Track(trackind);
			NodeType* endpoint = m_Tree->Node(track->m_EndPoint);

			// Filter angle
			float dist = 0;
			NodeType* p = endpoint;
			int pindprev = track->m_EndPoint;
			#ifndef FILTER_ANGLE
				float *VectForMedian = new float[(int)m_Scale+1];
				VectForMedian[0] = p->m_Dir;
			#endif
			int nbElts=1;
			while(dist<m_Scale){
				if(p->m_Father<0)	break;

				dist++;
				pindprev = p->m_Father;
				p = m_Tree->Node(pindprev);

				#ifndef FILTER_ANGLE
					VectForMedian[nbElts] = p->m_Dir; 
				#endif
				nbElts++;
			}
			
			#ifndef FILTER_ANGLE
				float *StackForMedian = new float[(int)m_Scale+1];
				endpoint->m_FilteredDir = LtMedian(VectForMedian, StackForMedian, 0
													, (int)(nbElts-1)/2, nbElts-1);

				//printf("\nMedian %.2f\n", endpoint->m_FilteredDir);

				delete[]VectForMedian; delete[]StackForMedian;
			#endif

			

			// * Look for past point
			if (nbElts<m_Scale ||p->m_Father<0 || pindprev==track->m_EndPoint){
				endpoint->m_Forbear = -1;
			}else{
				pindprev = endpoint->m_Father;
				p = m_Tree->Node(pindprev);

				endpoint->m_Forbear = pindprev;

				float diff = endpoint->m_Dir-p->m_FilteredDir;
				if(diff<0)	diff = MIN( fabs(diff) , fabs(diff+2*_PI) ); 
				else		diff = MIN( fabs(diff) , fabs(diff-2*_PI) ); 
				//TODO angles in degrees and precomp denom
				float irreg = (m_IrregA - diff)/(m_IrregA-m_IrregB);
/*
				if (irreg > 0){
					printf("%f %f %f %f\n", endpoint->m_Pos[0], endpoint->m_Pos[1]
					, p->m_Pos[0], p->m_Pos[1]);
					irreg = irreg;
				}*/
				track->m_IrregFilteredLargeScale += MIN(1,MAX(0,irreg));
			}
		}

		float m_IrregA;
		float m_IrregB;
		float m_Scale;

		
	private:
		float LtMedian(float * stack1, float * stack2, int x0, int pos, int xEnd)
		{

			int xMinus, xPlus, i;
			float pivot;

			xMinus = x0;
			xPlus = xEnd;
			pivot = stack1[x0];

			for (i=x0+1; i<=xEnd;i++)
				if (stack1[i]<pivot){
					stack2[xMinus] = stack1[i];
					xMinus ++;
				} else if (stack1[i]>pivot) {
					stack2[xPlus] = stack1[i];
					xPlus --;
				}
			
			xMinus--; xPlus++;

			if ( (xMinus < pos) && (xPlus > pos) )
				return pivot;
			else if (xMinus >= pos)
				return LtMedian(stack2,stack1,x0,pos,xMinus);
			else 
				return LtMedian(stack2,stack1,xPlus,pos,xEnd);
		}
		
	};

}
