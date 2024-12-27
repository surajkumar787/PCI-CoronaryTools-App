#pragma once

namespace Lt{

	/** 
	* \class LtFilterCompletedTrack
	* \brief Abstract class for functions that may remove or modify tracks when they are completed
	*/
	template<typename TTree>
	class LtFilterCompletedTrack{
	public:
		typedef TTree TreeType;
		
		LtFilterCompletedTrack(){}
		virtual ~LtFilterCompletedTrack(){}
		void SetTree(TTree* t){m_Tree=t;}

		virtual void Filter(int trackind){}
		virtual void Reset(){}

	protected:
		TTree* m_Tree;
	};

}