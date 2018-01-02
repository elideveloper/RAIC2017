#include "Utility.h"

Point p0;

double getDamageVsUnit(model::VehicleType defUnitType, const model::Vehicle & attacking)
{
	double damage = 0.0;
	double protection = 0.0;
	switch (defUnitType) {
	case model::VehicleType::TANK: {
		protection = (attacking.isAerial()) ? 60.0 : 80.0;
		damage = attacking.getGroundDamage() - protection;
	} break;
	case model::VehicleType::IFV: {
		protection = (attacking.isAerial()) ? 80.0 : 60.0;
		damage = attacking.getGroundDamage() - protection;
	} break;
	case model::VehicleType::FIGHTER: {
		protection = 70.0;
		damage = attacking.getAerialDamage() - protection;
	} break;
	case model::VehicleType::HELICOPTER: {
		protection = 40.0;
		damage = attacking.getAerialDamage() - protection;
	} break;
	case model::VehicleType::ARRV: {
		protection = (attacking.isAerial()) ? 20.0 : 50.0;
		damage = attacking.getGroundDamage() - protection;
	} break;
	}
	if (damage < 0.0) damage = 0.0;
	return damage;
}

Point getCellXY(const Point & p)
{
	int x = p.x / 32.0;
	int y = p.y / 32.0;
	return Point(x, y);
}

Point getCellXY(const model::Vehicle & veh)
{
	int x = veh.getX() / 32.0;
	int y = veh.getY() / 32.0;
	return Point(x, y);
}

double getRealVisionRangeForUnit(const model::Vehicle & veh, const model::World & w)
{
	double visionFactor = 1.0;
	if (veh.getType() == model::VehicleType::FIGHTER || veh.getType() == model::VehicleType::HELICOPTER) {
		model::WeatherType wType = getWeatherForUnit(veh, w);
		switch (wType) {
		case model::WeatherType::RAIN: visionFactor = 0.6;
			break;
		case model::WeatherType::CLOUD: visionFactor = 0.8;
			break;
		}
	}
	else {
		model::TerrainType tType = getTerrainForUnit(veh, w);
		switch (tType) {
		case model::TerrainType::FOREST: visionFactor = 0.8;
			break;
		}
	}
	return visionFactor * veh.getVisionRange();
}

int getNearestUnitID(const Point & dest, const std::map<long, model::Vehicle> & vehicles)
{
	long id = vehicles.begin()->first;
	double bestDistance = WORLD_LENGTH + WORLD_LENGTH;
	double dist = 0.0;
	for (const auto & v : vehicles) {
		dist = dest.distanceTo(v.second.getX(), v.second.getY());
		if (dist < bestDistance) {
			id = v.first;
			bestDistance = dist;
		}
	}
	return id;
}

double findDistToNearestUnit(const model::Vehicle & unit, std::map<long, model::Vehicle> & vehicles)
{
	int nearestUnitID = getNearestUnitID(Point(unit.getX(), unit.getY()), vehicles);
	return Point(unit.getX(), unit.getY()).distanceTo(vehicles.find(nearestUnitID)->second.getX(), vehicles.find(nearestUnitID)->second.getY());
}

Point getFartherVisiblePointInDirectionTo(const model::Vehicle & veh, double x, double y, const model::World & w)
{
	double visRange = getRealVisionRangeForUnit(veh, w);
	visRange -= visRange * 0.01;											// немного см€гчим дальность из-за флуктуаций
	if (veh.getDistanceTo(x, y) <= visRange) return Point(x, y);
	double lam = visRange / (veh.getDistanceTo(x, y) - visRange);
	return Point((veh.getX() + lam * x) / (1.0 + lam), (veh.getY() + lam * y) / (1.0 + lam));
}

Point getPointInDirectionForDistance(const Point & p1, const Point & p2, double dist)
{											
	double lam = dist / (p1.distanceTo(p2) - dist);
	return Point((p1.x + lam * p2.x) / (1.0 + lam), (p1.y + lam * p2.y) / (1.0 + lam));
}

model::Move generateSelectMove(const Point & topLeft, const Point & bottomRight, model::VehicleType vType)
{
	model::Move move;
	move.setRight(bottomRight.x);
	move.setBottom(bottomRight.y);
	move.setLeft(topLeft.x);
	move.setTop(topLeft.y);
	move.setVehicleType(vType);
	move.setAction(model::ActionType::CLEAR_AND_SELECT);
	return move;
}

model::Move generateSelectMove(int groupNo)
{
	model::Move move;
	move.setGroup(groupNo);
	move.setAction(model::ActionType::CLEAR_AND_SELECT);
	return move;
}

