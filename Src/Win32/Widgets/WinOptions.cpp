#include "WinOptions.h"

#include "../WinUTF8.h"
using namespace WinUTF8;

/* TODO: Consider using Template functions to make this neater. */


CWinOptions::CWinOptions(const std::string& Group, const std::string& Product)
{
	// Windows requires strings as Tstring
	Tstring TGroup, TProduct;
	UTF8string_to_Tstring(Group, &TGroup);
	UTF8string_to_Tstring(Product, &TProduct);
	
	// Get hold of HKEY_CURRENT_USER\Software
	HKEY SoftwareKey;
	if (GetOrCreate(HKEY_CURRENT_USER, TEXT("Software"), KEY_WRITE, &SoftwareKey)!=0) {
		// Can't open or create key - do something...
		// Probably flag registry as failed and just return default options
		exit(1); // give up for now.
	}
	
	// Then HKEY_CURRENT_USER\Software\<Group>
	HKEY GroupKey;
	if (GetOrCreate(SoftwareKey, TGroup.c_str(), KEY_WRITE, &GroupKey)!=0) {exit(1);}
	RegCloseKey(SoftwareKey);
	
	// Then HKEY_CURRENT_USER\Software\<Group>\<Product>
	if (GetOrCreate(GroupKey, TProduct.c_str(), KEY_ALL_ACCESS, &ProductKey)!=0) {exit(1);}
	RegCloseKey(GroupKey);
}


CWinOptions::~CWinOptions()
{
	RegCloseKey(ProductKey);
}



/* Private functions -- Used to load and save to the Windows Registry
--------------------------------------------------------------------------*/


bool CWinOptions::LoadSetting(const std::string& Key, bool* Value)
{
	Tstring TKey;
	UTF8string_to_Tstring(Key, &TKey);
	BYTE* Data=0;
	
	if (!GetlpByte(TKey, &Data)) {
		delete[] Data;
		return false;
	}

	if ((DWORD)*Data==0)
		*Value = false;
	else
		*Value = true;
	
	delete[] Data;
	return true;
}


bool CWinOptions::LoadSetting(const std::string& Key, long* Value)
{
	Tstring TKey;
	UTF8string_to_Tstring(Key, &TKey);
	BYTE* Data=0;
		
	if (!GetlpByte(TKey, &Data)) {
		delete[] Data;
		return false;
	}

	// Evil casting to make sure I can retrieve signed longs, even
	// though windows registry only stores +ve values.
	*Value = *((long*) Data);
	delete[] Data;
	return true;
}


bool CWinOptions::LoadSetting(const std::string& Key, std::string* Value)
{
	Tstring TKey;
	UTF8string_to_Tstring(Key, &TKey);
	BYTE* Data=0;
	
	if (!GetlpByte(TKey, &Data)) {
		delete[] Data;
		return false;
	}
	
	*Value = (char*)Data;
	delete[] Data;
	return true;
}


void CWinOptions::SaveSetting(const std::string& Key, bool Value)
{
	if (Value)
		SaveSetting(Key, 1l);
	else
		SaveSetting(Key, 0l);
}


void CWinOptions::SaveSetting(const std::string& Key, long Value)
{
	Tstring TKey;
	UTF8string_to_Tstring(Key, &TKey);
	// Evil casting. Registry stores DWORD's (unsigned longs)
	// I'm forcing in signed longs and if I force them out again in the same
	// way I should get a sensible result.
	DWORD* RegValue = (DWORD*) &Value;
	DWORD MemAllow = sizeof(DWORD);
	LONG ErrVal = RegSetValueEx(ProductKey, TKey.c_str(), 0,
		REG_DWORD, (const unsigned char*) RegValue, MemAllow);
}


void CWinOptions::SaveSetting(const std::string& Key, const std::string& Value)
{
	Tstring TKey;
	UTF8string_to_Tstring(Key, &TKey);
	
	DWORD MemAllow = (Value.size()+1) * sizeof(char);
	
	const unsigned char* StrInput = (const unsigned char*) Value.c_str();
	LONG ErrVal = RegSetValueEx(ProductKey, TKey.c_str(), 0,
		REG_SZ, StrInput, MemAllow);
}


// Used for getting a handle on the Dasher key in the constructor.
int CWinOptions::GetOrCreate(HKEY hKey, LPCTSTR lpSubKey, REGSAM samDesired, HKEY* lpNewKey)
{
	if (!(RegOpenKeyEx(hKey, lpSubKey,
	0, samDesired, lpNewKey)==ERROR_SUCCESS)) {
	
		if (!(RegCreateKeyEx(hKey, lpSubKey,
			0, NULL, REG_OPTION_NON_VOLATILE, samDesired,
			NULL, lpNewKey, NULL)==ERROR_SUCCESS)) {
			
			return(1);
		}
	}
	return 0;
}


bool CWinOptions::GetlpByte(const Tstring& Key, BYTE** Data) const
{
	*Data = new BYTE[1];
	unsigned long datasize = sizeof(BYTE[1]);
	DWORD Type;
	
	LONG ErrVal = RegQueryValueEx(ProductKey, Key.c_str(), 0, &Type, *Data, &datasize);
	while (ErrVal==ERROR_MORE_DATA) {
		delete[] *Data;
		*Data = new BYTE[datasize];
		ErrVal = RegQueryValueEx(ProductKey, Key.c_str(), 0, &Type, *Data, &datasize);
	}
	
	// Perhaps I should spend lots of time working out why the registry doesn't work.
	// when this fails. Would probably just confuse users though. Users with a broken
	// registry can live with defaults :)
	if ((ErrVal==ERROR_SUCCESS) && (*Data!=0))
		return true;
	else
		return false;
}
