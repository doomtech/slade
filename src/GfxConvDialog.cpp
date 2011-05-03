
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    GfxConvDialog.cpp
 * Description: A dialog UI for converting between different gfx
 *              formats, including options for conversion
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *******************************************************************/


/*******************************************************************
 * INCLUDES
 *******************************************************************/
#include "Main.h"
#include "WxStuff.h"
#include "GfxConvDialog.h"
#include "Console.h"
#include "ArchiveManager.h"
#include "Misc.h"
#include "PaletteManager.h"
#include "SplashWindow.h"
#include "Icons.h"
#include "SIFormat.h"


/*******************************************************************
 * GFXCONVDIALOG CLASS FUNCTINOS
 *******************************************************************/

/* GfxConvDialog::GfxConvDialog
 * GfxConvDialog class constructor
 *******************************************************************/
GfxConvDialog::GfxConvDialog()
: wxDialog(NULL, -1, "Graphic Format Conversion", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER) {
	current_item = 0;

	// Set dialog icon
	wxIcon icon;
	icon.CopyFromBitmap(getIcon("t_convert"));
	SetIcon(icon);

	setupLayout();
}

/* GfxConvDialog::~GfxConvDialog
 * GfxConvDialog class destructor
 *******************************************************************/
GfxConvDialog::~GfxConvDialog() {
}

/* GfxConvDialog::nextItem
 * Opens the next item to be converted. Returns true if the selected
 * format was valid for the next image, false otherwise
 *******************************************************************/
bool GfxConvDialog::nextItem() {
	// Go to next image
	current_item++;
	if (current_item >= items.size()) {
		this->Close(true);
		return false;
	}

	// Load image if needed
	if (!items[current_item].image.isValid()) {
		if (!Misc::loadImageFromEntry(&(items[current_item].image), items[current_item].entry))
			return nextItem();	// Skip if not a valid image entry
	}

	// Update valid formats
	combo_target_format->Clear();
	conv_formats.clear();
	vector<SIFormat*> all_formats;
	SIFormat::getAllFormats(all_formats);
	int current_index = -1;
	int default_index = -1;
	for (unsigned a = 0; a < all_formats.size(); a++) {
		// Check if the image can be written to this format
		if (all_formats[a]->canWrite(items[current_item].image)) {
			// Add conversion formats depending on what colour types this image format can handle
			if (all_formats[a]->canWriteType(PALMASK)) {
				// Add format
				conv_formats.push_back(conv_format_t(all_formats[a], PALMASK));
				combo_target_format->Append(all_formats[a]->getName() + " (Paletted)");

				// Check for match with current format
				if (current_format.format == all_formats[a] && current_format.coltype == PALMASK)
					current_index = conv_formats.size() - 1;

				// Default format is 'doom gfx'
				if (all_formats[a]->getId() == "doom")
					default_index = conv_formats.size() - 1;
			}

			if (all_formats[a]->canWriteType(RGBA)) {
				// Add format
				conv_formats.push_back(conv_format_t(all_formats[a], RGBA));
				combo_target_format->Append(all_formats[a]->getName() + " (Truecolour)");

				// Check for match with current format
				if (current_format.format == all_formats[a] && current_format.coltype == RGBA)
					current_index = conv_formats.size() - 1;
			}

			if (all_formats[a]->canWriteType(ALPHAMAP)) {
				// Add format
				conv_formats.push_back(conv_format_t(all_formats[a], ALPHAMAP));
				combo_target_format->Append(all_formats[a]->getName() + " (Alpha Map)");

				// Check for match with current format
				if (current_format.format == all_formats[a] && current_format.coltype == ALPHAMAP)
					current_index = conv_formats.size() - 1;
			}
		}
	}

	// If the image cannot be converted to the selected format
	bool ok = true;
	if (current_index < 0) {
		// Default to Doom Gfx
		current_index = default_index;
		ok = false;
	}

	// Set current format
	combo_target_format->SetSelection(current_index);
	current_format = conv_formats[current_index];

	// Update UI
	updatePreviewGfx();
	theSplashWindow->setProgressMessage(S_FMT("%d of %d", current_item, items.size()));
	theSplashWindow->setProgress((float)current_item / (float)items.size());

	return ok;
}

