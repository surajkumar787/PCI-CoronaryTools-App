#pragma once

#define EPSILON 1E-7
#define _PI (4.f * atan(1.f))

#include <cmath>
#include "LtSamplerBase.h"

namespace Lt{

	/** \class LtSamplerProfile
	*	\brief Extract image samples on line
	*/
	template<typename T>
	struct LtSamplerProfile : public LtSamplerBase<T>{
		LtSamplerProfile(T* im, int iminc, int imsize[2], int halfnbsamples):LtSamplerBase(im,iminc,imsize,2*halfnbsamples+1){
			m_NbAngles = 64;
			m_Step = 0.75;
			m_NbSamples = 2*halfnbsamples+1;

			m_Offsets = new long[m_NbSamples*m_NbAngles];
			m_iDx = new int[m_NbSamples*m_NbAngles];
			m_iDy = new int[m_NbSamples*m_NbAngles];;
			
			m_MaxDist = 0;

			m_OffsetsEnd = m_Offsets+m_NbSamples;
			for(int a=0 ; a<m_NbAngles ; ++a){
				float dx = m_Step*cos( a * 2 * _PI / m_NbAngles);
				float dy = -m_Step*sin( a * 2 * _PI / m_NbAngles);
				for(int i=0 ; i<=2*halfnbsamples ; ++i){
					m_iDx[a*m_NbSamples+i] = (int)floor(dx*(i-halfnbsamples)+0.5f);
					m_iDy[a*m_NbSamples+i] = (int)floor(dy*(i-halfnbsamples)+0.5f);
					if(m_iDx[a*m_NbSamples+i] > m_MaxDist) m_MaxDist = (float) m_iDx[a*m_NbSamples+i];
					if(m_iDy[a*m_NbSamples+i] > m_MaxDist) m_MaxDist = (float) m_iDy[a*m_NbSamples+i];
					m_Offsets[a*m_NbSamples+i] = m_iDx[a*m_NbSamples+i]+m_iDy[a*m_NbSamples+i]*iminc;
				}
			}

			m_Samples = new T[m_NbSamples];
			m_AreSamplesIn = new bool[m_NbSamples];

		}
		#ifdef VIRTUAL_VINCENT
			LtSamplerProfile(T* im, int iminc, int imsize[2], int halfnbsamples, float angleLimitForVirtual):LtSamplerBase(im,iminc,imsize,2*halfnbsamples+1, angleLimitForVirtual){
				m_NbAngles = 64;
				m_Step = 0.75;
				m_NbSamples = 2*halfnbsamples+1;

				m_Offsets = new long[m_NbSamples*m_NbAngles];
				m_iDx = new int[m_NbSamples*m_NbAngles];
				m_iDy = new int[m_NbSamples*m_NbAngles];;
				
				m_MaxDist = 0;

				m_OffsetsEnd = m_Offsets+m_NbSamples;
				for(int a=0 ; a<m_NbAngles ; ++a){
					float dx = m_Step*cos( a * 2 * _PI / m_NbAngles);
					float dy = -m_Step*sin( a * 2 * _PI / m_NbAngles);
					for(int i=0 ; i<=2*halfnbsamples ; ++i){
						m_iDx[a*m_NbSamples+i] = (int)floor(dx*(i-halfnbsamples)+0.5f);
						m_iDy[a*m_NbSamples+i] = (int)floor(dy*(i-halfnbsamples)+0.5f);
						if(m_iDx[a*m_NbSamples+i] > m_MaxDist) m_MaxDist = (float) m_iDx[a*m_NbSamples+i];
						if(m_iDy[a*m_NbSamples+i] > m_MaxDist) m_MaxDist = (float) m_iDy[a*m_NbSamples+i];
						m_Offsets[a*m_NbSamples+i] = m_iDx[a*m_NbSamples+i]+m_iDy[a*m_NbSamples+i]*iminc;
					}
				}

				m_Samples = new T[m_NbSamples];
				m_AreSamplesIn = new bool[m_NbSamples];

			}
		#endif
		~LtSamplerProfile(){
			delete []m_Offsets;
			delete []m_iDx;
			delete []m_iDy;
			delete []m_Samples;
			delete []m_AreSamplesIn;
		}

		template<bool FillIsInside>
		int EvalNearest(float pos[2], float dir){
			int ipos[2] = {(int)(pos[0]+0.5),(int)(pos[1]+0.5)};
			return EvalNearest<FillIsInside>(ipos,dir);
		}

		template<bool FillIsInside>
		int EvalNearest(int pos[2], float dir){
			bool checkborder = false;

			int angleind = (int)floor(dir / (2 * _PI) * m_NbAngles+0.5);
			while(angleind<0) angleind+=m_NbAngles;
			while(angleind>=m_NbAngles) angleind-=m_NbAngles;

			if(pos[0]-m_MaxDist<-0.5+EPSILON || pos[1]-m_MaxDist<-0.5+EPSILON || 
				pos[0]+m_MaxDist>=m_BufSize[0]-1+0.5-EPSILON || pos[1]+m_MaxDist>=m_BufSize[1]-1+0.5-EPSILON) checkborder=true;

			T* ptres = m_Samples;
			T* centerpos = m_Buf + pos[0] + pos[1]*m_BufInc;
			long* offsets = m_Offsets+angleind*m_NbSamples;
			long* offsetsend = offsets+m_NbSamples;
			
			if(!checkborder){
				for(long* pto = offsets ; pto != offsetsend ; ++pto,++ptres)
					*ptres = *(centerpos+*pto);
				if(FillIsInside) std::fill_n(m_AreSamplesIn,m_NbSamples,true);
			}else{
				int* ptdx = m_iDx+angleind*m_NbSamples, *ptdy = m_iDy+angleind*m_NbSamples;
				bool *ptii = m_AreSamplesIn;
				for(long* pto = offsets ; pto != offsetsend; ++pto,++ptdx,++ptdy,++ptres){
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

		int m_NbAngles;
		float m_Step;
		float m_MaxDist;
	};
}
