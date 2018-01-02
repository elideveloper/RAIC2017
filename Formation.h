#pragma once

#ifndef FORMATION_H_
#define FORMATION_H_

//#include <queue>
#include <list>

#include "DamageFeatures.h"
#include "Task.h"


class Formation {
	//std::queue<model::Move> currTask;
	//std::queue<Task> taskQueue;
	std::map<long, model::Vehicle> *allUnits;
	std::map<long, model::Vehicle> *allPrevUnits;

	Rect rectangle;
	Point center;
	DamageFeatures damageFeats;
	std::vector<model::WeatherType> findCurrWeatherTypes(const model::World& world) const;
	std::vector<model::TerrainType> findCurrTerrainTypes(const model::World& world) const;
	std::vector<Point> convexHull;

	std::vector<Point> getAllPoints() const;
	std::vector<Point> findConvexHull();
	Rect findRectangle() const;
	Point findCenter() const;
	void computeDamageFeatures();
	bool  isSimilarToLastMove(const model::Move & move);
	bool checkUnitForMoving(int unitKey) const;

public:
	FormationUnitsType unitsType;
	unsigned short index;
	std::list<int> unitsKeys;
	Point lastDestPoint;
	model::ActionType lastAction;
	//unsigned int nextMoveTick; or lastActionTickIndex =)

	Formation();
	Formation(unsigned short index);
	Formation(unsigned short index, const std::vector<int> vehKeys, FormationUnitsType unitsType, std::map<long, model::Vehicle> * allUnits, std::map<long, model::Vehicle> * allPrevUnits);
	Formation(unsigned short index, const std::list<int> vehKeys, FormationUnitsType unitsType, std::map<long, model::Vehicle> * allUnits, std::map<long, model::Vehicle> * allPrevUnits);
	Formation(const Formation& formation);
	Formation & operator= (const Formation& formation);
	void refresh();
	std::vector<Point> getConvexHull() const;
	Rect getRectangle() const;
	Point getCenter() const;
	std::vector<model::Vehicle> getUnits() const;
	bool checkFormationForMoving() const;
	bool canIntersect(const Formation * f) const;
	bool isSelected() const;
	bool isIntersect(const Formation * f) const;
	bool willIntersect(const Formation * f, const Point & myShift, const Point & fShift) const;
	double findAvgDurability() const;
	double findOptimalSpeed(const model::World& world) const;
	double getAvgDamageVsUnit(model::VehicleType vType) const;
	double getAvgDamageVsFormation(const Formation & formation) const;
	int getUnitIDHavingInVisionRangePoint(const Point & dest, const model::World & world) const;
	Task scale(double factor, Point p, std::function<bool(const model::World&, const model::Player & me)> canDoFunc = [](const model::World & w, const model::Player & me) -> bool {
		return true;
	});
	Task scale(double factor, std::function<bool(const model::World&, const model::Player & me)> canDoFunc = [](const model::World & w, const model::Player & me) -> bool {
		return true;
	});
	Task moveTo(const Point & dest, std::function<bool(const model::World&, const model::Player & me)> canDoFunc = [](const model::World & w, const model::Player & me) -> bool {
		return true;
	});
	Task rotate(double angle, std::function<bool(const model::World&, const model::Player & me)> canDoFunc = [](const model::World & w, const model::Player & me) -> bool {
		return true;
	});
	Task restructureGroundArmy(std::function<bool(const model::World&, const model::Player & me)> canDoFunc = [](const model::World & w, const model::Player & me) -> bool {
		return true;
	});
	Task squeezeGroundArmy(std::function<bool(const model::World&, const model::Player & me)> canDoFunc = [](const model::World & w, const model::Player & me) -> bool {
		return true;
	});

	// насколько ранена группа, т.е. сколько сильно раненных но еще живых юнитов осталось
	// направление движения и скорость группы
};


#endif