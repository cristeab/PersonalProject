#ifndef HONDUROTA_H
#define HONDUROTA_H

class Hondurota
{
	// Public stuff
public:
	// Fields
	//
	Hondurota(double odom, double kpl) :
		fuel_(0.0), speed_(0.0), odometer_(odom), consumption_rate_(kpl),
		tank_capacity_(1000/kpl)
	{
	}
	// Operations
	double getFuel() const
	{
		return fuel_;
	}
	double getSpeed() const
	{
		return speed_;
	}
	double getOdometer() const
	{
		return odometer_;
	}
	double getTankCapacity() const
	{
		return tank_capacity_;
	}
	//
	double drive(double speed, int minutes)
	{
		speed_ = speed;
		if (0 == fuel_ || 0 >= speed_ || 0 >= minutes)
		{
			return fuel_;
		}
		double dist = speed_*double(minutes)/60;//km
		fuel_ -= dist/consumption_rate_;
		if (0 > fuel_)
		{
			odometer_ += fuel_*consumption_rate_;
			fuel_ = 0;
		}
		odometer_ += dist;
		return fuel_;
	}
	//
	double addFuel(double lit)
	{
		if (0 == lit)
		{
			fuel_ = tank_capacity_ ;
		} else if (0 > lit)
		{
			return -1;
		} else
		{
			fuel_ = (lit > tank_capacity_)?tank_capacity_:lit;
		}
		return fuel_;
	}
	// Private stuff
private:
	// Fields
	double fuel_;
	double speed_;
	double odometer_;
	double consumption_rate_;//km/l
	double tank_capacity_;
};
#endif //HONDUROTA_H
