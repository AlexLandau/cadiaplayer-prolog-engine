/*
 *  statistics.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 5/19/11.
 *  Copyright 2011 Reykjavik University. All rights reserved.
 *
 */

#include "statistics.h"

double cadiaplayer::utils::variance(std::vector<double>& samples, double sum)
{
	if(sum == 0.0)
	{
		for(int n = 0 ; n < samples.size() ; n++)
		{
			sum+=samples[n];
		}
	}
	double mean = sum/samples.size();
	double powsum = 0.0;
	for(int n = 0 ; n < samples.size() ; n++)
	{
		powsum += pow((samples[n]-mean),2);
	}
	return powsum/(samples.size()-1);
}

