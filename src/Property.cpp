
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    Property.cpp
 * Description: The Property class. Basically acts as a 'dynamic'
 *              variable type, for use in the PropertyList class.
 *              Can contain a boolean, integer, floating point (double)
 *              or string (wxString) value.
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
#include "Property.h"


/*******************************************************************
 * PROPERTY CLASS FUNCTIONS
 *******************************************************************/

/* Property::Property
 * Property class default constructor
 *******************************************************************/
Property::Property(uint8_t type) {
	// Set property type
	this->type = type;

	// Set default value depending on type
	if (type == PROP_BOOL)
		value.Boolean = true;
	else if (type == PROP_INT)
		value.Integer = 0;
	else if (type == PROP_FLOAT)
		value.Floating = 0.0f;
	else if (type == PROP_STRING)
		val_string = wxEmptyString;
	else if (type == PROP_FLAG)
		value.Boolean = true;
	else {
		// Invalid type given, default to boolean
		type = PROP_BOOL;
		value.Boolean = true;
	}
}

/* Property::Property
 * Property class constructor (boolean)
 *******************************************************************/
Property::Property(bool value) {
	// Init boolean property
	this->type = PROP_BOOL;
	this->value.Boolean = value;
}

/* Property::Property
 * Property class constructor (integer)
 *******************************************************************/
Property::Property(int value) {
	// Init integer property
	this->type = PROP_INT;
	this->value.Integer = value;
}

/* Property::Property
 * Property class constructor (floating point)
 *******************************************************************/
Property::Property(double value) {
	// Init float property
	this->type = PROP_FLOAT;
	this->value.Floating = value;
}

/* Property::Property
 * Property class constructor (string)
 *******************************************************************/
Property::Property(string value) {
	// Init string property
	this->type = PROP_STRING;
	this->val_string = value;
}

/* Property::~Property
 * Property class destructor
 *******************************************************************/
Property::~Property() {
}

/* Property::getBoolValue
 * Returns the property value as a bool. If [warn_wrong_type] is
 * true, a warning message is written to the log if the property is
 * not of boolean type
 *******************************************************************/
bool Property::getBoolValue(bool warn_wrong_type) {
	// If this is a flag, just return boolean 'true' (or equivalent)
	if (type == PROP_FLAG)
		return true;

	// Write warning to log if needed
	if (warn_wrong_type && type != PROP_BOOL)
		wxLogMessage("Warning: Requested Boolean value of a %s Property", typeString().c_str());

	// Return value (convert if needed)
	if (type == PROP_BOOL)
		return value.Boolean;
	else if (type == PROP_INT)
		return !!value.Integer;
	else if (type == PROP_FLOAT)
		return !!((int)value.Floating);
	else if (type == PROP_STRING) {
		// Anything except "0", "no" or "false" is considered true
		if (!val_string.Cmp("0") || !val_string.CmpNoCase("no") || !val_string.CmpNoCase("false"))
			return false;
		else
			return true;
	}

	// Return default boolean value
	return true;
}

/* Property::getIntValue
 * Returns the property value as an int. If [warn_wrong_type] is
 * true, a warning message is written to the log if the property is
 * not of integer type
 *******************************************************************/
int Property::getIntValue(bool warn_wrong_type) {
	// If this is a flag, just return boolean 'true' (or equivalent)
	if (type == PROP_FLAG)
		return 1;

	// Write warning to log if needed
	if (warn_wrong_type && type != PROP_INT)
		wxLogMessage("Warning: Requested Integer value of a %s Property", typeString().c_str());

	// Return value (convert if needed)
	if (type == PROP_INT)
		return value.Integer;
	else if (type == PROP_BOOL)
		return (int)value.Boolean;
	else if (type == PROP_FLOAT)
		return (int)value.Floating;
	else if (type == PROP_STRING)
		return atoi(CHR(val_string));

	// Return default integer value
	return 0;
}

/* Property::getFloatValue
 * Returns the property value as a double. If [warn_wrong_type] is
 * true, a warning message is written to the log if the property is
 * not of floating point type
 *******************************************************************/
