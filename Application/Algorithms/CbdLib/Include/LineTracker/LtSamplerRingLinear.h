#pragma once

#define EPSILON 1E-7
#ifndef _PI
	#define _PI (4.f * atan(1.f))
#endif


#include <cmath>
#include "LtSamplerBase.h"

namespace Lt{
	/** \class LtSamplerRingLinear
	*	\brief Extract image samples on a circle using linear interpolation
	*/
	template<typename T>
	struct LtSamplerRingLinear : public LtSamplerRingBase<T>{
		LtSamplerRingLinear(T* im, int iminc, int imsize[2],float radius, int nbsamples)
			:LtSamplerRingBase(im,iminc,imsize,radius,nbsamples){
		}
		#ifdef VIRTUAL_VINCENT
			LtSamplerRingLinear(T* im, int iminc, int imsize[2],float radius, int nbsamples, float angleLimitForVirtual)
				:LtSamplerRingBase(im,iminc,imsize,radius,nbsamples, angleLimitForVirtual){
			}
		#endif
		~LtSamplerRingLinear(){
		}

		template<bool FillIsInside>
		int Eval(float pos[2]){
			bool checkborder = false;
			if(pos[0]-m_Radius<-0.5+EPSILON || pos[1]-m_Radius<-0.5+EPSILON || 
				pos[0]+m_Radius>=m_BufSize[0]-1+0.5-EPSILON || pos[1]+m_Radius>=m_BufSize[1]-1+0.5-EPSILON) checkborder=true;

			T* ptres = m_Samples;

			float *ptdx = m_Dx;
			float *ptdy = m_Dy;
			float *ptdxend = m_Dx+m_NbSamples;

			if(!checkborder){
				for( ; ptdx!=ptdxend ; ++ptdx,++ptdy,++ptres){
					float x = pos[0]+*ptdx;
					float y = pos[1]+*ptdy;

					int ix = (int) x;
					int iy = (int) y;

					float rx = x-ix;
					float ry = y-iy;

					T* pt = m_Buf + ix + iy*m_BufInc;
					T v = *pt;
					T vx = *(pt+1);
					T vy = *(pt+m_BufInc);
					T vxy = *(pt+1+m_BufInc);

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

					if(FillIsInside)	*ptii = true;
					if(ix<0){
						ix=0 ; x=0;
						if(FillIsInside)	*ptii = false;
					}
					if(iy<0){
						iy=0 ; y=0;
						if(FillIsInside)	*ptii = false;
					}
					if(ix+1>=m_BufSize[0]){
						ix=m_BufSize[0]-2; x=(float)m_BufSize[0]-1;
						if(FillIsInside)	*ptii = false;
					}
					if(iy+1>=m_BufSize[1]){
						iy=m_BufSize[0]-2; y=(float)m_BufSize[0]-1;
						if(FillIsInside)	*ptii = false;
					}


					if(FillIsInside)	ptii++;
					float rx = x-ix;
					float ry = y-iy;

					T* pt = m_Buf + ix + iy*m_BufInc;
					T v = *pt;
					T vx = *(pt+1);
					T vy = *(pt+m_BufInc);
					T vxy = *(pt+1+m_BufInc);

					*ptres = rx*(-v + vx + ry*(v-vx-vy+vxy) ) + ry*(-v+vy) + v;
				}
			}

			return 0;
		}

	};

}
