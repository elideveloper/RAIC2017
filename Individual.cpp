#include "Individual.h"

#include <random>

Individual::Individual() : score(0.0)
{
}

// сначала будут с большим очком, за ними с меньшим
bool Individual::operator<(const Individual & ind) const
{
	return (this->score < ind.score);
}

void cross(Individual & mom, Individual & dad)
{
	unsigned short l = 0;
	if (mom.indexes.size() > 3) l = rand() % (mom.indexes.size() - 2) + 1;
	else if (mom.indexes.size() <= 1) return;
	else l = 1;
	unsigned short tmp = 0;
	for (l; l < mom.indexes.size(); l++) {
		tmp = mom.indexes[l];
		mom.indexes[l] = dad.indexes[l];
		dad.indexes[l] = tmp;
	}
}

Generation getRandomGeneration(const std::vector<std::vector<Point> > & potentialPositions, unsigned short generationSize)
{
	Generation generation;
	Individual individual;
	for (int i = 0; i < generationSize; i++) {
		individual.indexes.clear(); individual.indexes.resize(0);
		for (auto & f : potentialPositions) {
			unsigned short pos = rand() % f.size();
			individual.indexes.push_back(pos);
		}
		generation.push_back(individual);
	}
	return generation;
}
