#pragma once

#define EPSILON 1E-7
#define _PI (4.f * atan(1.f))

#include <cmath>

#include "LtSamplerBase.h"

namespace Lt{
	/** \class LtSamplerRingNearest
	*	\brief Extract image samples on a circle using nearest neighboor interpolation
	*/
	template<typename T>
	struct LtSamplerRingNearest : public LtSamplerRingBase<T>{
		LtSamplerRingNearest(T* im, int iminc, int imsize[2],float radius, int nbsamples)
			: LtSamplerRingBase(im,iminc,imsize,radius,nbsamples){
			m_Offsets = new long[nbsamples];
			m_iDx = new int[nbsamples];
			m_iDy = new int[nbsamples];;
			m_OffsetsEnd = m_Offsets+nbsamples;
			for(int i=0 ; i<nbsamples ; ++i){
				m_iDx[i] = (int)floor(m_Dx[i]+0.5);
				m_iDy[i] = (int)floor(m_Dy[i]+0.5);
				m_Offsets[i] = m_iDx[i]+m_iDy[i]*iminc;
			}
		}
		#ifdef VIRTUAL_VINCENT
			LtSamplerRingNearest(T* im, int iminc, int imsize[2],float radius, int nbsamples, float angleLimitForVirtual)
				: LtSamplerRingBase(im,iminc,imsize,radius,nbsamples,angleLimitForVirtual){
				m_Offsets = new long[nbsamples];
				m_iDx = new int[nbsamples];
				m_iDy = new int[nbsamples];;
				m_OffsetsEnd = m_Offsets+nbsamples;
				for(int i=0 ; i<nbsamples ; ++i){
					m_iDx[i] = (int)floor(m_Dx[i]+0.5);
					m_iDy[i] = (int)floor(m_Dy[i]+0.5);
					m_Offsets[i] = m_iDx[i]+m_iDy[i]*iminc;
				}
			}
		#endif
		~LtSamplerRingNearest(){
			delete []m_Offsets;
			delete []m_iDx;
			delete []m_iDy;
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
			T* centerpos = m_Buf + pos[0] + pos[1]*m_BufInc;
			if(!checkborder){
				for(long* pto = m_Offsets ; pto != m_OffsetsEnd ; ++pto,++ptres)	*ptres = *(centerpos+*pto);
				if(FillIsInside) std::fill_n(m_AreSamplesIn,m_NbSamples,true);
			}else{
				int* ptdx = m_iDx, *ptdy = m_iDy;
				bool *ptii = m_AreSamplesIn;
				for(long* pto = m_Offsets ; pto != m_OffsetsEnd ; ++pto,++ptdx,++ptdy,++ptres){
					if(pos[0]+*ptdx<0 || pos[1]+*ptdy<0 || pos[0]+*ptdx>=m_BufSize[0] || pos[1]+*ptdy>=m_BufSize[1]){
						if(FillIsInside)	*(ptii++) = false;
						*ptres = 0;
					}else{
						if(FillIsInside)	*(ptii++) = true;
						*ptres = *(centerpos+*pto);
					}
				}
			}

			return 0;
		}

		// offsets of the samples
		long *m_Offsets,*m_OffsetsEnd;
		// Rounded positions of the samples
		int *m_iDx, *m_iDy;
	};

}
