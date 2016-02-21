/*
 *  rolepiecetiling.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 1/7/10.
 *  Copyright 2010 Reykjavik University. All rights reserved.
 *
 */

#include "rolepiecetiling.h"

using namespace std;
using namespace cadiaplayer::play;
using namespace cadiaplayer::play::parsing;
using namespace cadiaplayer::play::tiling;

RolePieceTiling::RolePieceTiling():
index(2),
role(0)
{}

RolePieceTiling::RolePieceTiling(cadiaplayer::play::parsing::SymbolTable* s, TileArguments& args) : 
Tiling(s, args),
index(2),
role(0) 
{
	if(args.size())
		index = args[0];
	if(args.size() > 1)
		role = args[1];
	roleindex = index+1;
}


TileKey RolePieceTiling::getFeature(Compound* c)
{
	if(c->getArguments()->size() <= roleindex || c->getArgument(roleindex)->getName() != role)
		return FEATURE_NONE;
	TileKey key = c->getArgument(index-1)->getName();
	/*size_t shift = 7;
	key += (c->getArgument(index)->getName() << shift);
	shift += 7;
	key += (c->getArgument(roleindex)->getName() << shift);*/
	key += (c->getArgument(index)->getName() << 7);
	key += c->getArgument(roleindex)->getName();
	return key^RolePieceSalt;
}

TileKey RolePieceTiling::getFeature(Move* m, cadiaplayer::play::RoleIndex r)
{
	Compound* c = &(m->compound);
	//std::cerr << c->toString(symbols) << std::endl;
	if(c->getArguments()->size() <= index)
		return FEATURE_NONE;
	TileKey key = c->getArgument(index-1)->getName();
	/*size_t shift = 7;
	key += (c->getArgument(index)->getName() << shift);
	shift += 7;
	key += (r << shift);*/
	key += (c->getArgument(index)->getName() << 7);
	key += r;
	return key^RolePieceSalt;
}


std::string RolePieceTiling::toString(cadiaplayer::play::parsing::Compound* c)
{
	if(c->getArguments()->size() <= roleindex || c->getArgument(roleindex)->getName() != role)
		return "[n/a]";
	std::string str = symbols->getName(role) + " at ";
	str += symbols->getName(c->getArgument(index-1)->getName());
	str += " ";
	str += symbols->getName(c->getArgument(index)->getName());
	return str;
}

