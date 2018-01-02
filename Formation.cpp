#include "Formation.h"

extern Point p0;

#include <stack>

Formation::Formation() : index(0), unitsType(FormationUnitsType::_UNDEFINED_), lastAction(model::ActionType::_UNKNOWN_)
{
}

Formation::Formation(unsigned short index) : index(index), unitsType(FormationUnitsType::_UNDEFINED_), lastAction(model::ActionType::_UNKNOWN_)
{
}

Formation::Formation(unsigned short index, const std::vector<int> vehKeys, FormationUnitsType unitsType, std::map<long, model::Vehicle> * allUnits, std::map<long, model::Vehicle> * allPrevUnits)
	: index(index), lastAction(model::ActionType::_UNKNOWN_), unitsType(unitsType), allUnits(allUnits), allPrevUnits(allPrevUnits)
{
	for (int k : vehKeys) this->unitsKeys.push_back(k);
	this->refresh();
}

Formation::Formation(unsigned short index, const std::list<int> vehKeys, FormationUnitsType unitsType, std::map<long, model::Vehicle> * allUnits, std::map<long, model::Vehicle> * allPrevUnits)
	: index(index), lastAction(model::ActionType::_UNKNOWN_), unitsType(unitsType), allUnits(allUnits), allPrevUnits(allPrevUnits)
{
	for (int k : vehKeys) this->unitsKeys.push_back(k);
	this->refresh();
}

Formation::Formation(const Formation & formation)
{
	this->center = formation.center;
	this->rectangle = formation.rectangle;
	this->unitsType = formation.unitsType;
	this->index = formation.index;
	this->lastAction = formation.lastAction;
	this->lastDestPoint = formation.lastDestPoint;
	this->damageFeats = formation.damageFeats;
	this->allUnits = formation.allUnits;
	this->allPrevUnits = formation.allPrevUnits;
	this->unitsKeys.clear(); this->unitsKeys.resize(0);
	this->convexHull.clear(); this->convexHull.resize(0);
	for (int k : formation.unitsKeys) this->unitsKeys.push_back(k);
	for (const Point & p : formation.convexHull) this->convexHull.push_back(p);
}

Formation & Formation::operator=(const Formation & formation)
{
	if (&formation == this) return *this;
	this->center = formation.center;
	this->rectangle = formation.rectangle;
	this->unitsType = formation.unitsType;
	this->index = formation.index;
	this->lastAction = formation.lastAction;
	this->lastDestPoint = formation.lastDestPoint;
	this->damageFeats = formation.damageFeats;
	this->allUnits = formation.allUnits;
	this->allPrevUnits = formation.allPrevUnits;
	this->unitsKeys.clear(); this->unitsKeys.resize(0);
	this->convexHull.clear(); this->convexHull.resize(0);
	for (int k : formation.unitsKeys) this->unitsKeys.push_back(k);
	for (const Point & p : formation.convexHull) this->convexHull.push_back(p);
	return *this;
}

void Formation::refresh()
{
	this->rectangle = this->findRectangle();
	this->center = Point((this->rectangle.bottomRight.x + this->rectangle.topLeft.x) * 0.5, (this->rectangle.bottomRight.y + this->rectangle.topLeft.y) * 0.5);
	this->computeDamageFeatures();
	this->findConvexHull();
}

std::vector<Point> Formation::getConvexHull() const
{
	return this->convexHull;
}

Rect Formation::getRectangle() const
{
	return this->rectangle;
}

Point Formation::getCenter() const
{
	return this->center;
}

std::vector<model::Vehicle> Formation::getUnits() const
{
	std::vector<model::Vehicle> units;
	for (int k : this->unitsKeys) {
		if (this->allUnits->find(k)->second.getDurability() > 0) units.push_back(this->allUnits->find(k)->second);
	}
	return units;
}

bool Formation::isSelected() const
{
	for (int k : this->unitsKeys) if (!this->allUnits->find(k)->second.isSelected()) return false;
	return true;
}

