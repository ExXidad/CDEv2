//
// Created by xidad on 26.10.2020.
//

#include "Solver.h"

Solver::Solver(BoundingRect &boundingRect, const double &h, const double &c, const double &saveTRate, const double &CN)
{
	this->h = h;
	this->c = c;
	this->boundingRect = &boundingRect;
	this->CN = CN;

	dt = std::abs(CN * h / c);
	N = static_cast<int>(boundingRect.getYSize() / dt);

	if (c <= 0) this->CN *= -1;

	if (saveTRate <= 0)
	{
		this->saveTStep = 1;
		NT = N;
	} else
	{
		this->saveTStep = std::round(saveTRate / dt);
		NT = static_cast<int>(1. * N / saveTStep) + 1;
	}

	NX = static_cast<int>(boundingRect.getXSize() / h) - 1;

	u = new double *[NT];
	for (int i = 0; i < NT; ++i)
		u[i] = new double[NX];

	uTempNext = new double[NX];
	uTempPrevious = new double[NX];

	std::cout << "Initialized solver: " << std::endl;
	std::cout << "[0::dt::tMax]\t" << "[0::" << dt << "::" << boundingRect.getSize()[1][1] << "]" << std::endl;
	std::cout << "[xMin::h::xMax]\t" << "[" << boundingRect.getSize()[0][0] << "::" << h << "::"
	          << boundingRect.getSize()[0][1] << "]" << std::endl;
}

Solver::~Solver()
{
	for (int j = 0; j < NT; ++j)
		delete[] u[j];
	delete[] u;

	delete[] uTempPrevious;
	delete[] uTempNext;
}

double Solver::iToX(const int &i)
{
	return boundingRect->getSize()[0][0] + h / 2 + i * h;
}

double Solver::jToY(const int &j)
{
	return (j * dt*saveTStep);//boundingRect->getSize()[1][1] -
}

void Solver::solve(double(&ICF)(const double &, const double &), const TVDLimitersTypes &type)
{

	for (int i = 0; i < NX; ++i)
	{
		uTempPrevious[i] = ICF(iToX(i), jToY(0));
	}


	double
	(Solver::*TVDLimiterFunction)(const int &) = &Solver::minmodFunction;

	switch (type)
	{
		case MINMOD:
			TVDLimiterFunction = &Solver::minmodFunction;
			break;

		case MC:
			TVDLimiterFunction = &Solver::mcFunction;
			break;

		case SUPERBEE:
			TVDLimiterFunction = &Solver::superbeeFunction;
			break;
	}

	int tSavePos = 0;

	for (int j = 0; j < N; ++j)
	{
		if (j % saveTStep == 0)
		{
			for (int i = 0; i < NX; ++i)
				u[tSavePos][i] = uTempPrevious[i];

			++tSavePos;
		}

		for (int i = 0; i < NX; ++i)
		{
			double uWavePos = uWavePlusHalf(TVDLimiterFunction, i);
			double uWaveNeg = uWavePlusHalf(TVDLimiterFunction, i - 1);

			uTempNext[i] = uTempPrevious[i] - CN * (uWavePos - uWaveNeg);//*(uf(i+1)-uf(i-1))/2;
		}

		double *tmpPtr = uTempPrevious;
		uTempPrevious = uTempNext;
		uTempNext = tmpPtr;
	}
}


void Solver::exportData(std::fstream &file)
{
	for (int j = 0; j < NT; ++j)
	{
		for (int i = 0; i < NX; ++i)
		{
			file << j * dt << "\t" << boundingRect->getSize()[0][0] + i * h << "\t" << u[j][i] << "\t" << std::endl;;
		}
	}
}

void Solver::save()
{
	std::fstream file;
	for (int j = 0; j < NT; ++j)
	{
		file.open(std::to_string(jToY(j)), std::ios::out);
		for (int i = 0; i < NX; ++i)
		{
			file << boundingRect->getSize()[0][0] + i * h << "\t" << u[j][i] << "\t" << std::endl;;
		}
		file.close();
	}
}

double Solver::uf(const int &i)
{
	if (i < 0)
		return 0;
	else if (i >= NX) return 0;

	return uTempPrevious[i];
}

double
Solver::uWavePlusHalf(double (Solver::*TVDLimiterFunction)(const int &), const int &i)
{
	if (c >= 0)
		return uf(i) + (1 - CN) / 2 * (this->*TVDLimiterFunction)(i);
	else
		return uf(i) - (1 - CN) / 2 * (this->*TVDLimiterFunction)(i + 1);
}


double Solver::sgn(const double &x)
{
	if (x > 0) return 1;
	else if (x < 0) return -1;
	return 0;
}

double Solver::minmodFunction(const int &i)
{
	double f_im1 = uf(i - 1), f_i = uf(i), f_ip1 = uf(i + 1);
	return std::min(std::abs(f_ip1 - f_i), std::abs(f_i - f_im1)) * sgn(f_ip1 - f_i);
}

double Solver::superbeeFunction(const int &i)
{
	double f_im1 = uf(i - 1), f_i = uf(i), f_ip1 = uf(i + 1);
	return std::max(std::min(std::abs(f_ip1 - f_i), 2 * std::abs(f_i - f_im1)),
	                std::min(2 * std::abs(f_ip1 - f_i), std::abs(f_i - f_im1)));
}

double Solver::mcFunction(const int &i)
{
	double f_im1 = uf(i - 1), f_i = uf(i), f_ip1 = uf(i + 1);
	return std::min({std::abs(f_ip1 - f_im1) / 2, 2 * std::abs(f_ip1 - f_i), 2 * std::abs(f_i - f_im1)}) *
	       sgn(f_ip1 - f_i);
}
