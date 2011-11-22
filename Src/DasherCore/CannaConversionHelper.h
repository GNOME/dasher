#ifndef __CANNA_CONVERSION_HELPER_H__
#define __CANNA_CONVERSION_HELPER_H__

#define BUFSIZE 10240

#include "ConversionManager.h"
#include <iconv.h>

#include <iostream> 
/// \ingroup Model
/// @{
class CCannaConversionHelper : public Dasher::CConversionManager {
 public:
  CCannaConversionHelper(CNodeCreationManager *pNCManager, Dasher::const CAlphInfo *pAlphabet, int Type, int Order);
  ~CCannaConversionHelper();

  virtual bool Convert(const std::string &strSource, SCENode ** pRoot);


  virtual void AssignSizes(SCENode ** pStart, Dasher::CLanguageModel::Context context, long normalization, int uniform, int iNChildren);


  virtual Dasher::CLanguageModel * GetLanguageModel() { return NULL; };



/*   virtual bool GetPhraseList(int HZIndex, SCENode ** psOutput, int CMid) { */
/*     return false; */
/*   }; */

/*   virtual void BuildDataBase() { */
/*   }; */

   virtual void ClearData(int CMid) { 
   }; 
  
/*   virtual std::vector<std::vector<std::vector<std::vector<std::vector<int> > > > > * GetDP(int CMid) { */
/*     return NULL; */
/*   };//get data pointer */

 private:
  void ProcessCandidate(std::string strCandidate, SCENode *pRoot, SCENode *pTail);

  unsigned int iUTF8Decode(const std::string &strChar) {
    int iLength = strChar.size();

    unsigned int iCode = 0;

    switch(iLength) {
    case 0:
      // Invalid - return 0;
      iCode = 0;
      break;
    case 1:
      iCode =  static_cast<int>(strChar[0]) & 127;
      break;
    case 2:
      iCode = static_cast<int>(strChar[1]) & 63 + ((static_cast<int>(strChar[0]) & 31) << 6);
      break;
    case 3:
      iCode = static_cast<int>(strChar[2]) & 63 + ((static_cast<int>(strChar[1]) & 63) << 6) + 
	((static_cast<int>(strChar[0]) & 15) << 12);
      break;
    case 4:
      // Invalid (as far as we're concerned) - return 0;
      iCode = 0;
      break;
    }

    return iCode;
  }

//Kazue 


  #define JMS1(c) (((((unsigned char)(c))>=0x81)&&(((unsigned char)(c))<=0x9F))||((((unsigned char)(c))>=0xE0)&&(((unsigned char)(c))<=0xFC)))
  #define JMS2(c) ((((unsigned char)(c))!=0x7F)&&(((unsigned char)(c))>=0x40)&&(((unsigned char)(c))<=0xFC))

  int isJMS( const char *str, int nPos )
  {
        int i;
        int state; // { 0, 1, 2 } = { 1byte, 1st of 2byte, 2nd of 2byte }

        state = 0;
        for( i = 0; str[i] != '\0'; i++ )
        {
                if      ( ( state == 0 ) && ( JMS1( str[i] ) ) ) state = 1; // 0 -> 1
                else if ( ( state == 1 ) && ( JMS2( str[i] ) ) ) state = 2; // 1 -> 2
                else if ( ( state == 2 ) && ( JMS1( str[i] ) ) ) state = 1; // 2 -> 1
                else                                             state = 0; // 2 -> 0, others

                if ( i == nPos ) return state;
        }
        return 0;
  }//isJMS

  #define MAKEJMS(lead,trail)  ((unsigned short)(((unsigned char)(trail) & 0xff) | (((unsigned char)(lead) & 0xff) << 8)))

  int jmscode( const char *p )
  {
        if ( JMS1( p[0] ) && ('\0' != p[1]) )
                return MAKEJMS(p[0],p[1]);

        return (unsigned char)p[0];
  }//jmscode   

  unsigned int iShiftJISDecode(const std::string &strChar) {
    
    if(strChar.size() == 0)
     return false;

    char *pQuery = (char *)strChar.c_str();

    unsigned char *buf = (unsigned char *)malloc(sizeof(unsigned char) * BUFSIZE);

    char *inbuf = (char *)pQuery;
    char *outbuf = (char *)buf;
    size_t inbytesleft = strChar.length();
    size_t outbytesleft = BUFSIZE;

    //iconv_t cd = iconv_open("SJIS", "UTF8");
    //iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
    iconv(this->icon, &inbuf, &inbytesleft, &outbuf, &outbytesleft);

    const std::string strSJIS = (char *)buf;

    *outbuf = '\0';
    inbuf = (char *)buf;
    //iconv_close(cd);
    
    int iCode = 0;
    
    if(strSJIS.size() == 2)
       iCode = jmscode((char *)buf);
   
    return iCode;
  }
 
  int context_id;
  bool IsInit;

  int iType; // 0 = uniform, 1 = 1/(n+1),2 = 1/(n+1)/n
  int iOrder; // 0 = canna, 1 = Unicode , 2 = Shift_JIS
  
  iconv_t icon;
 
};
/// @}
#endif