bool Formation::isIntersect(const Formation * f) const
{
	if (this->convexHull.size() < 3 || f->convexHull.size() < 3) return false;
	for (int i = 0; i < this->convexHull.size() - 1; i++) {
		Point a(this->convexHull[i]);
		Point b(this->convexHull[i + 1]);
		for (int j = 0; j < f->convexHull.size() - 1; j++) {
			Point c(f->convexHull[j]);
			Point d(f->convexHull[j + 1]);
			if (intersect(a, b, c, d)) {
				return true;
			}
		}
	}
	return false;
}

bool Formation::willIntersect(const Formation * f, const Point & myShift, const Point & fShift) const
{
	if (this->convexHull.size() < 3 || f->convexHull.size() < 3) return false;
	for (int i = 0; i < this->convexHull.size() - 1; i++) {
		Point a(this->convexHull[i] + myShift);
		Point b(this->convexHull[i + 1] + myShift);
		for (int j = 0; j < f->convexHull.size() - 1; j++) {
			Point c(f->convexHull[j] + fShift);
			Point d(f->convexHull[j + 1] + fShift);
			if (intersect(a, b, c, d)) {
				return true;
			}
		}
	}
	return false;
}

std::vector<Point> Formation::getAllPoints() const
{
	std::vector<Point> points;
	for (int k : this->unitsKeys) {
		points.push_back(Point(this->allUnits->find(k)->second.getX(), this->allUnits->find(k)->second.getY()));
	}
	return points;
}

std::vector<Point> Formation::findConvexHull()
{
	this->convexHull.clear(); this->convexHull.resize(0);
	// чтобы не падало
	Rect r = findRectangle();
	this->convexHull.push_back(r.topLeft);
	this->convexHull.push_back(Point(r.bottomRight.x, r.topLeft.y));
	this->convexHull.push_back(r.bottomRight);
	this->convexHull.push_back(Point(r.topLeft.x, r.bottomRight.y));
	return this->convexHull;

	std::vector<Point> points = this->getAllPoints();
	int n = points.size();
	if (n < 3) {
		Rect r = findRectangle();
		this->convexHull.push_back(r.topLeft);
		this->convexHull.push_back(Point(r.bottomRight.x, r.topLeft.y));
		this->convexHull.push_back(r.bottomRight);
		this->convexHull.push_back(Point(r.topLeft.x, r.bottomRight.y));
		return this->convexHull;
	}

	int ymin = points[0].y, min = 0;
	for (int i = 1; i < n; i++) {
		int y = points[i].y;
		if ((y < ymin) || (ymin == y && points[i].x < points[min].x))
			ymin = points[i].y, min = i;
	}

	std::swap(points[0], points[min]);

	p0 = points[0];
	std::qsort(&points[1], n - 1, sizeof(Point), compare);

	int m = 1;
	for (int i = 1; i < n; i++) {
		while (i < n - 1 && orientation(p0, points[i], points[i + 1]) == 0) i++;
		points[m] = points[i];
		m++; 
	}

	if (m < 3) {
		Rect r = findRectangle();
		this->convexHull.push_back(r.topLeft);
		this->convexHull.push_back(Point(r.bottomRight.x, r.topLeft.y));
		this->convexHull.push_back(r.bottomRight);
		this->convexHull.push_back(Point(r.topLeft.x, r.bottomRight.y));
		return this->convexHull;
	}

	std::stack<Point> S;
	S.push(points[0]);
	S.push(points[1]);
	S.push(points[2]);

	for (int i = 3; i < m; i++) {
		while (orientation(nextToTop(S), S.top(), points[i]) != 2) S.pop();
		S.push(points[i]);
	}

	Point topPoint = S.top();
	while (!S.empty()) {
		Point p = S.top();
		this->convexHull.push_back(p);
		S.pop();
	}
	this->convexHull.push_back(topPoint);

	return this->convexHull;
}

Rect Formation::findRectangle() const
{
	Rect rec(Point(WORLD_LENGTH, WORLD_LENGTH), Point());
	for (int k : this->unitsKeys) {
		if (this->allUnits->find(k)->second.getX() > rec.bottomRight.x) {
			rec.bottomRight.x = this->allUnits->find(k)->second.getX();
		}
		if (this->allUnits->find(k)->second.getY() > rec.bottomRight.y) {
			rec.bottomRight.y = this->allUnits->find(k)->second.getY();
		}
		if (this->allUnits->find(k)->second.getX() < rec.topLeft.x) {
			rec.topLeft.x = this->allUnits->find(k)->second.getX();
		}
		if (this->allUnits->find(k)->second.getY() < rec.topLeft.y) {
			rec.topLeft.y = this->allUnits->find(k)->second.getY();
		}
	}
	return rec;
}

