#pragma once

#include "LtPruningTracks.h"
#include <map>

namespace Lt{

	/** \class LtPruningTracksHeadDistance
	*	\brief Select N according to their score and to the distance of their endpoint
	*
	*	Iteratively we select the best track according to its score and we penalize the tracks close to it.
	*	
	*	TODO improve performances by freezing tests of tracks that are far enough
	*/
	template<typename TTree>
	class LtPruningTracksHeadDistance : public LtPruningTracks<TTree>{
	public:
		LtPruningTracksHeadDistance();

		std::map<int,int>* m_DistMatInds;
		float** m_DistMat;

		typedef struct{
			double score;
			int ind;
			bool remove;
			float pos[2];
			int indnode;
			int indprevnode;
		} TIndScore;

		struct IndScoreOrder {
			bool operator()(TIndScore i,TIndScore j) { return (j.score<i.score); }
		} indScoreOrder;

		virtual void Prune();
		virtual void Reset();

		float m_PenDistA;
		float m_PenDistB;
	};

}

#include "LtPruningTracksHeadDistance.txx"
