
#include "Main.h"
#include "Tree.h"
#include <wx/filename.h>

STreeNode::STreeNode(STreeNode* parent) {
	if (parent)
		parent->addChild(this);
	else
		this->parent = NULL;
}

STreeNode::~STreeNode() {
	// Delete children
	for (unsigned a = 0; a < children.size(); a++)
		delete children[a];
}

string STreeNode::getPath() {
	if (!parent)
		return getName() + "/";
	else
		return parent->getPath() + getName() + "/";
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

	// If name ends with /, remove it
	if (name.EndsWith("/"))
		name.RemoveLast(1);

	// Convert name to path for processing
	wxFileName fn(name);

	// If no directories were given
	if (fn.GetDirCount() == 0) {
		// Find child of this node
		for (unsigned a = 0; a < children.size(); a++) {
			if (s_cmpnocase(name, children[a]->getName()))
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
			return child->getChild(fn.GetFullPath(wxPATH_UNIX));
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

	// If name ends with /, remove it
	if (name.EndsWith("/"))
		name.RemoveLast(1);

	// Convert name to path for processing
	wxFileName fn(name);

	// If no directories were given
	if (fn.GetDirCount() == 0) {
		// Check if a child with this name exists
		STreeNode* child = getChild(name);

		// If it doesn't exist, create it
		if (!child) {
			child = createChild(name);
			addChild(child);
		}

		// Return the created child
		return child;
	}
	else {
		// Directories were given, get the first directory
		string dir = fn.GetDirs()[0];

		// Check if a child with this name exists
		STreeNode* child = getChild(dir);

		// If it doesn't exist, create it
		if (!child) {
			child = createChild(dir);
			addChild(child);
		}

		// Continue adding child nodes
		fn.RemoveDir(0);
		return child->addChild(fn.GetFullPath(wxPATH_UNIX));
	}
}

bool STreeNode::removeChild(STreeNode* child) {
	// Find child node
	for (unsigned a = 0; a < children.size(); a++) {
		if (children[a] == child) {
			// Reset child's parent
			children[a]->parent = NULL;

			// Remove child from list
			children.erase(children.begin() + a);

			return true;
		}
	}

	// Child didn't exist
	return false;
}
