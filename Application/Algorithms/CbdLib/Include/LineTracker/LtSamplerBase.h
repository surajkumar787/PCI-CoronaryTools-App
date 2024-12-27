#pragma once

#define EPSILON 1E-7
#define _PI (4.f * atan(1.f))

#include <cmath>

namespace Lt{
	/** \class LtSamplerBase
	*	\brief base class from samplers that extract several points in a row (for instance on a ring)
	*/
	template<typename T>
	struct LtSamplerBase{
		typedef T ImagePixelType;

		LtSamplerBase(T* im, int iminc, int imsize[2], int nbsamples){
			m_Buf = im;
			m_BufInc = iminc;
			m_BufSize[0] = imsize[0];
			m_BufSize[1] = imsize[1];
	
			m_NbSamples = nbsamples;
			m_Samples = new T[nbsamples];
			m_AreSamplesIn = new bool[nbsamples];

			#ifdef VIRTUAL_VINCENT
				m_AngleLimitForVirtual = 0;
			#endif
		}

		#ifdef VIRTUAL_VINCENT
			LtSamplerBase(T* im, int iminc, int imsize[2], int nbsamples, float angleLimitForVirtual){
				m_Buf = im;
				m_BufInc = iminc;
				m_BufSize[0] = imsize[0];
				m_BufSize[1] = imsize[1];
		
				m_NbSamples = nbsamples;
				m_Samples = new T[nbsamples];
				m_AreSamplesIn = new bool[nbsamples];

				m_AngleLimitForVirtual = angleLimitForVirtual;
			}
		#endif

		virtual ~LtSamplerBase(){
			delete []m_Samples;
			delete []m_AreSamplesIn;
		}

		// input image
		T* m_Buf;
		// input image line increment
		int m_BufInc;
		// input image size
		int m_BufSize[2];

		int m_NbSamples;
		// Array that contains the interpolation result
		T* m_Samples;
		// True if the sample is within the image
		bool* m_AreSamplesIn;
		#ifdef VIRTUAL_VINCENT
			// Angle limit to consider a virtual
			float m_AngleLimitForVirtual;
		#endif
	};


	/** \class LtSamplerRingLinear
	*	\brief Extract image samples on a circle using linear interpolation
	*/
	template<typename T>
	struct LtSamplerRingBase : public LtSamplerBase<T>{

		LtSamplerRingBase(T* im, int iminc, int imsize[2],float radius, int nbsamples):LtSamplerBase(im,iminc,imsize,nbsamples){
			m_Radius = radius;

			m_Dx = new float[nbsamples];
			m_Dy = new float[nbsamples];
			m_Angle = new float[nbsamples];
			
			for(int i=0 ; i<nbsamples ; ++i){
				m_Dx[i] = radius * cos( i * 2 * _PI / nbsamples);
				m_Dy[i] = -radius * sin( i * 2 * _PI / nbsamples);

				m_Angle[i] = i * 2 * _PI / nbsamples;
			}

		}
		#ifdef VIRTUAL_VINCENT
			LtSamplerRingBase(T* im, int iminc, int imsize[2],float radius, int nbsamples, float angleLimitForVirtual)
				:LtSamplerBase(im,iminc,imsize,nbsamples, angleLimitForVirtual){
				m_Radius = radius;

				m_Dx = new float[nbsamples];
				m_Dy = new float[nbsamples];
				m_Angle = new float[nbsamples];
				
				for(int i=0 ; i<nbsamples ; ++i){
					m_Dx[i] = radius * cos( i * 2 * _PI / nbsamples);
					m_Dy[i] = -radius * sin( i * 2 * _PI / nbsamples);

					m_Angle[i] = i * 2 * _PI / nbsamples;
				}

			}
		#endif
		~LtSamplerRingBase(){
			delete []m_Dx;
			delete []m_Dy;
			delete []m_Angle;
		}

		// radius of the circle
		float m_Radius;

		// Position of the samples
		float *m_Dx,*m_Dy;
		// Angles of the samples
		float* m_Angle;
	};

}