/* GfxConvDialog::setupLayout
 * Sets up the dialog UI layout
 *******************************************************************/
void GfxConvDialog::setupLayout() {
	wxBoxSizer* m_vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(m_vbox);

	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	m_vbox->Add(hbox, 0, wxEXPAND|wxTOP|wxLEFT|wxRIGHT, 4);

	// Add 'Convert To' combo box
	hbox->Add(new wxStaticText(this, -1, "Convert to:"), 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);

	// This must remain in sync with the ConversionFormat enum in the header
	combo_target_format = new wxChoice(this, -1);
	hbox->Add(combo_target_format, 1, wxEXPAND|wxALL, 4);


	// Add Gfx previews
	wxStaticBox *frame = new wxStaticBox(this, -1, "Graphic");
	wxStaticBoxSizer *framesizer = new wxStaticBoxSizer(frame, wxHORIZONTAL);
	m_vbox->Add(framesizer, 1, wxEXPAND|wxALL, 4);

	wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);
	framesizer->Add(vbox, 1, wxEXPAND|wxALL, 0);

	vbox->Add(new wxStaticText(this, -1, "Current Graphic"), 0, wxEXPAND|wxLEFT|wxRIGHT, 4);

	gfx_current = new GfxCanvas(this, -1);
	gfx_current->SetInitialSize(wxSize(192, 192));
	gfx_current->setViewType(GFXVIEW_CENTERED);
	vbox->Add(gfx_current->toPanel(this), 1, wxEXPAND|wxTOP|wxLEFT|wxRIGHT, 4);

	pal_chooser_current = new PaletteChooser(this, -1);
	vbox->Add(pal_chooser_current, 0, wxEXPAND|wxALL, 4);


	vbox = new wxBoxSizer(wxVERTICAL);
	framesizer->Add(vbox, 1, wxEXPAND|wxALL, 0);

	vbox->Add(new wxStaticText(this, -1, "Converted Graphic"), 0, wxEXPAND|wxLEFT|wxRIGHT, 4);

	gfx_target = new GfxCanvas(this, -1);
	gfx_target->SetInitialSize(wxSize(192, 192));
	gfx_target->setViewType(1);
	vbox->Add(gfx_target->toPanel(this), 1, wxEXPAND|wxTOP|wxLEFT|wxRIGHT, 4);

	pal_chooser_target = new PaletteChooser(this, -1);
	vbox->Add(pal_chooser_target, 0, wxEXPAND|wxALL, 4);


	// 'Enable transparency' checkbox
	cb_enable_transparency = new wxCheckBox(this, -1, "Enable Transparency");
	cb_enable_transparency->SetValue(true);
	cb_enable_transparency->SetToolTip("Uncheck this to remove any existing transparency from the graphic");
	m_vbox->AddSpacer(4);
	m_vbox->Add(cb_enable_transparency, 0, wxEXPAND|wxALL, 4);

	// Add transparency options
	frame = new wxStaticBox(this, -1, "Transparency Options");
	framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	m_vbox->Add(framesizer, 0, wxEXPAND|wxALL, 4);

	wxBoxSizer* vbox_ttypes = new wxBoxSizer(wxVERTICAL);
	framesizer->Add(vbox_ttypes, 1, wxEXPAND|wxALL, 0);

	// Keep existing transparency
	hbox = new wxBoxSizer(wxHORIZONTAL);
	vbox_ttypes->Add(hbox, 0, wxEXPAND|wxALL, 0);
	rb_transparency_existing = new wxRadioButton(this, 100, "Existing w/Threshold:", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	rb_transparency_existing->SetValue(true);
	hbox->Add(rb_transparency_existing, 0, wxEXPAND|wxALL, 4);

	// Alpha threshold
	slider_alpha_threshold = new wxSlider(this, -1, 0, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_LABELS|wxSL_BOTTOM);
	slider_alpha_threshold->SetToolTip("Specifies the 'cutoff' transparency level, anything above this will be fully opaque, anything equal or below will be completely transparent");
	hbox->Add(slider_alpha_threshold, 1, wxEXPAND|wxALL, 4);


	// Transparent colour
	hbox = new wxBoxSizer(wxHORIZONTAL);
	vbox_ttypes->Add(hbox, 0, wxEXPAND|wxALL, 0);
	rb_transparency_colour = new wxRadioButton(this, 101, "Transparent Colour:", wxDefaultPosition, wxDefaultSize, 0);
	rb_transparency_colour->SetValue(false);
	hbox->Add(rb_transparency_colour, 0, wxEXPAND|wxALL, 4);

	colbox_transparent = new ColourBox(this, -1, false);
	colbox_transparent->setColour(rgba_t(0, 255, 255, 255));
	hbox->Add(colbox_transparent, 0, wxEXPAND|wxALL, 4);


	// From brightness
	rb_transparency_brightness = new wxRadioButton(this, 102, "Transparency from Brightness");
	rb_transparency_brightness->SetValue(false);
	vbox_ttypes->Add(rb_transparency_brightness, 0, wxEXPAND|wxALL, 4);


	// Buttons
	hbox = new wxBoxSizer(wxHORIZONTAL);
	m_vbox->Add(hbox, 0, wxEXPAND|wxALL, 4);

	btn_convert = new wxButton(this, -1, "Convert");
	btn_convert_all = new wxButton(this, -1, "Convert All");
	btn_skip = new wxButton(this, -1, "Skip");
	btn_skip_all = new wxButton(this, -1, "Skip All");

	hbox->AddStretchSpacer(1);
	hbox->Add(btn_convert, 0, wxEXPAND|wxRIGHT, 4);
	hbox->Add(btn_convert_all, 0, wxEXPAND|wxRIGHT, 4);
	hbox->Add(btn_skip, 0, wxEXPAND|wxRIGHT, 4);
	hbox->Add(btn_skip_all, 0, wxEXPAND, 0);


	// Bind events
	Bind(wxEVT_SIZE, &GfxConvDialog::onResize, this);
	btn_convert->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GfxConvDialog::onBtnConvert, this);
	btn_convert_all->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GfxConvDialog::onBtnConvertAll, this);
	btn_skip->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GfxConvDialog::onBtnSkip, this);
	btn_skip_all->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GfxConvDialog::onBtnSkipAll, this);
	combo_target_format->Bind(wxEVT_COMMAND_CHOICE_SELECTED, &GfxConvDialog::onTargetFormatChanged, this);
	pal_chooser_current->Bind(wxEVT_COMMAND_CHOICE_SELECTED, &GfxConvDialog::onCurrentPaletteChanged, this);
	pal_chooser_target->Bind(wxEVT_COMMAND_CHOICE_SELECTED, &GfxConvDialog::onTargetPaletteChanged, this);
	slider_alpha_threshold->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &GfxConvDialog::onAlphaThresholdChanged, this);
	cb_enable_transparency->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &GfxConvDialog::onEnableTransparencyChanged, this);
	rb_transparency_colour->Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, &GfxConvDialog::onTransTypeChanged, this);
	rb_transparency_existing->Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, &GfxConvDialog::onTransTypeChanged, this);
	rb_transparency_brightness->Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, &GfxConvDialog::onTransTypeChanged, this);
	Bind(wxEVT_COLOURBOX_CHANGED, &GfxConvDialog::onTransColourChanged, this, colbox_transparent->GetId());


	// Autosize to fit contents (and set this as the minimum size)
	SetInitialSize(wxSize(-1, -1));
	SetMinSize(GetSize());
}

