
#include "Main.h"
#include "WxStuff.h"
#include "TranslationEditorDialog.h"
#include "Icons.h"
#include "MathStuff.h"
#include "Tokenizer.h"
#include <wx/ffile.h>
#include <wx/gbsizer.h>




GradientBox::GradientBox(wxWindow* parent, int steps) : OGLCanvas(parent, -1) {
	// Init variables
	col_start = COL_BLACK;
	col_end = COL_WHITE;
	this->steps = steps;

	// Minimum height 16
	SetInitialSize(wxSize(-1, 16));
}

GradientBox::~GradientBox() {
}

void GradientBox::draw() {
	// Setup the viewport
	glViewport(0, 0, GetSize().x, GetSize().y);

	// Setup the screen projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, GetSize().x, GetSize().y, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Clear
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// Translate to middle of pixel (otherwise inaccuracies can occur on certain gl implemenataions)
	glTranslatef(0.375f, 0.375f, 0);

	// Draw gradient
	if (steps < 0) {
		// No steps defined, draw smooth gradient
		glBegin(GL_QUADS);
		col_start.set_gl();
		glVertex2d(0, 0);
		glVertex2d(0, GetSize().y);
		col_end.set_gl();
		glVertex2d(GetSize().x, GetSize().y);
		glVertex2d(GetSize().x, 0);
		glEnd();
	}

	// Swap buffers (ie show what was drawn)
	SwapBuffers();
}





