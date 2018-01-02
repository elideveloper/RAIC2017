#pragma once

#ifndef TASK_H_
#define TASK_H_

#include <vector>
#include <functional>

#include "Utility.h"

struct Task {
	int formationIndex;
	std::vector<std::function<model::Move(const model::World & w)> > defMoves;
	std::function<bool(const model::World&, const model::Player & me)> canDo;
	Task();
	Task(int formationIndex);
	Task(const Task & task);
	Task & operator= (const Task & task);
	void refresh();							// clear vector
};

#endif