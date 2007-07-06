#include <cstdio>
#include <expat.h>
#include <iostream>
#include <string>
#include <vector>

class CPinyinParser {
public:
  CPinyinParser();

std::string g_strCurrentGroup;
std::string g_strLastGroup;

static void XML_StartElement(void *userData, const XML_Char * name, const XML_Char ** atts);
static void XML_EndElement(void *userData, const XML_Char * name);

std::vector<std::string> *ParseGroupName(const std::string &strName);
void ParsePhrase(const std::string &strPhrase);

std::vector<std::string> *pCurrentList;
class CTrieNode {
public:
  CTrieNode(char cSymbol) {
    m_pChild = NULL;
    m_pNext = NULL;
    m_pList = NULL;

    m_cSymbol = cSymbol;
  };

  void AddChild(CTrieNode *pNewChild) {
    pNewChild->SetNext(m_pChild);
    m_pChild = pNewChild;
  };

  void SetNext(CTrieNode *pNewNode) {
    m_pNext = pNewNode;
  };

  CTrieNode *GetNext() {
    return m_pNext;
  };

  char GetSymbol() {
    return m_cSymbol;
  };

  void SetList(std::vector<std::string> *pList) {
    m_pList = pList;
  }

  std::vector<std::string> *GetList() {
    return m_pList;
  }

  CTrieNode *LookupChild(char cSymbol) {
    CTrieNode *pCurrentChild = m_pChild;

    while(pCurrentChild) {
      if(pCurrentChild->GetSymbol() == cSymbol)
	return pCurrentChild;
      pCurrentChild = pCurrentChild->GetNext();
    }

    return NULL;
  };

  void RecursivelyDump(int iDepth) {
    for(int i(0); i < iDepth; ++i)
      std::cout << " ";
    
    std::cout << m_cSymbol << std::endl;

    CTrieNode *pCurrentChild = m_pChild;

    while(pCurrentChild) {
      pCurrentChild->RecursivelyDump(iDepth + 1);
      pCurrentChild = pCurrentChild->GetNext();
    }
  }

private:
  CTrieNode *m_pChild;
  CTrieNode *m_pNext;

  std::vector<std::string> *m_pList;

  char m_cSymbol;
};

  class CLatticeNode {
  public:
  char m_cSymbol;
  // It actually makes more sense here to work backwards, so store pointers to parent
  CLatticeNode *m_pParent;
  std::vector<std::string> *m_pList;
};

CTrieNode *g_pRoot;

};

int main(int argc, char **argv) {  
  //g_pRoot->RecursivelyDump(0);

  CPinyinParser *pParser = new CPinyinParser;

  pParser->ParsePhrase("shanghai");


  return 0;
}

CPinyinParser::CPinyinParser() {
  g_strLastGroup = "";
  pCurrentList = NULL;

  g_pRoot = new CTrieNode('0');

  FILE *Input;
  if((Input = fopen("Data/alphabets/alphabet.chineseRuby.xml", "r")) == (FILE *) 0) {
    // could not open file
    return;
  }

  XML_Parser Parser = XML_ParserCreate(NULL);
  XML_SetUserData(Parser, this);
  XML_SetElementHandler(Parser, XML_StartElement, XML_EndElement);

  const unsigned int BufferSize = 1024;
  char Buffer[BufferSize];
  int Done;
  do {
    size_t len = fread(Buffer, 1, sizeof(Buffer), Input);
    Done = len < sizeof(Buffer);
    if(XML_Parse(Parser, Buffer, len, Done) == XML_STATUS_ERROR) {
      break;
    }
  } while(!Done);
}

void 
CPinyinParser::XML_StartElement(void *userData, const XML_Char * name, const XML_Char ** atts) {

  CPinyinParser *pThis = static_cast<CPinyinParser *>(userData);

  if(!strcmp("group", name)) {
     while(*atts != 0) {
      if(!strcmp("name", *atts)) {
	pThis->pCurrentList = pThis->ParseGroupName(*(atts + 1));
      }
      atts += 2;
    }
  }
  else if(!strcmp("s", name)) {
    while(*atts != 0) {
      if(!strcmp("t", *atts)) {
	if(pThis->pCurrentList)
	  pThis->pCurrentList->push_back(*(atts + 1));
      }
      atts += 2;
    }
  }
}

void 
CPinyinParser::XML_EndElement(void *userData, const XML_Char * name) {
}

