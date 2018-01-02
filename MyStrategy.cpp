#include "MyStrategy.h"

#include <iostream>
#include <cmath>
#include <algorithm>
#include <cstdlib>

const int TANK_FORMATION = 1;
const int IFV_FORMATION = 2;
const int FIGHTER_FORMATION = 3;
const int HELICOPTER_FORMATION = 4;
const int ARRV_FORMATION = 5;
const int GROUND_ARMY = 10;
const int GROUND_ARMY_1 = 11;
const int GROUND_ARMY_2 = 12;
const int IFV_1 = 21;
const int IFV_2 = 22;
const int IFV_3 = 23;
const int IFV_4 = 24;

const double MAX_DIST_BETWEEN_UNITS_INSIDE_FORMATION = 6.5;
const int ACTION_STEP_TICKS = 60;
const double FORMATION_RADIUS = 19.5;

const int GENERATION_SIZE = 10;
const int NUM_CROSS = 6;
//const double MUTATION_PROBABILITY = 0.1;
const int NUM_ITERATIONS = 2000;


using namespace std;


std::map<long, Vehicle> allUnits;
std::map<long, Vehicle> allPrevUnits;


// можно из одной формации выводить вторую когда юниты расходятся, при этом новую пушить в вектор формаций и дублировать задание для нее

// добавить штраф также за близость к оппоненту как за край карты

// после производства техники брать ее в новую формацию

// поворачивать формацию по направлению движения фронтом, а потом идти уже.
// откорректировать коэффициент дистанции

// сжимать периодически формации если расходятся, есть готовая фукция

// нужна функция чтобы определить тип своей формации, чтобы в зависимости от типа назначались задания

void MyStrategy::move(const Player& me, const World& world, const Game& game, Move& move) {
	move.setAction(ActionType::NONE);
	if (world.getTickIndex() == 0) {
		// init
		this->setFirstTasksPool(world, me);
		this->facilities = world.getFacilities();
	}
	else {
		// refresh info about all vehicles, keep prev state
		if (world.getNewVehicles().size() > 0) {
			for (int i = 0; i < world.getNewVehicles().size(); i++) {
				allUnits[world.getNewVehicles()[i].getId()] = world.getNewVehicles()[i];	// получаем информацию о новосозданных юнитах
			}
		}
		this->facilities = world.getFacilities();
		allPrevUnits = allUnits;
		for (const VehicleUpdate & u : world.getVehicleUpdates()) allUnits[u.getId()] = Vehicle(allUnits.find(u.getId())->second, u);
		this->refreshMyFormations();
	}

	if (world.getTickIndex() >= 450) {
		// ищем вражеские формации 
		std::vector<Formation> oppFormations = this->findOppFormations();

		if (!this->doNuclearStrike && me.getRemainingNuclearStrikeCooldownTicks() == 0 && me.getNextNuclearStrikeTickIndex() == -1) this->tryPerformNuclearStrike(oppFormations, world);

		// avoid Nuclear Strike
		if (!this->doNuclearStrike && world.getOpponentPlayer().getNextNuclearStrikeTickIndex() - world.getTickIndex() == 29 && me.getRemainingActionCooldownTicks() <= 20) {
			this->avoidNuclearStrike(world, Point(world.getOpponentPlayer().getNextNuclearStrikeX(), world.getOpponentPlayer().getNextNuclearStrikeY()), 29 - me.getRemainingActionCooldownTicks());
		}
		// assign actions for my formations
		else if (this->canIDoActions(world.getTickIndex()) && this->myFormations.size() > 0) {

			//printf("can do in tick: %d", world.getTickIndex());

			//printf("\nnumFormations=%d;", this->constructingFormations.size());
			//for (Formation* f : this->constructingFormations) {
			//	printf("\n	formation %d size: %d;", f->index, f->unitsKeys.size());
			//	/*for (int u : f->unitsKeys) {
			//	printf("	id:%d;", u);
			//	}*/
			//}

			//printf("\nnumFormations=%d;", this->myFormations.size());
			//for (Formation* f : this->myFormations) {
			//	printf("\n	formation %d size: %d;", f->index, f->unitsKeys.size());
			//	/*for (int u : f->unitsKeys) {
			//		printf("	id:%d;", u);
			//	}*/
			//}

			// find target points and worst points for each my formation
			std::vector<Point> targetPoints;
			std::vector<Point> worstPoints;		// нежелательных точек несколько должно быть
			for (Formation* f : this->myFormations) {
				//printf("\n index=%d; durabilityPercent=%.2f;, size=%d;", f.index, f.findAvgDurability(), f.unitsKeys.size());
				//printf("\n index=%d; FuturePosition x=%.2f; y=%.2f\n", f.index, this->computeFormationPositionAfterNTicks(f, ACTION_STEP_TICKS).x, this->computeFormationPositionAfterNTicks(f, ACTION_STEP_TICKS).y);
				targetPoints.push_back(this->getOptimalTargetPoint(oppFormations, *f, world));
				worstPoints.push_back(this->getWorstTargetPoint(oppFormations, *f, world));
			}

			std::vector<Point> nextStepPoints = computeRouteNextStepsForMyFormations(targetPoints, world, worstPoints);
			for (int i = 0; i < this->myFormations.size(); i++) {
				//printf("\n nextStepPoint: index=%d; x=%.2f, y=%.2f;\n", this->myFormations[i].index, nextStepPoints[i].x, nextStepPoints[i].y);
				this->taskQueue.push_back(this->myFormations[i]->moveTo(nextStepPoints[i]));
			}
		}
	}

	// производство техники - если меньше 500 то можно спокойно производить
	// закомментирую пока нет нормального управления новой техникой в связи с новыми правилами
	//if (world.getTickIndex() % 100 == 0) {
	//	for (const Facility & fac : this->facilities) {
	//		if (fac.getType() == FacilityType::VEHICLE_FACTORY && fac.getOwnerPlayerId() == this->myID) {
	//			if (fac.getVehicleType() == VehicleType::_UNKNOWN_) {
	//				std::function<Move(const World &)> defMove = [fac](const World & world) -> Move {
	//					Move m;
	//					m.setVehicleType(VehicleType::TANK);
	//					m.setFacilityId(fac.getId());
	//					m.setAction(ActionType::SETUP_VEHICLE_PRODUCTION);
	//					return m;
	//				};
	//				this->currTask.push_back(defMove);
	//			}
	//			else {
	//				if (world.getTickIndex() % 2400 == 0) {
	//					Rect rec(Point(fac.getLeft(), fac.getTop()), Point(fac.getLeft() + 2 * TILE_LENGTH, fac.getTop() + 2 * TILE_LENGTH));
	//					std::function<Move(const World &)> defMove;
	//					if (fac.getVehicleType() == VehicleType::TANK) {
	//						defMove = [fac](const World & world) -> Move {
	//							Move m;
	//							m.setVehicleType(VehicleType::HELICOPTER);
	//							m.setFacilityId(fac.getId());
	//							m.setAction(ActionType::SETUP_VEHICLE_PRODUCTION);
	//							return m;
	//						};
	//						this->currTask.push_back(defMove);
	//						//this->formDivision(rec, FREE_FORM_INDEX + this->newFormsCounter, 0, VehicleType::TANK);
	//						//this->myFormations.push_back(Formation(FREE_FORM_INDEX + this->newFormsCounter, this->getVehicleIDsInRectangle(rec), FormationUnitsType::MOSTLY_GROUND, &allUnits, &allPrevUnits));
	//					}
	//					else {
	//						defMove = [fac](const World & world) -> Move {
	//							Move m;
	//							m.setVehicleType(VehicleType::TANK);
	//							m.setFacilityId(fac.getId());
	//							m.setAction(ActionType::SETUP_VEHICLE_PRODUCTION);
	//							return m;
	//						};
	//						this->currTask.push_back(defMove);
	//						//this->formDivision(rec, FREE_FORM_INDEX + this->newFormsCounter, 0, VehicleType::HELICOPTER);
	//						//this->myFormations.push_back(Formation(FREE_FORM_INDEX + this->newFormsCounter, this->getVehicleIDsInRectangle(rec), FormationUnitsType::MOSTLY_AERIAL, &allUnits, &allPrevUnits));
	//					}
	//					// scale
	//					defMove = [rec](const World & world) -> Move {
	//						model::Move move;
	//						move.setRight(rec.bottomRight.x);
	//						move.setBottom(rec.bottomRight.y);
	//						move.setLeft(rec.topLeft.x);
	//						move.setTop(rec.topLeft.y);
	//						move.setVehicleType(VehicleType::_UNKNOWN_);
	//						move.setAction(model::ActionType::CLEAR_AND_SELECT);
	//						return move;
	//					};
	//					this->currTask.push_back(defMove);
	//					defMove = [rec](const World & world) -> Move {
	//						model::Move move;
	//						move.setX(rec.topLeft.x);
	//						move.setY(rec.topLeft.y);
	//						move.setFactor(0.1);
	//						move.setAction(ActionType::SCALE);
	//						return move;
	//					};
	//					this->currTask.push_back(defMove);
	//					//this->newFormsCounter++;
	//					//this->convergeMyFormations(this->myFormations);
	//				}
	//			}
	//		}
	//	}
	//}

	this->setNextMove(me, world, move);

	if (move.getAction() != ActionType::NONE) this->actionsHistory[world.getTickIndex()] = true;

	return;
}

