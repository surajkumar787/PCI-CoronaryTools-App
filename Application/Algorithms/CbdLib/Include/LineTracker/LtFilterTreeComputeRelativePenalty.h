#pragma once

#include "LtFilterTree.h"

namespace Lt{

	/** 
	* \class LtFilterTreeComputeRelativePenalty
	* \brief Compute a penalty from each track that depends on tracks that share a segment
	*
	* 
	*/
	template<typename TTree>
	class LtFilterTreeComputeRelativePenalty : public LtFilterTree<TTree>{
	public:
		LtFilterTreeComputeRelativePenalty(){
			m_RelScoreA = 0.f;
			m_RelScoreB = 3.f;
		}

		virtual void Filter(){
			std::vector<int> tracksinds = m_Tree->GetTracks();
//			std::vector<int> tracksinds = m_Tree->GetSleepingTracks();
			if(tracksinds.size()==0) return;

			for(unsigned int i=0 ; i<tracksinds.size() ; ++i){
				m_Tree->Track(tracksinds[i])->m_RelativePen = 1;
			}

			for(unsigned int i=0 ; i<tracksinds.size() ; ++i){
				typename TTree::TrackType* curtrack = m_Tree->Track(tracksinds[i]);
				typename TTree::NodeType* curnode = m_Tree->Node(curtrack->m_EndPoint);

				while(true){
					if(curnode->m_MaxScoreRatio == -FLT_MAX) break;
					curnode->m_MaxScoreRatio = -FLT_MAX;
					
					if(curnode->m_Father == -1) break;
					curnode = m_Tree->Node(curnode->m_Father);
				}
			}

			for(unsigned int i=0 ; i<tracksinds.size() ; ++i){
				typename TTree::TrackType* curtrack = m_Tree->Track(tracksinds[i]);
				typename TTree::NodeType* curnode = m_Tree->Node(curtrack->m_EndPoint);

				float cumulscore = 0;
				float cumullength = 0;

				while(curnode->m_Father != -1){
					cumulscore += curnode->m_Score;
					cumullength += curnode->m_Length;

					float ratio = cumulscore / cumullength;
					if(ratio > curnode->m_MaxScoreRatio)
						curnode->m_MaxScoreRatio = ratio;

					curnode = m_Tree->Node(curnode->m_Father);
				}
			}


			//tracksinds = m_Tree->GetSleepingTracks();
			for(unsigned int i=0 ; i<tracksinds.size() ; ++i){
				typename TTree::TrackType* curtrack = m_Tree->Track(tracksinds[i]);
				typename TTree::NodeType* curnode = m_Tree->Node(curtrack->m_EndPoint);

				double curscore = curtrack->m_Score;
				if(curscore<=0) continue;
				float cumulscore = 0;
				float cumullength = 0;

				float maxpen = 1;

				while(curnode->m_Father != -1){
					cumulscore += curnode->m_Score;
					cumullength += curnode->m_Length;

					float maxoscore = curnode->m_MaxScoreRatio;

					float relscore = (float)((maxoscore - (cumulscore/cumullength)) / curscore); 
					float pen = MAX(0,MIN(1,(m_RelScoreB-relscore)/(m_RelScoreB-m_RelScoreA)));
					if(pen<maxpen)maxpen=pen;

					curnode = m_Tree->Node(curnode->m_Father);
				}
				curtrack->m_RelativePen = maxpen;
				curtrack->ComputeScore();
			}


		}
		
		float m_RelScoreA;
		float m_RelScoreB;

		void Reset(){
		
		}

	};

}






/*			for(unsigned int i=0 ; i<tracksinds.size() ; ++i){
				typename TTree::TrackType* curtrack = m_Tree->Track(tracksinds[i]);
				typename TTree::NodeType* curnode = m_Tree->Node(curtrack->m_EndPoint);

				float cumulscore = 0;
				float cumullength = 0;

				float maxpen = 1;

				while(true){
					cumulscore += curnode->m_Score;
					cumullength += curnode->m_Length;

					if(curnode->m_Next != -1){
						typename TTree::NodeType* nextnode = m_Tree->Node(curnode->m_Next);

						float maxoscore = -FLT_MAX;
						while(true){
							float nextscore = ComputePen(nextnode,0,0);
							if(nextscore>maxoscore) maxoscore = nextscore;

							if(nextnode->m_Next == -1) break;
							nextnode = m_Tree->Node(nextnode->m_Next);
						}

						float relscore = (maxoscore - (cumulscore/cumullength)) / MAX(fabs(maxoscore),fabs(cumulscore/cumullength)); 
						float pen = MAX(0,MIN(1,(m_RelScoreB-relscore)/(m_RelScoreB-m_RelScoreA)));
						if(pen<maxpen)maxpen=pen;
					}

					if(curnode->m_Father == -1) break;
					curnode = m_Tree->Node(curnode->m_Father);
				}
				m_Tree->Track(tracksinds[i])->m_RelativePen = maxpen;
			}



		float ComputePen(typename TTree::NodeType* node, double cumul, float length){
			cumul+=node->m_Score;
			length+=node->m_Length;
			if(node->m_FirstChild == -1)	return cumul / length;

			float maxres = -FLT_MAX;

			typename TTree::NodeType* child = m_Tree->Node(node->m_FirstChild);
			while(true){
				float res = ComputePen(child, cumul, length);
				if(res>maxres) maxres = res;

				if(child->m_Next==-1) break;
				child = m_Tree->Node(child->m_Next);
			}

			return maxres;
		}*/
