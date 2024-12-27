#include <iostream>

namespace Lt{

	template<typename TTrackAttribs, typename TNodeAttribs, typename TBasicTreeStruct>
	LtTree<TTrackAttribs,TNodeAttribs,TBasicTreeStruct>::LtTree(int nballocatednodes,int nballocatedtracks) : TBasicTreeStruct(nballocatednodes){
		m_AllocatedTracks.resize(nballocatedtracks);
		m_FreeTracks.reserve(nballocatedtracks);
		for(int i=nballocatedtracks-1 ; i>=0 ; --i) m_FreeTracks.push_back(i);
	}

	template<typename TTrackAttribs, typename TNodeAttribs, typename TBasicTreeStruct>
	LtTree<TTrackAttribs,TNodeAttribs,TBasicTreeStruct>::~LtTree(){

	}

	template<typename TTrackAttribs, typename TNodeAttribs, typename TBasicTreeStruct>
	void LtTree<TTrackAttribs,TNodeAttribs,TBasicTreeStruct>::Reset(){
		TBasicTreeStruct::Reset();
		m_FreeTracks.insert(m_FreeTracks.end() , m_RunningTracks.begin() , m_RunningTracks.end());
		m_FreeTracks.insert(m_FreeTracks.end() , m_SleepingTracks.begin() , m_SleepingTracks.end());
		m_RunningTracks.clear();
		m_SleepingTracks.clear();
	}

	template<typename TTrackAttribs, typename TNodeAttribs, typename TBasicTreeStruct>
	typename std::vector<int> LtTree<TTrackAttribs,TNodeAttribs,TBasicTreeStruct>::GetTracks(){
		std::vector<int> res;
		res.reserve(m_RunningTracks.size()+m_SleepingTracks.size());
		res.insert(res.end(),m_RunningTracks.begin(),m_RunningTracks.end());
		res.insert(res.end(),m_SleepingTracks.begin(),m_SleepingTracks.end());
		return res;
	}

	template<typename TTrackAttribs, typename TNodeAttribs, typename TBasicTreeStruct>
	typename std::vector<int> LtTree<TTrackAttribs,TNodeAttribs,TBasicTreeStruct>::GetRunningTracks(){
		return std::vector<int>(m_RunningTracks.begin(),m_RunningTracks.end());
	}

	template<typename TTrackAttribs, typename TNodeAttribs, typename TBasicTreeStruct>
	typename std::vector<int> LtTree<TTrackAttribs,TNodeAttribs,TBasicTreeStruct>::GetSleepingTracks(){
		return std::vector<int>(m_SleepingTracks.begin(),m_SleepingTracks.end());
	}

	template<typename TTrackAttribs, typename TNodeAttribs, typename TBasicTreeStruct>
	long LtTree<TTrackAttribs,TNodeAttribs,TBasicTreeStruct>::GetNumberOfTracks(){
		return m_RunningTracks.size()+m_SleepingTracks.size();
	}

	template<typename TTrackAttribs, typename TNodeAttribs, typename TBasicTreeStruct>
	long LtTree<TTrackAttribs,TNodeAttribs,TBasicTreeStruct>::GetNumberOfRunningTracks(){
		return m_RunningTracks.size();
	}

	template<typename TTrackAttribs, typename TNodeAttribs, typename TBasicTreeStruct>
	long LtTree<TTrackAttribs,TNodeAttribs,TBasicTreeStruct>::GetNumberOfSleepingTracks(){
		return m_SleepingTracks.size();
	}

	template<typename TTrackAttribs, typename TNodeAttribs, typename TBasicTreeStruct>
	int LtTree<TTrackAttribs,TNodeAttribs,TBasicTreeStruct>::ReleaseTrack(int trackind){
		m_FreeTracks.push_back(trackind);
		return 0;
	}

	template<typename TTrackAttribs, typename TNodeAttribs, typename TBasicTreeStruct>
	int LtTree<TTrackAttribs,TNodeAttribs,TBasicTreeStruct>::RemoveTrack(int trackind){
		m_RunningTracks.erase(trackind);
		m_SleepingTracks.erase(trackind);
		m_FreeTracks.push_back(trackind);
		return 0;
	}

	template<typename TTrackAttribs, typename TNodeAttribs, typename TBasicTreeStruct>
	int LtTree<TTrackAttribs,TNodeAttribs,TBasicTreeStruct>::AddTrack(int &trackind){
		bool resize = false;
		if(m_FreeTracks.size()==0){
			int prevsize = (int)m_AllocatedTracks.size();
			m_AllocatedTracks.resize(2*prevsize);
			for(int i=prevsize ; i<2*prevsize ; ++i) m_FreeTracks.push_back(i);
			std::cout<<"warning resize m_AllocatedTracks"<<std::endl;
			resize = true;
		}
		trackind = m_FreeTracks.back();
		m_FreeTracks.pop_back();
		m_RunningTracks.insert(trackind);
		if(resize)return 1;
		else return 0;
	}

	template<typename TTrackAttribs, typename TNodeAttribs, typename TBasicTreeStruct>
	int LtTree<TTrackAttribs,TNodeAttribs,TBasicTreeStruct>::SendTrackToSleep(int trackind){
		m_RunningTracks.erase(trackind);
		m_SleepingTracks.insert(trackind);
		return 0;
	}

	template<typename TTrackAttribs, typename TNodeAttribs, typename TBasicTreeStruct>
	int LtTree<TTrackAttribs,TNodeAttribs,TBasicTreeStruct>::GetTrack(int trackind, LtTrack* restrack){
		TrackType* track = Track(trackind);

		restrack->Clear();
		NodeType* curnode;
		int curind = track->m_EndPoint;
		
		while(curind != -1){
			curnode = Node(curind);
			restrack->m_Pts.push_back(LtPt(curnode->m_Pos[0],curnode->m_Pos[1],curnode->m_Dir));
			curind = curnode->m_Father;
		}

		return 0;
	}

	template<typename TTrackAttribs, typename TNodeAttribs, typename TBasicTreeStruct>
	int LtTree<TTrackAttribs,TNodeAttribs,TBasicTreeStruct>::GetTrack(int trackind, LtTrackV* restrack){
		TrackType* track = Track(trackind);
		NodeType* endpoint = Node(track->m_EndPoint);

		restrack->Clear();
		restrack->m_X.push_back(endpoint->m_Pos[0]);
		restrack->m_Y.push_back(endpoint->m_Pos[1]);
		NodeType* curnode;
		int curind = endpoint->m_Father;
		
		while(curind != -1){
			curnode = Node(curind);
			restrack->m_X.push_back(curnode->m_Pos[0]);
			restrack->m_Y.push_back(curnode->m_Pos[1]);
			curind = curnode->m_Father;
		}

		return 0;
	}
}