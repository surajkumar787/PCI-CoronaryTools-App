#pragma once

#include <vector>

///////////////////////////////////////////////////////////////////////////////
// Only used to export tracks.
// Not used by the tracked
///////////////////////////////////////////////////////////////////////////////
namespace Lt{
	struct LtPt{
		LtPt(float x,float y,float angle):X(x),Y(y),A(angle){}
		//floating point position in the image
		float X;
		float Y;
		float A;
	};

	struct LtTrack{
		LtTrack(){m_Pts.reserve(30);}
		LtTrack(int size){m_Pts.reserve(size);}
		int Size(){return (int)m_Pts.size();}
		void Clear(){m_Pts.clear();}

		std::vector<LtPt> m_Pts;
	};

	struct LtTrackV{
		LtTrackV(){m_X.reserve(30);m_Y.reserve(30);}
		LtTrackV(int size){m_X.reserve(size);m_Y.reserve(size);}
		int Size(){return (int)m_X.size();}
		void Clear(){m_X.clear();m_Y.clear();}

		std::vector<float> m_X;
		std::vector<float> m_Y;
	};
}