void GfxConvDialog::openEntry(ArchiveEntry* entry) {
	// Add item
	items.push_back(gcd_item_t(entry));

	// Open it
	current_item = -1;
	nextItem();
}

void GfxConvDialog::openEntries(vector<ArchiveEntry*> entries) {
	// Add entries to item list
	for (unsigned a = 0; a < entries.size(); a++)
		items.push_back(gcd_item_t(entries[a]));

	// Open the first item
	current_item = -1;
	nextItem();
}

/* GfxConvDialog::updatePreviewGfx
 * Updates the current and target preview windows
 *******************************************************************/
void GfxConvDialog::updatePreviewGfx() {
	// Check current item is valid
	if (items.size() <= current_item)
		return;

	// Get the current image entry
	gcd_item_t& item = items[current_item];

	// Set palettes
	gfx_current->setPalette(pal_chooser_current->getSelectedPalette(item.entry));
	gfx_target->setPalette(pal_chooser_target->getSelectedPalette(item.entry));

	// Load the image to both gfx canvases
	gfx_current->getImage()->copyImage(&item.image);
	gfx_target->getImage()->copyImage(&item.image);

	// Update controls
	updateControls();


	// --- Apply image conversion to target preview ---

	// Get conversion options
	SIFormat::convert_options_t opt;
	getConvertOptions(opt);

	// Do conversion
	wxLogMessage("Converting to %s", CHR(current_format.format->getName()));
	current_format.format->convertWritable(*(gfx_target->getImage()), opt);


	// Refresh
	gfx_current->zoomToFit(true, 0.05f);
	gfx_current->Refresh();
	gfx_target->zoomToFit(true, 0.05f);
	gfx_target->Refresh();
}

