/*
 *  uctraverolloutplayer.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 9/23/12.
 *  Copyright 2012 Reykjavik University. All rights reserved.
 *
 */

#include "uctraverolloutplayer.h"
using namespace cadiaplayer::play::players;
using namespace cadiaplayer::play::utils;

// UCTRolloutPlayer
bool UCTRAVERolloutPlayer::adjustArrays(std::size_t branch)
{
	if(!UCTRAVEPlayer::adjustArrays(branch))
		return false;
	
	m_softmax.resize(branch);
	return true;
}

// Override for move selection for 2+ player games (adversary and multiplayer).
std::size_t UCTRAVERolloutPlayer::selectAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleMoves& moves)
{
	if(moves.size() == 1)
		return 0;
	unsigned int t = getDepth(theory);
	MTState* state = m_multitree->lookupState(theory.getStateRef(), t);
	MTAction* node;
	
	m_action = 0;
	m_val = NEG_INF;
	m_curVal = NEG_INF;
	m_foundexp = 0;
	m_foundmax = 0;
	
	m_divider = 0;
	
	adjustArrays(moves.size());
	
	for(size_t n = 0 ; n < moves.size() ; n++)
	{
		node = m_multitree->lookupAction(m_workrole, state, moves[n]);
		actionAvailable(theory, moves[n], node, t, m_workrole);
		if(node == NULL || node->n == 0)
		{
			m_softmax[m_foundexp] = exp(playoutMoveValue(theory, state, node, moves[n])/getTemperature());
			m_divider+=m_softmax[m_foundexp];
			m_randexp[m_foundexp] = n;
			m_foundexp++;
			continue;
		}
		
		m_curVal = selectionNodeValue(theory, state, node);
		if(m_curVal == m_val)
			m_randmax[m_foundmax++] = n;
		else if(m_curVal > m_val)
		{
			m_val = m_curVal;
			m_action = n;
			m_foundmax=1;
			m_randmax[0] = n;
		}
	}
	if(!m_foundmax)
		m_action = m_randexp[selectFromDistribution(theory, m_foundexp, m_softmax, m_divider)];
	else if(m_foundexp && m_val < unexploredNodeValue(theory, state, moves, m_foundexp))
		m_action = m_randexp[selectFromDistribution(theory, m_foundexp, m_softmax, m_divider)];
	else if(m_foundmax > 1)
		m_action = m_randmax[rand() % m_foundmax];
	
	return m_action;
}
// Override for move selection for single player games (puzzles).
std::size_t UCTRAVERolloutPlayer::selectSingleAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleMoves& moves)
{
	unsigned int t = getDepth(theory);
	MTState* state = m_multitree->lookupState(theory.getStateRef(), t);
	MTNode* actionNode = NULL;
	if(moves.size() == 1)
	{
		actionNode = state ? m_multitree->lookupSingleNode(state, moves[0]) : NULL;
		if(actionNode && actionNode->expanded)
			state->expanded = true;
		return 0;
	}
	MTAction* node;
	
	m_action = 0;
	m_val = NEG_INF;
	m_curVal = NEG_INF;
	m_foundexp = 0;
	m_foundmax = 0;
	
	m_divider = 0;
	
	adjustArrays(moves.size());
	
	m_expanded = true;
	
	for(size_t n = 0 ; n < moves.size() ; n++)
	{
		node = m_multitree->lookupAction(m_workrole, state, moves[n]);
		actionAvailable(theory, moves[n], node, t, m_workrole);
		actionNode = state ? m_multitree->lookupSingleNode(state, moves[n]) : NULL;
		m_expanded = actionNode ? (m_expanded && actionNode->expanded) : false;
		if(node == NULL || node->n == 0)
		{
			m_softmax[m_foundexp] = exp(playoutMoveValue(theory, state, node, moves[n])/getTemperature());
			m_divider+=m_softmax[m_foundexp];
			m_randexp[m_foundexp] = n;
			m_foundexp++;
			continue;
		}
		
		if(actionNode && actionNode->expanded)
			m_curVal = 0;
		else
			m_curVal = selectionNodeValue(theory, state, node);
		
		if(m_curVal == m_val)
			m_randmax[m_foundmax++] = n;
		else if(m_curVal > m_val)
		{
			m_val = m_curVal;
			m_action = n;
			m_foundmax=1;
			m_randmax[0] = n;
		}
	}
	if(!m_foundmax)
		m_action = selectGibbsUnexplored();
	else if(m_foundexp && m_val < unexploredNodeValue(theory, state, moves, m_foundexp))
		m_action = selectGibbsUnexplored();
	else 
		m_action = tiebreakExploited();
	
	// If all child state fully expanded -> mark this as expanded
	if(m_expanded)
		state->expanded = true;
	
	return m_action;
}
// The return values will be selected from with the softmax policy. 
std::size_t	UCTRAVERolloutPlayer::selectFromDistribution(cadiaplayer::play::GameTheory& theory, std::size_t count, std::vector<double>& values, double& sum)
{
	return selectGibbAction(count, values, sum);
}
double		UCTRAVERolloutPlayer::playoutMoveValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTState* state, cadiaplayer::play::utils::MTAction* node, cadiaplayer::play::Move* move)
{
	// Just use uniform big number to make this the default best action when on the fringe
	return 99999;
}

// UCTRAVEPlayer
/*void UCTRAVERolloutPlayer::newSearch(cadiaplayer::play::GameTheory& theory)
{
	UCTRAVEPlayer::newSearch(theory);
}
double UCTRAVERolloutPlayer::selectionNodeValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTState* state, cadiaplayer::play::utils::MTAction* node)
{
	UCTRAVEPlayer::selectionNodeValue(theory, state, node);
}
cadiaplayer::play::Move* UCTRAVERolloutPlayer::bestAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTState* state, double &reward)
{
	UCTRAVEPlayer::UCTRAVERolloutPlayer(theory, state, reward);
}
cadiaplayer::play::Move* UCTRAVERolloutPlayer::controlAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTState* state, double &reward)
{
	UCTRAVEPlayer::UCTRAVERolloutPlayer(theory, state, reward);
}
void UCTRAVERolloutPlayer::bestActionRatings(RoleIndex role, cadiaplayer::play::utils::MTState* state,cadiaplayer::play::MoveRatings& ratings)
{
	UCTRAVEPlayer::bestActionRatings(role, ratings);
}
void UCTRAVERolloutPlayer::controlActionRatings(RoleIndex role, cadiaplayer::play::utils::MTState* state,cadiaplayer::play::MoveRatings& ratings)
{
	UCTRAVEPlayer::controlActionRatings(role, ratings);
}
void UCTRAVERolloutPlayer::updateMoveValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::Move* move, double& q)
{
	UCTRAVEPlayer::updateMoveValue(theory, move, q);
}
void UCTRAVERolloutPlayer::updateNodeValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTNode* node, cadiaplayer::play::utils::MTAction* action, double& q)
{
	UCTRAVEPlayer::updateNodeValue(theory, node, action, q);
}*/
