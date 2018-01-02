
#include "Rectangle.h"

#include <math.h>


Rect::Rect(Point topLeft, Point bottomRight) : topLeft(topLeft), bottomRight(bottomRight)
{
}

Rect::Rect() : topLeft(Point()), bottomRight(Point())
{
}

bool Rect::isIntersect(const Rect & rec) const
{
	return !(this->bottomRight.y < rec.topLeft.y || this->topLeft.y > rec.bottomRight.y || this->topLeft.x > rec.bottomRight.x || this->bottomRight.x < rec.topLeft.x);
}

bool Rect::isInclude(Point point) const
{
	return point.x >= this->topLeft.x && point.y >= this->topLeft.y && point.x <= this->bottomRight.x && point.y <= this->bottomRight.y;
}

bool Rect::isInclude(double x, double y) const
{
	return x >= this->topLeft.x && y >= this->topLeft.y && x <= this->bottomRight.x && y <= this->bottomRight.y;
}

void Rect::raiseInDirection(const Rect & dirRectangle, double step)
{
	if (dirRectangle.bottomRight.x >= this->bottomRight.x - step) this->bottomRight.x = dirRectangle.bottomRight.x + step;
	if (dirRectangle.bottomRight.y >= this->bottomRight.y - step) this->bottomRight.y = dirRectangle.bottomRight.y + step;
	if (dirRectangle.topLeft.y <= this->topLeft.y + step) this->topLeft.y = dirRectangle.topLeft.y - step;
	if (dirRectangle.topLeft.x <= this->topLeft.x + step) this->topLeft.x = dirRectangle.topLeft.x - step;
}

double Rect::getDiagonal() const
{
	return this->topLeft.distanceTo(this->bottomRight);
}

double Rect::getSpace() const
{
	if (this->bottomRight.x - this->topLeft.x == 0.0) return fabs(this->bottomRight.y - this->topLeft.y);
	if (this->bottomRight.y - this->topLeft.y == 0.0) return fabs(this->bottomRight.x - this->topLeft.x);
	return fabs(this->bottomRight.y - this->topLeft.y) * fabs(this->bottomRight.x - this->topLeft.x);
}