Point Formation::findCenter() const
{
	Rect rec = findRectangle();
	return Point((rec.bottomRight.x + rec.topLeft.x) / 2.0, (rec.bottomRight.y + rec.topLeft.y) / 2.0);
}

double Formation::findAvgDurability() const
{
	int fullDurability = 100 * this->unitsKeys.size();
	int actualDurability = 0;
	for (int k : this->unitsKeys) actualDurability += this->allUnits->find(k)->second.getDurability();
	return (double)actualDurability / (double)fullDurability;
}

double Formation::findOptimalSpeed(const model::World & world) const
{
	// !! тут можно смотреть находится ли юнит на тайле и только если находится - уменьшать скорость
	double worstSpeed = 9.9;		
	std::vector<model::WeatherType> weatherTypes = findCurrWeatherTypes(world);							// можно оптимизировать сразу найдя худшую погоду
	std::vector<model::TerrainType> terrainTypes = findCurrTerrainTypes(world);							// можно оптимизировать сразу найдя худшую местность
	for (int k : this->unitsKeys) {
		if (this->allUnits->find(k)->second.isAerial()) {
			for (const model::WeatherType& wt : weatherTypes) {
				switch (wt) {
				case model::WeatherType::RAIN: if (this->allUnits->find(k)->second.getMaxSpeed() * 0.6 < worstSpeed) worstSpeed = this->allUnits->find(k)->second.getMaxSpeed() * 0.6;
					break;
				case model::WeatherType::CLOUD: if (this->allUnits->find(k)->second.getMaxSpeed() * 0.8 < worstSpeed) worstSpeed = this->allUnits->find(k)->second.getMaxSpeed() * 0.8;
					break;
				case model::WeatherType::CLEAR: if (this->allUnits->find(k)->second.getMaxSpeed() * 1.0 < worstSpeed) worstSpeed = this->allUnits->find(k)->second.getMaxSpeed() * 1.0;
					break;
				}
			}
		}
		else {
			for (const model::TerrainType& tt : terrainTypes) {
				switch (tt) {
				case model::TerrainType::SWAMP: if (this->allUnits->find(k)->second.getMaxSpeed() * 0.6 < worstSpeed) worstSpeed = this->allUnits->find(k)->second.getMaxSpeed() * 0.6;
					break;
				case model::TerrainType::FOREST: if (this->allUnits->find(k)->second.getMaxSpeed() * 0.8 < worstSpeed) worstSpeed = this->allUnits->find(k)->second.getMaxSpeed() * 0.8;
					break;
				case model::TerrainType::PLAIN: if (this->allUnits->find(k)->second.getMaxSpeed() * 1.0 < worstSpeed) worstSpeed = this->allUnits->find(k)->second.getMaxSpeed() * 1.0;
					break;
				}
			}
		}
	}
	return worstSpeed;
}

Task Formation::moveTo(const Point & dest, std::function<bool(const model::World&, const model::Player & me)> canDoFunc)// добавить опционально макс скорость
{
	Task task(this->index);
	std::function<model::Move(const model::World &)> defMove;
	defMove = [this](const model::World & world) -> model::Move {
		// здесь сначала проверку поставить надо ли выделять вообще =)
		// зная какой мув будет делаться, можно проверить и сразу возвращать _UNKNOWN_ даже если группа не выбрана!!!
		//printf("\nfrom formation#%d", this->index);
		if (!this->isSelected()) {
			model::Move move = generateSelectMove(this->index);
			if (move.getGroup() == -1) return generateSelectMove(this->findRectangle().topLeft, this->findRectangle().bottomRight);
			else return move;
		}
		model::Move move;
		move.setAction(model::ActionType::_UNKNOWN_);
		return move;
	};
	task.defMoves.push_back(defMove);
	defMove = [this, dest](const model::World & world) -> model::Move {
		model::Move move;
		move.setAction(model::ActionType::MOVE);
		Point cen = this->findCenter();
		move.setY(dest.y - cen.y);
		move.setX(dest.x - cen.x);
		move.setMaxSpeed(this->findOptimalSpeed(world));
		/*printf("center My formation: x=%.2f, y=%.2f\n", cen.x, cen.y);
		printf("shift for My formation: x=%.2f, y=%.2f\n", dest.x - cen.x, dest.y - cen.y);*/
		if (this->isSimilarToLastMove(move)) {
			move.setAction(model::ActionType::NONE);					// если действие повторяется с предыдущим, то не повторяем команду
			this->lastAction = model::ActionType::NONE;
			return move;
		}
		this->lastAction = model::ActionType::MOVE;
		this->lastDestPoint = dest;
		return move;
	};
	task.defMoves.push_back(defMove);
	//if (this->nextMoveTick < ) 
	task.canDo = canDoFunc;
	return task;
}