std::vector<std::string> *CPinyinParser::ParseGroupName(const std::string &strName) {
  int i1 = strName.find('(');
  int i2 = strName.find(')');

  if((i1 > 0) && (i2 > 0)) {

    std::string strSymbol;

    
    if(isdigit(strName[i2-1]))
      strSymbol = strName.substr(i1+1, i2-i1-2);
    else
      strSymbol = strName.substr(i1+1, i2-i1-1);

    std::string strShortName = strSymbol;

    strSymbol += '1';

    std::string strTone = strName.substr(i2-1,1);
    int iTone = atoi(strTone.c_str());

    CTrieNode *pCurrentNode = g_pRoot;

    for(std::string::iterator it = strSymbol.begin(); it != strSymbol.end(); ++it) {
      CTrieNode *pChild = pCurrentNode->LookupChild(*it);

      if(!pChild) {
	pChild = new CTrieNode(*it);
	pCurrentNode->AddChild(pChild);
      }

      pCurrentNode = pChild;
    }

    std::vector<std::string> *pList;

    // TODO: It seems like we're getting double instances of some here

    // Handle same symbol with different tones
    if(strShortName == g_strLastGroup) 
      pList = pCurrentList; 
    else
      pList = new std::vector<std::string>;

    g_strLastGroup = strShortName;

    pCurrentNode->SetList(pList);
    return pList;
  }
  else {
    return NULL;
  }
}

void CPinyinParser::ParsePhrase(const std::string &strPhrase) {

  // TODO: Need to implement real latice here with merges

  typedef std::pair<CTrieNode *, CLatticeNode *> tLPair;

  std::vector<tLPair> *pCurrentList = new std::vector<tLPair>;
  pCurrentList->push_back(tLPair(g_pRoot, NULL));

  for(std::string::const_iterator it = strPhrase.begin(); it != strPhrase.end(); ++it) {
    std::vector<tLPair> *pNewList = new std::vector<tLPair>;

    for(std::vector<tLPair>::iterator it2 = pCurrentList->begin(); it2 != pCurrentList->end(); ++it2) {

      // First see if we can directly continue:
      CTrieNode *pCurrentChild = it2->first->LookupChild(*it);

      if(pCurrentChild) {
	CLatticeNode *pNewLNode = new CLatticeNode;
	pNewLNode->m_cSymbol = pCurrentChild->GetSymbol();
	pNewLNode->m_pParent = it2->second;
	pNewLNode->m_pList = NULL;

	pNewList->push_back(tLPair(pCurrentChild, pNewLNode));
      }

      // Now see if we can start a new symbol here
      pCurrentChild = it2->first->LookupChild('1');

      if(pCurrentChild) {
	CTrieNode *pCurrentChild2 = g_pRoot->LookupChild(*it);
	if(pCurrentChild2) {
	  CLatticeNode *pNewLNode = new CLatticeNode;
	  pNewLNode->m_cSymbol = '|';
	  pNewLNode->m_pParent = it2->second;
	  pNewLNode->m_pList = pCurrentChild->GetList();
	  
	  CLatticeNode *pNewLNode2 = new CLatticeNode;
	  pNewLNode2->m_cSymbol = pCurrentChild->GetSymbol();
	  pNewLNode2->m_pParent = pNewLNode;
	  pNewLNode2->m_pList = NULL;

	  pNewList->push_back(tLPair(pCurrentChild2, pNewLNode2));
	}
      }
    }

    delete pCurrentList;
    pCurrentList = pNewList;
  }

  // Now trace back through the remaining paths
  for(std::vector<tLPair>::iterator it2 = pCurrentList->begin(); it2 != pCurrentList->end(); ++it2) {

    CLatticeNode *pCurrentNode = it2->second;

    // Only print conversions which terminate at the end of a sylable
    if(it2->first->LookupChild('1')) {
      

      std::vector<std::vector<std::string> *> vCurrentPhrase;

      vCurrentPhrase.push_back(it2->first->LookupChild('1')->GetList());

      while(pCurrentNode) {
	if(pCurrentNode->m_pList)
	  vCurrentPhrase.push_back(pCurrentNode->m_pList);
	pCurrentNode = pCurrentNode->m_pParent;
      }

      for(std::vector<std::vector<std::string> *>::reverse_iterator it = vCurrentPhrase.rbegin(); 
	  it != vCurrentPhrase.rend(); ++it) {

	std::cout << "(";

	//	std::cout << *it << " - ";

	for(std::vector<std::string>::iterator j_it = (*it)->begin(); j_it != (*it)->end(); ++j_it) 
	  std::cout << *j_it;

	std::cout << ")";

      }

      std::cout << std::endl;
    }

  }

}
