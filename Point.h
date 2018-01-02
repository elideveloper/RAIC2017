#pragma once

#ifndef POINT_H_
#define POINT_H_

struct Point {
	double x;
	double y;
	Point();
	Point(double x, double y);
	Point(const Point & point);
	Point & operator= (const Point & point);
	Point operator+(const Point & p) const;
	Point operator-(const Point & p) const;
	double distanceTo(double x, double y) const;
	double distanceTo(const Point & point) const;
	bool inCircle(const Point & center, double radius) const;
};

#endif