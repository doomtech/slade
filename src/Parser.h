
#ifndef __PARSER_H__
#define __PARSER_H__

#include "Tree.h"
#include "Tokenizer.h"
#include "Property.h"

class ParseTreeNode : public STreeNode {
private:
	string				name;
	string				inherit;
	string				type;
	vector<Property>	values;

protected:
	STreeNode*	createChild(string name) { ParseTreeNode* ret = new ParseTreeNode(); ret->setName(name); return ret; }

public:
	ParseTreeNode(ParseTreeNode* parent = NULL);
	~ParseTreeNode();

	string		getName() { return name; }
	void		setName(string name) { this->name = name; }
	string		getInherit() { return inherit; }
	string		getType() { return type; }
	unsigned	nValues() { return values.size(); }
	Property	getValue(unsigned index = 0);
	string		getStringValue(unsigned index = 0);
	int			getIntValue(unsigned index = 0);
	bool		getBoolValue(unsigned index = 0);
	double		getFloatValue(unsigned index = 0);

	bool	parse(Tokenizer& tz);
};

class Parser {
private:
	ParseTreeNode*	pt_root;

public:
	Parser();
	~Parser();

	ParseTreeNode*	parseTreeRoot() { return pt_root; }

	bool	parseText(MemChunk& mc);
	bool	parseText(string& text);
};

#endif//__PARSER_H__
