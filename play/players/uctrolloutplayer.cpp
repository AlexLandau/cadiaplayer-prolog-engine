/*
 *  uctrolloutplayer.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 9/23/12.
 *  Copyright 2012 Reykjavik University. All rights reserved.
 *
 */

#include "uctrolloutplayer.h"

/*
 *  mastplayer.cc
 *  src
 *
 *  Created by Hilmar Finnsson on 10/25/07.
 *  Copyright 2007 CADIA Reykjavík University. All rights reserved.
 *
 */

#include "mastplayer.h"
using namespace cadiaplayer::play::players;
using namespace cadiaplayer::play::utils;

bool UCTRolloutPlayer::adjustArrays(std::size_t branch)
{
	if(!UCTPlayer::adjustArrays(branch))
		return false;
	
	m_softmax.resize(branch);
	return true;
}

size_t UCTRolloutPlayer::selectAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleMoves& moves)
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

size_t UCTRolloutPlayer::selectSingleAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleMoves& moves)
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
std::size_t UCTRolloutPlayer::selectFromDistribution(cadiaplayer::play::GameTheory& theory, std::size_t count, std::vector<double>& values, double& sum)
{
	return selectGibbAction(count, values, sum);
}

double UCTRolloutPlayer::playoutMoveValue(cadiaplayer::play::GameTheory& /*theory*/, MTState* state, MTAction* node, Move* move)
{
	// Just use uniform big number to make this the default best action when on the fringe
	return 99999;
}