int MyStrategy::countActionsForLastPeriod(int tickIndex)
{
	int acts = 0;
	for (int i = tickIndex; i > tickIndex - ACTION_STEP_TICKS && i >= 0; i--) {
		if (this->actionsHistory[i]) acts++;
	}
	return acts;
}

bool MyStrategy::canIDoActions(int tickIndex)
{
	// пока построение не закончено то будем редко действовать
	if (!this->canAttack) {
		if (tickIndex % 60 == 0) return true;
		else return false;
	}

	// если формаций будет больше то искусственно увеличивать придется период и количество действий за период
	int numNeedActions = this->myFormations.size() * 2;
	int numActionsPerPeriod = 12 + 3 * this->countMyControlCenters();
	if (this->countActionsForLastPeriod(tickIndex + numNeedActions) + numNeedActions <= numActionsPerPeriod && tickIndex > this->minNextTickIndex) {
		this->minNextTickIndex = tickIndex + numNeedActions * 2;
		return true;
	}
	return false;
}

void MyStrategy::setFirstTasksPool(const World & world, const Player& me)
{
	this->myID = me.getId();
	Rect recT = Rect(Point(world.getWidth(), world.getHeight()), Point());
	Rect recF = Rect(Point(world.getWidth(), world.getHeight()), Point());
	Rect recH = Rect(Point(world.getWidth(), world.getHeight()), Point());
	Rect recI = Rect(Point(world.getWidth(), world.getHeight()), Point());
	Rect recA = Rect(Point(world.getWidth(), world.getHeight()), Point());
	Rect recAll = Rect(Point(world.getWidth(), world.getHeight()), Point());
	initFirstLocationRects(recT, recI, recF, recH, recA, recAll, world);				

	// Истребитель
	/*unsigned short indexF1 = 31;
	unsigned short indexF2 = 32;
	Point center = findCenter(recF);
	this->formDivision(Rect(Point(recF.bottomRight.x, center.y),Point(recF.bottomRight.x, recF.bottomRight.y)), indexF1, 0, VehicleType::FIGHTER);
	this->myFormations.push_back(Formation(indexF1, this->getVehiclesInRectangle(Rect(Point(recF.bottomRight.x, center.y), Point(recF.bottomRight.x, recF.bottomRight.y)))));
	this->formDivision(Rect(Point(recF.bottomRight.x, recF.topLeft.y),Point(recF.bottomRight.x, center.y)), indexF2, 0, VehicleType::FIGHTER);
	this->myFormations.push_back(Formation(indexF2, this->getVehiclesInRectangle(Rect(Point(recF.bottomRight.x, recF.topLeft.y), Point(recF.bottomRight.x, center.y)))));
	*/

	this->myFormations.reserve(10);
	this->constructingFormations.reserve(10);
	double deltaShift = 4.002;

	//// FIGHTERs
	//this->formDivision(recF, FIGHTER_FORMATION, 0, VehicleType::FIGHTER);
	//this->myFormations.push_back(new Formation(FIGHTER_FORMATION, this->getVehicleIDsInRectangle(recF), FormationUnitsType::MOSTLY_AERIAL, &allUnits, &allPrevUnits));
	//// HELICOPTERs
	//this->formDivision(recH, HELICOPTER_FORMATION, 0, VehicleType::HELICOPTER);
	//this->myFormations.push_back(new Formation(HELICOPTER_FORMATION, this->getVehicleIDsInRectangle(recH), FormationUnitsType::MOSTLY_AERIAL, &allUnits, &allPrevUnits));
	//// IFVs
	//this->formDivision(recI, IFV_FORMATION, 0, VehicleType::IFV);
	//this->myFormations.push_back(new Formation(IFV_FORMATION, this->getVehicleIDsInRectangle(recI), FormationUnitsType::MOSTLY_GROUND, &allUnits, &allPrevUnits));
	//// ARRVs
	//this->formDivision(recA, ARRV_FORMATION, 0, VehicleType::ARRV);
	//this->myFormations.push_back(new Formation(ARRV_FORMATION, this->getVehicleIDsInRectangle(recA), FormationUnitsType::MOSTLY_GROUND, &allUnits, &allPrevUnits));
	//// TANKs
	//this->formDivision(recT, TANK_FORMATION, 0, VehicleType::TANK);
	//this->myFormations.push_back(new Formation(TANK_FORMATION, this->getVehicleIDsInRectangle(recT), FormationUnitsType::MOSTLY_GROUND, &allUnits, &allPrevUnits));
	//
	//// сначала в одном порядке потом в обратном чтобы экономить действия на выделение
	//for (std::vector<Formation*>::reverse_iterator itF = this->myFormations.rbegin(); itF < this->myFormations.rend(); ++itF) {
	//	this->taskQueue.push_back((*itF)->rotate(PI / 4.0));
	//}

	//for (std::vector<Formation*>::iterator itF = this->myFormations.begin(); itF < this->myFormations.end(); ++itF) {
	//	this->taskQueue.push_back((*itF)->scale(0.20, (*itF)->getCenter(), [itF](const model::World & w, const model::Player & me) -> bool {
	//		return !(*itF)->checkFormationForMoving();
	//	}));
	//}


	Formation* fighters = new Formation(FIGHTER_FORMATION, this->getVehicleIDsInRectangle(recF), FormationUnitsType::MOSTLY_AERIAL, &allUnits, &allPrevUnits);
	this->myFormations.push_back(fighters);
	Formation* helics = new Formation(HELICOPTER_FORMATION, this->getVehicleIDsInRectangle(recH), FormationUnitsType::MOSTLY_AERIAL, &allUnits, &allPrevUnits);
	this->constructingFormations.push_back(helics);
	Formation* ifvs = new Formation(IFV_FORMATION, this->getVehicleIDsInRectangle(recI), FormationUnitsType::MOSTLY_GROUND, &allUnits, &allPrevUnits);
	this->constructingFormations.push_back(ifvs);
	Formation* arrvs = new Formation(ARRV_FORMATION, this->getVehicleIDsInRectangle(recA), FormationUnitsType::MOSTLY_GROUND, &allUnits, &allPrevUnits);
	this->constructingFormations.push_back(arrvs);
	Formation* tanks = new Formation(TANK_FORMATION, this->getVehicleIDsInRectangle(recT), FormationUnitsType::MOSTLY_GROUND, &allUnits, &allPrevUnits);
	this->constructingFormations.push_back(tanks);

	// двигаем ARRV к Танкам
	Point aDest1;
	Point aDest2;
	Point ifvPos(ifvs->getCenter());
	this->formDivision(recI, IFV_FORMATION, 0, VehicleType::IFV);
	// если по вертикали в один ряд стоят и БМП мешают
	if (abs(recA.topLeft.x - recT.topLeft.x) < 1.0) {
		if (abs(recA.topLeft.x - recI.topLeft.x) < 1.0 && recI.topLeft.y < MAX(recT.topLeft.y, recA.topLeft.y) && recI.topLeft.y > MIN(recT.topLeft.y, recA.topLeft.y)) {
			ifvPos = Point(ifvs->getCenter().x + 70.0, ifvs->getCenter().y);
			this->taskQueue.push_back(ifvs->moveTo(ifvPos));
		}
		aDest1 = Point(arrvs->getCenter().x - deltaShift, arrvs->getCenter().y);
		aDest2 = Point(arrvs->getCenter().x - deltaShift, tanks->getCenter().y - deltaShift);
	}
	// если по горизонтали в один ряд стоят и БМП мешают
	else if (abs(recA.topLeft.y - recT.topLeft.y) < 1.0) {
		if (abs(recA.topLeft.y - recI.topLeft.y) < 1.0 && recI.topLeft.x < MAX(recT.topLeft.x, recA.topLeft.x) && recI.topLeft.x > MIN(recT.topLeft.x, recA.topLeft.x)) {
			ifvPos = Point(ifvs->getCenter().x, ifvs->getCenter().y + 80.0);
			this->taskQueue.push_back(ifvs->moveTo(ifvPos));
		}
		aDest1 = Point(arrvs->getCenter().x, arrvs->getCenter().y - deltaShift);
		aDest2 = Point(tanks->getCenter().x - deltaShift, arrvs->getCenter().y - deltaShift);
	}
	// общий случай
	else {
		Point upperCommon((tanks->getCenter().y < arrvs->getCenter().y) ? arrvs->getCenter().x : tanks->getCenter().x, MIN(tanks->getCenter().y, arrvs->getCenter().y));
		Point bottomCommon((arrvs->getCenter().y > tanks->getCenter().y) ? tanks->getCenter().x : arrvs->getCenter().x, MAX(tanks->getCenter().y, arrvs->getCenter().y));
		// если верхняя точка свободна
		if (!recI.isIntersect((upperCommon.x > recA.bottomRight.x) ? Rect(recA.topLeft, upperCommon) : Rect(upperCommon, recA.bottomRight))) {
			aDest1 = (abs(upperCommon.y - arrvs->getCenter().y) < 1.0) ? Point(upperCommon.x - deltaShift, upperCommon.y) : Point(upperCommon.x, upperCommon.y - deltaShift);
			aDest2 = (abs(upperCommon.y - arrvs->getCenter().y) < 1.0) ? Point(upperCommon.x - deltaShift, tanks->getCenter().y - deltaShift) : Point(tanks->getCenter().x - deltaShift, upperCommon.y - deltaShift);
		}
		// если верхняя НЕ свободна то идем в нижнюю точку
		else {
			aDest1 = (abs(bottomCommon.y - arrvs->getCenter().y) < 1.0) ? Point(bottomCommon.x - deltaShift, bottomCommon.y) : Point(bottomCommon.x, bottomCommon.y - deltaShift);
			aDest2 = (abs(bottomCommon.y - arrvs->getCenter().y) < 1.0) ? Point(bottomCommon.x - deltaShift, tanks->getCenter().y - deltaShift) : Point(tanks->getCenter().x - deltaShift, bottomCommon.y - deltaShift);
		}
	}

	this->formDivision(recA, ARRV_FORMATION, 0, VehicleType::ARRV);
	// ARRVs становятся в одну линию с танками
	this->taskQueue.push_back(arrvs->moveTo(aDest1, [ifvs](const model::World & w, const model::Player & me) -> bool {
		return !ifvs->checkFormationForMoving();
	}));
	// ARRVs раскрываются и идут в танки
	this->taskQueue.push_back(arrvs->scale(1.334, aDest1, [arrvs](const model::World & w, const model::Player & me) -> bool {
		return !arrvs->checkFormationForMoving() && arrvs->lastAction == ActionType::MOVE;
	}));
	this->taskQueue.push_back(arrvs->moveTo(aDest2, [arrvs, tanks](const model::World & w, const model::Player & me) -> bool {
		return !arrvs->checkFormationForMoving() && arrvs->lastAction == ActionType::SCALE 
			&& tanks->lastAction == ActionType::SCALE && !tanks->checkFormationForMoving();
	}));
	// Танки раскрываются
	this->formDivision(recT, TANK_FORMATION, 0, VehicleType::TANK);
	this->taskQueue.push_back(tanks->scale(1.334, tanks->getCenter(), [arrvs](const model::World & w, const model::Player & me) -> bool {
		return arrvs->lastAction == ActionType::SCALE;
	}));

	Point commonCenter(tanks->getCenter().x - deltaShift * 0.5, tanks->getCenter().y - deltaShift * 0.5);
	Point commonCenterNext(commonCenter);

	// Form F group
	this->formDivision(recH, HELICOPTER_FORMATION, 0, VehicleType::HELICOPTER);
	this->formDivisionInChessOrder(recF, FIGHTER_FORMATION, 0, VehicleType::FIGHTER);
	this->taskQueue.push_back(fighters->scale(0.1, fighters->getCenter(), [fighters](const model::World & w, const model::Player & me) -> bool {
		return !fighters->checkFormationForMoving();
	}));
	// убрать вертолеты с пути если мешают
	if (recH.isIntersect(Rect(fighters->getCenter(), Point(WORLD_LENGTH * 0.5, WORLD_LENGTH * 0.5)))) {
		Point dest;
		Point top(fighters->getCenter().x + 120.0, fighters->getCenter().y - 10.0);
		Point left(fighters->getCenter().x - 10.0, fighters->getCenter().y + 120.0);
		double k = (WORLD_LENGTH * 0.5 - fighters->getCenter().y) / (WORLD_LENGTH * 0.5 - fighters->getCenter().x);
		double b = (WORLD_LENGTH * 0.5*fighters->getCenter().y - fighters->getCenter().x* WORLD_LENGTH * 0.5) / (WORLD_LENGTH * 0.5 - fighters->getCenter().x);
		if ((k * helics->getCenter().x - helics->getCenter().y + b) * (k * left.x - left.y + b) > 0) dest = left;
		else dest = top;
		this->taskQueue.push_back(helics->moveTo(dest));		// отлетаем
	}
	this->taskQueue.push_back(fighters->moveTo(Point(WORLD_LENGTH * 0.25, WORLD_LENGTH * 0.25), [fighters, helics](const model::World & w, const model::Player & me) -> bool {
		return !fighters->checkFormationForMoving() && !helics->checkFormationForMoving() && fighters->lastAction == ActionType::SCALE;
	}));

	// направим вертолеты к общей земной армии
	this->taskQueue.push_back(helics->moveTo(commonCenter, [tanks, fighters, helics](const model::World & w, const model::Player & me) -> bool {
		return tanks->lastAction == ActionType::SCALE && !tanks->checkFormationForMoving() 
			&& helics->getCenter().x < fighters->getCenter().x && helics->getCenter().y < fighters->getCenter().y && w.getTickIndex() > 200;
	}));

	// собираем наземную армию
	Task task(GROUND_ARMY);
	std::function<Move(const World &)> defMove;
	commonCenterNext = Point(MAX(70.0, commonCenter.x), MAX(70.0, commonCenter.y));
	task.refresh();
	defMove = [commonCenter, this](const World & world) -> Move {
		/*this->constructingFormations.erase(std::remove_if(this->constructingFormations.begin(), this->constructingFormations.end(), [](Formation* f) {
			return f->index == TANK_FORMATION || f->index == HELICOPTER_FORMATION || f->index == ARRV_FORMATION;
		}), this->constructingFormations.end());*/
		Formation* gr = new Formation(GROUND_ARMY, this->getVehicleIDsInRectangle(Rect(Point(commonCenter.x - 40.0, commonCenter.y - 40.0), Point(commonCenter.x + 40.0, commonCenter.y + 40.0))), FormationUnitsType::MIXED, &allUnits, &allPrevUnits);
		this->constructingFormations.push_back(gr);
		this->taskQueue.push_back(gr->rotate(PI / 4.0, [this](const World & w, const Player & me) -> bool {
			for (const Formation* f : this->constructingFormations) if (f->index == GROUND_ARMY && !f->checkFormationForMoving()) return true;
			return false;
		}));
		return generateSelectMove(Point(commonCenter.x - 40.0, commonCenter.y - 40.0), Point(commonCenter.x + 40.0, commonCenter.y + 40.0));
	};
	task.defMoves.push_back(defMove);
	defMove = [](const World & world) -> Move {
		Move move;
		move.setAction(ActionType::ASSIGN);
		move.setGroup(GROUND_ARMY);
		return move;
	};
	task.defMoves.push_back(defMove);
	if (commonCenterNext.x != commonCenter.x || commonCenterNext.y != commonCenter.y) {
		defMove = [commonCenterNext, commonCenter](const World & world) -> Move {
			Move move;
			move.setAction(ActionType::MOVE);
			move.setY(commonCenterNext.y - commonCenter.y);
			move.setX(commonCenterNext.x - commonCenter.x);
			move.setMaxSpeed(0.3);
			return move;
		};
		task.defMoves.push_back(defMove);
	}
	task.canDo = [helics, commonCenter, arrvs](const World & w, const Player & me) -> bool {
		return w.getTickIndex() >= 380 && helics->getCenter().distanceTo(commonCenter) < 10.0 && !helics->checkFormationForMoving()
			&& arrvs->getCenter().distanceTo(commonCenter) < 10.0 && !arrvs->checkFormationForMoving();
	};
	this->taskQueue.push_back(task);

	// двигаем IFVs на нужное место для дальнейшей переформировки и разделяем на 3 группы
	Point ifvDest(WORLD_LENGTH * 0.28, WORLD_LENGTH * 0.2);
	Point iDest;
	if (ifvPos.x > commonCenterNext.x + 50.0 || ifvPos.y > commonCenterNext.y + 100.0) {
		// если построение армии не мешает то сразу идем на точку назначения
		this->taskQueue.push_back(ifvs->moveTo(ifvDest, [ifvs, ifvPos, arrvs, aDest2](const World & w, const Player & me) -> bool {
			return !ifvs->checkFormationForMoving() && ifvs->getCenter().distanceTo(ifvPos) < 10.0 && arrvs->getCenter().distanceTo(aDest2) < 20.0;
		}));
	}
	else {
		Point upperCommon(ifvDest.x, ifvPos.y);
		Point bottomCommon(ifvPos.x, ifvDest.y);
		if (!recT.isIntersect(Rect(Point(ifvPos.x, ifvPos.y - 20.0), Point(upperCommon.x, upperCommon.y + 20.0)))) iDest = upperCommon;
		else iDest = bottomCommon;
		this->taskQueue.push_back(ifvs->moveTo(iDest, [ifvs, ifvPos, arrvs, aDest2](const World & w, const Player & me) -> bool {
			return !ifvs->checkFormationForMoving() && ifvs->getCenter().distanceTo(ifvPos) < 10.0 && arrvs->getCenter().distanceTo(aDest2) < 20.0;
		}));
		this->taskQueue.push_back(ifvs->moveTo(ifvDest, [ifvs, iDest](const World & w, const Player & me) -> bool {
			return !ifvs->checkFormationForMoving() && ifvs->getCenter().distanceTo(iDest) < 10.0 && ifvs->lastAction == ActionType::MOVE;
		}));
	}
	task.refresh();
	defMove = [ifvDest, this](const World & world) -> Move {
		Formation* i1 = new Formation(IFV_1, this->getVehicleIDsInRectangle(Rect(Point(ifvDest.x - 30.0, ifvDest.y - 30.0), ifvDest), VehicleType::IFV), FormationUnitsType::MOSTLY_GROUND, &allUnits, &allPrevUnits);
		this->constructingFormations.push_back(i1);
		this->taskQueue.push_back(i1->scale(0.1));
		Formation* i2 = new Formation(IFV_2, this->getVehicleIDsInRectangle(Rect(Point(ifvDest.x - 30.0, ifvDest.y), Point(ifvDest.x, ifvDest.y + 30.0)), VehicleType::IFV), FormationUnitsType::MOSTLY_GROUND, &allUnits, &allPrevUnits);
		this->constructingFormations.push_back(i2);
		this->taskQueue.push_back(i2->scale(0.1));
		Formation* i3 = new Formation(IFV_3, this->getVehicleIDsInRectangle(Rect(Point(ifvDest.x, ifvDest.y - 30.0), Point(ifvDest.x + 30.0, ifvDest.y + 30.0)), VehicleType::IFV), FormationUnitsType::MOSTLY_GROUND, &allUnits, &allPrevUnits);
		this->constructingFormations.push_back(i3);
		this->taskQueue.push_back(i3->scale(0.1));
		return generateSelectMove(Point(ifvDest.x - 30.0, ifvDest.y - 30.0), ifvDest, VehicleType::IFV);
	};
	task.defMoves.push_back(defMove);
	defMove = [](const World & world) -> Move {
		Move move;
		move.setAction(ActionType::ASSIGN);
		move.setGroup(IFV_1);
		return move;
	};
	task.defMoves.push_back(defMove);
	defMove = [ifvDest](const World & world) -> Move {
		return generateSelectMove(Point(ifvDest.x - 30.0, ifvDest.y), Point(ifvDest.x, ifvDest.y + 30.0), VehicleType::IFV);
	};
	task.defMoves.push_back(defMove);
	defMove = [](const World & world) -> Move {
		Move move;
		move.setAction(ActionType::ASSIGN);
		move.setGroup(IFV_2);
		return move;
	};
	task.defMoves.push_back(defMove);
	defMove = [ifvDest](const World & world) -> Move {
		return generateSelectMove(Point(ifvDest.x, ifvDest.y - 30.0), Point(ifvDest.x + 30.0, ifvDest.y + 30.0), VehicleType::IFV);
	};
	task.defMoves.push_back(defMove);
	defMove = [](const World & world) -> Move {
		Move move;
		move.setAction(ActionType::ASSIGN);
		move.setGroup(IFV_3);
		return move;
	};
	task.defMoves.push_back(defMove);
	task.canDo = [ifvs, ifvDest](const World & w, const Player & me) -> bool {
		return !ifvs->checkFormationForMoving() && ifvs->getCenter().distanceTo(ifvDest) < 10.0;
	};
	this->taskQueue.push_back(task);


	// выделяем нижнюю часть армии и направляем ниже
	task.refresh();
	defMove = [commonCenterNext, this](const World & world) -> Move {
		/*this->constructingFormations.erase(std::remove_if(this->constructingFormations.begin(), this->constructingFormations.end(), [](Formation* f) {
			return f->index == GROUND_ARMY;
		}), this->constructingFormations.end());*/
		Formation* gr1 = new Formation(GROUND_ARMY_1, this->getVehicleIDsInRectangle(Rect(Point(commonCenterNext.x - 55.0, commonCenterNext.y + 1.0), Point(commonCenterNext.x + 55.0, commonCenterNext.y + 55.0))), FormationUnitsType::MIXED, &allUnits, &allPrevUnits);
		this->constructingFormations.push_back(gr1);
		this->taskQueue.push_back(gr1->rotate(3 * PI / 4.0, [this](const World & w, const Player & me) -> bool {
			for (const Formation* f : this->constructingFormations) if (f->index == GROUND_ARMY_1 && !f->checkFormationForMoving()) return true;
			return false;
		}));
		this->taskQueue.push_back(gr1->scale(0.1, [this](const World & w, const Player & me) -> bool {
			for (const Formation* f : this->constructingFormations) if (f->index == GROUND_ARMY_1 && !f->checkFormationForMoving() && f->lastAction == ActionType::ROTATE) return true;
			return false;
		}));
		Formation* gr2 = new Formation(GROUND_ARMY_2, this->getVehicleIDsInRectangle(Rect(Point(commonCenterNext.x - 55.0, commonCenterNext.y - 55.0), Point(commonCenterNext.x + 55.0, commonCenterNext.y + 1.0))), FormationUnitsType::MIXED, &allUnits, &allPrevUnits);
		this->constructingFormations.push_back(gr2);
		this->taskQueue.push_back(gr2->rotate(-PI / 4.0, [this](const World & w, const Player & me) -> bool {
			for (const Formation* f : this->constructingFormations) if (f->index == GROUND_ARMY_2) return true;
			return false;
		}));
		this->taskQueue.push_back(gr2->scale(0.1, [this](const World & w, const Player & me) -> bool {
			for (const Formation* f : this->constructingFormations) if (f->index == GROUND_ARMY_2 && !f->checkFormationForMoving() && f->lastAction == ActionType::ROTATE) return true;
			return false;
		}));
		return generateSelectMove(Point(commonCenterNext.x - 55.0, commonCenterNext.y + 1.0), Point(commonCenterNext.x + 55.0, commonCenterNext.y + 55.0));
	};
	task.defMoves.push_back(defMove);
	defMove = [](const World & world) -> Move {
		Move move;
		move.setAction(ActionType::MOVE);
		move.setY(52.0);
		move.setX(0.0);
		move.setMaxSpeed(0.3);
		return move;
	};
	task.defMoves.push_back(defMove);
	defMove = [](const World & world) -> Move {
		Move move;
		move.setAction(ActionType::ASSIGN);
		move.setGroup(GROUND_ARMY_1);
		return move;
	};
	task.defMoves.push_back(defMove);
	defMove = [commonCenterNext](const World & world) -> Move {
		return generateSelectMove(Point(commonCenterNext.x - 55.0, commonCenterNext.y - 55.0), Point(commonCenterNext.x + 55.0, commonCenterNext.y + 1.0));
	};
	task.defMoves.push_back(defMove);
	defMove = [](const World & world) -> Move {
		Move move;
		move.setAction(ActionType::ASSIGN);
		move.setGroup(GROUND_ARMY_2);
		return move;
	};
	task.defMoves.push_back(defMove);
	task.canDo = [this](const World & w, const Player & me) -> bool {
		for (const Formation* f : this->constructingFormations) if (f->index == GROUND_ARMY && !f->checkFormationForMoving() && f->lastAction == ActionType::ROTATE) return true;
		return false;
	};
	this->taskQueue.push_back(task);

	// распознавание сигнала когда можно начинать атаку
	task.refresh();
	defMove = [this](const World & world) -> Move {
		this->canAttack = true;
		for (Formation* f : this->constructingFormations) 
			if (f->index > 10) this->myFormations.push_back(f);						// вместо удаления из вектора в лямбдах, просто скопируем только нужные формации в основную очередь
		this->constructingFormations.clear(); this->constructingFormations.resize(0);
		Move move;
		move.setAction(ActionType::NONE);
		return move;
	};
	task.defMoves.push_back(defMove);
	task.canDo = [this](const World & w, const Player & me) -> bool {
		bool scaledGr1 = false;
		for (const Formation* f : this->constructingFormations) {
			if (f->index == GROUND_ARMY_1 && !f->checkFormationForMoving() && f->lastAction == ActionType::SCALE) {
				scaledGr1 = true;
				break;
			}
		}
		if (!scaledGr1) return false;
		for (const Formation* f : this->constructingFormations) if (f->index == IFV_3 && f->lastAction == ActionType::SCALE) return true;
		return false;
	};
	this->taskQueue.push_back(task);
}