Task Formation::rotate(double angle, std::function<bool(const model::World&, const model::Player&me)> canDoFunc)
{
	Task task(this->index);
	std::function<model::Move(const model::World &)> defMove;
	defMove = [this](const model::World & world) -> model::Move {
		//printf("\nfrom formation#%d", this->index);
		if (!this->isSelected()) {
			model::Move move = generateSelectMove(this->index);
			if (move.getGroup() == -1) return generateSelectMove(this->findRectangle().topLeft, this->findRectangle().bottomRight);
			else return move;
		}
		model::Move move;
		move.setAction(model::ActionType::_UNKNOWN_);
		return move;
	};
	task.defMoves.push_back(defMove);
	defMove = [this, angle](const model::World & world) -> model::Move {
		model::Move move;
		move.setAction(model::ActionType::ROTATE);
		Point cen = this->findCenter();
		move.setY(cen.y);
		move.setX(cen.x);
		move.setAngle(angle);
		if (this->isSimilarToLastMove(move)) {
			move.setAction(model::ActionType::NONE);					// если действие повторяется с предыдущим, то не повторяем команду
			this->lastAction = model::ActionType::NONE;
			return move;
		}
		this->lastAction = model::ActionType::ROTATE;
		this->lastDestPoint = cen;
		return move;
	};
	task.defMoves.push_back(defMove);
	task.canDo = canDoFunc;
	return task;
}

Task Formation::restructureGroundArmy(std::function<bool(const model::World&, const model::Player&me)> canDoFunc)
{
	Task task(this->index);
	std::function<model::Move(const model::World &)> defMove;
	defMove = [this](const model::World & world) -> model::Move {
		return generateSelectMove(Point(this->findRectangle().topLeft.x, this->findCenter().y), this->findRectangle().bottomRight);
	};
	task.defMoves.push_back(defMove);
	defMove = [this](const model::World & world) -> model::Move {
		model::Move move;
		move.setAction(model::ActionType::ROTATE);
		move.setY(this->findCenter().y);
		move.setX(this->findRectangle().bottomRight.x + 2.0);
		move.setMaxSpeed(this->findOptimalSpeed(world));
		move.setAngle(-PI);
		this->lastAction = model::ActionType::NONE;
		return move;
	};
	task.defMoves.push_back(defMove);
	task.canDo = canDoFunc;
	return task;
}

Task Formation::squeezeGroundArmy(std::function<bool(const model::World&, const model::Player&me)> canDoFunc)
{
	Task task(this->index);
	std::function<model::Move(const model::World &)> defMove;
	defMove = [this](const model::World & world) -> model::Move {
		return generateSelectMove(this->findRectangle().topLeft, Point(this->findCenter().x, this->findRectangle().bottomRight.y));
	};
	task.defMoves.push_back(defMove);
	defMove = [this](const model::World & world) -> model::Move {
		model::Move move;
		move.setAction(model::ActionType::SCALE);
		move.setY(this->findCenter().y);
		move.setX((this->findRectangle().topLeft.x + this->findCenter().x) * 0.5);
		move.setFactor(0.1);
		this->lastAction = model::ActionType::SCALE;
		return move;
	};
	task.defMoves.push_back(defMove);
	defMove = [this](const model::World & world) -> model::Move {
		return generateSelectMove(Point(this->findCenter().x, this->findRectangle().topLeft.y), this->findRectangle().bottomRight);
	};
	task.defMoves.push_back(defMove);
	defMove = [this](const model::World & world) -> model::Move {
		model::Move move;
		move.setAction(model::ActionType::SCALE);
		move.setY(this->findCenter().y);
		move.setX((this->findRectangle().bottomRight.x + this->findCenter().x) * 0.5);
		move.setFactor(0.1);
		this->lastAction = model::ActionType::SCALE;
		return move;
	};
	task.defMoves.push_back(defMove);
	task.canDo = canDoFunc;
	return task;
}

