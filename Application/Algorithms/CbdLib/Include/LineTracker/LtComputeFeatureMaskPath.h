#pragma once

#include "LtComputeFeature.h"

#include "LtTrack.h"

namespace Lt{

	/**	\class LtComputeFeatureMaskPath
	*	\brief Computes penalty related to the distance to a previously extracted path.
	*
	*
	*/
	template<typename TTree>
	class LtComputeFeatureMaskPath : public LtComputeFeature<TTree>{
	public:

		LtComputeFeatureMaskPath(LtTrack* prevtrack){
			m_DirA = _PI/10.f;
			m_DirB = _PI/6.f;;
			m_DistA = 4.f;
			m_DistB = 10.f;

			m_NbPts = prevtrack->Size();
			m_X = new float[m_NbPts];
			m_Y = new float[m_NbPts];
			m_A = new float[m_NbPts];
			for(int i=0 ; i<m_NbPts ; ++i){
				m_X[i] = prevtrack->m_Pts[i].X;
				m_Y[i] = prevtrack->m_Pts[i].Y;
				m_A[i] = prevtrack->m_Pts[i].A;
			}
		}

		~LtComputeFeatureMaskPath(){
			delete []m_X;
			delete []m_Y;
			delete []m_A;
		}

		virtual void Compute(int trackind){
			TrackType* track = m_Tree->Track(trackind);
			NodeType* endpoint = m_Tree->Node(track->m_EndPoint);
			float x = endpoint->m_Pos[0];
			float y = endpoint->m_Pos[1];

			float mindist = FLT_MAX;
			int minind = 0;
			for(int i=1 ; i<m_NbPts ; ++i){
				float x1 = m_X[i];
				float x2 = m_X[i-1];
				float y1 = m_Y[i];
				float y2 = m_Y[i-1];
				
				float dist;
				float ps = ((x-x1)*(x2-x1)+(y-y1)*(y2-y1))/((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
				if(ps<0) dist = sqrt((x-x1)*(x-x1)+(y-y1)*(y-y1));
				else	if(ps>1) dist = sqrt((x-x2)*(x-x2)+(y-y2)*(y-y2));
				else{
					float px = (1-ps)*x1+ps*x2;
					float py = (1-ps)*y2+ps*y2;
					dist = sqrt((x-px)*(x-px)+(y-py)*(y-py));
				}
				if(dist<mindist){
					mindist = dist;
					minind = i;
				}
			}

			float dir = endpoint->m_Dir;
			float diffdir = dir-m_A[minind-1];
			while(diffdir>_PI/2.f) diffdir-=_PI;
			while(diffdir<-_PI/2.f) diffdir+=_PI;

			diffdir = fabs(diffdir);
			
			float dirpen = MIN(1,MAX(0,(m_DirB - diffdir)/(m_DirB-m_DirA)));
			float distpen = MIN(1,MAX(0,(m_DistB - mindist)/(m_DistB-m_DistA)));

			track->m_ProxymityToPrevTrack += dirpen*distpen*endpoint->m_Length;
		}

		int m_NbPts;
		float* m_X;
		float* m_Y;
		float* m_A;

		float m_DirA;
		float m_DirB;
		float m_DistA;
		float m_DistB;
	};

}