/*
 * Facade_Pattern.cpp
 *
 *  Created on: Dec 17, 2010
 *      Author: bogdan
 */

#include <gtest/gtest.h>
#include "Program_Nodes.h"
#include "Code_Generator.h"

class Token
{
public:
	Token() :
		c_('\0')
	{}
	Token(char c) :
		c_(c)
	{}
	char GetChar()
	{
		return c_;
	}
private:
	char c_;
};

class Scanner
{
public:
	Scanner(std::ifstream &stream) :
		stream_(stream)
	{}
	virtual ~Scanner()
	{}
	virtual Token Scan()
	{
		char c;
		stream_ >> c;
		return Token(c);
	}
private:
	std::ifstream &stream_;
};

class RISCCodeGenerator : public CodeGenerator
{
public:
	RISCCodeGenerator(ByteCodeStream &bstream) :
		CodeGenerator(bstream)
	{}
};

class ProgramNodeBuilder
{
public:
	ProgramNodeBuilder() :
		node_(NULL)
	{}
	virtual ProgramNode* NewVariable(char variableName)
	{
		if (node_)
		{
			node_->AddComponent(new ExpressionNode(variableName));
		} else
		{
			node_ = new ExpressionNode(variableName);
		}
		return node_;
	}
	virtual ProgramNode* NewAssignment(ProgramNode* var, ProgramNode *expr)
	{
		expr->AddComponent(var);
		if (node_)
		{
			node_->AddComponent(expr);
		} else
		{
			node_ = new ExpressionNode(*expr);
		}
		return expr;
	}
	virtual ProgramNode* NewReturnStatement(ProgramNode *value)
	{
		if (node_)
		{
			node_->AddComponent(value);
		} else
		{
			node_ = new StatementNode(*value);
		}
		return value;
	}
	virtual ProgramNode* NewCondition(ProgramNode *condition, ProgramNode *truePart, ProgramNode *falsePart)
	{
		condition->AddComponent(truePart);
		condition->AddComponent(falsePart);
		if (node_)
		{
			node_->AddComponent(condition);
		} else
		{
			node_ = new ExpressionNode(*condition);
		}
		return condition;
	}
	virtual ~ProgramNodeBuilder()
	{
		if (node_)
		{
			delete node_;
			node_ = NULL;
		}
	}
	ProgramNode* GetRootNode()
	{
		return node_;
	}
private:
	ProgramNode* node_;
};

class Parser
{
public:
	Parser()
	{}
	virtual ~Parser()
	{}
	virtual void Parse(Scanner &scanner, ProgramNodeBuilder &builder)
	{
		//scan the first 10 chars and build a tree
		Token tok;
		char c;
		for (unsigned int n = 0; n< 10; ++n)
		{
			tok = scanner.Scan();
			c = tok.GetChar();
			if (isalpha(c))
			{
				builder.NewVariable(tok.GetChar());
			} else if ('=' == c)
			{
				StatementNode stat(c);
				tok = scanner.Scan();
				ExpressionNode exp(tok.GetChar());
				builder.NewAssignment(&stat, &exp);
			} else
			{
				StatementNode stat(c);
				tok = scanner.Scan();
				ExpressionNode exp1(tok.GetChar());
				tok = scanner.Scan();
				ExpressionNode exp2(tok.GetChar());
				builder.NewCondition(&stat, &exp1, &exp2);
			}
		}
	}
};

//Facade class
class Compiler
{
public:
	virtual void Compile(std::ifstream &istream, ByteCodeStream &bstream)
	{
		Scanner scanner(istream);
		ProgramNodeBuilder builder;
		Parser parser;

		parser.Parse(scanner, builder);

		RISCCodeGenerator gen(bstream);//target arch is hard coded
		ProgramNode *parseTree = builder.GetRootNode();
		parseTree->Traverse(gen);
	}
};

TEST (test_case, your_test)
{
	Compiler comp;
	std::ifstream istream("Facade_Pattern.cpp");
	if (!istream.is_open())
	{
		ASSERT_TRUE(false);
	}
	ByteCodeStream ostream("out.txt");
	if (!ostream.is_open())
	{
		ASSERT_TRUE(false);
	}
	comp.Compile(istream, ostream);
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