TranslationEditorDialog::TranslationEditorDialog(wxWindow* parent, Palette8bit* pal, string title, ArchiveEntry* preview_image)
: wxDialog(parent, -1, title) {
	// Init variables
	palette = pal;

	// Create sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(sizer);


	// --- Top half (translation origin) ---

	// Translations list
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(hbox, 0, wxEXPAND|wxALL, 4);

	wxStaticBox *frame = new wxStaticBox(this, -1, "Translation Ranges");
	wxStaticBoxSizer *framesizer = new wxStaticBoxSizer(frame, wxHORIZONTAL);
	hbox->Add(framesizer, 0, wxEXPAND|wxALL, 4);

	list_translations = new wxListBox(this, -1);
	framesizer->Add(list_translations, 1, wxEXPAND|wxLEFT|wxBOTTOM|wxRIGHT, 4);

	// Add translation button
	wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);
	framesizer->Add(vbox, 0, wxEXPAND|wxBOTTOM|wxRIGHT, 4);

	btn_add = new wxBitmapButton(this, -1, getIcon("t_plus"));
	vbox->Add(btn_add, 0, wxEXPAND|wxBOTTOM, 4);

	// Remove translation button
	btn_remove = new wxBitmapButton(this, -1, getIcon("t_minus"));
	vbox->Add(btn_remove, 0, wxEXPAND|wxBOTTOM, 4);

	// Move up button
	btn_up = new wxBitmapButton(this, -1, getIcon("t_up"));
	vbox->Add(btn_up, 0, wxEXPAND|wxBOTTOM, 4);

	// Move down button
	btn_down = new wxBitmapButton(this, -1, getIcon("t_down"));
	vbox->Add(btn_down, 0, wxEXPAND);


	// Origin range
	frame = new wxStaticBox(this, -1, "Origin Range");
	framesizer = new wxStaticBoxSizer(frame, wxHORIZONTAL);
	hbox->Add(framesizer, 1, wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 4);

	// Origin palette
	pal_canvas_original = new PaletteCanvas(this, -1);
	pal_canvas_original->doubleWidth(true);
	pal_canvas_original->setPalette(palette);
	pal_canvas_original->SetInitialSize(wxSize(514, 130));
	framesizer->Add(pal_canvas_original, 1, wxEXPAND|wxLEFT|wxBOTTOM|wxRIGHT, 4);


	// --- Bottom half (translation target) ---

	// Target type
	hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	frame = new wxStaticBox(this, -1, "Target Range Type");
	framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	hbox->Add(framesizer, 0, wxEXPAND|wxBOTTOM|wxLEFT|wxRIGHT, 4);

	// Palette range
	rb_type_palette = new wxRadioButton(this, -1, "Palette Range", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	framesizer->Add(rb_type_palette, 0, wxEXPAND|wxALL, 4);

	// Colour gradient
	rb_type_colour = new wxRadioButton(this, -1, "Colour Gradient");
	framesizer->Add(rb_type_colour, 0, wxEXPAND|wxLEFT|wxRIGHT, 4);

	// Desaturated colour gradient
	rb_type_desaturate = new wxRadioButton(this, -1, "Desaturated Colour Gradient");
	framesizer->Add(rb_type_desaturate, 0, wxEXPAND|wxALL, 4);


	// Target range
	frame = new wxStaticBox(this, -1, "Target Range");
	framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	hbox->Add(framesizer, 1, wxEXPAND|wxBOTTOM|wxRIGHT, 4);


	// Target palette range panel
	panel_target_palette = new wxPanel(this, -1);
	vbox = new wxBoxSizer(wxVERTICAL);
	panel_target_palette->SetSizer(vbox);

	// Target palette
	pal_canvas_target = new PaletteCanvas(panel_target_palette, -1);
	pal_canvas_target->doubleWidth(true);
	pal_canvas_target->setPalette(palette);
	pal_canvas_target->SetInitialSize(wxSize(514, 130));
	vbox->Add(pal_canvas_target, 1, wxEXPAND);


	// Target colour gradient panel
	panel_target_gradient = new wxPanel(this, -1);
	vbox = new wxBoxSizer(wxVERTICAL);
	panel_target_gradient->SetSizer(vbox);

	// Start colour
	vbox->AddStretchSpacer();
	hbox = new wxBoxSizer(wxHORIZONTAL);
	vbox->Add(hbox, 0, wxEXPAND|wxBOTTOM, 4);

	cp_range_begin = new wxColourPickerCtrl(panel_target_gradient, -1, WXCOL(COL_BLACK));
	hbox->Add(cp_range_begin, 0, wxEXPAND|wxRIGHT, 4);
	hbox->Add(new wxStaticText(panel_target_gradient, -1, "From"), 0, wxALIGN_CENTER_VERTICAL);

	// End colour
	cp_range_end = new wxColourPickerCtrl(panel_target_gradient, -1, WXCOL(COL_WHITE));
	hbox->AddStretchSpacer();
	hbox->Add(new wxStaticText(panel_target_gradient, -1, "To"), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	hbox->Add(cp_range_end, 0, wxEXPAND);

	// Gradient preview
	gb_gradient = new GradientBox(panel_target_gradient);
	vbox->Add(gb_gradient, 0, wxEXPAND);
	vbox->AddStretchSpacer();


	// --- Dialog buttons ---
	wxSizer* buttonsizer = CreateButtonSizer(wxOK|wxCANCEL);
	sizer->Add(buttonsizer, 0, wxEXPAND|wxALL, 4);

	// Load button
	btn_load = new wxButton(this, -1, "Load from File");
	buttonsizer->InsertStretchSpacer(0);
	buttonsizer->Insert(0, btn_load, 0, wxRIGHT, 4);

	// Save button
	btn_save = new wxButton(this, -1, "Save to File");
	buttonsizer->Insert(1, btn_save, 0);


	// Show initial target panel (palette)
	framesizer->Add(panel_target_palette, 1, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);
	panel_target_gradient->Show(false);


	// Bind events
	list_translations->Bind(wxEVT_COMMAND_LISTBOX_SELECTED, &TranslationEditorDialog::onTranslationListItemSelected, this);
	rb_type_palette->Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, &TranslationEditorDialog::onRBPaletteSelected, this);
	rb_type_colour->Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, &TranslationEditorDialog::onRBColourSelected, this);
	rb_type_desaturate->Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, &TranslationEditorDialog::onRBDesaturateSelected, this);
	cp_range_begin->Bind(wxEVT_COMMAND_COLOURPICKER_CHANGED, &TranslationEditorDialog::onBeginColourChanged, this);
	cp_range_end->Bind(wxEVT_COMMAND_COLOURPICKER_CHANGED, &TranslationEditorDialog::onEndColourChanged, this);
	pal_canvas_original->Bind(wxEVT_LEFT_UP, &TranslationEditorDialog::onPalOriginLeftUp, this);
	pal_canvas_target->Bind(wxEVT_LEFT_UP, &TranslationEditorDialog::onPalTargetLeftUp, this);
	btn_add->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TranslationEditorDialog::onBtnAdd, this);
	btn_remove->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TranslationEditorDialog::onBtnRemove, this);
	btn_up->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TranslationEditorDialog::onBtnUp, this);
	btn_down->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TranslationEditorDialog::onBtnDown, this);
	btn_load->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TranslationEditorDialog::onBtnLoad, this);
	btn_save->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TranslationEditorDialog::onBtnSave, this);

	// Setup layout
	Layout();
	SetInitialSize(wxSize(-1, -1));
	SetMinSize(GetSize());
	list_translations->SetSizeHints(list_translations->GetSize(), list_translations->GetSize());
}

