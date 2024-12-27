#pragma once


#include "LtComputeFeature.h"


namespace Lt{

	/**	\class LtComputeFeatureLinInterScalarIm
	*	\brief Performs of bilinear interpolation on a scalar image
	*
	*/
	template<typename TTree>
	class LtComputeFeatureLinInterScalarIm : public LtComputeFeature<TTree>{
	public:

		LtComputeFeatureLinInterScalarIm(){}

		virtual void Compute(int trackind){
			TrackType* track = m_Tree->Track(trackind);
			NodeType* endpoint = m_Tree->Node(track->m_EndPoint);

			float x = endpoint->m_Pos[0];
			float y = endpoint->m_Pos[1];

			int ix = (int) x;
			int iy = (int) y;

			if(ix<0){ ix=0; x=0;}
			if(iy<0){ iy=0; y=0;}
			if(ix>m_BufSize[0]-2){ ix=m_BufSize[0]-2;x=(float)m_BufSize[0]-1; }
			if(iy>m_BufSize[1]-2){ iy=m_BufSize[1]-2;y=(float)m_BufSize[1]-1; }

			float rx = x-ix;
			float ry = y-iy;

			float* pt = m_Buf + ix + iy*m_BufInc;
			float v = *pt;
			float vx = *(pt+1);
			float vy = *(pt+m_BufInc);
			float vxy = *(pt+1+m_BufInc);

			float feat = rx*(-v + vx + ry*(v-vx-vy+vxy) ) + ry*(-v+vy) + v;
			track->m_FeatSum += feat * endpoint->m_Length;
		}

		void SetFeatureIm(float* im, int iminc, int imsize[2]){
				m_Buf = im;
				m_BufInc = iminc;
				m_BufSize[0] = imsize[0];
				m_BufSize[1] = imsize[1];
		}

		// input image
		float* m_Buf;
		// input image line increment
		int m_BufInc;
		// input image size
		int m_BufSize[2];
	};

}

