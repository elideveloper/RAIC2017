#include "Point.h"

#include <math.h>

Point::Point() : x(0.0), y(0.0)
{
}

Point::Point(double x, double y) : x(x), y(y)
{
}


Point::Point(const Point & point)
{
	this->x = point.x;
	this->y = point.y;
}

Point & Point::operator=(const Point & point)
{
	if (&point == this) return *this;
	this->x = point.x;
	this->y = point.y;
	return *this;
}

Point Point::operator+(const Point & p) const
{
	return Point(this->x + p.x, this->y + p.y);
}

Point Point::operator-(const Point & p) const
{
	return Point(this->x - p.x, this->y - p.y);
}


double Point::distanceTo(double x, double y) const
{
	return sqrt((x - this->x)*(x - this->x) + (y - this->y)*(y - this->y));
}

double Point::distanceTo(const Point & point) const
{
	return sqrt((point.x - this->x)*(point.x - this->x) + (point.y - this->y)*(point.y - this->y));
}

bool Point::inCircle(const Point & center, double radius) const
{
	return this->distanceTo(center) <= radius;
}
