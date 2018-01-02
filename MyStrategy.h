#pragma once

#ifndef _MY_STRATEGY_H_
#define _MY_STRATEGY_H_

#include "Strategy.h"
#include "Formation.h"
#include "Individual.h"

//#include <queue>
#include <deque>

using namespace model;

// хранить завоевынные мной здания производящие технику. чтобы туда не наступали и чтобы через N тиков собрать оттуда войска и оформить в формацию
class MyStrategy : public Strategy {
	long myID;
	int newFormsCounter;					// пока так потом нормально сделать
	int actionStepTicks;
	bool doNuclearStrike;
	bool canAttack;
	int minNextTickIndex;
	std::vector<Formation*> myFormations;
	std::vector<Formation*> constructingFormations;
	std::vector<Facility> facilities;
	std::deque<std::function<model::Move(const model::World & w)> > currTask;
	std::deque<Task> taskQueue;
	bool* actionsHistory;
	int countActionsForLastPeriod(int tickIndex);
	bool canIDoActions(int tickIndex);
	void setFirstTasksPool(const World & world, const Player& me);
	void initFirstLocationRects(Rect & recT, Rect & recI, Rect & recF, Rect & recH, Rect & recA, Rect & recAll, const World & world);
	//void prepareNuclearStrikes(const World & world, const Player& me, int fromTickIndex);
	void move(const Player& me, const World& world, const Game& game, Move& move) override;
	//std::vector<Vehicle> getVehiclesOfGroup(int groupNo) const;
	std::vector<Vehicle> getSelectedVehicles() const;
	std::vector<int> getVehicleIDsInRectangle(const Rect & rec, VehicleType vType = VehicleType::_UNKNOWN_);
	//void formRegiments(const Rect & rec, const World & world, VehicleType vehType);
	void formDivision(const Rect & rec, short indexSuffix, int tickIndex = 0, VehicleType vType = VehicleType::_UNKNOWN_);
	void formDivisionInChessOrder(const Rect & rec, short indexSuffix, int tickIndex = 0, VehicleType vType = VehicleType::_UNKNOWN_);
	void refreshMyFormations();
	std::vector<Formation> findOppFormations();			// находит формирования противника
	//std::vector<Formation> findMyFormations();			
	Rect findGroupRectangle(std::vector<int> vehIDs);
	void patrol(Formation & formation, std::vector<Point> points, const World & world);
	void stayNear(Formation & formation, Point dest, double dist, double deviation);
	void convergeMyFormations(std::vector<Formation> & myFormations);
	int getOptimalTargetNo(const std::vector<Formation> & oppFormations, const Formation & myFormation);
	Point getOptimalTargetPoint(const std::vector<Formation> & oppFormations, const Formation & myFormation, const model::World & world);
	int getWorstTargetNo(const std::vector<Formation> & oppFormations, const Formation & myFormation);
	Point getWorstTargetPoint(const std::vector<Formation> & oppFormations, const Formation & myFormation, const model::World & world);
	Formation mergeFormations(const Formation & f1, const Formation & f2, unsigned short index);

	std::vector<Point> findPotentialTargetPoints(int myFormationIndex, const std::vector<Formation> & oppFormations);
	std::vector<Point> computeRouteNextStepsForMyFormations(const std::vector<Point> & targetPoints, const World & world, const std::vector<Point> & worstPoints);
	void evaluateAndSortPositions(const std::vector<Point> & targetPoints, const std::vector<std::vector<Point> > & formationsPotentialPositions, Generation & generation, const std::vector<Point> & worstPoints);
	Point findNearestNewFacilityCenter(const Formation & formation);
	int getMyNearestFormationIndex(Point p) const;
	std::list<Point> findPointsForNuclearStrike(const std::vector<Formation> & oppFormations) const;
	int getUnitIDHavingInVisionRangePoint(const Point & nsPoint, const model::World& world);
	void tryPerformNuclearStrike(const std::vector<Formation> & oppFormations, const World & world);
	void avoidNuclearStrike(const model::World& world, Point dest, int ticksLeft);
	int countMyControlCenters() const ;

	void setNextMove(const Player& me, const World& world, Move& move);
	
public:
	//std::map<long, Vehicle> allUnits;
	//std::map<long, Vehicle> allPrevUnits;
    MyStrategy();
	Point computeFormationLastTickShift(const Formation & formation) const;
	Point computeFormationPositionAfterNTicks(const Formation & formation, int numTicks) const;
};


#endif