void MyStrategy::initFirstLocationRects(Rect & recT, Rect & recI, Rect & recF, Rect & recH, Rect & recA, Rect & recAll, const World & world)
{
	Point zeroPoint;
	vector<Vehicle> allVehicles = world.getNewVehicles();
	for (int i = 0; i < allVehicles.size(); i++) {
		allUnits[allVehicles[i].getId()] = allVehicles[i];
		if (allVehicles[i].getPlayerId() != this->myID) continue;					// юнитов противника перематываем
		double x = allVehicles[i].getX();
		double y = allVehicles[i].getY();
		Point veh(x, y);
		if (x > recAll.bottomRight.x) recAll.bottomRight.x = x;
		if (y > recAll.bottomRight.y) recAll.bottomRight.y = y;
		if (x < recAll.topLeft.x) recAll.topLeft.x = x;
		if (y < recAll.topLeft.y) recAll.topLeft.y = y;
		if (allVehicles[i].getType() == VehicleType::TANK) {
			if (zeroPoint.distanceTo(recT.bottomRight) < zeroPoint.distanceTo(veh)) {
				recT.bottomRight.x = x;
				recT.bottomRight.y = y;
			}
			if (zeroPoint.distanceTo(recT.topLeft) > zeroPoint.distanceTo(veh)) {
				recT.topLeft.x = x;
				recT.topLeft.y = y;
			}
		}
		else if (allVehicles[i].getType() == VehicleType::FIGHTER) {
			if (zeroPoint.distanceTo(recF.bottomRight) < zeroPoint.distanceTo(veh)) {
				recF.bottomRight.x = x;
				recF.bottomRight.y = y;
			}
			if (zeroPoint.distanceTo(recF.topLeft) > zeroPoint.distanceTo(veh)) {
				recF.topLeft.x = x;
				recF.topLeft.y = y;
			}
		}
		else if (allVehicles[i].getType() == VehicleType::HELICOPTER) {
			if (zeroPoint.distanceTo(recH.bottomRight) < zeroPoint.distanceTo(veh)) {
				recH.bottomRight.x = x;
				recH.bottomRight.y = y;
			}
			if (zeroPoint.distanceTo(recH.topLeft) > zeroPoint.distanceTo(veh)) {
				recH.topLeft.x = x;
				recH.topLeft.y = y;
			}
		}
		else if (allVehicles[i].getType() == VehicleType::IFV) {
			if (zeroPoint.distanceTo(recI.bottomRight) < zeroPoint.distanceTo(veh)) {
				recI.bottomRight.x = x;
				recI.bottomRight.y = y;
			}
			if (zeroPoint.distanceTo(recI.topLeft) > zeroPoint.distanceTo(veh)) {
				recI.topLeft.x = x;
				recI.topLeft.y = y;
			}
		}
		else if (allVehicles[i].getType() == VehicleType::ARRV) {
			if (zeroPoint.distanceTo(recA.bottomRight) < zeroPoint.distanceTo(veh)) {
				recA.bottomRight.x = x;
				recA.bottomRight.y = y;
			}
			if (zeroPoint.distanceTo(recA.topLeft) > zeroPoint.distanceTo(veh)) {
				recA.topLeft.x = x;
				recA.topLeft.y = y;
			}
		}
	}
	allPrevUnits = allUnits;
}