TranslationEditorDialog::~TranslationEditorDialog() {
}

void TranslationEditorDialog::openTranslation(Translation& trans) {
	// Read translation
	translation.copy(trans);

	// Update translation list
	list_translations->Clear();
	for (unsigned a = 0; a < translation.nRanges(); a++)
		list_translations->Append(translation.getRange(a)->asText());

	// Select+open first range if it exists
	if (list_translations->GetCount() > 0) {
		list_translations->SetSelection(0);
		openRange(0);
	}

	// Update preview
	//if (frame_preview)
	//	frame_preview->updatePreview(translation, palette);
}

void TranslationEditorDialog::openRange(int index) {
	// Check index
	if (index < 0 || index >= (int)translation.nRanges())
		return;

	// Get translation range to open
	TransRange* tr = translation.getRange(index);

	// Set origin selection
	pal_canvas_original->setSelection(tr->oStart(), tr->oEnd());
	pal_canvas_original->Refresh();

	// Check translation range type
	if (tr->getType() == TRANS_PALETTE) {
		// Palette range
		TransRangePalette* tpr = (TransRangePalette*)tr;

		// Select palette type radiobox
		rb_type_palette->SetValue(true);
		showPaletteTarget();

		// Set target range selection
		pal_canvas_target->setSelection(tpr->dStart(), tpr->dEnd());
		pal_canvas_target->Refresh();
	}
	else if (tr->getType() == TRANS_COLOUR) {
		// Colour gradient
		TransRangeColour* tcr = (TransRangeColour*)tr;

		// Select colour type radiobox
		rb_type_colour->SetValue(true);
		showGradientTarget();

		// Set beginning colour
		gb_gradient->setStartCol(tcr->dStart());
		cp_range_begin->SetColour(WXCOL(tcr->dStart()));

		// Set ending colour
		gb_gradient->setEndCol(tcr->dEnd());
		cp_range_end->SetColour(WXCOL(tcr->dEnd()));

		// Update UI
		gb_gradient->Refresh();
	}
	else if (tr->getType() == TRANS_DESAT) {
		// Desaturated colour gradient
		TransRangeDesat* tdr = (TransRangeDesat*)tr;

		// Select desaturated colour type radiobox
		rb_type_desaturate->SetValue(true);
		showGradientTarget();

		// Set beginning colour
		rgba_t col;
		col.r = MathStuff::clamp(tdr->dSr() * 128, 0, 255);
		col.g = MathStuff::clamp(tdr->dSg() * 128, 0, 255);
		col.b = MathStuff::clamp(tdr->dSb() * 128, 0, 255);
		cp_range_begin->SetColour(WXCOL(col));
		gb_gradient->setStartCol(col);

		// Set ending colour
		col.r = MathStuff::clamp(tdr->dEr() * 128, 0, 255);
		col.g = MathStuff::clamp(tdr->dEg() * 128, 0, 255);
		col.b = MathStuff::clamp(tdr->dEb() * 128, 0, 255);
		cp_range_end->SetColour(WXCOL(col));
		gb_gradient->setEndCol(col);

		// Update UI
		gb_gradient->Refresh();
	}
}

void TranslationEditorDialog::updateListItem(int index) {
	// Check index
	if (index < 0 || index >= (int)list_translations->GetCount())
		return;

	// Get translation range
	TransRange* tr = translation.getRange(index);

	// Update list item text
	if (tr) {
		list_translations->Delete(index);
		list_translations->Insert(tr->asText(), index);
		list_translations->SetSelection(index);
	}
}

