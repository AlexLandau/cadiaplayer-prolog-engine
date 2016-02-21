/*
 *  valuetiling.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 11/30/09.
 *  Copyright 2009 Reykjavik University. All rights reserved.
 *
 */

#include "valuetiling.h"

using namespace std;
using namespace cadiaplayer::play;
using namespace cadiaplayer::play::parsing;
using namespace cadiaplayer::play::tiling;


TileKey ValueTiling::getFeature(Compound* c)
{
	if(c->getArguments()->size() <= index)
		return FEATURE_NONE;
	return (c->getName() << HalfTileKeySize) + (c->getArgument(index)->getName() << QuartTileKeySize) + index;
}
