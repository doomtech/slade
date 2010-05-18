
#ifndef __PARSER_H__
#define __PARSER_H__

#include "Tree.h"
#include "Property.h"
#include "Tokenizer.h"

class ParseTreeNode : public STreeNode {
private:
	string				name;
	string				inherit;
	vector<Property>	values;

protected:
	STreeNode*	createChild(string name) { ParseTreeNode* ret = new ParseTreeNode(); ret->setName(name); return ret; }

public:
	ParseTreeNode(ParseTreeNode* parent = NULL);
	~ParseTreeNode();

	string	getName() { return name; }
	void	setName(string name) { this->name = name; }

	bool	parse(Tokenizer& tz);
};

class Parser {
private:
	ParseTreeNode*	pt_root;

public:
	Parser();
	~Parser();

	bool	parseText(MemChunk& mc);
};

#endif//__PARSER_H__
