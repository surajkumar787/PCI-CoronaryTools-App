#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// This file contains several class holding nodes attributes. Those class may be combined through 
// multiple inheritance depending on the attributes used by the tracker. 
//
// For instance if the instancied tracker makes use of large scale smoothness attribute, a LtTreeNode class 
// can be written as: 
// 
//struct LtTreeNode : public Lt::LtTreeNodeAttribsBase, Lt::LtTreeNodeAttribsGlobalSmoothness{
//
//};
//////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Lt{

	/** \class LtTreeNodeAttribsBase
	*	\brief Simpliest node type. Only allows bottom up access and contains a reduce set of attributes
	*	such as the direction and length of the last segment
	*/
	struct LtTreeNodeAttribsBase{
		LtTreeNodeAttribsBase(){}
		/** First anscestor */
		int m_Father;
		/** Position of the point ending at this point*/
		float m_Pos[2];
		/** Orientation of the segment ending at this point*/
		float m_Dir;
		/** Length of the segment ending at this point*/
		float m_Length;
		/** Depth of thenode in the tree*/
		float m_Depth;
		/** Filtered orientation of the track at this - computed only */
		/* if FILTER_ANGLE_VINCENT defined, or filtered global smoothness chosen */
		float m_FilteredDir;
		#ifdef VIRTUAL_VINCENT
			/** Flag: node has generated 'aligned' sons */
			bool m_possiblyExtendVirtual;
			/** Flag: node has been chosen as to be extended (no 'aligned son' + high enough score) */
			bool m_ExtendVirtual;
		#endif
	};

	/** \class LtTreeNodeAttribsGlobalSmoothness
	*	\brief Contains attributes required if LtComputeFeatureGlobalSmoothness is used to 
	*	compute large scale smoothness attribute.
	*/
	struct LtTreeNodeAttribsGlobalSmoothness{
		/** Indice of a node (in the same track) at a given (large scale) distance of that node */
		int m_Forbear;
		/** Orientation of the current node wrt the "forbear" node */
		float m_FarDir;
	};
	
	/** \class LtTreeNodeAttribsGlobalFilteredSmoothness
	*	\brief Contains attributes required if LtComputeFeatureGlobalFilteredSmoothness is used to 
	*	compute large scale smoothness attribute.
	*/
	struct LtTreeNodeAttribsGlobalFilteredSmoothness{
		/** Indice of a node (in the same track) at a given (large scale) distance of that node */
		int m_Forbear;
	};

}