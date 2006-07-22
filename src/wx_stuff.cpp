// << ------------------------------------ >>
// << SLADE - SlayeR's LeetAss Doom Editor >>
// << By Simon Judd, 2004-06               >>
// << ------------------------------------ >>
// << wx_stuff.cpp - WxWidgets related     >>
// << ------------------------------------ >>

// Includes ------------------------------ >>
#include "main.h"
#include "wx_stuff.h"

// wx_to_str: Converts a wxString to a std::string (also from unicode to ascii)
// ------------------------------------------------------------------------- >>
string wx_to_str(wxString str)
{
	string ret;

	wxCharBuffer buffer = str.ToAscii();
	const char* data = buffer;
	ret = data;

	return ret;
}

// str_to_wx: Converts a std::string to a wxString
// -------------------------------------------- >>
wxString str_to_wx(string str)
{
#if wxUSE_UNICODE
	if(!str.length())
		return wxString();

	wchar_t *multibyteString = new wchar_t[str.length() * 5];
	mbstowcs(multibyteString, str.c_str(), str.length() * 5);
	wxString ret(multibyteString);
	delete[] multibyteString;
	return ret;
#else
	return wxString(str.c_str());
#endif
}