// переделать на возврат IDшников
// эта функция не срабатывает в одном таске сразу же после назначения группы, поскольку надо сначала чтобы назначение группы прошло в move
//std::vector<Vehicle> MyStrategy::getVehiclesOfGroup(int groupNo) const
//{
//	std::vector<Vehicle> group;
//	for (auto &v : allUnits) {
//		for (auto & grNo : v.second.getGroups()) {
//			if (grNo == groupNo) {
//				group.push_back(v.second);
//				break;
//			}
//		}
//	}
//	return group;
//}

// переделать на возврат IDшников
std::vector<Vehicle> MyStrategy::getSelectedVehicles() const
{
	std::vector<Vehicle> group;
	for (auto &v : allUnits) {
		if (v.second.isSelected()) {
			group.push_back(v.second);
		}
	}
	return group;
}

std::vector<int> MyStrategy::getVehicleIDsInRectangle(const Rect & rec, VehicleType vType)
{
	std::vector<int> vehicles;
	for (auto const &v : allUnits) {
		if (rec.isInclude(v.second.getX(), v.second.getY())) {
			if (vType == VehicleType::_UNKNOWN_ || vType == v.second.getType()) {
				vehicles.push_back(v.first);
			}
		}
	}
	return vehicles;
}

void MyStrategy::formDivision(const Rect & rec, short indexSuffix, int tickIndex, VehicleType vType)
{
	Task task(indexSuffix);
	std::function<Move(const World &)> defMove;
	defMove = [rec, vType](const World & world) -> Move {
		return generateSelectMove(rec.topLeft, rec.bottomRight, vType);
	};
	task.defMoves.push_back(defMove);
	defMove = [indexSuffix](const World & world) -> Move {
		Move move;
		move.setAction(ActionType::ASSIGN);
		move.setGroup(indexSuffix);
		return move;
	};
	task.defMoves.push_back(defMove);
	task.canDo = [tickIndex](const World & w, const Player & me) -> bool {
		return w.getTickIndex() >= tickIndex;
	};
	this->taskQueue.push_back(task);
}

