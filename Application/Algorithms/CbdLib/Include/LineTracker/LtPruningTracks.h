#pragma once

namespace Lt{

	/** \class LtPruningTracks
	*	\brief Abstract class for tracks pruning algorithms. 
	*/
	template<typename TTree>
	class LtPruningTracks{
	public:
		typedef TTree TreeType;
		
		LtPruningTracks(){
			m_MaxTracks = 20;
		}
		virtual ~LtPruningTracks(){}
		void SetTree(TTree* t){m_Tree=t;}

		virtual void Prune()=0;

		virtual void Reset(){}

		// Maximum number of tracks that are selected at the end of an iteration. 
		// The number of tracks considered within the iterator may be much larger.
		int m_MaxTracks;
	protected:
		TTree* m_Tree;
	};

}