void TranslationEditorDialog::setStartColour(rgba_t col) {
	// Get currently selected translation range
	TransRange* tr = translation.getRange(list_translations->GetSelection());

	// Check it's type
	if (tr->getType() == TRANS_COLOUR) {
		// Colour range
		TransRangeColour* tcr = (TransRangeColour*)tr;

		// Set destination start colour
		tcr->setDStart(col);
	}
	else if (tr->getType() == TRANS_DESAT) {
		// Desaturated colour range
		TransRangeDesat* tdr = (TransRangeDesat*)tr;

		// Determine start colour
		float sr = MathStuff::clamp(col.r / 127.0f, 0, 2);
		float sg = MathStuff::clamp(col.g / 127.0f, 0, 2);
		float sb = MathStuff::clamp(col.b / 127.0f, 0, 2);

		// Set destination start colour
		tdr->setDStart(sr, sg, sb);
	}

	// Set gradient box end colour
	gb_gradient->setStartCol(col);

	// Update UI
	gb_gradient->Refresh();
	updateListItem(list_translations->GetSelection());

	// Update preview
	//if (frame_preview)
	//	frame_preview->updatePreview(translation, palette);
}

void TranslationEditorDialog::setEndColour(rgba_t col) {
	// Get currently selected translation range
	TransRange* tr = translation.getRange(list_translations->GetSelection());

	// Check it's type
	if (tr->getType() == TRANS_COLOUR) {
		// Colour range
		TransRangeColour* tcr = (TransRangeColour*)tr;

		// Set destination end colour
		tcr->setDEnd(col);
	}
	else if (tr->getType() == TRANS_DESAT) {
		// Desaturated colour range
		TransRangeDesat* tdr = (TransRangeDesat*)tr;

		// Determine end colour
		float er = MathStuff::clamp(col.r / 127.0f, 0, 2);
		float eg = MathStuff::clamp(col.g / 127.0f, 0, 2);
		float eb = MathStuff::clamp(col.b / 127.0f, 0, 2);

		// Set destination end colour
		tdr->setDEnd(er, eg, eb);
	}

	// Set gradient box end colour
	gb_gradient->setEndCol(col);

	// Update UI
	gb_gradient->Refresh();
	updateListItem(list_translations->GetSelection());

	// Update preview
	//if (frame_preview)
	//	frame_preview->updatePreview(translation, palette);
}

void TranslationEditorDialog::showPaletteTarget() {
	// Swap gradient panel for palette panel
	if (panel_target_gradient->IsShown()) {
		panel_target_gradient->Show(false);
		GetSizer()->Replace(panel_target_gradient, panel_target_palette, true);
		panel_target_palette->Show(true);

		// Update UI
		Layout();
		SetInitialSize(wxSize(-1, -1));
		SetMinSize(GetSize());
	}
}

void TranslationEditorDialog::showGradientTarget() {
	// Swap palette panel for gradient panel
	if (panel_target_palette->IsShown()) {
		panel_target_palette->Show(false);
		GetSizer()->Replace(panel_target_palette, panel_target_gradient, true);
		panel_target_gradient->Show(true);

		// Update UI
		Layout();
		SetInitialSize(wxSize(-1, -1));
		SetMinSize(GetSize());
	}
}



void TranslationEditorDialog::onTranslationListItemSelected(wxCommandEvent& e) {
	// Open what was selected
	openRange(e.GetInt());
}

void TranslationEditorDialog::onRBPaletteSelected(wxCommandEvent& e) {
	// Swap to target palette panel
	showPaletteTarget();

	// If a range is selected
	int index = list_translations->GetSelection();
	if (index >= 0) {
		// Remove it
		translation.removeRange(index);

		// Recreate it
		translation.addRange(TRANS_PALETTE, index);
		TransRangePalette* tr = (TransRangePalette*)translation.getRange(index);
		tr->setOStart(pal_canvas_original->getSelectionStart());
		tr->setOEnd(pal_canvas_original->getSelectionEnd());
		tr->setDStart(pal_canvas_target->getSelectionStart());
		tr->setDEnd(pal_canvas_target->getSelectionEnd());

		// Update UI
		updateListItem(index);
		openRange(index);

		// Update preview
		//if (frame_preview)
		//	frame_preview->updatePreview(translation, palette);
	}
}

