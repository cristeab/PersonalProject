/*
 * memento.cpp
 *
 *  Created on: Dec 28, 2010
 *      Author: bogdan
 */

#include <gtest/gtest.h>
#include <iostream>
#include <cstdlib>

class ConstraintSolver;

//memento
class ConstraintSolverMemento
{
public:
	virtual ~ConstraintSolverMemento()
	{}
private:
	friend class ConstraintSolver;
	ConstraintSolverMemento() //only its friend could instantiate this class
	{}
};

struct Point
{
	Point(double ix, double iy) :
		x(ix), y(iy)
	{}
	double x;
	double y;
};
Point operator-(const Point &p)
{
	return Point(-p.x, -p.y);
}

class Graphic
{
public:
	void Move(const Point &delta)
	{
		std::cout << "Moving target with a delta (x,y) = (" << delta.x << ", " << delta.y << ")" << std::endl;
	}
};

//originator for the memento (is a singleton)
class ConstraintSolver
{
public:
	static ConstraintSolver* Instance()
	{
		if (NULL == instance_)
		{
			instance_ = new ConstraintSolver();
		}
		return instance_;
	}
	int Solve()
	{
		if (memento_)
		{
			std::cout << "solving using memento ..." << std::endl;
			return EXIT_SUCCESS;
		} else
		{
			std::cout << "memento is NULL, cannot solve" << std::endl;
			return EXIT_FAILURE;
		}
	}
	void AddConstraint(Graphic *startConnection, Graphic *endConnection)
	{}
	void RemoveConstraint(Graphic *startConnection, Graphic *endConnection)
	{}
	ConstraintSolverMemento* CreateMemento()
	{
		return new ConstraintSolverMemento();
	}
	void SetMemento(ConstraintSolverMemento *m)
	{
		memento_ = m;
	}
private:
	ConstraintSolver()
	{}
	ConstraintSolver(const ConstraintSolver&);
	ConstraintSolver& operator=(const ConstraintSolver&);
	static ConstraintSolver *instance_;
	ConstraintSolverMemento *memento_;
};

ConstraintSolver* ConstraintSolver::instance_ = NULL;

//this is the client
class MoveCommand
{
public:
	MoveCommand(Graphic *target, const Point &delta) :
		state_(NULL), delta_(delta), target_(target)
	{
	}
	int Execute()
	{
		ConstraintSolver *solver = ConstraintSolver::Instance();
		if (state_)
		{
			delete state_;
		}
		state_ = solver->CreateMemento();
		target_->Move(delta_);
		return solver->Solve();
	}
	int Unexecute()
	{
		ConstraintSolver *solver = ConstraintSolver::Instance();
		target_->Move(-delta_);
		solver->SetMemento(state_);
		return solver->Solve();
	}
	~MoveCommand()
	{
		if (state_)
		{
			delete state_;
		}
	}
private:
	ConstraintSolverMemento *state_;
	Point delta_;
	Graphic *target_;
};

TEST (test_case, your_test)
{
	Graphic graph;
	Point point(0.1, 0.2);
	MoveCommand cmd(&graph, point);

	ASSERT_EQ(cmd.Execute(), EXIT_FAILURE);
	ASSERT_EQ(cmd.Unexecute(), EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