/* GfxConvDialog::updateControls
 * Disables/enables controls based on what is currently selected
 *******************************************************************/
void GfxConvDialog::updateControls() {
	// Check current item is valid
	if (items.size() <= current_item)
		return;

	// Set colourbox palette if source image has one
	SIType coltype = gfx_current->getImage()->getType();
	if (coltype == PALMASK) {
		colbox_transparent->setPalette(gfx_current->getPalette());
	}
	else
		colbox_transparent->setPalette(NULL);

	// Disable/enable transparency options depending on transparency checkbox
	if (cb_enable_transparency->GetValue()) {
		// Disable/enable alpha threshold slider as needed
		if (coltype == RGBA || coltype == ALPHAMAP)
			slider_alpha_threshold->Enable(true);
		else
			slider_alpha_threshold->Enable(false);

		rb_transparency_colour->Enable(true);
		rb_transparency_existing->Enable(true);
	}
	else {
		rb_transparency_colour->Enable(false);
		rb_transparency_existing->Enable(false);
		slider_alpha_threshold->Enable(false);
	}
}

void GfxConvDialog::getConvertOptions(SIFormat::convert_options_t& opt) {
	// Set transparency options
	opt.transparency = cb_enable_transparency->GetValue();
	if (rb_transparency_existing->GetValue()) {
		opt.mask_source = SIFormat::MASK_ALPHA;
		opt.alpha_threshold = slider_alpha_threshold->GetValue();
	}
	else if (rb_transparency_colour->GetValue()) {
		opt.mask_source = SIFormat::MASK_COLOUR;
		opt.mask_colour = colbox_transparent->getColour();
	}
	else
		opt.mask_source = SIFormat::MASK_BRIGHTNESS;

	// Set conversion palettes
	opt.pal_current = pal_chooser_current->getSelectedPalette(items[current_item].entry);
	opt.pal_target = pal_chooser_target->getSelectedPalette(items[current_item].entry);

	// Set conversion colour format
	opt.col_format = current_format.coltype;
}

bool GfxConvDialog::itemModified(int index) {
	// Check index
	if (index < 0 || index >= (int)items.size())
		return false;

	return items[index].modified;
}

SImage* GfxConvDialog::getItemImage(int index) {
	// Check index
	if (index < 0 || index >= (int)items.size())
		return NULL;

	return &(items[index].image);
}

SIFormat* GfxConvDialog::getItemFormat(int index) {
	// Check index
	if (index < 0 || index >= (int)items.size())
		return NULL;

	return items[index].new_format;
}

Palette8bit* GfxConvDialog::getItemPalette(int index) {
	// Check index
	if (index < 0 || index >= (int)items.size())
		return NULL;

	return items[index].palette;
}

