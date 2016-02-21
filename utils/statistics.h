/*
 *  statistics.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 5/19/11.
 *  Copyright 2011 Reykjavik University. All rights reserved.
 *
 */


#ifndef STATISTICS_H
#define STATISTICS_H

#include <cmath>
#include <vector>

namespace cadiaplayer
{
	namespace utils
	{
		// If you want to save calculations if the sum has already been calculated, call this directly
		double variance(std::vector<double>& samples, double sum = 0.0);
	}
}
#endif //STATISTICS_H
			