void MyStrategy::formDivisionInChessOrder(const Rect & rec, short indexSuffix, int tickIndex, VehicleType vType)
{
	Task task;
	std::function<model::Move(const model::World &)> defMove;
	task.refresh();
	for (int i = 0; i < 5; i++) {
		defMove = [rec, i, vType](const World & world) -> Move {
			return generateSelectMove(Point(rec.topLeft.x + i * 12.0, rec.topLeft.y), Point(rec.topLeft.x + i * 12.0, rec.bottomRight.y), vType);
		};
		task.defMoves.push_back(defMove);
		defMove = [](const World & world) -> Move {
			Move move;
			move.setAction(ActionType::MOVE);
			move.setX(0.0);
			move.setY(3);
			return move;
		};
		task.defMoves.push_back(defMove);
	}
	for (int i = 0; i < 4; i++) {
		defMove = [](const World & world) -> Move {
			Move move;
			move.setAction(ActionType::NONE);
			return move;
		};
		task.defMoves.push_back(defMove);
	}
	defMove = [vType](const World & world) -> Move {
		return generateSelectMove(Point(), Point(WORLD_LENGTH, WORLD_LENGTH), vType);
	};
	task.defMoves.push_back(defMove);
	defMove = [rec](const World & world) -> Move {
		Move move;
		move.setAction(ActionType::ROTATE);
		move.setX((rec.topLeft.x + rec.bottomRight.x) / 2.0);
		move.setY((rec.topLeft.y + rec.bottomRight.y) / 2.0);
		move.setAngle(PI / 4.0);
		return move;
	};
	task.defMoves.push_back(defMove);
	defMove = [indexSuffix](const World & world) -> Move {
		Move move;
		move.setAction(ActionType::ASSIGN);
		move.setGroup(indexSuffix);
		return move;
	};
	task.defMoves.push_back(defMove);
	task.canDo = [tickIndex](const World & w, const Player & me) -> bool {return w.getTickIndex() >= tickIndex; };
	this->taskQueue.push_back(task);
}

void MyStrategy::refreshMyFormations()
{
	// обновляем строящиеся формации 
	if (this->constructingFormations.size() > 0) {
		for (Formation* f : this->constructingFormations) {
			f->refresh();
		}
	}

	for (std::vector<Formation*>::iterator itF = this->myFormations.begin(); itF != this->myFormations.end(); itF++) {
		// удаляем из формации всех мертвых юнитов
		for (std::list<int>::iterator it = (*itF)->unitsKeys.begin(); it != (*itF)->unitsKeys.end();) {
			if (allUnits.find(*it)->second.getDurability() == 0) it = (*itF)->unitsKeys.erase(it);
			else ++it;
		}
		if ((*itF)->unitsKeys.size() > 0) (*itF)->refresh();						// здесь надо refresh для каждой формации вызвать чтобы все данные типа центра и прямоугольника просчитаны были!
	}
	// удаляем уничтоженную формацию!
	this->myFormations.erase(std::remove_if(this->myFormations.begin(), this->myFormations.end(), [](Formation* f) {
		return f->unitsKeys.size() == 0;
	}), this->myFormations.end());
}

std::vector<Formation> MyStrategy::findOppFormations()
{
	double l = MAX_DIST_BETWEEN_UNITS_INSIDE_FORMATION;
	std::list<Vehicle>::iterator it;
	std::list<Vehicle> oppUnits;
	for (auto const &v : allUnits) {
		if (v.second.getPlayerId() != this->myID && v.second.getDurability() != 0) oppUnits.push_back(v.second);
	}
	std::vector<Formation> formations;
	std::vector<int> currGroup;
	Rect realRec;
	Rect rec;
	int prevGroupSize = -1;
	//unsigned short count = 0;

	while (oppUnits.begin() != oppUnits.end()) {
		//count++;
		Vehicle oppUnit = *(oppUnits.begin());
		rec = Rect(Point(oppUnit.getX(), oppUnit.getY()), Point(oppUnit.getX(), oppUnit.getY()));
		rec = Rect(Point(rec.topLeft.x - l, rec.topLeft.y - l), Point(rec.bottomRight.x + l, rec.bottomRight.y + l));
		while (prevGroupSize != currGroup.size()) {
			prevGroupSize = currGroup.size();
			for (it = oppUnits.begin(); it != oppUnits.end();) {
				if (rec.isInclude(it->getX(), it->getY())) {
					currGroup.push_back(it->getId());
					it = oppUnits.erase(it);
				}
				else ++it;
			}
			realRec = this->findGroupRectangle(currGroup);
			rec.raiseInDirection(realRec, l);
		}
		formations.push_back(Formation(0, currGroup, FormationUnitsType::_UNDEFINED_, &allUnits, &allPrevUnits));

		//printf("\nGroupSize: %d\n", currGroup.size());
		//printf("Real Rectangle: x1=%.2f, y1=%.2f, x2=%.2f, y2=%.2f\n", realRec.topLeft.x, realRec.topLeft.y, realRec.bottomRight.x, realRec.bottomRight.y);
		
		prevGroupSize = -1;
		currGroup.clear();
		currGroup.resize(0);
	}

	return formations;
}

Rect MyStrategy::findGroupRectangle(std::vector<int> vehIDs)
{
	Rect rec(Point(WORLD_LENGTH, WORLD_LENGTH), Point());
	for (int id : vehIDs) {
		if (allUnits[id].getX() > rec.bottomRight.x) {
			rec.bottomRight.x = allUnits[id].getX();
		}
		if (allUnits[id].getY() > rec.bottomRight.y) {
			rec.bottomRight.y = allUnits[id].getY();
		}
		if (allUnits[id].getX() < rec.topLeft.x) {
			rec.topLeft.x = allUnits[id].getX();
		}
		if (allUnits[id].getY() < rec.topLeft.y) {
			rec.topLeft.y = allUnits[id].getY();
		}
	}
	return rec;
}

