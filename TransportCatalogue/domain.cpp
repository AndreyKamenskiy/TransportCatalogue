#include "domain.h"

inline const double EPSILON = 1e-6;

bool IsZero(double value)
{
	return std::abs(value) < EPSILON;
}