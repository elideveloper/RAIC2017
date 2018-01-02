#pragma once

#ifndef INDIVIDUAL_H_
#define INDIVIDUAL_H_

#include <vector>

#include "Point.h"

struct Individual {
	double score;
	std::vector<unsigned short> indexes;

	Individual();
	bool operator<(const Individual & ind) const;
};

typedef std::vector<Individual> Generation;

void cross(Individual & mom, Individual & dad);
Generation getRandomGeneration(const std::vector<std::vector<Point> > & potentialPositions, unsigned short generationSize);

#endif
