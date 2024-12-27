#pragma once

#include "LtTrack.h"

#include <set>
#include <vector>

namespace Lt{

	namespace treedetails{
		/** \class AbstractTreeArrayStorage
		*	\brief Base class for tree representation. Manage allocation and access to nodes
		*/
		template<typename TNode>
		class AbstractTreeArrayStorage{
		public:
			AbstractTreeArrayStorage(int nballocatednodes){
				m_AllocatedNodes.resize(nballocatednodes);
				m_FreeNodes.reserve(nballocatednodes);
				for(int i=nballocatednodes-1 ; i>=0 ; --i) m_FreeNodes.push_back(i);
			}
			virtual ~AbstractTreeArrayStorage(){}

			inline TNode* Node(int i){return &m_AllocatedNodes[i];}

			int Reset(){
				m_FreeNodes.clear();
				m_FreeNodes.reserve(m_AllocatedNodes.size());
				for(int i=(int)m_AllocatedNodes.size()-1 ; i>=0 ; --i) m_FreeNodes.push_back(i);
				return 0;
			}

			int RemoveNode(int nodeind){
				m_FreeNodes.push_back(nodeind);
				return 0;
			}

			int AddNode(int &nodeind){
				bool resize = false;
				if(m_FreeNodes.size()==0){
					int prevsize = (int)m_AllocatedNodes.size();
					m_AllocatedNodes.resize(2*prevsize);
					for(int i=2*prevsize-1 ; i>=0 ; --i) m_FreeNodes.push_back(i);
					std::cout<<"warning resize m_AllocatedNodes"<<std::endl;
					resize = true;
				}

				nodeind = m_FreeNodes.back();
				m_FreeNodes.pop_back();
				if(resize)return 1;
				else return 0;
			}

			std::vector<int> m_FreeNodes;
			std::vector<TNode> m_AllocatedNodes;
		};
		
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Basic tree struct. Only allows bottom up access
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		template<typename TAttrib>
		class SimpleNode : public TAttrib{
		public:
			SimpleNode(){}
			/** First anscestor */
			int m_Father;
		};

		template<typename TNodeAttribs>
		class SimpleTree : public AbstractTreeArrayStorage<SimpleNode<TNodeAttribs>>{
		public:
			typedef SimpleNode<TNodeAttribs> NodeType;

			SimpleTree(int nballocatednodes):AbstractTreeArrayStorage(nballocatednodes){}

			void LinkNode(int nodeind, int fatherind){
				Node(nodeind)->m_Father = fatherind;
			}
		};

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Usual tree struct. Allows bottom up and top down access
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		template<typename TAttrib>
		struct StdTreeNode : public TAttrib{
			StdTreeNode(){}
			/** First anscestor */
			int m_Father;
			
			int m_FirstChild;
			int m_Next;
		};

		template<typename TNodeAttribs>
		class StdTree : public AbstractTreeArrayStorage<StdTreeNode<TNodeAttribs>>{
		public:
			typedef StdTreeNode<TNodeAttribs> NodeType;

			StdTree(int nballocatednodes):AbstractTreeArrayStorage(nballocatednodes){
			}

			void LinkNode(int nodeind, int fatherind){
				NodeType* n = Node(nodeind);
				n->m_Father = fatherind;
				n->m_FirstChild = -1;
				n->m_Next = -1;

				if(fatherind == -1){
					if(std::find(m_Roots.begin() , m_Roots.end() , nodeind) == m_Roots.end() ) 
						m_Roots.push_back(nodeind);
					return;
				}

				NodeType* f = Node(fatherind);
				if(f->m_FirstChild == -1){
					f->m_FirstChild = nodeind;
					n->m_Next = -1;
				}else{
					int childind = f->m_FirstChild;
					while(Node(childind)->m_Next!=-1){
						childind = Node(childind)->m_Next;
					}
					Node(childind)->m_Next = nodeind;
				}

			}

			int Reset(){
				m_Roots.clear();
				return AbstractTreeArrayStorage::Reset();
			}

			std::vector<int> m_Roots;
		};


	}




	/** \class LtTreeTrack
	*	\brief track type used within the tree. The template class contains the track attributes and a function to compute the score. 
	*/
	template<typename TAttrib>
	struct SimpleTrack : public TAttrib{
		SimpleTrack& operator=(const SimpleTrack& o){
			m_EndPoint = o.m_EndPoint;
			TAttrib::operator=(o);
			return *this;
		}

		int m_EndPoint;

		void Reset(){
			m_EndPoint = -1;
			TAttrib::Reset();
		};
	};


	/** \class LtTree
	*	\brief Minimalist tree structure. It allows only bottom up parsing of the tree
	*
	*	The leaves and the nodes are stored in distinct containers as their type differ.
	*
	*/
	template<typename TTrackAttribs, typename TNodeAttribs, typename TBasicTreeStruct = treedetails::SimpleTree<TNodeAttribs> > 
	class LtTree : public TBasicTreeStruct{
	public:
		//////////////////////////////////////////////////////////////////////////
		// typedefs
		//////////////////////////////////////////////////////////////////////////
		typedef typename TBasicTreeStruct::NodeType NodeType;
		typedef SimpleTrack<TTrackAttribs> TrackType;

		//////////////////////////////////////////////////////////////////////////
		// Constructor 
		//////////////////////////////////////////////////////////////////////////
		LtTree(int nballocednodes,int nballocatedtracks);
		~LtTree();

		void Reset();

		//////////////////////////////////////////////////////////////////////////
		// Accessors
		//////////////////////////////////////////////////////////////////////////
		inline std::vector<int> GetRunningTracks();
		inline std::vector<int> GetSleepingTracks();
		inline std::vector<int> GetTracks();
		long GetNumberOfRunningTracks();
		long GetNumberOfSleepingTracks();
		long GetNumberOfTracks();

		inline TrackType* Track(int i){return &m_AllocatedTracks[i];}

		int GetTrack(int trackind, LtTrack* track);
		int GetTrack(int trackind, LtTrackV* track);

		//////////////////////////////////////////////////////////////////////////
		// Editing functions
		//////////////////////////////////////////////////////////////////////////
		/** Mark a track as free so that it may be reused*/
		int ReleaseTrack(int trackind);

		int RemoveTrack(int trackind);

		/** Add a track and returns the id*/
		int AddTrack(int &trackind);

		/** */
		int SendTrackToSleep(int trackind);

		//////////////////////////////////////////////////////////////////////////
		// Containers
		//////////////////////////////////////////////////////////////////////////
		std::set<int> m_RunningTracks;
		std::set<int> m_SleepingTracks;
		std::vector<int> m_FreeTracks;

		std::vector<TrackType> m_AllocatedTracks;
	};



}

#include "LtTree.txx"

