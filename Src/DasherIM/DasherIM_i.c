/* this file contains the actual definitions of */
/* the IIDs and CLSIDs */

/* link this file in with the server and any clients */


/* File created by MIDL compiler version 5.01.0164 */
/* at Sun Jun 10 22:43:47 2007
 */
/* Compiler settings for .\DasherIM.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

const IID IID_ICDasherIM = {0xFF1A0DAE,0xD689,0x42BC,{0x8E,0xAB,0xC2,0x1A,0xA7,0xA7,0x5B,0xB9}};


const IID LIBID_DasherIMLib = {0x805B97E3,0x3254,0x42F8,{0x96,0x46,0x2A,0xAA,0xC4,0x71,0xC7,0x2B}};


const CLSID CLSID_CDasherIM = {0x3DA2F5A4,0x6992,0x4BD3,{0xAC,0xB4,0xCA,0xE1,0x17,0xE0,0x01,0xD0}};


#ifdef __cplusplus
}
#endif

