/*
 *  piecevaluetiling.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 1/13/10.
 *  Copyright 2010 Reykjavik University. All rights reserved.
 *
 */

#include "piecevaluetiling.h"

using namespace std;
using namespace cadiaplayer::play;
using namespace cadiaplayer::play::parsing;
using namespace cadiaplayer::play::tiling;

PieceValueTiling::PieceValueTiling(cadiaplayer::play::parsing::SymbolTable* s, TileArguments& args) : 
ValueTiling(s, args) 
{
	if(!args.size())
		index = 0;
	else 
		index = args[0];
	
	if(args.size() < 2)
		roleCount = 0;
	else 
		roleCount = args[1];
	
}

TileKey PieceValueTiling::getFeature(Compound* c)
{
	if(c->getArguments()->size() <= index)
		return FEATURE_NONE;
	if(c->getArgument(index)->getName() < roleCount)
		return FEATURE_NONE;
	
	if(!strcmp(symbols->lookup(c->getArgument(index)->getName())->getLexeme().c_str(), "b"))
		return FEATURE_NONE;
	
	return (c->getName() << HalfTileKeySize) + (c->getArgument(index)->getName() << QuartTileKeySize) + index;
}
