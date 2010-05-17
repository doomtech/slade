
#ifndef __TREE_H__
#define __TREE_H__

/*
#include <wx/filename.h>

template <class T>
class TreeNode {
private:
	string				name;
	TreeNode*			parent;
	vector<T>			items;
	vector<TreeNode*>	children;

public:
	TreeNode(string name, TreeNode* parent = NULL) {
		// Init variables
		this->name = name;
		this->parent = parent;
	}

	~TreeNode() {
		// Delete child nodes
		for (size_t a = 0; a < children.size(); a++)
			delete children[a];
	}

	string	getName() { return name; }
	vector<T>& getItems() { return items; }

	void	setName(string name) { this->name = name; }

	virtual void addItem(T item) {
		items.push_back(item);
	}

	virtual TreeNode* addChild(string name) {
		// Convert the given name to a wxFileName for processing
		wxFileName fn(name);

		// If the given name has no directories, add the name as a child and return it
		if (fn.GetDirCount() == 0) {
			TreeNode<T>* child = new TreeNode<T>(fn.GetName(), this);
			children.push_back(child);
			return child;
		}

		// Get first directory in given path
		string first = fn.GetDirs()[0] + _T("/");

		// Check the first dir doesn't already exist
		TreeNode* node_add = NULL;
		for (size_t a = 0; a < children.size(); a++) {
			if (!children[a]->getName().Cmp(first)) {
				// If it does, set the node to add to
				node_add = children[a];
			}
		}

		// If the first directory didn't exist within the current node, add it
		if (!node_add)
			node_add = new TreeNode<T>(first, this);

		// If it did exist, remove the first directory from the name and
		// continue adding subsequent children
		fn.RemoveDir(0);
		return node_add->addChild(fn.GetPath(true, wxPATH_UNIX));
	}

	virtual string getFullPath() {
		if (parent)
			return parent->getFullPath() + _T("/") + name;
		else
			return name;
	}

	virtual bool itemExists(T item, bool full_search = true) {
		// Search items
		if (std::find(items.begin(), items.end(), item) != items.end())
			return true;

		// Search children's items recursively
		if (full_search) {
			for (size_t a = 0; a < children.size(); a++) {
				if (children[a]->itemExists(item, true))
					return true;
			}
		}

		return false;
	}

	virtual bool childExists(string name, bool full_search = true) {
		// Search children
		for (size_t a = 0; a < children.size(); a++) {
			if (!children[a]->getName().Cmp(name))
				return true;
		}

		// Search children's children recursively
		if (full_search) {
			for (size_t a = 0; a < children.size(); a++) {
				if (children[a]->childExists(name))
					return true;
			}
		}

		return false;
	}

	virtual TreeNode* getChild(string name) {
		// If name ends in /, remove it
		if (name.EndsWith(_T("/")))
			name.RemoveLast();

		// Convert the given name to a wxFileName for processing
		wxFileName fn(name);

		// If the given name has no directories, find the child in this node
		if (fn.GetDirCount() == 0) {
			for (size_t a = 0; a < children.size(); a++) {
				if (children[a]->getName().Cmp(fn.GetName()))
					return children[a];
			}

			// No matching child node found
			return NULL;
		}

		// Get first directory in given path
		string first = fn.GetDirs()[0] + _T("/");

		// Check the first dir is a child of this node
		TreeNode* node_add = NULL;
		for (size_t a = 0; a < children.size(); a++) {
			if (!children[a]->getName().Cmp(first)) {
				// The child node exists within the current node
				// Remove the first directory from the dirname
				fn.RemoveDir(0);

				// Continue searching from the child node
				return children[a]->getChild(fn.GetPath(true, wxPATH_UNIX));
			}
		}

		// If we get here the child node does not exist
		return NULL;
	}

	virtual void getItemList(vector<T>& list, bool child_items = true) {
		// Add items from child nodes first
		if (child_items) {
			for (size_t a = 0; a < children.size(); a++)
				children[a]->getItemList(list);
		}

		// Add items from this node
		for (size_t a = 0; a < items.size(); a++)
			list.push_back(items[a]);
	}
};
*/

// Let's try something else
class STreeNode {
protected:
	vector<STreeNode*>	children;
	STreeNode*			parent;

	virtual STreeNode*	createChild(string name) = 0;

public:
	STreeNode(STreeNode* parent);
	virtual ~STreeNode();

	STreeNode*		getParent() { return parent; }
	virtual string	getName() = 0;
	virtual void	setName(string name) = 0;
	virtual string	getPath();

	virtual STreeNode*	getChild(string name);
	virtual void 		addChild(STreeNode* child);
	virtual STreeNode*	addChild(string name);
	virtual bool 		removeChild(STreeNode* child);

	virtual bool	isLeaf() { return children.size() == 0; }
};

#endif//__TREE_H__
