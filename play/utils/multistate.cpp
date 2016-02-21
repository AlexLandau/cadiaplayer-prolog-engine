/*
 *  multistate.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 12/9/11.
 *  Copyright 2011 Reykjavik University. All rights reserved.
 *
 */

#include "multistate.h"

using namespace cadiaplayer::play;
using namespace cadiaplayer::play::utils;

QValue MTNode::getMiniMaxOpti(RoleIndex role)
{
	if(isTerminal())
		return goals[role];
	return child == NULL ? MM_INIT_OPTI : child->minimax[role].opti;
}

QValue MTNode::getMiniMaxPess(RoleIndex role)
{
	if(isTerminal())
		return goals[role];
	return child == NULL ? MM_INIT_PESS : child->minimax[role].pess;
}

bool MTNode::isSolved(RoleIndex role)
{
	return getMiniMaxOpti(role) == getMiniMaxPess(role);
}

std::string MTNode::toString(cadiaplayer::play::GameTheory &theory)
{
	std::stringstream ss;
	ss << "Parent:   " << parent << std::endl;
	for(size_t n = 0 ; n < parent->facts.size() ; n++)
	{
		ss << theory.stateFactToString(parent->facts[n]) << std::endl;
	}
	ss << "child:    " << child << std::endl;
	ss << "terminal: " << toTerminal << std::endl;
	ss << "Actions:  " << std::endl;
	for(size_t n = 0 ; n < actions.size() ; n++)
	{
		ss << actions[n]->move->compound.toString(theory.getSymbols()) << " : goal -> " << goals[n] << std::endl;
	}
	return ss.str();
}
