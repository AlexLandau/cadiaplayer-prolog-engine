/*
 *  compoundtiling.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 11/30/09.
 *  Copyright 2009 Reykjavik University. All rights reserved.
 *
 */

#include "compoundtiling.h"
using namespace std;
using namespace cadiaplayer::play;
using namespace cadiaplayer::play::parsing;
using namespace cadiaplayer::play::tiling;


TileKey CompoundTiling::getFeature(Compound* c)
{
	TileKey key = c->getName();
	unsigned int shift = 0;
	key = key^encodeArgs(c, shift);
#ifdef DEBUG_TILES
	printf("Compoundtiling mapped %s into %u\n", c->toString(symbols).c_str(), key);
#endif
	return key;
}

TileKey CompoundTiling::encodeArgs(Compound* c, unsigned int& shift)
{
	if(c->getArguments() == NULL)
		return FEATURE_NONE;
	TileKey key = FEATURE_NONE;
	for(size_t n  = 0 ; n < c->getArguments()->size() ; ++n)
	{
		shift += COMPOUNDTILING_SHIFT_SIZE;
		if(shift >= COMPOUNDTILING_SHIFT_BOUND)
			shift -= COMPOUNDTILING_SHIFT_BOUND;
		key = key^(encodeCompound(c->getArgument(n), shift));
	}
#ifdef DEBUG_TILES
	printf("[Args]Compoundtiling mapped %s into %u\n", c->toString(symbols).c_str(), key);
#endif
	return key;
}
TileKey CompoundTiling::encodeCompound(Compound* c, unsigned int& shift)
{
	TileKey key = c->getName() << shift;
	if(c->getType() == ct_COMPOUND)
		key = key^encodeArgs(c, shift);
#ifdef DEBUG_TILES
	printf("[Compound]Compoundtiling mapped %s into %u\n", c->toString(symbols).c_str(), key);
#endif
	return key;
}
