/*
 *  uctraveplayer.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 2/26/09.
 *  Copyright 2009 Reykjavik University. All rights reserved.
 *
 */

#include "uctraveplayer.h"

using namespace cadiaplayer::play::players;
using namespace cadiaplayer::play::utils;

UCTRAVEPlayer::~UCTRAVEPlayer()
{
	for(size_t n = 0 ; n < m_moveTrails.size() ; ++n)
	{
		delete m_moveTrails[n];
	}
	m_moveTrails.clear();
}

void UCTRAVEPlayer::newSearch(cadiaplayer::play::GameTheory& theory)
{
	UCTPlayer::newSearch(theory);
	
	for(size_t n = 0 ; n < m_moveTrails.size() ; ++n)
	{
		delete m_moveTrails[n];
	}
	m_moveTrails.clear();
	for(size_t n = 0 ; n < theory.getRoles()->size() ; ++n)
	{
		m_moveTrails.push_back(new MoveTrail());
	}

}

cadiaplayer::play::Move* UCTRAVEPlayer::bestAction(cadiaplayer::play::GameTheory& theory, MTState* state, double &reward)
{
	return m_multitree->bestRAVEAction(m_roleindex, state, reward);
}
cadiaplayer::play::Move* UCTRAVEPlayer::controlAction(cadiaplayer::play::GameTheory& theory, MTState* state, double &reward)
{
	return m_multitree->controlRAVEAction(m_roleindex, state, reward);
}
void UCTRAVEPlayer::bestActionRatings(RoleIndex role, MTState* state,cadiaplayer::play::MoveRatings& ratings)
{
	m_multitree->bestRAVEActionRatings(role, state, ratings);
}
void UCTRAVEPlayer::controlActionRatings(RoleIndex role, MTState* state,cadiaplayer::play::MoveRatings& ratings)
{
	m_multitree->controlRAVEActionRatings(role, state, ratings);
}

double UCTRAVEPlayer::selectionNodeValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTState* state, cadiaplayer::play::utils::MTAction* node)
{
	double val = UCTPlayer::selectionNodeValue(theory, state, node);
	return rave(node, m_workrole, val);
}

double UCTRAVEPlayer::rave(MTAction* node, RoleIndex role, double basevalue)
{
	double beta = sqrt(m_equiv/(3.0*node->state->n+m_equiv));
	if(beta < 0.01)
		return basevalue;
	
	double tRAVE = static_cast<double>(node->state->nRAVE[role]); 
	unsigned int sRAVE = node->nRAVE;
	
	double ravevalue = node->qRAVE + DEFAULT_C*sqrt((log(tRAVE)/sRAVE));
	
	return beta*ravevalue + (1.0-beta)*basevalue;
}

void UCTRAVEPlayer::updateMoveValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::Move* move, double& q)
{
	MoveTrail* moveTrail = m_moveTrails[m_workrole];
	GameStateID moveKey = theory.getGameStateID(move);
	if(moveTrail->find(moveKey) == moveTrail->end())
		(*moveTrail)[moveKey] = q;
}
void UCTRAVEPlayer::updateNodeValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTNode* node, cadiaplayer::play::utils::MTAction* action, double& q)
{
	MoveTrail* moveTrail = m_moveTrails[m_workrole];
	GameStateID moveKey = node->actions[m_workrole]->id;
	
	// RAVE - All moves as first
	MTAction* rave = NULL;
	m_multitree->updateValueRAVE(m_workrole, node, q);
	for(MoveTrail::iterator itr = moveTrail->begin() ; itr != moveTrail->end() ; itr++)
	{
		if(itr->first == moveKey)
			continue;
		rave = m_multitree->lookupAction(m_workrole, action->state, itr->first);
		if(rave == NULL)
			continue;
		m_multitree->updateValueRAVESibling(m_workrole, rave, itr->second);
	}
	if(moveTrail->find(moveKey) == moveTrail->end())
		(*moveTrail)[moveKey] = q;
}
