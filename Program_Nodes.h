#ifndef PROGRAM_NODES_H_
#define PROGRAM_NODES_H_

#include <vector>

class CodeGenerator;

class ProgramNode
{
public:
	virtual ~ProgramNode()
	{
	}
	virtual void AddComponent(ProgramNode *node)
	{
	}
	virtual void RemoveComponent(ProgramNode *node)
	{
	}
	virtual void GetSourcePosition(int &line, int &index)
	{
	}
	virtual void Traverse(CodeGenerator &gen)
	{
	}
	virtual bool IsEqual(const ProgramNode *other) const
	{
		return c_ == other->c_;
	}
	char GetChar()
	{
		return c_;
	}
protected:
	ProgramNode(char c) :
		c_(c)
	{
	}
private:
	char c_;
};

class ExpressionNode : public ProgramNode
{
public:
	ExpressionNode(char c) :
		ProgramNode(c)
	{}
	ExpressionNode(const ProgramNode &node) :
		ProgramNode(node)
	{
	}
	virtual void AddComponent(ProgramNode *node)
	{
		nodes_.push_back(node);
	}
	virtual void RemoveComponent(ProgramNode *node);
	virtual void Traverse(CodeGenerator &gen);
private:
	std::vector<ProgramNode*> nodes_;
};

class StatementNode : public ProgramNode
{
public:
	StatementNode(char c) :
		ProgramNode(c)
	{}
	StatementNode(const ProgramNode &node) :
		ProgramNode(node)
	{}
	virtual void AddComponent(ProgramNode *node)
	{
		nodes_.push_back(node);
	}
	virtual void RemoveComponent(ProgramNode *node);
	virtual void Traverse(CodeGenerator &gen);
private:
	std::vector<ProgramNode*> nodes_;
};

#endif
