
#include "main.h"
#include "wx_stuff.h"
#include "memlump.h"

#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/ptr_scpd.h>
#include <wx/image.h>

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

	wxFFileInputStream in(str_to_wx(c_path("slade.pk3", DIR_APP)));
	wxZipInputStream zip(in);

	entry = zip.GetNextEntry();
	while (entry)
	{
		string name = wx_to_str(entry->GetName(wxPATH_UNIX));

		if (entry->GetSize() > 0 && name == entry_name)
		{
			BYTE* buffer = new BYTE[entry->GetSize()];
			zip.Read(buffer, entry->GetSize());
			FILE *fp = fopen(c_path("sladetemp", DIR_TMP).c_str(), "wb");
			fwrite(buffer, entry->GetSize(), 1, fp);
			fclose(fp);
			delete[] buffer;
			return true;
		}

		entry = zip.GetNextEntry();
	}

	return false;
}

bool get_from_pk3(MemLump &ml, string entry_name)
{
	wxZipEntry* entry;

	wxFFileInputStream in(str_to_wx(c_path("slade.pk3", DIR_APP)));
	wxZipInputStream zip(in);

	entry = zip.GetNextEntry();
	while (entry)
	{
		string name = wx_to_str(entry->GetName(wxPATH_UNIX));

		if (entry->GetSize() > 0 && name == entry_name)
		{
			ml.allocate(entry->GetSize(), true);
			zip.Read(ml.get_data(), ml.get_size());
			return true;
		}

		entry = zip.GetNextEntry();
	}

	log_message(s_fmt("Couldn't find %s in slade.pk3", entry_name.c_str()));
	return false;
}

MemLump* get_from_pk3(string entry_name)
{
	MemLump* ret = new MemLump();
	wxZipEntry* entry;

	wxFFileInputStream in(str_to_wx(c_path("slade.pk3", DIR_APP)));
	wxZipInputStream zip(in);

	entry = zip.GetNextEntry();
	while (entry)
	{
		string name = wx_to_str(entry->GetName(wxPATH_UNIX));

		if (entry->GetSize() > 0 && name == entry_name)
		{
			ret->allocate(entry->GetSize(), true);
			zip.Read(ret->get_data(), ret->get_size());
			return ret;
		}

		entry = zip.GetNextEntry();
	}

	log_message(s_fmt("Couldn't find %s in slade.pk3", entry_name.c_str()));
	return ret;
}

/*
wxImage* get_image_from_pk3(string entry_name, int type)
{
	wxZipEntry* entry;

	wxFFileInputStream in(str_to_wx(c_path("slade.pk3", DIR_APP)));
	wxZipInputStream zip(in);

	entry = zip.GetNextEntry();
	while (entry)
	{
		string name = wx_to_str(entry->GetName(wxPATH_UNIX));

		if (entry->GetSize() > 0 && name == entry_name)
			return new wxImage(zip, type);

		entry = zip.GetNextEntry();
	}

	log_message(s_fmt("Couldn't find %s in slade.pk3", entry_name.c_str()));
	return new wxImage();
}
*/

wxImage get_image_from_pk3(string entry_name, int type)
{
	wxImage ret;

	wxZipEntry* entry;

	wxFFileInputStream in(str_to_wx(c_path("slade.pk3", DIR_APP)));
	wxZipInputStream zip(in);

	entry = zip.GetNextEntry();
	while (entry)
	{
		string name = wx_to_str(entry->GetName(wxPATH_UNIX));

		if (entry->GetSize() > 0 && name == entry_name)
		{
			if (ret.LoadFile(zip, type))
				break;
		}

		entry = zip.GetNextEntry();
	}

	log_message(s_fmt("Couldn't find %s in slade.pk3", entry_name.c_str()));
	return ret;
}