Point computeFacilityCenter(const model::Facility & fac)
{
	return Point(fac.getLeft() + TILE_LENGTH, fac.getTop() + TILE_LENGTH);
}

bool checkGroupForUpdateSmth(const std::vector<model::Vehicle> & group, const std::vector<model::VehicleUpdate> & vehUpds)
{
	for (int i = 0; i < group.size(); i++) {
		for (int j = 0; j < vehUpds.size(); j++) {
			if (group[i].getId() == vehUpds[j].getId()) {
				return true;
			}
		}
	}
	return false;
}

Rect findGroupRectangle(const std::vector<model::Vehicle>& group)
{
	Rect rec(Point(1024.0, 1024.0), Point());
	for (int i = 0; i < group.size(); i++) {
		if (group[i].getX() > rec.bottomRight.x) {
			rec.bottomRight.x = group[i].getX();
		}
		if (group[i].getY() > rec.bottomRight.y) {
			rec.bottomRight.y = group[i].getY();
		}
		if (group[i].getX() < rec.topLeft.x) {
			rec.topLeft.x = group[i].getX();
		}
		if (group[i].getY() < rec.topLeft.y) {
			rec.topLeft.y = group[i].getY();
		}
	}
	return rec;
}

double findGroupDensity(const std::vector<model::Vehicle>& group)
{
	Rect rec = findGroupRectangle(group);
	return (double)group.size() / rec.getSpace();
}

Point findCenter(const Rect & rect)
{
	return Point((rect.bottomRight.x + rect.topLeft.x) / 2.0, (rect.bottomRight.y + rect.topLeft.y) / 2.0);
}

Point findCenter(const std::vector<model::Vehicle>& group)
{
	Rect rec = findGroupRectangle(group);
	return Point((rec.bottomRight.x + rec.topLeft.x) / 2.0, (rec.bottomRight.y + rec.topLeft.y) / 2.0);
}

// находит типа ближайшего моего к какой-то группе
double findDistToNearestUnit(const std::vector<model::Vehicle> & group, const std::map<long, model::Vehicle> & vehicles)
{
	Rect rec = findGroupRectangle(group);
	int nearTopLeftID = getNearestUnitID(rec.topLeft, vehicles);
	int nearBottomRightID = getNearestUnitID(rec.bottomRight, vehicles);
	double topLeftDist = rec.topLeft.distanceTo(vehicles.find(nearTopLeftID)->second.getX(), vehicles.find(nearTopLeftID)->second.getY());
	double bottomRightDist = rec.bottomRight.distanceTo(vehicles.find(nearBottomRightID)->second.getX(), vehicles.find(nearBottomRightID)->second.getY());
	return (topLeftDist <= bottomRightDist) ? topLeftDist : bottomRightDist;
}

model::WeatherType getWeatherForUnit(const model::Vehicle & veh, const model::World & w)
{
	Point p = getCellXY(veh);
	return w.getWeatherByCellXY()[p.x][p.y];
}

model::TerrainType getTerrainForUnit(const model::Vehicle & veh, const model::World & w)
{
	Point p = getCellXY(veh);
	return w.getTerrainByCellXY()[p.x][p.y];
}

int orientation(Point p, Point q, Point r)
{
	int val = (q.y - p.y) * (r.x - q.x) -
		(q.x - p.x) * (r.y - q.y);

	if (val == 0) return 0;  // colinear
	return (val > 0) ? 1 : 2; // clock or counterclock wise
}

int distSq(Point p1, Point p2)
{
	return (p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y);
}

int compare(const void *vp1, const void *vp2)
{
	Point *p1 = (Point *)vp1;
	Point *p2 = (Point *)vp2;
	int o = orientation(p0, *p1, *p2);
	if (o == 0) return (distSq(p0, *p2) >= distSq(p0, *p1)) ? -1 : 1;
	return (o == 2) ? -1 : 1;
}

Point nextToTop(std::stack<Point> &S)
{
	Point p = S.top();
	S.pop();
	Point res = S.top();
	S.push(p);
	return res;
}

int area(Point a, Point b, Point c) 
{
	return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

bool intersect_1(int a, int b, int c, int d) 
{
	if (a > b)  std::swap(a, b);
	if (c > d)  std::swap(c, d);
	return MAX(a, c) <= MIN(b, d);
}

bool intersect(Point a, Point b, Point c, Point d) 
{
	return intersect_1(a.x, b.x, c.x, d.x)
		&& intersect_1(a.y, b.y, c.y, d.y)
		&& area(a, b, c) * area(a, b, d) <= 0
		&& area(c, d, a) * area(c, d, b) <= 0;
}