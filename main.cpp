#include <iostream>
#include <random>

#include "Domain.h"
#include "BoundingRect.h"
#include "Solver.h"

//DF - domain function
bool DF1(const double &x, const double &y)
{
	return y <= 1;
}

bool DF2(const double &x, const double &y)
{
	return (3.33 <= x && x <= 6.66) && (1 <= y && y <= 2);
}

int main()
{
	BoundingRect boundingRect(0, 10, 0, 10);

	Domain domain;
	domain.addDomainFunction(DF1);
	domain.addDomainFunction(DF2);

	Solver solver(boundingRect, domain, 0.02);
	solver.solve(10000, 1);

	std::fstream file;
	file.open("domain.txt", std::ios::out);
	solver.exportComputationRegion(file);
	file.close();

	file.open("dendrite.txt", std::ios::out);
	solver.exportDendrite(file);
	file.close();

	file.open("data.txt", std::ios::out);
	solver.exportData(file);
	file.close();
	return 0;
}