double Property::getFloatValue(bool warn_wrong_type) {
	// If this is a flag, just return boolean 'true' (or equivalent)
	if (type == PROP_FLAG)
		return 1;

	// Write warning to log if needed
	if (warn_wrong_type && type != PROP_FLOAT)
		wxLogMessage("Warning: Requested Float value of a %s Property", typeString().c_str());

	// Return value (convert if needed)
	if (type == PROP_FLOAT)
		return value.Floating;
	else if (type == PROP_BOOL)
		return (double)value.Boolean;
	else if (type == PROP_INT)
		return (double)value.Integer;
	else if (type == PROP_STRING)
		return (double)atof(CHR(val_string));

	// Return default float value
	return 0.0f;
}

/* Property::getStringValue
 * Returns the property value as a string. If [warn_wrong_type] is
 * true, a warning message is written to the log if the property is
 * not of string type
 *******************************************************************/
string Property::getStringValue(bool warn_wrong_type) {
	// If this is a flag, just return boolean 'true' (or equivalent)
	if (type == PROP_FLAG)
		return "1";

	// Write warning to log if needed
	if (warn_wrong_type && type != PROP_STRING)
		wxLogMessage("Warning: Requested String value of a %s Property", typeString().c_str());

	// Return value (convert if needed)
	if (type == PROP_STRING)
		return val_string;
	else if (type == PROP_INT)
		return S_FMT("%d", value.Integer);
	else if (type == PROP_BOOL) {
		if (value.Boolean)
			return "1";
		else
			return "0";
	}
	else if (type == PROP_FLOAT)
		return S_FMT("%f", value.Floating);

	// Return default string value
	return wxEmptyString;
}

/* Property::setValue
 * Sets the property to [val], and changes its type to boolean
 * if necessary
 *******************************************************************/
void Property::setValue(bool val) {
	// Change type if necessary
	if (type != PROP_BOOL)
		changeType(PROP_BOOL);

	// Set value
	value.Boolean = val;
}

/* Property::setValue
 * Sets the property to [val], and changes its type to integer
 * if necessary
 *******************************************************************/
void Property::setValue(int val) {
	// Change type if necessary
	if (type != PROP_INT)
		changeType(PROP_INT);

	// Set value
	value.Integer = val;
}

/* Property::setValue
 * Sets the property to [val], and changes its type to floating
 * point if necessary
 *******************************************************************/
void Property::setValue(double val) {
	// Change type if necessary
	if (type != PROP_FLOAT)
		changeType(PROP_FLOAT);

	// Set value
	value.Floating = val;
}

/* Property::setValue
 * Sets the property to [val], and changes its type to string
 * if necessary
 *******************************************************************/
void Property::setValue(string val) {
	// Change type if necessary
	if (type != PROP_STRING)
		changeType(PROP_STRING);

	// Set value
	val_string = val;
}

/* Property::changeType
 * Changes the property's value type and gives it a default value
 *******************************************************************/
void Property::changeType(uint8_t newtype) {
	// Do nothing if changing to same type
	if (type == newtype)
		return;

	// Clear string data if changing from string
	if (type == PROP_STRING)
		val_string.Clear();

	// Update type
	type = newtype;

	// Update value
	if (type == PROP_BOOL)
		value.Boolean = true;
	else if (type == PROP_INT)
		value.Integer = 0;
	else if (type == PROP_FLOAT)
		value.Floating = 0.0f;
	else if (type == PROP_STRING)
		val_string = wxEmptyString;
	else if (type == PROP_FLAG)
		value.Boolean = true;
}

/* Property::typeString
 * Returns a string representing the property's value type
 *******************************************************************/
string Property::typeString() {
	switch (type) {
	case PROP_BOOL:
		return "Boolean";
	case PROP_INT:
		return "Integer";
	case PROP_FLOAT:
		return "Float";
	case PROP_STRING:
		return "String";
	case PROP_FLAG:
		return "Flag";
	default:
		return "Unknown";
	}
}
