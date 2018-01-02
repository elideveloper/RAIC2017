#pragma once

#ifndef RECTANGLE_H_
#define RECTANGLE_H_

#include "Point.h"


struct Rect {
	Point topLeft;
	Point bottomRight;
	Rect(Point topLeft, Point bottomRight);
	Rect();
	bool isIntersect(const Rect & rec) const;
	bool isInclude(Point point) const;
	bool isInclude(double x, double y) const;
	void raiseInDirection(const Rect & dirRectangle, double step);
	double getSpace() const;
	double getDiagonal() const;
};

#endif