void MyStrategy::patrol(Formation & formation, std::vector<Point> points, const World & world)
{
	// Patrol
	/*std::vector<Point> route1;
	route1.push_back(Point(240, 240));
	route1.push_back(Point(512, 512));
	route1.push_back(Point(512, 900));
	route1.push_back(Point(240, 240));
	std::vector<Point> route2;
	route2.push_back(Point(512, 240));
	route2.push_back(Point(640, 640));
	route2.push_back(Point(128, 640));
	route2.push_back(Point(512, 240));
	this->patrol(this->myFormations[0], route1, world);
	this->patrol(this->myFormations[1], route2, world);*/

	if (world.getTickIndex() == 0) {
		this->taskQueue.push_back(formation.moveTo(points[0]));
	}
	// пока приходится страдать от дублирования действия MOVE, если данная группа не выбрана в текущий момент
	// м.б. реализовать фичу с предварительным селектом
	else {
		for (int i = 1; i < points.size(); i++) {
			if (formation.getCenter().distanceTo(points[i - 1]) <= 0.1
				&& !formation.checkFormationForMoving()) {
				this->taskQueue.push_back(formation.moveTo(points[i]));
			}
		}
	}
}

void MyStrategy::stayNear(Formation & formation, Point dest, double dist, double deviation)
{
	if (formation.getCenter().distanceTo(dest) <= dist - deviation) {
		this->taskQueue.push_back(formation.moveTo(getPointInDirectionForDistance(dest, formation.getCenter(), dist)));
	} else if (formation.getCenter().distanceTo(dest) >= dist + deviation) {
		this->taskQueue.push_back(formation.moveTo(dest));
	}
}

void MyStrategy::convergeMyFormations(std::vector<Formation> & myFormations)
{
	for (Formation & f : myFormations) {
		if (findGroupDensity(f.getUnits()) < 0.04) this->taskQueue.push_back(f.scale(0.25, f.getCenter()));
	}
}

int MyStrategy::getOptimalTargetNo(const std::vector<Formation> & oppFormations, const Formation & myFormation)
{
	int bestTarget = -1;
	double minScore = 1.1;
	double bestScore = 1.05;
	double bestScoreDist = WORLD_LENGTH + WORLD_LENGTH;
	for (int i = 0; i < oppFormations.size(); ++i) {
		double sizeFactor = (myFormation.unitsKeys.size() > oppFormations[i].unitsKeys.size()) ? ((double)myFormation.unitsKeys.size() / (double)oppFormations[i].unitsKeys.size()) : 1.0;
		double oppDamage = (oppFormations[i].getAvgDamageVsFormation(myFormation) <= 0.1) ? 0.01 : oppFormations[i].getAvgDamageVsFormation(myFormation);
		double score = sizeFactor 
			* (myFormation.getAvgDamageVsFormation(oppFormations[i]) / oppDamage)
			* ((double)myFormation.findAvgDurability() / (double)oppFormations[i].findAvgDurability());
		double dist = myFormation.getCenter().distanceTo(oppFormations[i].getCenter());
		if (score > minScore && dist < bestScoreDist) {
			// пока просто будем уничтожать ближайшую цель которую мы сильнее а не делить на градации
			bestTarget = i;
			bestScoreDist = dist;
		}
		/*if (score > bestScore) {
			bestScore = score;
			bestTarget = i;
			bestScoreDist = dist;
		}
		else if (bestScore == score && dist < bestScoreDist) {
			bestScore = score;
			bestTarget = i;
			bestScoreDist = dist;
		}*/
		//printf("\ncenter Opp formation: x=%.2f, y=%.2f, score=%.2f, oppDamage=%.2f, myDamage=%.2f ", oppFormations[i].getCenter().x, oppFormations[i].getCenter().y, score, oppDamage, myFormation.getAvgDamageVsFormation(oppFormations[i]));
	}
	//printf("\nbest score: %.2f\n", bestScore);
	return bestTarget;
}

// чтобы найти оптимальную точку надо сначала худшие точки найти, и их передать сюда
// !! это еще нужно для того чтобы формация хилок не шла тупо на бутерброд который на территории здания находится!
Point MyStrategy::getOptimalTargetPoint(const std::vector<Formation> & oppFormations, const Formation & myFormation, const model::World & world)
{
	Point bestPoint;

	// для отдельных формаций - свои цели
	if (myFormation.index == IFV_3) {
		// это охотятся на здания
		return this->findNearestNewFacilityCenter(myFormation);
	}
	/*else if () {

	}*/

	int bestTarget = this->getOptimalTargetNo(oppFormations, myFormation);
	// цель найдена
	if (bestTarget >= 0) {
		bestPoint = this->computeFormationPositionAfterNTicks(oppFormations[bestTarget], ACTION_STEP_TICKS);	// идем атаковать эту формацию
	}
	// альтернативные варианты
	else {
		if (myFormation.unitsType == FormationUnitsType::MOSTLY_GROUND) bestPoint = this->findNearestNewFacilityCenter(myFormation);
		else {
			bestPoint = this->computeFormationPositionAfterNTicks(*(this->myFormations[this->myFormations.size() - 1]), ACTION_STEP_TICKS); // (заглушка) прсто центр одной из своих формаций
			if (world.getTickIndex() < 420) bestPoint = Point(WORLD_LENGTH / 2.0, WORLD_LENGTH / 2.0);
		}
	}
	//printf("center Opp formation: x=%.2f, y=%.2f\n", f.findCenter().x, f.findCenter().y);
	//printf("best score: %.2f\n", bestScore);
	return bestPoint;
}

int MyStrategy::getWorstTargetNo(const std::vector<Formation>& oppFormations, const Formation & myFormation)
{
	int worstTarget = -1;
	double minScore = 1.1;
	double bestScore = 1.05;
	double bestScoreDist = WORLD_LENGTH + WORLD_LENGTH;
	for (int i = 0; i < oppFormations.size(); ++i) {
		double myDamage = (myFormation.getAvgDamageVsFormation(oppFormations[i]) <= 0.1) ? 0.01 : myFormation.getAvgDamageVsFormation(oppFormations[i]);
		double score = (oppFormations[i].getAvgDamageVsFormation(myFormation) / myDamage)
			* ((double)oppFormations[i].unitsKeys.size() / (double)myFormation.unitsKeys.size())
			* ((double)oppFormations[i].findAvgDurability() / (double)myFormation.findAvgDurability());
		double dist = myFormation.getCenter().distanceTo(oppFormations[i].getCenter());
		if (score > minScore && dist < bestScoreDist) {
			// убегаем тоже от ближайшего которого не сможем уничтожить
			worstTarget = i;
			bestScoreDist = dist;
		}
		/*if (score > bestScore) {
			bestScore = score;
			worstTarget = i;
			bestScoreDist = dist;
		}
		else if (bestScore == score && dist < bestScoreDist) {
			bestScore = score;
			worstTarget = i;
			bestScoreDist = dist;
		}*/
	}
	return worstTarget;
}

Point MyStrategy::getWorstTargetPoint(const std::vector<Formation>& oppFormations, const Formation & myFormation, const model::World & world)
{
	Point worstPoint;
	int worstTarget = this->getWorstTargetNo(oppFormations, myFormation);

	// худшая контр формация найдена
	if (worstTarget >= 0) {
		worstPoint = this->computeFormationPositionAfterNTicks(oppFormations[worstTarget], ACTION_STEP_TICKS);	// убегаем от этой формации оппонента
	}
	// нет контрящих вражеских формаций
	else {
		worstPoint = myFormation.getCenter();			// (заглушка) просто всегда двигаемся
	}

	return worstPoint;
}

Formation MyStrategy::mergeFormations(const Formation & f1, const Formation & f2, unsigned short index)
{
	std::list<int> vehs1(f1.unitsKeys);
	std::list<int> vehs2(f2.unitsKeys);
	vehs1.insert(std::end(vehs1), std::begin(vehs2), std::end(vehs2));
	return Formation(index, vehs1, FormationUnitsType::_UNDEFINED_, &allUnits, &allPrevUnits);
}

// пока не использую потому что нужна одна целевая точка
std::vector<Point> MyStrategy::findPotentialTargetPoints(int myFormationIndex, const std::vector<Formation>& oppFormations)
{
	std::vector<Point> targetPoints;
	double bestScore = 1.0;
	for (int i = 0; i < oppFormations.size(); ++i) {
		double oppDamage = (oppFormations[i].getAvgDamageVsFormation(*(this->myFormations[myFormationIndex])) <= 0.1) ? 0.1 : oppFormations[i].getAvgDamageVsFormation(*(this->myFormations[myFormationIndex]));
		double score = 1.0 * (this->myFormations[myFormationIndex]->getAvgDamageVsFormation(oppFormations[i]) / oppDamage)
			* ((double)this->myFormations[myFormationIndex]->unitsKeys.size() / (double)oppFormations[i].unitsKeys.size())
			* ((double)this->myFormations[myFormationIndex]->findAvgDurability() / (double)oppFormations[i].findAvgDurability())
			* (1000.0 / (1000.0 + this->myFormations[myFormationIndex]->getCenter().distanceTo(oppFormations[i].getCenter())));
		if (score > bestScore) targetPoints.push_back(oppFormations[i].getCenter());

		// здесь сначала запоминать все которые контрим
		// а потом уже выбирать оптимально ту группу которую атаковать выгодно																		
		//printf("center Opp formation: x=%.2f, y=%.2f\n", f.findCenter().x, f.findCenter().y);

	}
	// если больше или равно 1 то идем на этого а если меньше то уходим
	//myFormation.findCenter().distanceTo(oppFormations[bestScore].findCenter());
	//printf("best score: %.2f\n", bestScore);
	return targetPoints;
}