Task Formation::scale(double factor, Point p, std::function<bool(const model::World&, const model::Player & me)> canDoFunc)
{
	Task task(this->index);
	std::function<model::Move(const model::World &)> defMove;
	defMove = [this](const model::World & world) -> model::Move {
		if (!this->isSelected()) {
			model::Move move = generateSelectMove(this->index);
			if (move.getGroup() == -1) return generateSelectMove(this->findRectangle().topLeft, this->findRectangle().bottomRight);
			else return move;
		}
		model::Move move;
		move.setAction(model::ActionType::_UNKNOWN_);
		return move;
	};
	task.defMoves.push_back(defMove);
	defMove = [this, factor, p](const model::World & world) -> model::Move {
		model::Move move;
		move.setAction(model::ActionType::SCALE);
		move.setY(p.y);
		move.setX(p.x);
		move.setMaxSpeed(this->findOptimalSpeed(world));
		move.setFactor(factor);
		if (this->isSimilarToLastMove(move)) {
			move.setAction(model::ActionType::NONE);					// если действие повторяется с предыдущим, то не повторяем команду
			this->lastAction = model::ActionType::NONE;
			return move;
		}
		this->lastAction = model::ActionType::SCALE;
		this->lastDestPoint = p;
		return move;
	};
	task.defMoves.push_back(defMove);
	task.canDo = canDoFunc;
	return task;
}

Task Formation::scale(double factor, std::function<bool(const model::World&, const model::Player&me)> canDoFunc)
{
	Task task(this->index);
	std::function<model::Move(const model::World &)> defMove;
	defMove = [this](const model::World & world) -> model::Move {
		if (!this->isSelected()) {
			model::Move move = generateSelectMove(this->index);
			if (move.getGroup() == -1) return generateSelectMove(this->findRectangle().topLeft, this->findRectangle().bottomRight);
			else return move;
		}
		model::Move move;
		move.setAction(model::ActionType::_UNKNOWN_);
		return move;
	};
	task.defMoves.push_back(defMove);
	defMove = [this, factor](const model::World & world) -> model::Move {
		model::Move move;
		move.setAction(model::ActionType::SCALE);
		move.setY(this->findCenter().y);
		move.setX(this->findCenter().x);
		move.setMaxSpeed(this->findOptimalSpeed(world));
		move.setFactor(factor);
		if (this->isSimilarToLastMove(move)) {
			move.setAction(model::ActionType::NONE);					// если действие повторяется с предыдущим, то не повторяем команду
			this->lastAction = model::ActionType::NONE;
			return move;
		}
		this->lastAction = model::ActionType::SCALE;
		this->lastDestPoint = this->findCenter();
		return move;
	};
	task.defMoves.push_back(defMove);
	task.canDo = canDoFunc;
	return task;
}

void Formation::computeDamageFeatures()
{
	DamageFeatures damFs;
	for (int k : this->unitsKeys) {
		damFs.damageVsT += getDamageVsUnit(model::VehicleType::TANK, this->allUnits->find(k)->second);
		damFs.damageVsI += getDamageVsUnit(model::VehicleType::IFV, this->allUnits->find(k)->second);
		damFs.damageVsA += getDamageVsUnit(model::VehicleType::ARRV, this->allUnits->find(k)->second);
		damFs.damageVsF += getDamageVsUnit(model::VehicleType::FIGHTER, this->allUnits->find(k)->second);
		damFs.damageVsH += getDamageVsUnit(model::VehicleType::HELICOPTER, this->allUnits->find(k)->second);
	}
	damFs.damageVsT /= this->unitsKeys.size();
	damFs.damageVsI /= this->unitsKeys.size();
	damFs.damageVsA /= this->unitsKeys.size();
	damFs.damageVsF /= this->unitsKeys.size();
	damFs.damageVsH /= this->unitsKeys.size();
	this->damageFeats = damFs;
}

