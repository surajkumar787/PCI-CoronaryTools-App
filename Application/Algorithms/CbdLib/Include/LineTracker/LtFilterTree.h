#pragma once

namespace Lt{

	/** 
	* \class LtFilterTree
	* \brief Abstract class for algos that apply on the whole tree once
	*/
	template<typename TTree>
	class LtFilterTree{
	public:
		typedef TTree TreeType;
		
		LtFilterTree(){}
		virtual ~LtFilterTree(){}
		void SetTree(TTree* t){m_Tree=t;}

		virtual void Filter(){}
		virtual void Reset(){}

	protected:
		TTree* m_Tree;
	};

}