// после выбора оптимальных целевых точек, выбирать нужные текущие точки для движения в след шаг,
// учитывая факторы типа пересечения маршрутов и вражеские контр силы
std::vector<Point> MyStrategy::computeRouteNextStepsForMyFormations(const std::vector<Point> & targetPoints, const World & world, const std::vector<Point> & worstPoints)
{
	double phi = 0.0;
	std::vector<std::vector<Point> > formationsPotentialPositions;

	// возможно в пространство точек вставить текущие координаты чтобы можно было и не сдвинуться!
	// и если точка смещения и целевая точка совпадать будут то просто останемся на месте =)

	// найдем позиции с шагом 15grad поворота, на кот могут оказаться формации в радиусе перемещения
	for (const Formation* f : this->myFormations) {
		Point formCen = f->getCenter();
		double r = f->findOptimalSpeed(world) * ACTION_STEP_TICKS;
		std::vector<Point> potentialPositions;
		potentialPositions.push_back(formCen);			// текущее положение тоже добавим
		for (int i = 0; i < 24; i++) {
			phi = i * PI / 12.0;
			potentialPositions.push_back(Point(formCen.x + r * 0.25 * cos(phi), formCen.y + r * 0.25 * sin(phi)));
		}
		for (int i = 0; i < 24; i++) {
			phi = i * PI / 12.0;
			potentialPositions.push_back(Point(formCen.x + r * 0.5 * cos(phi), formCen.y + r * 0.5 * sin(phi)));
		}
		for (int i = 0; i < 24; i++) {
			phi = i * PI / 12.0;
			potentialPositions.push_back(Point(formCen.x + r * 0.75 * cos(phi), formCen.y + r * 0.75 * sin(phi)));
		}
		for (int i = 0; i < 24; i++) {
			phi = i * PI / 12.0;
			potentialPositions.push_back(Point(formCen.x + r * cos(phi), formCen.y + r * sin(phi)));
		}

		//printf(" Potential positions:\n");
		//for (Point p : potentialPositions) {
		//	printf(" x=%.2f, y=%.2f;\n", p.x, p.y);
		//}

		formationsPotentialPositions.push_back(potentialPositions);
	}

	// геном это массив индексов позиций по одной на каждую формацию
	Generation generation = getRandomGeneration(formationsPotentialPositions, GENERATION_SIZE);

	for (int i = 0; i < NUM_ITERATIONS; i++) {
		// sort generation
		this->evaluateAndSortPositions(targetPoints, formationsPotentialPositions, generation, worstPoints);
		// cross
		for (int j = 1; j < NUM_CROSS; j += 2) {
			cross(generation[j], generation[j + 1]);
		}
		// add some new inds
		Generation gen = getRandomGeneration(formationsPotentialPositions, GENERATION_SIZE);
		for (int j = NUM_CROSS; j < GENERATION_SIZE; j++) {
			generation[j] = gen[j];
		}
	}

	this->evaluateAndSortPositions(targetPoints, formationsPotentialPositions, generation, worstPoints);
	std::vector<Point> nextStepPositions;
	for (int i = 0; i < formationsPotentialPositions.size(); i++) {
		nextStepPositions.push_back(formationsPotentialPositions[i][generation[0].indexes[i]]);
	}

	return nextStepPositions;
}

void MyStrategy::evaluateAndSortPositions(const std::vector<Point> & targetPoints, const std::vector<std::vector<Point> > & formationsPotentialPositions, Generation & generation, const std::vector<Point> & worstPoints)
{
	// добавить снижение баллов за близость к контр армии оппонента
	// потом добавить чтобы можно было не только лучшую цель выбирать а любую положительную

	// в дальнейшем еще добавить оценку тайла и его влияние на формацию, чтобы лучший вариант выбирать

	// !!! удалить
	std::vector<double> halfDiagonals;
	for (const Formation* f : this->myFormations) halfDiagonals.push_back(f->getRectangle().getDiagonal() * 0.5);

	// здесь заранее считаем диагонали, чтобы каждый раз одно и то же не считать
	std::vector<Point> shifts;
	for (Individual & ind : generation) {
		shifts.clear(); shifts.resize(0);
		for (int j = 0; j < this->myFormations.size(); j++) {
			shifts.push_back(formationsPotentialPositions[j][ind.indexes[j]] - this->myFormations[j]->getCenter());
		}
		// посчитать сразу таблицу разрешений на пересечения для каждой пары

		double sumDist = 0.0;
		double factor = 1.0;
		for (int i = 0; i < this->myFormations.size(); i++) {
			Rect myRec = this->myFormations[i]->getRectangle();
			double halfX = (myRec.bottomRight.x - myRec.topLeft.x) * 0.5;
			double halfY = (myRec.bottomRight.y - myRec.topLeft.y) * 0.5;
			if (formationsPotentialPositions[i][ind.indexes[i]].x < halfX * 0.9
				|| formationsPotentialPositions[i][ind.indexes[i]].y < halfY * 0.9
				|| formationsPotentialPositions[i][ind.indexes[i]].x > WORLD_LENGTH - halfX * 0.9
				|| formationsPotentialPositions[i][ind.indexes[i]].y > WORLD_LENGTH - halfY * 0.9) factor *= 10.0;
			if (formationsPotentialPositions[i][ind.indexes[i]].distanceTo(worstPoints[i]) < 100.0) factor *= 20.0;		// избегаем формацию оппонента
			for (int j = 0; j < this->myFormations.size(); j++) {
				if (j == i) continue;
				if (!this->myFormations[i]->canIntersect(this->myFormations[j])) {
					if (formationsPotentialPositions[i][ind.indexes[i]].distanceTo(formationsPotentialPositions[j][ind.indexes[j]])
						< halfDiagonals[i] + halfDiagonals[j] + 4.0) {
						factor *= 30.0;
					}
					/*if (this->myFormations[i]->willIntersect(this->myFormations[j], shifts[i], shifts[j])) {
						factor *= 30.0;
					}*/
				}
			}
			sumDist += formationsPotentialPositions[i][ind.indexes[i]].distanceTo(targetPoints[i]);	// сумма дистанций до целей для всех формаций
		}
		ind.score = sumDist * factor;

		//printf("\n sumDist=%.2f;  pos: x=%.2f; y=%.2f\n", sumDist, formationsPotentialPositions[0][ind.indexes[0]].x, formationsPotentialPositions[0][ind.indexes[0]].y);
	}

	std::sort(generation.begin(), generation.end());
}

Point MyStrategy::findNearestNewFacilityCenter(const Formation & formation)
{
	double minDist = WORLD_LENGTH;
	Point myCen = formation.getCenter();
	Point bestPoint(myCen);
	for (const Facility & fac : this->facilities) {
		if (fac.getOwnerPlayerId() != this->myID && myCen.distanceTo(computeFacilityCenter(fac)) < minDist) {
			minDist = myCen.distanceTo(computeFacilityCenter(fac));
			bestPoint = computeFacilityCenter(fac);
		}
	}
	return bestPoint;
}

int MyStrategy::getMyNearestFormationIndex(Point p) const
{
	double bestDist = WORLD_LENGTH + WORLD_LENGTH;
	int fIndex = -1;
	for (int i = 0; i < this->myFormations.size(); i++) {
		if (this->myFormations[i]->getCenter().distanceTo(p) < bestDist) {
			fIndex = i;
			bestDist = this->myFormations[i]->getCenter().distanceTo(p);
		}
	}
	return fIndex;
}

std::list<Point> MyStrategy::findPointsForNuclearStrike(const std::vector<Formation> & oppFormations) const
{
	// сначала положим все потенциальные точки удара
	double phi = 0.0;
	std::list<Point> points;
	Point futurePos;
	for (const Formation & f : oppFormations) {
		if (f.unitsKeys.size() < 10 && oppFormations.size() > 1) continue;
		futurePos = this->computeFormationPositionAfterNTicks(f, NUCLEAR_STRIKE_DELAY);
		Point avgDestPoint = Point((f.getCenter().x + futurePos.x) / 2, (f.getCenter().y + futurePos.y) / 2.0);
		points.push_back(avgDestPoint);
	}

	for (const Formation & f : oppFormations) {
		double diag = f.getRectangle().getDiagonal();
		double r = 0.2 * diag;
		for (int i = 0; i < 8; i++) {
			phi = i * PI / 4.0;
			points.push_back(Point(f.getCenter().x + r * cos(phi), f.getCenter().y + r * sin(phi)));
		}
	}

	//printf("size before %d\n", points.size());
	// отсеим неподходящие точки удара которые будут близки к положениям фоих формаций
	for (std::list<Point>::iterator it = points.begin(); it != points.end();) {
		bool erased = false;
		for (const Formation* f : this->myFormations) {
			if (f->unitsKeys.size() > 10
				&& f->getCenter().distanceTo(*it) < f->getRectangle().getDiagonal() * 0.5 + 30.0) {
				it = points.erase(it);
				erased = true;
				break;
			}
		}
		if (erased == false) ++it;
	}
	//printf("size after %d\n", points.size());

	return points;
}

