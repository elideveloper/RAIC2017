#pragma once

#ifndef UTILITY_H_
#define UTILITY_H_

#include <map>
#include <stack>

#include "Rectangle.h"
#include "model/World.h"
#include "model/Move.h"
#include "model/ActionType.h"

#define PI 3.14159265358979323846
#define _USE_MATH_DEFINES

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

const double WORLD_LENGTH = 1024.0;
const double TILE_LENGTH = 32.0;
const int NUCLEAR_STRIKE_DELAY = 30;

enum FormationUnitsType { _UNDEFINED_ = -1, MOSTLY_AERIAL, MOSTLY_GROUND, MIXED };		// пока так, чтобы возможность пересечения формация отслеживать

double getDamageVsUnit(model::VehicleType defUnitType, const model::Vehicle & attacking);
Point getCellXY(const Point & p);
Point getCellXY(const model::Vehicle & veh);
double getRealVisionRangeForUnit(const model::Vehicle & veh, const model::World & w);
int getNearestUnitID(const Point & dest, const std::map<long, model::Vehicle> & vehicles);
Point getFartherVisiblePointInDirectionTo(const model::Vehicle & veh, double x, double y, const model::World & w);
Point getPointInDirectionForDistance(const Point & p1, const Point & p2, double dist = 100.0);
bool checkGroupForUpdateSmth(const std::vector<model::Vehicle> & group, const std::vector<model::VehicleUpdate> & vehUpds);
Rect findGroupRectangle(const std::vector<model::Vehicle> & group);
double findGroupDensity(const std::vector<model::Vehicle> & group);
Point findCenter(const Rect & rect);
Point findCenter(const std::vector<model::Vehicle> & group);
double findDistToNearestUnit(const std::vector<model::Vehicle>& group, std::map<long, model::Vehicle>& vehicles);
double findDistToNearestUnit(const model::Vehicle & unit, std::map<long, model::Vehicle>& vehicles);
model::WeatherType getWeatherForUnit(const model::Vehicle & veh, const model::World & w);
model::TerrainType getTerrainForUnit(const model::Vehicle & veh, const model::World & w);
model::Move generateSelectMove(const Point & topLeft, const Point & bottomRight, model::VehicleType vType = model::VehicleType::_UNKNOWN_);
model::Move generateSelectMove(int groupNo);
Point computeFacilityCenter(const model::Facility & fac);
int orientation(Point p, Point q, Point r);
int distSq(Point p1, Point p2);
int compare(const void *vp1, const void *vp2);
Point nextToTop(std::stack<Point> & S);
int area(Point a, Point b, Point c);
bool intersect_1(int a, int b, int c, int d);
bool intersect(Point a, Point b, Point c, Point d);

#endif