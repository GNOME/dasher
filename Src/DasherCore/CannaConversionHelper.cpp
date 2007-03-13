#include "config.h"

#include "CannaConversionHelper.h"

#include <canna/jrkanji.h>
#include <canna/RK.h>
#include <iconv.h>

#define BUFSIZE 10240

#include <iostream>             //For testing 23 June 2005


CCannaConversionHelper::CCannaConversionHelper() {
  int ret;
  char *buf;
  int dicnum;

  IsInit = 0;

  /* Initialize */
  ret = RkInitialize("");
  if(ret < 0) {
    return;
  }
  buf = (char *)malloc(sizeof(char) * BUFSIZE);
  if(buf == NULL)
    return;

  /* Create Context ID */
  context_id = RkCreateContext();

  /* Load Dictionaries */
  dicnum = RkGetDicList(context_id, buf, BUFSIZE);      //      Find all useable dictionaries
  char *p = buf;
  for(int i = 0; i < dicnum; i++) {
    ret = RkMountDic(context_id, p, 0); //      Mount a dictionary
    if(ret)
      std::cout << "Error loading:" << p << std::endl;

    p += (strlen(p) + 1);       //      Move to next dictionary name
  }

  free(buf);
  IsInit = 1;

  //std::cout << "Init Canna OK." << endl;
}

CCannaConversionHelper::~CCannaConversionHelper() {
  RkCloseContext(context_id);   // Close working context
  //std::cout << "Finalizing Canna OK." << endl;

  /* exit */
  RkFinalize();
}

bool CCannaConversionHelper::Convert(const std::string &strSource, SCENode ** pRoot, int * childCount, int CMid) {

  if(strSource.size() == 0)
    return false;

  int nbun;
  char *pQuery = (char *)strSource.c_str();

  unsigned char *buf = (unsigned char *)malloc(sizeof(unsigned char) * BUFSIZE);
  unsigned char *str_utf8 = (unsigned char *)malloc(sizeof(unsigned char) * BUFSIZE);

  char *inbuf = (char *)pQuery;
  char *outbuf = (char *)buf;
  size_t inbytesleft = strSource.length();
  size_t outbytesleft = BUFSIZE;

  // Use EUC for Canna
  //
  // NOTE: As far as I can tell, this requires NFC rather than NFD
  // normalisation. http://www.cl.cam.ac.uk/~mgk25/unicode.html
  //
  iconv_t cd = iconv_open("EUC-JP", "UTF8");
  iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
  *outbuf = '\0';
  inbuf = (char *)buf;
  iconv_close(cd);

  /* Divide given string into phrases */
  nbun = RkBgnBun(context_id,   // context ID
                  inbuf,        // given string
                  strlen(inbuf),        // length of given string
                  (RK_XFER << RK_XFERBITS) | RK_KFER);  // mode


  if(nbun == -1) {
    // Crude error detection - I don't know enough Japanese to figure out how to do this properly :-(
    
    std::cerr << "Error - Canna conversion failed, possibly could not connect to server." << std::endl;
  }

  SCENode *pDummyRoot(new SCENode);
  pDummyRoot->pChild = NULL;

  /* Convert each phrase into Kanji */
  cd = iconv_open("UTF8", "EUC-JP");
  for(int i = nbun-1; i >= 0; --i) {
    SCENode *pTail = pDummyRoot->pChild;

    RkGoTo(context_id, i);      // Move to a specific phrase
    int len = RkGetKanjiList(context_id, buf, BUFSIZE); // Get a list of Kanji candidates

    // Use UTF-8 for Dasher
    char *p = (char *)buf;

    std::vector<std::string> vCandidates;

    for(int j = 0; j < len; ++j) {
      inbuf = p;
      //std::cout << "Canna:" << j << "[" << inbuf << "] ";
      outbuf = (char *)str_utf8;
      inbytesleft = strlen((char *)inbuf);
      outbytesleft = BUFSIZE;
      //for( int k=0; k<20; k++ ){
      //      std::cout << (int) inbuf[k] << " ";
      //}
      //std::cout << inbytesleft << " ->";
      iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
      *outbuf = '\0';
      
      if(strlen((char *)str_utf8)) 
	vCandidates.push_back((char *)str_utf8);

      //std::cout << "[" << str_utf8 << "] " << outbytesleft << std::endl;
      p += (strlen(p) + 1);
    }

    for(std::vector<std::string>::reverse_iterator it(vCandidates.rbegin()); it != vCandidates.rend(); ++it) {
      ProcessCandidate(*it, pDummyRoot, pTail);
    }
  }
  RkEndBun(context_id, 0);      // Close phrase division

  iconv_close(cd);
  free(buf);
  free(str_utf8);

  *pRoot = pDummyRoot->pChild;

  delete pDummyRoot;

  return true;
}


void CCannaConversionHelper::ProcessCandidate(std::string strCandidate, SCENode *pRoot, SCENode *pTail) {

  SCENode *pCurrentNode(pRoot);

  int iIdx(0);
      
  // TODO: Need phrase-based conversion
  while(iIdx < strCandidate.size()) {
	
    int iLength;
	
    // TODO: Really dodgy UTF-8 parser - find a library routine to do this
    if((static_cast<int>(strCandidate[iIdx]) & 0x80) == 0)
      iLength = 1;
    else if((static_cast<int>(strCandidate[iIdx]) & 0xE0) == 0xC0) 
      iLength = 2;
    else if((static_cast<int>(strCandidate[iIdx]) & 0xF0) == 0xE0)
      iLength = 3;
    else if((static_cast<int>(strCandidate[iIdx]) & 0xF8) == 0xF0)
      iLength = 4;
    else if((static_cast<int>(strCandidate[iIdx]) & 0xFC) == 0xF8)
      iLength = 5;
    else
      iLength = 6;

    std::string strSymbol(strCandidate.substr(iIdx, iLength));

    iIdx += iLength;

    SCENode *pCurrentChild(pCurrentNode->pChild); // TODO: Initialise

    while(pCurrentChild) {
      if(strSymbol == pCurrentChild->pszConversion)
	break;
      pCurrentChild = pCurrentChild->pNext;
    }

    if(!pCurrentChild) { // Need a new child
      pCurrentChild = new SCENode;
      pCurrentChild->pNext = pCurrentNode->pChild;
      if(iIdx >= strCandidate.size())
	pCurrentChild->pChild = pTail;
      else
	pCurrentChild->pChild = NULL;

      pCurrentChild->pszConversion = new char[strSymbol.size() + 1];
      strcpy(pCurrentChild->pszConversion, strSymbol.c_str());

      pCurrentNode->pChild = pCurrentChild;
    }

    pCurrentNode = pCurrentChild;
  }
}

void CCannaConversionHelper::AssignSizes(SCENode *pStart, Dasher::CLanguageModel::Context context, long normalization, int uniform, int iNChildren) {

  SCENode *pNode(pStart);

  int iRemaining = iNChildren;
  int iLeft = normalization;
  
  int iCheck(0);

  while(pNode) {
    pNode->NodeSize = iLeft / iRemaining;
    iLeft -= pNode->NodeSize;

    iCheck += pNode->NodeSize;
    
    --iRemaining;
    pNode = pNode->pNext;
  }
}
