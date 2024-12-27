#pragma once

#define EPSILON 1E-7
#define _PI (4.f * atan(1.f))

#include <cmath>
#include "LtSamplerBase.h"

namespace Lt{
	/** \class LtSamplerRing2x2MatrixNearest
	*	\brief Compute v = Xt M X on a circle from a 2x2 matrix M, where Xt is the offset vector from the centerpoint. 
	*
	*	Nearest neighboor interpolation is used. 
	*	If M is positive definite the result is the square of the arc length with respect to the metric tensor M
	*/
	template<typename T>
	struct LtSamplerRing2x2MatrixNearest : public LtSamplerRingBase<T>{
		LtSamplerRing2x2MatrixNearest(T* im, int iminc, int imsize[2],float radius, int nbsamples)
			:LtSamplerRingBase(im,iminc,imsize,radius,nbsamples){
			m_Offsets = new long[nbsamples];
			m_iDx = new int[nbsamples];
			m_iDy = new int[nbsamples];;
			m_OffsetsEnd = m_Offsets+nbsamples;
			
			m_Dxy = new float[nbsamples];
			m_Dx2 = new float[nbsamples];
			m_Dy2 = new float[nbsamples];
			
			for(int i=0 ; i<nbsamples ; ++i){
				m_iDx[i] = (int)floor(m_Dx[i]+0.5);
				m_iDy[i] = (int)floor(m_Dy[i]+0.5);
				m_Offsets[i] = 4*m_iDx[i]+m_iDy[i]*iminc;

				m_Dxy[i] = -m_Dx[i]*m_Dy[i];
				m_Dx2[i] = m_Dx[i]*m_Dx[i];
				m_Dy2[i] = m_Dy[i]*m_Dy[i];
			}
		}
		#ifdef VIRTUAL_VINCENT
			LtSamplerRing2x2MatrixNearest(T* im, int iminc, int imsize[2],float radius, int nbsamples, float angleLimitForVirtual)
				:LtSamplerRingBase(im,iminc,imsize,radius,nbsamples,angleLimitForVirtual){
				m_Offsets = new long[nbsamples];
				m_iDx = new int[nbsamples];
				m_iDy = new int[nbsamples];;
				m_OffsetsEnd = m_Offsets+nbsamples;
				
				m_Dxy = new float[nbsamples];
				m_Dx2 = new float[nbsamples];
				m_Dy2 = new float[nbsamples];
				
				for(int i=0 ; i<nbsamples ; ++i){
					m_iDx[i] = (int)floor(m_Dx[i]+0.5);
					m_iDy[i] = (int)floor(m_Dy[i]+0.5);
					m_Offsets[i] = 4*m_iDx[i]+m_iDy[i]*iminc;

					m_Dxy[i] = -m_Dx[i]*m_Dy[i];
					m_Dx2[i] = m_Dx[i]*m_Dx[i];
					m_Dy2[i] = m_Dy[i]*m_Dy[i];
				}
			}
		#endif
		~LtSamplerRing2x2MatrixNearest(){
			delete []m_Offsets;
			delete []m_iDx;
			delete []m_iDy;

			delete []m_Dxy;
			delete []m_Dx2;
			delete []m_Dy2;
		}

		template<bool FillIsInside>
		int Eval(float pos[2]){
			int ipos[2] = {(int)(pos[0]+0.5),(int)(pos[1]+0.5)};
			return Eval<FillIsInside>(ipos);
		}

		template<bool FillIsInside>
		int Eval(int pos[2]){
			bool checkborder = false;
			if(pos[0]-m_Radius<-0.5+EPSILON || pos[1]-m_Radius<-0.5+EPSILON || 
				pos[0]+m_Radius>=m_BufSize[0]-1+0.5-EPSILON || pos[1]+m_Radius>=m_BufSize[1]-1+0.5-EPSILON) checkborder=true;

			T* ptres = m_Samples;
			T* centerpos = m_Buf + 4*pos[0] + pos[1]*m_BufInc;

			float* dxdy = m_Dxy;
			float* dx2 = m_Dx2;
			float* dy2 = m_Dy2;
			if(!checkborder){
				for(long* pto = m_Offsets ; pto != m_OffsetsEnd ; ++pto,++ptres, ++dxdy, ++dx2, ++dy2){
					T* pt = centerpos+*pto;
					*ptres = *pt * *dx2 + (*(pt+1)+*(pt+2)) * *dxdy + *(pt+3) * *dy2;
				}
				if(FillIsInside) std::fill_n(m_AreSamplesIn,m_NbSamples,true);
			}else{
				int* ptdx = m_iDx, *ptdy = m_iDy;
				bool *ptii = m_AreSamplesIn;
				for(long* pto = m_Offsets ; pto != m_OffsetsEnd ; ++pto,++ptdx,++ptdy,++ptres, ++dxdy, ++dx2, ++dy2){
					if(pos[0]+*ptdx<0 || pos[1]+*ptdy<0 || pos[0]+*ptdx>=m_BufSize[0] || pos[1]+*ptdy>=m_BufSize[1]){
						if(FillIsInside)	*(ptii++) = false;
						*ptres = 0;
					}else{
						if(FillIsInside)	*(ptii++) = true;
						T* pt = centerpos+*pto;
						*ptres = *pt * *dx2 + (*(pt+1)+*(pt+2)) * *dxdy + *(pt+3) * *dy2;
					}
				}
			}
	
			return 0;
		}


		// offsets of the samples
		long *m_Offsets,*m_OffsetsEnd;
		// Rounded positions of the samples
		int *m_iDx, *m_iDy;
		// Square position of the samples
		float *m_Dxy,*m_Dx2,*m_Dy2;

	};
}
