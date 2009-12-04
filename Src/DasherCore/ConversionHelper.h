// ConversionHelper.h
//
// Copyright (c) 2007 The Dasher Team
//
// This file is part of Dasher.
//
// Dasher is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Dasher is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Dasher; if not, write to the Free Software 
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef __CONVERSION_HELPER_H__
#define __CONVERSION_HELPER_H__

#include "ConversionManager.h"
#include <string>
#include <vector>
#include "SCENode.h"
#include "LanguageModelling/LanguageModel.h"
#include "LanguageModelling/PPMLanguageModel.h"


namespace Dasher{
	class CDasherNode;  //forward declaration

/// \ingroup Model
/// @{

/// The CConversionHelper class represents the language specific
/// aspect of the conversion process. Each CConversionManager
/// references one helper, which performs the conversion itself, as
/// well as assigning weights to each of the predictions. See
/// CConversionManager for further details of the conversion process.
///
  class CConversionHelper : public CConversionManager {
  public:
	CConversionHelper(CNodeCreationManager *pNCManager, CAlphabet *pAlphabet, CLanguageModel *pLanguageModel);
	
	/// Convert a given string to a lattice of candidates. Sizes for
	/// candidates aren't assigned at this point. The input string
	/// should be UTF-8 encoded.
	///
	/// @param strSource UTF-8 encoded input string.
	/// @param pRoot Used to return the root of the conversion lattice.
	/// @param childCount Unsure - document this.
	/// @param CMid A unique identifier for the conversion helper 'context'.
	///
	/// @return True if conversion succeeded, false otherwise
	virtual bool Convert(const std::string &strSource, SCENode ** pRoot) = 0;
	
	/// Assign sizes to the children of a given conversion node. This
	/// happens when the conversion manager populates the children of
	/// the Dasher node so as to avoid unnecessary computation.
	///
	/// @param pStart The parent of the nodes to be sized.
	/// @param context Unsure - document this, shouldn't be in general class (include userdata pointer).
	/// @param normalization Normalisation constant for the child sizes (TODO: check that this is a sensible value, ie the same as Dasher normalisation).
	/// @param uniform Unsure - document this.
	/// @param iNChildren The number of children to be expected (more efficient than iterating linked list).
	///
	virtual void AssignSizes(SCENode ** pStart, Dasher::CLanguageModel::Context context, long normalization, int uniform, int iNChildren)=0;
		
	/// Assign colours to the children of a given conversion node.
	/// This function needs a rethink.
	///
	/// @param parentClr 
	/// @param pNode 
	/// @param childIndex 
	///
	/// @return 
	/// 
	virtual int AssignColour(int parentClr, SCENode * pNode, int childIndex) {
		int which = -1;
		
		for (int i=0; i<2; i++)
			for(int j=0; j<3; j++)
				if (parentClr == colourStore[i][j])
					which = i;
		
		if(which == -1)
			return colourStore[0][childIndex%3];
		else if(which == 0)
			return colourStore[1][childIndex%3];
		else 
			return colourStore[0][childIndex%3]; 
	};
	
    ///
    /// Get a new root node owned by this manager
    ///
	
    virtual CConvNode *GetRoot(CDasherNode *pParent, int iLower, int iUpper, int iOffset);
	
    ///
    /// Calculate sizes for each of the children - default
    /// implementation assigns decending probabilities in a power law
    /// fashion (so assumes ordering), but specific subclasses are
    /// free to implement their own behaviour. The only restriction is
    /// that sizes should be positive and sum to the appropriate
    /// normalisation constant
    ///
    
    virtual void AssignChildSizes(SCENode **pNode, CLanguageModel::Context context, int iNChildren);
	
	protected:
    class CConvHNode : public CConvNode {
    public:
      CConvHNode(CDasherNode *pParent, int iLbnd, int iHbnd, CDasherNode::SDisplayInfo *pDispInfo, CConversionHelper *pMgr);
      ///
      /// Provide children for the supplied node
      ///
      virtual void PopulateChildren();
    	
    virtual void SetFlag(int iFlag, bool bValue);
    protected:
      inline CConversionHelper *mgr() {return static_cast<CConversionHelper *>(m_pMgr);}
    };
	  virtual CConvHNode *makeNode(CDasherNode *pParent, int iLbnd, int iHbnd, CDasherNode::SDisplayInfo *pDispInfo);
    /// 
    /// Build the conversion tree (lattice) for the given string -
    /// evaluated late to prevent unnecessary conversions when the
    /// children of the root node are never instantiated
    ///
    
    virtual void BuildTree(CConvHNode *pRoot);
	
    virtual Dasher::CLanguageModel *GetLanguageModel() {
      return m_pLanguageModel;
    }
    
  private:
    CLanguageModel *m_pLanguageModel;
	
    CLanguageModel::Context m_iLearnContext;
	
	int colourStore[2][3]; 
  };
/// @}
}
#endif
