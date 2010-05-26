
#include "Main.h"
#include "Tree.h"
#include <wx/filename.h>

STreeNode::STreeNode(STreeNode* parent) {
	if (parent)
		parent->addChild(this);
	else
		parent = NULL;
}

STreeNode::~STreeNode() {
	// Delete children
	for (unsigned a = 0; a < children.size(); a++)
		delete children[a];
}

string STreeNode::getPath() {
	if (!parent)
		return getName();
	else
		return parent->getPath() + "/" + getName();
}

STreeNode* STreeNode::getChild(unsigned index) {
	// Check index
	if (index >= children.size())
		return NULL;

	return children[index];
}

STreeNode* STreeNode::getChild(string name) {
	// Check name was given
	if (name.IsEmpty())
		return NULL;

	// Convert name to path for processing
	wxFileName fn(name);

	// If no directories were given
	if (fn.GetDirCount() == 0) {
		// Find child of this node
		for (unsigned a = 0; a < children.size(); a++) {
			if (s_cmp(name, children[a]->getName()))
				return children[a];
		}

		// Child not found
		return NULL;
	}
	else {
		// Directories were given, get the first directory
		string dir = fn.GetDirs()[0];

		// See if it is a child of this node
		STreeNode* child = getChild(dir);
		if (child) {
			// It is, remove the first directory and continue searching that child
			fn.RemoveDir(0);
			return child->getChild(fn.GetPath(true, wxPATH_UNIX));
		}
		else
			return NULL;	// Child doesn't exist
	}
}

void STreeNode::addChild(STreeNode* child) {
	children.push_back(child);
	child->parent = this;
}

STreeNode* STreeNode::addChild(string name) {
	// Check name was given
	if (name.IsEmpty())
		return NULL;

	// Convert name to path for processing
	wxFileName fn(name);

	// Get the name of the first directory in the path (or just the name if no directories were specified)
	string cname;
	if (fn.GetDirCount() == 0)
		cname = name;
	else
		cname = fn.GetDirs()[0];

	// Check if a child already exists with this name
	STreeNode* child = getChild(cname);
	if (!child) {
		// No child with the name exists, create+add one
		child = createChild(cname);
		addChild(child);
	}

	return child;
}

bool STreeNode::removeChild(STreeNode* child) {
	// Find child node
	for (unsigned a = 0; a < children.size(); a++) {
		if (children[a] == child) {
			// Remove child from list
			children.erase(children.begin() + a);

			// Reset child's parent
			children[a]->parent = NULL;

			return true;
		}
	}

	// Child didn't exist
	return false;
}
