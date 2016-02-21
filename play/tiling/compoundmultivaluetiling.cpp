/*
 *  compoundmultivaluetiling.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 11/30/09.
 *  Copyright 2009 Reykjavik University. All rights reserved.
 *
 */

#include "compoundmultivaluetiling.h"
using namespace std;
using namespace cadiaplayer::play;
using namespace cadiaplayer::play::parsing;
using namespace cadiaplayer::play::tiling;

TileKey CompoundMultiValueTiling::getFeature(Compound* c)
{
	if(arguments.size() == 0)
		return FEATURE_NONE;
	if(c->getArguments()->size() <= arguments[0])
		return FEATURE_NONE;
	TileKey key = FEATURE_NONE;(c->getArgument(arguments[0])->getName() << (HalfTileKeySize + arguments[0]));
	for(size_t n = 1 ; n < arguments.size() ; ++n)
	{
		if(c->getArguments()->size() <= arguments[n])
			break;
		key ^= (c->getArgument(arguments[n])->getName() << (HalfTileKeySize + arguments[n]));
	}
	return key | ((c->getName() << 6) + arguments[0]);
	
}
std::string CompoundMultiValueTiling::toString(cadiaplayer::play::parsing::Compound* c)
{
	if(arguments.size() == 0)
		return "[n/a]";
	string str = "[n/a]";
	if(arguments[0] < c->getArguments()->size())
	{
		str = symbols->getName(c->getName()) + "(";
		size_t counter = 0;
		while(counter < arguments[0])
		{
			++counter;
			str += "?,";
		}
		str += c->getArgument(arguments[0])->toString(symbols);
		for(size_t n = 1 ; n < arguments.size() ; ++n)
		{
			if(arguments[n] < c->getArguments()->size())
			{
				while(counter < c->getArguments()->size())
				{
					++counter;
					str += ", ?";
				}
				break;
			}
			while(counter < arguments[n])
			{
				++counter;
				str += ", ?";
			}
			str += c->getArgument(arguments[n])->toString(symbols);
		}
		str += ")";
	}
	return str;
}
