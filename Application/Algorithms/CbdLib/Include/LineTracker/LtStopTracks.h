#pragma once

namespace Lt{

	/** \class LtStopTracks
	*	\brief Abstract class for track ending criteriosn
	*/
	template<typename TTree>
	class LtStopTracks{
	public:
		typedef TTree TreeType;
		
		LtStopTracks(){}
		virtual ~LtStopTracks(){}
		void SetTree(TTree* t){m_Tree=t;}

		virtual int StopTracks(std::vector<int>& trackinds)=0;
		virtual void Reset(){}
	protected:
		TTree* m_Tree;
	};

}
