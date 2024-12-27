#pragma once

namespace Lt{

	/** 
	* \class LtExtendTrack
	* \brief Abstract class for track extension algorithms.
	*/
	template<typename TTree>
	class LtExtendTrack{
	public:
		typedef TTree TreeType;
		
		LtExtendTrack(){}
		virtual ~LtExtendTrack(){}
		void SetTree(TTree* t){m_Tree=t;}

		/** Create new tracks from the track designated by trackind. The new tracks have one extra node. 
		The result may depend on the orientation of the track endpoint (that must be appropriatly filled). */
		virtual void ExtendTrack(int trackind)=0;
		/** Create new tracks from the track designated by trackind. The new tracks have one extra node. 
		The result doesn't depend on the orientation of the track endpoint. Therefore that function can be used to initialize 
		a new track from a seed position. */
		virtual void ExtendTrackWOInitialDir(int trackind)=0;

		virtual void Reset(){}

		#ifdef VIRTUAL_VINCENT
			int maxNbTracks;	// Useful for virtual
		#endif

	protected:
		TTree* m_Tree;

	};

}