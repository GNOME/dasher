// KanjiConversionCanna.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 Takashi Kaburagi
//
/////////////////////////////////////////////////////////////////////////////

#include "../../Common/Common.h"

#include "KanjiConversionCanna.h"

#include <canna/jrkanji.h>
#include <canna/RK.h>
#include <iconv.h>

#include "KanjiConversion.h"

#define BUFSIZE 10240

#include <iostream>             //For testing 23 June 2005

using namespace Dasher;
using namespace std;

CKanjiConversionCanna::CKanjiConversionCanna() {
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
      cout << "Error loading:" << p << endl;

    p += (strlen(p) + 1);       //      Move to next dictionary name
  }

  free(buf);
  IsInit = 1;

  //cout << "Init Canna OK." << endl;
}

CKanjiConversionCanna::~CKanjiConversionCanna() {
  RkCloseContext(context_id);   // Close working context
  //cout << "Finalizing Canna OK." << endl;

  /* exit */
  RkFinalize();
}

int CKanjiConversionCanna::ConvertKanji(std::string str) {
  int nbun;
  char *pQuery = (char *)str.c_str();

  unsigned char *buf = (unsigned char *)malloc(sizeof(unsigned char) * BUFSIZE);
  unsigned char *str_utf8 = (unsigned char *)malloc(sizeof(unsigned char) * BUFSIZE);

  char *inbuf = (char *)pQuery;
  char *outbuf = (char *)buf;
  size_t inbytesleft = str.length();
  size_t outbytesleft = BUFSIZE;

  // Use EUC for Canna
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

  /* Convert each phrase into Kanji */
  cd = iconv_open("UTF8", "EUC-JP");
  for(int i = 0; i < nbun; i++) {
    CPhrase new_phrase;
    RkGoTo(context_id, i);      // Move to a specific phrase
    int len = RkGetKanjiList(context_id, buf, BUFSIZE); // Get a list of Kanji candidates

    // Use UTF-8 for Dasher
    char *p = (char *)buf;
    for(int j = 0; j < len; j++) {
      inbuf = p;
      //cout << "Canna:" << j << "[" << inbuf << "] ";
      outbuf = (char *)str_utf8;
      inbytesleft = strlen((char *)inbuf);
      outbytesleft = BUFSIZE;
      //for( int k=0; k<20; k++ ){
      //      cout << (int) inbuf[k] << " ";
      //}
      //cout << inbytesleft << " ->";
      iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
      *outbuf = '\0';
      if(strlen((char *)str_utf8))
        new_phrase.candidate_list.push_back((char *)str_utf8);
      //cout << "[" << str_utf8 << "] " << outbytesleft << endl;
      p += (strlen(p) + 1);
    }
    phrase.push_back(new_phrase);
    //CopyCandidate( &pSen->list_phrase[i], buf, len);
  }
  RkEndBun(context_id, 0);      // Close phrase division

  iconv_close(cd);
  free(buf);
  free(str_utf8);
  return 0;
}