void TranslationEditorDialog::onRBColourSelected(wxCommandEvent& e) {
	// Swap to target colour panel
	showGradientTarget();

	// If a range is selected
	int index = list_translations->GetSelection();
	if (index >= 0) {
		// Remove it
		translation.removeRange(index);

		// Recreate it
		translation.addRange(TRANS_COLOUR, index);
		TransRangeColour* tr = (TransRangeColour*)translation.getRange(index);
		tr->setOStart(pal_canvas_original->getSelectionStart());
		tr->setOEnd(pal_canvas_original->getSelectionEnd());
		wxColour sc = cp_range_begin->GetColour();
		wxColour ec = cp_range_end->GetColour();
		tr->setDStart(rgba_t(sc.Red(), sc.Green(), sc.Blue()));
		tr->setDEnd(rgba_t(ec.Red(), ec.Green(), ec.Blue()));

		// Update UI
		updateListItem(index);
		openRange(index);

		// Update preview
		//if (frame_preview)
		//	frame_preview->updatePreview(translation, palette);
	}
}

void TranslationEditorDialog::onRBDesaturateSelected(wxCommandEvent& e) {
	// Swap to target colour panel
	showGradientTarget();

	// If a range is selected
	int index = list_translations->GetSelection();
	if (index >= 0) {
		// Remove it
		translation.removeRange(index);

		// Recreate it
		translation.addRange(TRANS_DESAT, index);
		TransRangeDesat* tr = (TransRangeDesat*)translation.getRange(index);
		tr->setOStart(pal_canvas_original->getSelectionStart());
		tr->setOEnd(pal_canvas_original->getSelectionEnd());
		wxColour sc = cp_range_begin->GetColour();
		wxColour ec = cp_range_end->GetColour();
		tr->setDStart(MathStuff::clamp(sc.Red() / 127.0f, 0, 2),
						MathStuff::clamp(sc.Green() / 127.0f, 0, 2),
						MathStuff::clamp(sc.Blue() / 127.0f, 0, 2));
		tr->setDEnd(MathStuff::clamp(ec.Red() / 127.0f, 0, 2),
						MathStuff::clamp(ec.Green() / 127.0f, 0, 2),
						MathStuff::clamp(ec.Blue() / 127.0f, 0, 2));

		// Update UI
		updateListItem(index);
		openRange(index);

		// Update preview
		//if (frame_preview)
		//	frame_preview->updatePreview(translation, palette);
	}
}

void TranslationEditorDialog::onBeginColourChanged(wxColourPickerEvent& e) {
	// Set start colour to selected colour
	wxColour col = cp_range_begin->GetColour();
	setStartColour(rgba_t(col.Red(), col.Green(), col.Blue()));
}

void TranslationEditorDialog::onEndColourChanged(wxColourPickerEvent& e) {
	// Set end colour to selected colour
	wxColour col = cp_range_end->GetColour();
	setEndColour(rgba_t(col.Red(), col.Green(), col.Blue()));
}

void TranslationEditorDialog::onPalOriginLeftUp(wxMouseEvent& e) {
	// Get current translation range
	TransRange* tr = translation.getRange(list_translations->GetSelection());

	// Update it's origin range
	if (tr) {
		tr->setOStart(pal_canvas_original->getSelectionStart());
		tr->setOEnd(pal_canvas_original->getSelectionEnd());
	}

	// Update UI
	updateListItem(list_translations->GetSelection());

	// Update preview
	//if (frame_preview)
	//	frame_preview->updatePreview(translation, palette);
}

void TranslationEditorDialog::onPalTargetLeftUp(wxMouseEvent& e) {
	// Get current translation range
	TransRange* tr = translation.getRange(list_translations->GetSelection());

	// Update it's target range if it's a palette translation
	if (tr && tr->getType() == TRANS_PALETTE) {
		TransRangePalette* tpr = (TransRangePalette*)tr;
		tpr->setDStart(pal_canvas_target->getSelectionStart());
		tpr->setDEnd(pal_canvas_target->getSelectionEnd());
	}

	// Update UI
	updateListItem(list_translations->GetSelection());

	// Update preview
	//if (frame_preview)
	//	frame_preview->updatePreview(translation, palette);
}

