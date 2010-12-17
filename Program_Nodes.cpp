#include "Program_Nodes.h"
#include "Code_Generator.h"

void ExpressionNode::RemoveComponent(ProgramNode *node)
{
	std::vector<ProgramNode*>::iterator it = nodes_.begin(), it_end =
			nodes_.end();
	for (; it != it_end; ++it)
	{
		if (node->IsEqual(*it))
		{
			break;
		}
	}
	if (it != it_end)
	{
		nodes_.erase(it);
	}
}
void ExpressionNode::Traverse(CodeGenerator &gen)
{
	gen.Visit(this);
	std::vector<ProgramNode*>::iterator it = nodes_.begin(), it_end =
			nodes_.end();
	for (; it != it_end; ++it)
	{
		(*it)->Traverse(gen);
	}
}

void StatementNode::RemoveComponent(ProgramNode *node)
{
	std::vector<ProgramNode*>::iterator it = nodes_.begin(), it_end = nodes_.end();
	for (; it != it_end; ++it)
	{
		if (node->IsEqual(*it))
		{
			break;
		}
	}
	if (it != it_end)
	{
		nodes_.erase(it);
	}
}
void StatementNode::Traverse(CodeGenerator &gen)
{
	gen.Visit(this);
	std::vector<ProgramNode*>::iterator it = nodes_.begin(), it_end = nodes_.end();
	for(;it != it_end; ++it)
	{
		(*it)->Traverse(gen);
	}
}