double Formation::getAvgDamageVsUnit(model::VehicleType vType) const
{
	switch (vType) {
	case model::VehicleType::TANK: return this->damageFeats.damageVsT;
		break;
	case model::VehicleType::IFV: return this->damageFeats.damageVsI;
		break;
	case model::VehicleType::FIGHTER: return this->damageFeats.damageVsF;
		break;
	case model::VehicleType::HELICOPTER: return this->damageFeats.damageVsH;
		break;
	case model::VehicleType::ARRV: return this->damageFeats.damageVsA;
		break;
	}
	return 0.0;
}

double Formation::getAvgDamageVsFormation(const Formation & formation) const
{
	double avgDam = 0.0;
	for (int k : formation.unitsKeys) {
		avgDam += getAvgDamageVsUnit(this->allUnits->find(k)->second.getType());
	}
	return avgDam / (double)formation.unitsKeys.size();
}

int Formation::getUnitIDHavingInVisionRangePoint(const Point & dest, const model::World & world) const
{
	int bestDurability = 0;
	int bestID = -1;
	for (int id : this->unitsKeys) {
		//printf("\nvis range real: %.2f", getRealVisionRangeForUnit(this->allUnits->find(id)->second, world));
		if (getRealVisionRangeForUnit(this->allUnits->find(id)->second, world) >= dest.distanceTo(this->allUnits->find(id)->second.getX(), this->allUnits->find(id)->second.getY())) {
			if (this->allUnits->find(id)->second.getDurability() == 100) return id;
			if (bestDurability < this->allUnits->find(id)->second.getDurability()) {
				bestID = id;
				bestDurability = this->allUnits->find(id)->second.getDurability();
			}
		}
	}
	return bestID;
}

std::vector<model::WeatherType> Formation::findCurrWeatherTypes(const model::World& world) const
{
	std::vector<model::WeatherType> wTypes;
	Rect rec = this->findRectangle();
	Point topLeftTile = getCellXY(rec.topLeft);
	Point botRightTile = getCellXY(rec.bottomRight);
	for (int i = topLeftTile.x; i <= botRightTile.x; i++) {
		for (int j = topLeftTile.y; j <= botRightTile.y; j++) {
			wTypes.push_back(world.getWeatherByCellXY()[i][j]);
		}
	}
	return wTypes;
}

std::vector<model::TerrainType> Formation::findCurrTerrainTypes(const model::World& world) const
{
	std::vector<model::TerrainType> terTypes;
	Rect rec = this->findRectangle();
	Point topLeftTile = getCellXY(rec.topLeft);
	Point botRightTile = getCellXY(rec.bottomRight);
	for (int i = topLeftTile.x; i <= botRightTile.x; i++) {
		for (int j = topLeftTile.y; j <= botRightTile.y; j++) {
			terTypes.push_back(world.getTerrainByCellXY()[i][j]);
		}
	}
	return terTypes;
}

bool Formation::isSimilarToLastMove(const model::Move & move)
{
	if (move.getAction() != this->lastAction) return false;
	if (this->lastDestPoint.x == move.getX() + this->findCenter().x && this->lastDestPoint.y == move.getY() + this->findCenter().y) {
		return true;
	}
	return false;
}

bool Formation::checkFormationForMoving() const
{
	for (int k : this->unitsKeys) if (this->checkUnitForMoving(k)) return true;
	return false;
}

bool Formation::checkUnitForMoving(int unitKey) const
{
	return !(this->allPrevUnits->find(unitKey)->second.getX() == this->allUnits->find(unitKey)->second.getX()
		&& this->allPrevUnits->find(unitKey)->second.getY() == this->allUnits->find(unitKey)->second.getY());
}

bool Formation::canIntersect(const Formation * f) const
{
	if (this->unitsType == f->unitsType || f->unitsType == MIXED || this->unitsType == MIXED) {
		return false;
	}
	return true;
}


