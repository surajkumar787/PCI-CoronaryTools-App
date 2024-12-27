#pragma once

#include "LtExtendTrack.h"

namespace Lt{

	/**	\class LtExtendTrackAddVirtual
	*	\brief Extends tracks from a position predictor.
	*
	*	When this extension function is used, pruning function such as LtPruningTracksHeadDistance 
	*	are all the more usefull to avoid the compactness of the tracks
	*
	*	TODO for now it just extend straight forward. Implement finer position predictor
	*/
	template<typename TTree, typename TFeatureFunc>
	class LtExtendTrackAddVirtual : public LtExtendTrack<TTree>{
	public:
		LtExtendTrackAddVirtual();
		#ifdef VIRTUAL_VINCENT
			LtExtendTrackAddVirtual(int maxNbVirtual);	// Initialize the max number of virtual (default 10)
		#endif
		~LtExtendTrackAddVirtual();

		virtual void ExtendTrack(int trackind);
		virtual void ExtendTrackWOInitialDir(int trackind);

	protected:
		int Init();
		enum PositionPredictionType{
			STRAIGHT,
			FROMCURVATURE
		};

		int PredictStraightPosition();
		int PredictPositionFromCurvature();

	///////////////////////////////////////////////////////////////////////////
	// Parameters
	///////////////////////////////////////////////////////////////////////////

	public:
		TFeatureFunc m_FeatureFunc;
	};

	////////////////////////////////////////////////////////////////////////////////////////////////
	// The template parameter TFeatureFunc may be one of the following classes
	////////////////////////////////////////////////////////////////////////////////////////////////
	namespace details{
		/**	\class EvalScalarImFunc
		*	\brief Performs a linear interpolation at position (x,y) of the scalar input image
		*/
		struct EvalScalarImFunc{
		public:
			float operator()(float x, float y, float dx, float dy){
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

				return rx*(-v + vx + ry*(v-vx-vy+vxy) ) + ry*(-v+vy) + v;
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

		/**	\class EvalMatrixImFunc
		*	\brief The feature image contains at each pixel a 2x2 matrix. 
		*	Performs a linear interpolation of (dx,dy)t M (dx,dy) where M is the matrix at position (x,y).
		*/
		struct EvalMatrixImFunc{
		public:
			float operator()(float x, float y, float dx, float dy){
				int ix = (int) x;
				int iy = (int) y;

				float rx = x-ix;
				float ry = y-iy;

				float* pt = m_Buf + 4*ix + iy*m_BufInc;
				
				float dxdy = - dx * dy;
				float dx2 = dx*dx;
				float dy2 = dy*dy;
				float v = *pt * dx2 + (*(pt+1)+*(pt+2)) * dxdy + *(pt+3) * dy2;
				pt+=4;
				float vx = *pt * dx2 + (*(pt+1)+*(pt+2)) * dxdy + *(pt+3) * dy2;
				pt+=m_BufInc-4;
				float vy = *pt * dx2 + (*(pt+1)+*(pt+2)) * dxdy + *(pt+3) * dy2;
				pt+=4;
				float vxy = *pt * dx2 + (*(pt+1)+*(pt+2)) * dxdy + *(pt+3) * dy2;

				return rx*(-v + vx + ry*(v-vx-vy+vxy) ) + ry*(-v+vy) + v;
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
}


#include "LtExtendTrackAddVirtual.txx"