void TranslationEditorDialog::onBtnAdd(wxCommandEvent& e) {
	// Get index to add at
	int index = list_translations->GetSelection()+1;

	// Add new range to translation depending on current type selection
	if (rb_type_palette->GetValue())
		translation.addRange(TRANS_PALETTE, index);
	else if (rb_type_colour->GetValue())
		translation.addRange(TRANS_COLOUR, index);
	else
		translation.addRange(TRANS_DESAT, index);

	// Add it to the list
	list_translations->Insert(translation.getRange(index)->asText(), index);

	// Update UI
	list_translations->SetSelection(index);
	openRange(index);

	// Update preview
	//if (frame_preview)
	//	frame_preview->updatePreview(translation, palette);
}

void TranslationEditorDialog::onBtnRemove(wxCommandEvent& e) {
	// Get index of range to remove
	int index = list_translations->GetSelection();

	// Remove it from the translation
	translation.removeRange(index);

	// Remove it from the list
	list_translations->Delete(index);

	// Update UI
	if (index >= (int)list_translations->GetCount())
		index--;
	if (index >= 0) {
		list_translations->SetSelection(index);
		openRange(index);
	}

	// Update preview
	//if (frame_preview)
	//	frame_preview->updatePreview(translation, palette);
}

void TranslationEditorDialog::onBtnUp(wxCommandEvent& e) {
	// Get selection
	int index = list_translations->GetSelection();

	// Check selection is valid
	if (index <= 0)
		return;

	// Swap item with above
	translation.swapRanges(index, index-1);
	updateListItem(index);
	updateListItem(index-1);

	// Update selection
	list_translations->SetSelection(index-1);
	openRange(index-1);

	// Update preview
	//if (frame_preview)
	//	frame_preview->updatePreview(translation, palette);
}

void TranslationEditorDialog::onBtnDown(wxCommandEvent& e) {
	// Get selection
	int index = list_translations->GetSelection();

	// Check selection is valid
	if (index >= (int)translation.nRanges()-1)
		return;

	// Swap item with above
	translation.swapRanges(index, index+1);
	updateListItem(index);
	updateListItem(index+1);

	// Update selection
	list_translations->SetSelection(index+1);
	openRange(index+1);

	// Update preview
	//if (frame_preview)
	//	frame_preview->updatePreview(translation, palette);
}

void TranslationEditorDialog::onBtnLoad(wxCommandEvent& e) {
	// Get user directory
	string dir = appPath("translations", DIR_USER);

	// Create open file dialog
	wxFileDialog dialog_open(this, "Load Translation from File", dir, wxEmptyString,
			"Text Files (*.txt)|*.txt", wxFD_OPEN|wxFD_FILE_MUST_EXIST, wxDefaultPosition);

	// Run the dialog & check that the user didn't cancel
	if (dialog_open.ShowModal() == wxID_OK) {
		// Get the selected filename
		string filename = dialog_open.GetPath();

		// Load file in a tokenizer
		Tokenizer tz;
		tz.openFile(dialog_open.GetPath());

		// Parse translation
		Translation trans;
		string token = tz.getToken();
		while (!token.IsEmpty()) {
			// Parse translation range
			trans.parse(token);

			tz.getToken();			// Skip ,
			token = tz.getToken();
		}

		// Open it if parsed ok
		if (trans.nRanges() > 0)
			openTranslation(trans);
		else
			wxMessageBox("Not a valid translation file", "Error", wxICON_ERROR);
	}
}

void TranslationEditorDialog::onBtnSave(wxCommandEvent& e) {
	// If the directory doesn't exist create it
	string dir = appPath("translations", DIR_USER);
	if (!wxDirExists(dir))
		wxMkdir(dir);

	// Create save file dialog
	wxFileDialog dialog_save(this, "Save Translation to File", dir, wxEmptyString,
			"Text File (*.txt)|*.txt", wxFD_SAVE|wxFD_OVERWRITE_PROMPT, wxDefaultPosition);

	// Run the dialog & check that the user didn't cancel
	if (dialog_save.ShowModal() == wxID_OK) {
		// Get translation as text string
		string str = translation.asText();

		// Open file for writing
		wxFile file(dialog_save.GetPath(), wxFile::write);

		// Write string to file
		file.Write(str);

		// Close file
		file.Close();
	}
}