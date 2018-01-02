#include "Task.h"

void Task::refresh()
{
	formationIndex = 0;
	this->defMoves.clear();
	this->defMoves.resize(0);
}

Task::Task() : formationIndex(0)
{
}

Task::Task(int formationIndex) : formationIndex(formationIndex)
{
}

Task::Task(const Task & task)
{
	this->formationIndex = task.formationIndex;
	this->canDo = task.canDo;
	this->defMoves.clear(); this->defMoves.resize(0);
	for (auto const &v : task.defMoves) {
		this->defMoves.push_back(v);
	}
}

Task & Task::operator=(const Task & task)
{
	if (&task == this) return *this;
	this->formationIndex = task.formationIndex;
	this->canDo = task.canDo;
	this->defMoves.clear(); this->defMoves.resize(0);
	for (auto const &v : task.defMoves) {
		this->defMoves.push_back(v);
	}
	return *this;
}
