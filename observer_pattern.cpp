/*
 * observer_pattern.cpp
 *
 *  Created on: Dec 29, 2010
 *      Author: bogdan
 */

#include <vector>
#include <ctime>
#include <iostream>

class Subject;

class Observer
{
public:
	virtual ~Observer()
	{}
	virtual void Update(Subject *changedSubject) = 0;
protected:
	Observer()
	{}
};

class Subject
{
public:
	virtual ~Subject()
	{}
	virtual void Attach(Observer *obs)
	{
		observers_.push_back(obs);
	}
	virtual void Detach(Observer *obs)
	{
		std::vector<Observer*>::iterator it = observers_.begin();
		std::vector<Observer*>::iterator it_end = observers_.end();
		for (;it != it_end; ++it)
		{
			if (obs == *it)
			{
				break;
			}
		}
		if (it != it_end)
		{
			observers_.erase(it);
		}
	}
	virtual void Notify(bool end = false) //notify all observers
	{
		std::vector<Observer*>::iterator it = observers_.begin();
		std::vector<Observer*>::iterator it_end = observers_.end();
		for (;it != it_end; ++it)
		{
			(*it)->Update((false == end)?this:NULL);
		}
	}
protected:
	Subject()
	{}
private:
	std::vector<Observer*> observers_;
};

class ClockTimer : public Subject
{
public:
	ClockTimer() :
		tm_(NULL)
	{}
	virtual int GetHour()
	{
		return tm_?tm_->tm_hour:0;
	}
	virtual int GetMinute()
	{
		return tm_?tm_->tm_min:0;
	}
	virtual int GetSecond()
	{
		return tm_?tm_->tm_sec:0;
	}
	void Tick()
	{
		time_t tim = time(NULL);
		tm_ = localtime(&tim);
		Notify();//notify all observers
	}
	~ClockTimer()
	{
		Notify(true);
	}
private:
	const tm *tm_;
};

class DigitalClock : public Observer
{
public:
	DigitalClock(ClockTimer *clock):
		subject_(clock)
	{
		clock->Attach(this);
	}
	virtual ~DigitalClock()
	{
		if (subject_)
		{
			subject_->Detach(this);//notify the subject when the observer is destroyed
		}
	}
	virtual void Update(Subject *subj)
	{
		if (subj == subject_)
		{
			std::cout << "Update called from DigitalClock with non NULL subject" << std::endl;
			if (subject_)
			{
				std::cout << subject_->GetHour() << ":" << subject_->GetMinute() << ":" << subject_->GetSecond() << std::endl;
			}
		} else
		{
			subject_ = NULL;
			std::cout << "DigitalClock: Subject has been destroyed" << std::endl;
		}
	}
private:
	ClockTimer *subject_;
};

class AnalogClock : public Observer
{
public:
	AnalogClock(ClockTimer *clock):
		subject_(clock)
	{
		clock->Attach(this);
	}
	virtual ~AnalogClock()
	{
		if (subject_)
		{
			subject_->Detach(this);//notify the subject when the observer is destroyed
		}
	}
	virtual void Update(Subject *subj)
	{
		if (subj == subject_)
		{
			std::cout << "Update called from AnalogClock with non NULL subject" << std::endl;
			if (subject_)
			{
				std::cout << subject_->GetHour() << ":" << subject_->GetMinute() << ":" << subject_->GetSecond() << std::endl;
			}
		} else
		{
			subject_ = NULL;
			std::cout << "AnalogClock: Subject has been destroyed" << std::endl;
		}
	}
private:
	ClockTimer *subject_;
};

int main()
{
	ClockTimer *timer = new ClockTimer();//subject
	AnalogClock *aClock = new AnalogClock(timer);//observer 1
	DigitalClock *dClock = new DigitalClock(timer);//observer 2

	timer->Tick();
	sleep(2);
	timer->Tick();

	if (aClock)
	{
		delete aClock;
	}

	timer->Tick();

	if (timer)
	{
		delete timer;
	}
	if (dClock)
	{
		delete dClock;
	}
}
