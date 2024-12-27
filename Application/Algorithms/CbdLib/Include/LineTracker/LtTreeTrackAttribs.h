#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// This file contains several class holding tracks attributes. Those class may be combined through 
// multiple inheritance depending on the attributes used by the tracker. Moreover the class derived 
// must implement ComputeScore function. 
//
// For instance if the instancied tracker makes use of local smoothness attribute, a LtTreeTrack class 
// can be written as: 
// 
//struct LtTreeTrack : public Lt::LtTreeTrackAttribsBase, Lt::LtTreeTrackAttribsLocalSmoothness{
//	LtTreeTrack& operator=(const LtTreeTrack& o){
//		Lt::LtTreeTrackAttribsBase::operator=(o);
//		Lt::LtTreeTrackAttribsLocalSmoothness::operator=(o);
//		return *this;
//	}

//	void Reset(){
//		Lt::LtTreeTrackAttribsBase::Reset();
//		Lt::LtTreeTrackAttribsLocalSmoothness::Reset();
//	};

//	void ComputeScore(){
//		m_Score  = m_FeatSum/m_Length 
//					/ (1+m_IrregLocalScale*m_IrregLocalScale)
//					;
//	}
//};
//////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Lt{

	struct LtTreeTrackAttribsEmpty{
		LtTreeTrackAttribsEmpty& operator=(const LtTreeTrackAttribsEmpty& o){
			return *this;
		}
		void Reset(){
		};
	};

	/** \class LtTreeTrackAttribsBase
	*	\brief Contains common attributes for tracks. The track structure of the tracking algorithm 
	*	always inherits from that class
	*/
	template<typename TAttribBase>
	struct LtTreeTrackAttribsBase : public TAttribBase {
		LtTreeTrackAttribsBase& operator=(const LtTreeTrackAttribsBase& o){
			m_FeatSum = o.m_FeatSum;
			m_Length = o.m_Length;
			m_Score = o.m_Score;
			TAttribBase::operator=(o);
			return *this;
		}

		double m_Score;
		double m_FeatSum;
		float m_Length;

		void Reset(){
			m_FeatSum = 0;
			m_Length = 0;
			m_Score = 0;
			TAttribBase::Reset();
		};
	};

	/** \class LtTreeTrackAttribsLocalSmoothness
	*	\brief Attribute used to hold local smoothness criterions 
	*	(computed by LtComputeFeatureLocalSmoothness or LtComputeFeatureLocalAngleSecDer)
	*
	*	So the track structure has to inherit from that class if either of LtComputeFeatureLocalSmoothness or LtComputeFeatureLocalAngleSecDer
	*	are used.
	*/
	template<typename TAttribBase>
	struct LtTreeTrackAttribsLocalSmoothness : public TAttribBase{
		LtTreeTrackAttribsLocalSmoothness& operator=(const LtTreeTrackAttribsLocalSmoothness& o){
			m_IrregLocalScale = o.m_IrregLocalScale;
			TAttribBase::operator=(o);
			return *this;
		}

		void Reset(){
			m_IrregLocalScale = 0;
			TAttribBase::Reset();
		};


		double m_IrregLocalScale;
	};


	/** \class LtTreeTrackAttribsGlobalSmoothness
	*	\brief Attribute used to hold large scale smoothness criterions 
	*	(computed by LtComputeFeatureGlobalSmoothness)
	*/
	template<typename TAttribBase>
	struct LtTreeTrackAttribsGlobalSmoothness : public TAttribBase{
		LtTreeTrackAttribsGlobalSmoothness& operator=(const LtTreeTrackAttribsGlobalSmoothness& o){
			m_IrregLargeScale = o.m_IrregLargeScale;
			TAttribBase::operator=(o);
			return *this;
		}

		void Reset(){
			m_IrregLargeScale = 0;
			TAttribBase::Reset();
		};

		double m_IrregLargeScale;
	};


	/** \class LtTreeTrackAttribsFilteredGlobalSmoothness
	*	\brief Attribute used to hold large scale smoothness criterions 
	*	(computed by LtComputeFeatureGlobalSmoothness)
	*/
	template<typename TAttribBase>
	struct LtTreeTrackAttribsFilteredGlobalSmoothness : public TAttribBase{
		LtTreeTrackAttribsFilteredGlobalSmoothness& operator=(const LtTreeTrackAttribsFilteredGlobalSmoothness& o){
			m_IrregFilteredLargeScale = o.m_IrregFilteredLargeScale;
			TAttribBase::operator=(o);
			return *this;
		}

		void Reset(){
			m_IrregFilteredLargeScale = 0;
			TAttribBase::Reset();
		};

		double m_IrregFilteredLargeScale;
	};


	/** \class LtTreeTrackAttribswtProfile
	*	\brief Attribute used to hold a mean profile of the track
	*/
	template<typename TAttribBase, unsigned int TProfileSize>
	struct LtTreeTrackAttribswtProfile : public TAttribBase{
		static const unsigned int ProfileSize = TProfileSize;
		LtTreeTrackAttribswtProfile& operator=(const LtTreeTrackAttribswtProfile& o){
			std::copy(o.m_MeanProfile,o.m_MeanProfile+ProfileSize,m_MeanProfile);
			m_ProfileInitialized = o.m_ProfileInitialized;
			m_CumulatedProfilePen = o.m_CumulatedProfilePen;
			TAttribBase::operator=(o);
			return *this;
		}

		void Reset(){
			m_ProfileInitialized = false;
			m_CumulatedProfilePen = 0;
			TAttribBase::Reset();
		};
		bool m_ProfileInitialized;
		float m_MeanProfile[ProfileSize];

		float m_CumulatedProfilePen;
	};





}