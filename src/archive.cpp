
#include "main.h"
#include "wx_stuff.h"

#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/ptr_scpd.h>

extern string app_path;

/*
void testy()
{
	// 'smart pointer' type created with wxDEFINE_SCOPED_PTR_TYPE
	//wxZipEntryPtr entry;
	wxZipEntry* entry;

	wxFFileInputStream in(_T("data.zip"));
	wxZipInputStream zip(in);

	entry = zip.GetNextEntry();
	while (entry)
	{
		// access meta-data
		string name = wx_to_str(entry->GetName(wxPATH_UNIX));
		log_message(name);

		// read 'zip' to access the entry's data
		if (entry->GetSize() > 0)
		{
			BYTE* buffer = new BYTE[entry->GetSize()];
			zip.Read(buffer, entry->GetSize());
			FILE *fp = fopen(name.c_str(), "wb");
			if (fp)
			{
				fwrite(buffer, entry->GetSize(), 1, fp);
				fclose(fp);
			}
			delete buffer;
		}

		entry = zip.GetNextEntry();
	}
}
*/

// dump_from_pk3: Finds 'entry' in slade.pk3 and dumps it to sladetemp, 
// returns false if 'entry' isn't found in slade.pk3
// ----------------------------------------------------------------- >>
bool dump_from_pk3(string entry_name)
{
	wxZipEntry* entry;

	wxFFileInputStream in(wxString::Format(_T("%sslade.pk3"), app_path.c_str()));
	wxZipInputStream zip(in);

	entry = zip.GetNextEntry();
	while (entry)
	{
		string name = wx_to_str(entry->GetName(wxPATH_UNIX));

		if (entry->GetSize() > 0 && name == entry_name)
		{
			BYTE* buffer = new BYTE[entry->GetSize()];
			zip.Read(buffer, entry->GetSize());
			FILE *fp = fopen("sladetemp", "wb");
			fwrite(buffer, entry->GetSize(), 1, fp);
			fclose(fp);
			delete[] buffer;
			return true;
		}

		entry = zip.GetNextEntry();
	}

	return false;
}