int MyStrategy::getUnitIDHavingInVisionRangePoint(const Point & nsPoint, const model::World & world)
{
	int bestDurability = 0;
	int bestID = -1;
	for (const auto & u : allUnits) {
		if (u.second.getPlayerId() == this->myID && u.second.getDurability() > 0 && getRealVisionRangeForUnit(u.second, world) > nsPoint.distanceTo(u.second.getX(), u.second.getY())) {
			if (u.second.getDurability() == 100) return u.first;
			if (bestDurability < u.second.getDurability()) {
				bestID = u.first;
				bestDurability = u.second.getDurability();
			}
		}
	}
	return bestID;
}

void MyStrategy::tryPerformNuclearStrike(const std::vector<Formation> & oppFormations, const World & world)
{
	std::list<Point> nsPoints = this->findPointsForNuclearStrike(oppFormations);
	for (const Point & nsPoint : nsPoints) {
		int vehID = this->getUnitIDHavingInVisionRangePoint(nsPoint, world);
		if (vehID >= 0) {
			//printf("\nvis range for Nuclear Strike: %.2f", getRealVisionRangeForUnit(allUnits.find(vehID)->second, world));
			this->doNuclearStrike = true;
			//printf("\ndoNuclearStrike: %d, tick: %d", this->doNuclearStrike, world.getTickIndex());
			Task task;										// сразу ставим в true
			std::function<Move(const World &)> defMove;
			defMove = [nsPoint, vehID](const World & world) -> Move {
				Move m;
				m.setVehicleId(vehID);
				m.setX(nsPoint.x);
				m.setY(nsPoint.y);
				m.setAction(ActionType::TACTICAL_NUCLEAR_STRIKE);
				return m;
			};
			task.defMoves.push_back(defMove);
			if (allUnits.find(vehID)->second.getGroups().size() > 0) {
				defMove = [vehID](const World & world) -> Move {
					return generateSelectMove(allUnits.find(vehID)->second.getGroups()[0]);
				};
				task.defMoves.push_back(defMove);
				defMove = [vehID](const World & world) -> Move {
					Move m;
					m.setX(allUnits.find(vehID)->second.getX());
					m.setY(allUnits.find(vehID)->second.getY());
					m.setAction(ActionType::SCALE);
					m.setFactor(0.2);
					return m;
				};
				task.defMoves.push_back(defMove);
			}
			else {
				defMove = [vehID](const World & world) -> Move {
					Point pos(allUnits.find(vehID)->second.getX(), allUnits.find(vehID)->second.getY());
					return generateSelectMove(Point(pos.x - 1.0, pos.y - 1.0), Point(pos.x + 1.0, pos.y + 1.0));
				};
				task.defMoves.push_back(defMove);
				defMove = [vehID](const World & world) -> Move {
					Move m;
					m.setX(allUnits.find(vehID)->second.getX());
					m.setY(allUnits.find(vehID)->second.getY());
					m.setAction(ActionType::SCALE);
					m.setFactor(0.2);
					return m;
				};
				task.defMoves.push_back(defMove);
			}
			for (int i = 0; i < NUCLEAR_STRIKE_DELAY - 2; i++) {
				defMove = [](const model::World & world) -> model::Move {
					model::Move move;
					move.setAction(model::ActionType::NONE);
					return move;
				};
				task.defMoves.push_back(defMove);
			}
			// возвращаем в false по окончании удара - проверить!
			defMove = [this](const model::World & world) -> model::Move {
				model::Move move;
				this->doNuclearStrike = false;
				move.setAction(model::ActionType::NONE);
				//printf("\ndoNuclearStrike: %d, tick: %d, set this->doNuclearStrike to %d", this->doNuclearStrike, world.getTickIndex(), this->doNuclearStrike);
				return move;
			};
			task.defMoves.push_back(defMove);
			task.canDo = [](const model::World & w, const model::Player & me) -> bool {return true; };
			this->taskQueue.push_front(task);
			return;
		}
	}
}

void MyStrategy::avoidNuclearStrike(const model::World & world, Point dest, int ticksLeft)
{
	//this->currTask.clear(); 		// здесь лучше удалить весть таск кью, все равно много тиков потратим - !!! не можем убрать если построение долгое
	//this->taskQueue.clear();
	Task task;
	std::function<model::Move(const model::World &)> defMove
		= [dest](const model::World & world) -> model::Move {
		return generateSelectMove(Point(dest.x - 60.0, dest.y - 60.0), Point(dest.x + 60.0, dest.y + 60.0));
	};
	task.defMoves.push_back(defMove);
	defMove = [dest](const model::World & world) -> model::Move {
		model::Move move;
		move.setAction(model::ActionType::SCALE);
		move.setX(dest.x + 3.0);
		move.setY(dest.y + 3.0);
		move.setFactor(5.0);
		return move;
	};
	task.defMoves.push_back(defMove);
	for (int i = 0; i < ticksLeft - 1; i++) {
		defMove = [](const model::World & world) -> model::Move {
			model::Move move;
			move.setAction(model::ActionType::NONE);
			return move;
		};
		task.defMoves.push_back(defMove);
	}
	defMove = [dest](const model::World & world) -> model::Move {
		model::Move move;
		move.setAction(model::ActionType::SCALE);
		move.setX(dest.x + 3.0);
		move.setY(dest.y + 3.0);
		move.setFactor(0.19);
		return move;
	};
	task.defMoves.push_back(defMove);
	for (int i = 0; i < ticksLeft - 1; i++) {
		defMove = [](const model::World & world) -> model::Move {
			model::Move move;
			move.setAction(model::ActionType::NONE);
			return move;
		};
		task.defMoves.push_back(defMove);
	}
	task.canDo = [](const model::World & w, const model::Player & me) -> bool {return true; };
	this->taskQueue.push_front(task);
}

int MyStrategy::countMyControlCenters() const
{
	int count = 0;
	for (const Facility & fac : this->facilities) {
		if (fac.getOwnerPlayerId() == this->myID && fac.getType() == FacilityType::CONTROL_CENTER) count++;
	}
	return count;
}

Point MyStrategy::computeFormationLastTickShift(const Formation & formation) const
{
	std::vector<Vehicle> prevGroup;
	for (int k : formation.unitsKeys) {
		prevGroup.push_back(allPrevUnits.find(k)->second);
	}
	Point prevCenter = findCenter(prevGroup);
	return Point(formation.getCenter().x - prevCenter.x, formation.getCenter().y - prevCenter.y);
}

// перенос Formation
Point MyStrategy::computeFormationPositionAfterNTicks(const Formation & formation, int numTicks) const
{
	Point shift = this->computeFormationLastTickShift(formation);
	Point currCenter = formation.getCenter();
	return Point(currCenter.x + shift.x * numTicks, currCenter.y + shift.y * numTicks);
}

void MyStrategy::setNextMove(const Player& me, const World& world, Move& move)
{
	// no actions if cannot act
	move.setAction(ActionType::NONE);
	if (me.getRemainingActionCooldownTicks() > 0) return;

	if (this->currTask.empty()) {
		//printf("\ntask queue size: %d", this->taskQueue.size());
		// if curr Task is done
		if (this->taskQueue.empty()) return;
		// else go to the next Task
		Task task = this->taskQueue.front();
		this->taskQueue.pop_front();
		bool formationExists = false;
		for (Formation* f: this->myFormations) {
			if (f->index == task.formationIndex || task.formationIndex == 0) {
				formationExists = true;
				break;
			}
		}
		if (this->constructingFormations.size() > 0 && formationExists == false) {
			for (Formation* f : this->constructingFormations) {
				if (f->index == task.formationIndex || task.formationIndex == 0) {
					formationExists = true;
					break;
				}
			}
		}
		if (!formationExists) return;
		// if time to do it
		if (task.canDo(world, me)) {
			for (int i = 0; i < task.defMoves.size(); i++) this->currTask.push_back(task.defMoves[i]);
		}
		else {
			this->taskQueue.push_back(task);
			return;
		}
	}

	if (this->currTask.empty()) return;
	// follow doing curr task
	do {
		move = this->currTask.front()(world);							
		this->currTask.pop_front();
	} while (!this->currTask.empty() && move.getAction() == ActionType::_UNKNOWN_);
	if (move.getAction() == ActionType::_UNKNOWN_) move.setAction(ActionType::NONE);			// если неизвестное действие то принудительно ставим 0!
	/*printf("\ntime to do %d\n", world.getTickIndex());
	printf("type=%d;\n", move.getAction());
	printf("group=%d;\n", move.getGroup());*/
	//printf("veh_id=%d;\n", move.getVehicleId());
}

MyStrategy::MyStrategy() : newFormsCounter(0), doNuclearStrike(false), minNextTickIndex(0), canAttack(false)
{
	actionsHistory = new bool[21000]();
}