void GfxConvDialog::applyConversion() {
	// Get current item
	gcd_item_t& item = items[current_item];

	// Write converted image data to it
	item.image.copyImage(gfx_target->getImage());

	// Update item info
	item.modified = true;
	item.new_format = current_format.format;
	item.palette = pal_chooser_target->getSelectedPalette(item.entry);
}


/*******************************************************************
 * GFXCONVDIALOG EVENTS
 *******************************************************************/

/* GfxConvDialog::resize
 * Called when the dialog is resized
 *******************************************************************/
void GfxConvDialog::onResize(wxSizeEvent& e) {
	wxDialog::OnSize(e);

	gfx_current->zoomToFit(true, 0.05f);
	gfx_target->zoomToFit(true, 0.05f);

	e.Skip();
}

/* GfxConvDialog::btnConvertClicked
 * Called when the 'Convert' button is clicked
 *******************************************************************/
void GfxConvDialog::onBtnConvert(wxCommandEvent& e) {
	applyConversion();
	nextItem();
}

/* GfxConvDialog::btnConvertAllClicked
 * Called when the 'Convert All' button is clicked
 *******************************************************************/
void GfxConvDialog::onBtnConvertAll(wxCommandEvent& e) {
	// Show splash window
	theSplashWindow->show("Converting Gfx...", true);

	// Convert all images
	for (size_t a = current_item; a < items.size(); a++) {
		applyConversion();
		if (!nextItem())
			break;
	}

	// Hide splash window
	theSplashWindow->hide();
}

/* GfxConvDialog::btnSkipClicked
 * Called when the 'Skip' button is clicked
 *******************************************************************/
void GfxConvDialog::onBtnSkip(wxCommandEvent& e) {
	nextItem();
}

/* GfxConvDialog::btnSkipAllClicked
 * Called when the 'Skip All' button is clicked
 *******************************************************************/
void GfxConvDialog::onBtnSkipAll(wxCommandEvent& e) {
	this->Close(true);
}

/* GfxConvDialog::comboTargetFormatChanged
 * Called when the 'Convert To' combo box is changed
 *******************************************************************/
void GfxConvDialog::onTargetFormatChanged(wxCommandEvent& e) {
	current_format = conv_formats[combo_target_format->GetSelection()];
	updatePreviewGfx();
}

/* GfxConvDialog::paletteCurrentChanged
 * Called when the current image palette chooser is changed
 *******************************************************************/
void GfxConvDialog::onCurrentPaletteChanged(wxCommandEvent& e) {
	updatePreviewGfx();
}

/* GfxConvDialog::paletteTargetChanged
 * Called when the target image palette chooser is changed
 *******************************************************************/
void GfxConvDialog::onTargetPaletteChanged(wxCommandEvent& e) {
	updatePreviewGfx();
}

/* GfxConvDialog::sliderAlphaThresholdChanged
 * Called when the alpha threshold slider is changed
 *******************************************************************/
void GfxConvDialog::onAlphaThresholdChanged(wxCommandEvent& e) {
	// Ignore while slider is being dragged
	if (e.GetEventType() == wxEVT_SCROLL_THUMBTRACK) {
		e.Skip();
		return;
	}

	updatePreviewGfx();
}

/* GfxConvDialog::cbEnableTransparencyChanged
 * Called when the 'enable transparency' checkbox is changed
 *******************************************************************/
void GfxConvDialog::onEnableTransparencyChanged(wxCommandEvent& e) {
	updatePreviewGfx();
}

/* GfxConvDialog::transTypeChanged
 * Called when the 'existing' and 'colour' radio buttons are toggled
 *******************************************************************/
void GfxConvDialog::onTransTypeChanged(wxCommandEvent& e) {
	updatePreviewGfx();
}

/* GfxConvDialog::transColourChanged
 * Called when the transparent colour box is changed
 *******************************************************************/
void GfxConvDialog::onTransColourChanged(wxEvent& e) {
	updatePreviewGfx();
}
