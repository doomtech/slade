
#ifndef __TREE_H__
#define __TREE_H__

/* Some notes:
	createChild should simply create a STreeNode of the derived type, NOT set its parent (via the constructor or otherwise)
	deleting a STreeNode will not remove it from its parent, this must be done manually
*/
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

	unsigned			nChildren() { return children.size(); }
	STreeNode*			getChild(unsigned index);
	virtual STreeNode*	getChild(string name);
	virtual void 		addChild(STreeNode* child);
	virtual STreeNode*	addChild(string name);
	virtual bool 		removeChild(STreeNode* child);

	virtual bool	isLeaf() { return children.size() == 0; }
};

#endif//__TREE_H__
