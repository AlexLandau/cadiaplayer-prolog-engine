/*
 *  fastplayer.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 3/3/12.
 *  Copyright 2012 Reykjavik University. All rights reserved.
 *
 */

#include "fastplayer.h"

using namespace cadiaplayer::play;
using namespace cadiaplayer::play::players;
using namespace cadiaplayer::play::utils;
using namespace cadiaplayer::play::kits;

bool FASTPlayer::adjustArrays(std::size_t branch)
{
	if(!UCTPlayer::adjustArrays(branch))
		return false;
	
	m_fsoftmax.resize(branch);
	m_frandexp.resize(branch);
	return true;
}

void FASTPlayer::newGame(cadiaplayer::play::GameTheory& theory)
{
	UCTPlayer::newGame(theory);
	newGameTilingEndSegment(theory, m_roleindex);
}

void FASTPlayer::newSearch(cadiaplayer::play::GameTheory& theory)
{
	UCTPlayer::newSearch(theory);
	newSearchTilingEndSegment(theory);
}

void FASTPlayer::atTerminal(cadiaplayer::play::GameTheory& theory, std::vector<double>& qValues)
{
	UCTPlayer::atTerminal(theory, qValues);
	atTerminalTilingEndSegment(theory, qValues);
}

void FASTPlayer::makeAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::PlayMoves& pm, std::vector<double>& rewards)
{
	UCTPlayer::makeAction(theory, pm, rewards);
	makeActionTilingEndSegment(theory);
}
cadiaplayer::play::utils::MTNode* FASTPlayer::makeAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTStateID sid, cadiaplayer::play::utils::Depth depth, cadiaplayer::play::PlayMoves& moves)
{
	MTNode* node = UCTPlayer::makeAction(theory, sid, depth, moves);
	makeActionTilingEndSegment(theory);
	return node;
}

std::string FASTPlayer::getLastPlayInfo(cadiaplayer::play::GameTheory& theory) 
{
	std::stringstream ss;
	ss << UCTPlayer::getLastPlayInfo(theory);
	ss << std::endl;
	ss << getLastPlayInfoTilingEndSegment(theory);
	return ss.str();
}

size_t FASTPlayer::selectAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleMoves& moves)
{
	m_fvalid = isValid();
	if(!m_fvalid)
		return UCTPlayer::selectAction(theory, moves);
	
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
	
	double val = NEG_INF;
	double curVal = NEG_INF;
	m_foundmax = 0;
	m_foundexp = 0;
	m_score = 0;
	m_expanded = true;
	
	m_fscore = 0;
	m_fcount = 0;
	m_fdivider = 1.0;
	m_fuseTiling = false;	
	adjustArrays(moves.size());
	
	for(size_t n = 0 ; n < moves.size() ; n++)
	{
		node = m_multitree->lookupAction(m_workrole, state, moves[n]);
		actionAvailable(theory, moves[n], node, t, m_workrole);
		// Check if every child has been fully expanded/explored to a leaf
		actionNode = state ? m_multitree->lookupSingleNode(state, moves[n]) : NULL;
		m_expanded = actionNode ? (m_expanded && actionNode->expanded) : false;
		if(node == NULL || node->n == 0)
		{
			m_randexp[m_foundexp++] = n;
			
			if(!m_fvalid)
				continue;
			
			m_fscore = getTilingMoveEvaluation(theory, m_workrole, m_roleindex, moves[n]);
			if(m_fscore != TILING_KIT_IGNORE_VALUE)
				m_fuseTiling = true;
			
			m_fsoftmax[m_fcount] = exp(m_fscore);
			m_fdivider += m_fsoftmax[m_fcount];
			m_frandexp[m_fcount] = n;
			m_fcount++;
			
			continue;
		}
		else
			curVal = uct(node);
		if(curVal == val)
			m_randmax[m_foundmax++] = n;
		else if(curVal > val)
		{
			val = curVal;
			m_action = n;
			m_foundmax=1;
			m_randmax[0] = n;
		}
	}
	
	if(!m_foundmax) 
	{
		if(m_fuseTiling)
			m_action = selectFASTUnexplored();
		else
			m_action = selectRandomUnexplored();
		
	}
	else if(m_foundexp && m_val < unexploredNodeValue(theory, state, moves, m_foundexp))
	{
		if(m_fuseTiling)
			m_action = selectFASTUnexplored();
		else
			m_action = selectRandomUnexplored();
	}
	else  // Is tie breaking needed
		m_action = tiebreakExploited();
	
	// If all child state fully expanded -> mark this as expanded
	if(m_expanded)
		state->expanded = true;
	
	return m_action;
}


