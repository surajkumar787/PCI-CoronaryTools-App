#pragma once

#define EPSILON 1E-7
#define _PI (4.f * atan(1.f))

#include <cmath>
#include "LtSamplerBase.h"


namespace Lt{
	/** \class LtSamplerRing2x2MatrixLinear
	*	\brief Compute v = Xt M X on a circle from a 2x2 matrix M, where Xt is the offset vector from the centerpoint. 
	*
	*	Linear interpolation is used.
	*	If M is positive definite the result is the square of the arc length with respect to the metric tensor M
	*/
	template<typename T>
	struct LtSamplerRing2x2MatrixLinear : public LtSamplerRingBase<T>{
		LtSamplerRing2x2MatrixLinear(T* im, int iminc, int imsize[2],float radius, int nbsamples)
			:LtSamplerRingBase(im,iminc,imsize,radius,nbsamples){
			m_Dx2 = new float[nbsamples];
			m_Dy2 = new float[nbsamples];

			for(int i=0 ; i<nbsamples ; ++i){
				m_Dx2[i] = m_Dx[i]*m_Dx[i];
				m_Dy2[i] = m_Dy[i]*m_Dy[i];
			}
		}
		#ifdef VIRTUAL_VINCENT
			LtSamplerRing2x2MatrixLinear(T* im, int iminc, int imsize[2],float radius, int nbsamples, float angleLimitForVirtual)
				:LtSamplerRingBase(im,iminc,imsize,radius,nbsamples, angleLimitForVirtual){
				m_Dx2 = new float[nbsamples];
				m_Dy2 = new float[nbsamples];

				for(int i=0 ; i<nbsamples ; ++i){
					m_Dx2[i] = m_Dx[i]*m_Dx[i];
					m_Dy2[i] = m_Dy[i]*m_Dy[i];
				}
			}
		#endif
		~LtSamplerRing2x2MatrixLinear(){
			delete []m_Dx2;
			delete []m_Dy2;
		}

		template<bool FillIsInside>
		int Eval(float pos[2]){
			bool checkborder = false;
			if(pos[0]-m_Radius<-0.5+EPSILON || pos[1]-m_Radius<-0.5+EPSILON || 
				pos[0]+m_Radius>=m_BufSize[0]-1+0.5-EPSILON || pos[1]+m_Radius>=m_BufSize[1]-1+0.5-EPSILON) checkborder=true;

			T* ptres = m_Samples;

			float *ptdx = m_Dx;
			float *ptdy = m_Dy;
			float *ptdx2 = m_Dx2;
			float *ptdy2 = m_Dy2;

			float *ptdxend = m_Dx+m_NbSamples;

			if(!checkborder){
				for( ; ptdx!=ptdxend ; ++ptdx,++ptdy,++ptdx2,++ptdy2,++ptres){
					float x = pos[0]+*ptdx;
					float y = pos[1]+*ptdy;

					int ix = (int) x;
					int iy = (int) y;

					float rx = x-ix;
					float ry = y-iy;

					T* pt = m_Buf + 4*ix + iy*m_BufInc;
					
					float dxdy = - *ptdx * *ptdy;
					T v = *pt * *ptdx2 + (*(pt+1)+*(pt+2)) * dxdy + *(pt+3) * *ptdy2;
					pt+=4;
					T vx = *pt * *ptdx2 + (*(pt+1)+*(pt+2)) * dxdy + *(pt+3) * *ptdy2;
					pt+=m_BufInc-4;
					T vy = *pt * *ptdx2 + (*(pt+1)+*(pt+2)) * dxdy + *(pt+3) * *ptdy2;
					pt+=4;
					T vxy = *pt * *ptdx2 + (*(pt+1)+*(pt+2)) * dxdy + *(pt+3) * *ptdy2;

					*ptres = rx*(-v + vx + ry*(v-vx-vy+vxy) ) + ry*(-v+vy) + v;
				}
				if(FillIsInside) std::fill_n(m_AreSamplesIn,m_NbSamples,true);
			}else{
				bool *ptii = m_AreSamplesIn;
				for( ; ptdx!=ptdxend ; ++ptdx,++ptdy,++ptres){
					float x = pos[0]+*ptdx;
					float y = pos[1]+*ptdy;

					int ix = (int) x;
					int iy = (int) y;

					if(ix<0 || iy<0 || ix+1>=m_BufSize[0] || iy+1>=m_BufSize[1]){
						if(FillIsInside)	*(ptii++) = false;
							*ptres = 0;
					}else{
						if(FillIsInside)	*(ptii++) = true;
						float rx = x-ix;
						float ry = y-iy;

						T* pt = m_Buf + 4*ix + iy*m_BufInc;
					
						float dxdy = - *ptdx * *ptdy;
						T v = *pt * *ptdx2 + (*(pt+1)+*(pt+2)) * dxdy + *(pt+3) * *ptdy2;
						pt+=4;
						T vx = *pt * *ptdx2 + (*(pt+1)+*(pt+2)) * dxdy + *(pt+3) * *ptdy2;
						pt+=m_BufInc-4;
						T vy = *pt * *ptdx2 + (*(pt+1)+*(pt+2)) * dxdy + *(pt+3) * *ptdy2;
						pt+=4;
						T vxy = *pt * *ptdx2 + (*(pt+1)+*(pt+2)) * dxdy + *(pt+3) * *ptdy2;

						*ptres = rx*(-v + vx + ry*(v-vx-vy+vxy) ) + ry*(-v+vy) + v;
					}
				}
			}

			return 0;
		}

		// Square position of the samples
		float *m_Dx2,*m_Dy2;
	};
}
