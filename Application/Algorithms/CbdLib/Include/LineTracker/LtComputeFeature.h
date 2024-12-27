#pragma once

#define _PI (4.f * atan(1.f))

namespace Lt{

	/** \class LtComputeFeature
	*	\brief Abstract class for feature computation. 
	*/
	template<typename TTree>
	class LtComputeFeature{
	public:
		typedef TTree TreeType;
		typedef typename TTree::TrackType TrackType;
		typedef typename TTree::NodeType NodeType;

		LtComputeFeature(){}
		virtual ~LtComputeFeature(){}
		void SetTree(TTree* t){m_Tree=t;}

		virtual void Compute(int trackind)=0;

		virtual void Reset(){}

	protected:
		TTree* m_Tree;
	};

}