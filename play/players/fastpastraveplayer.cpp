/*
 *  pastcomboplayer.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 3/18/10.
 *  Copyright 2010 Reykjavik University. All rights reserved.
 *
 */

#include "fastpastraveplayer.h"

using namespace cadiaplayer::play;
using namespace cadiaplayer::play::players;
using namespace cadiaplayer::play::utils;
using namespace cadiaplayer::play::kits;

bool FASTPASTRAVEPlayer::adjustArrays(std::size_t branch)
{
	if(!PASTRAVEPlayer::adjustArrays(branch))
		return false;
	
	m_fsoftmax.resize(branch);
	m_frandexp.resize(branch);
	return true;
}

void FASTPASTRAVEPlayer::newGame(cadiaplayer::play::GameTheory& theory)
{
	PASTRAVEPlayer::newGame(theory);
	newGameTilingEndSegment(theory, m_roleindex);
}

void FASTPASTRAVEPlayer::newSearch(cadiaplayer::play::GameTheory& theory)
{
	PASTRAVEPlayer::newSearch(theory);
	newSearchTilingEndSegment(theory);
}

void FASTPASTRAVEPlayer::atTerminal(cadiaplayer::play::GameTheory& theory, std::vector<double>& qValues)
{
	PASTRAVEPlayer::atTerminal(theory, qValues);
	atTerminalTilingEndSegment(theory, qValues);
}

void FASTPASTRAVEPlayer::makeAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::PlayMoves& pm, std::vector<double>& rewards)
{
	PASTRAVEPlayer::makeAction(theory, pm, rewards);
	makeActionTilingEndSegment(theory);
}
cadiaplayer::play::utils::MTNode* FASTPASTRAVEPlayer::makeAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTStateID sid, cadiaplayer::play::utils::Depth depth, cadiaplayer::play::PlayMoves& moves)
{
	MTNode* node = PASTRAVEPlayer::makeAction(theory, sid, depth, moves);
	makeActionTilingEndSegment(theory);
	return node;
}

std::string FASTPASTRAVEPlayer::getLastPlayInfo(cadiaplayer::play::GameTheory& theory) 
{
	std::stringstream ss;
	ss << PASTRAVEPlayer::getLastPlayInfo(theory);
	ss << std::endl;
	ss << getLastPlayInfoTilingEndSegment(theory);
	return ss.str();
}

size_t FASTPASTRAVEPlayer::selectAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleMoves& moves)
{
	if(moves.size() == 1)
		return 0;
	unsigned int t = getDepth(theory);
	MTState* state = m_multitree->lookupState(theory.getStateRef(), t);
	MTAction* node;
	
	double val = NEG_INF;
	double curVal = NEG_INF;
	m_foundmax = 0;
	m_foundexp = 0;
	m_score = 0;
	m_divider = 0;
	
	m_fscore = 0;
	m_fcount = 0;
	m_fdivider = 1.0;
	m_fuseTiling = false;	
	m_fvalid = isValid();
	adjustArrays(moves.size());
#ifdef PASTRAVE_METHOD_2
	m_actionIndex.clear();
#endif		
	for(size_t n = 0 ; n < moves.size() ; n++)
	{
		node = m_multitree->lookupAction(m_workrole, state, moves[n]);
		actionAvailable(theory, moves[n], node, t, m_workrole);
		if(node == NULL || node->n == 0)
		{
			m_score = playoutMoveValue(theory, state, node, moves[n]);
			m_softmax[m_foundexp] = exp(m_score/getTemperature());
			m_divider += m_softmax[m_foundexp];
			m_randexp[m_foundexp] = n;
#ifdef PASTRAVE_METHOD_2
			m_actionIndex[theory.getGameStateID(moves[n])] = m_foundexp;
#endif
			m_foundexp++;
			
			if(!m_fvalid)
				continue;
			
			m_fscore = getTilingMoveEvaluation(theory, m_workrole, m_roleindex, moves[n]);
			if(m_fscore != TILING_KIT_IGNORE_VALUE)
				m_fuseTiling = true;
			m_fsoftmax[m_fcount] = exp((m_fscore + m_score)/getTemperature());
			m_fdivider += m_fsoftmax[m_fcount];
			m_frandexp[m_fcount] = n;
			m_fcount++;
			
			continue;
		}
		else
			curVal = selectionNodeValue(theory, state, node);
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
	// Selection Step?
	if(m_foundmax)
	{
		//Is on the fringe?
		if(m_foundexp)
		{
			// Explore or Exploit Fringe?
			if(val < unexploredNodeValue(theory, state, moves, m_foundexp))
			{
				// Explore
				
				// Are FAST features active?
				if(m_fuseTiling)
				{
#ifdef PASTRAVE_METHOD_2
					swapInPAST(theory, m_fsoftmax, m_fdivider);
#endif
					return selectFASTUnexplored();
				}
				// Fallback on PAST
#ifdef PASTRAVE_METHOD_2
				swapInPAST(theory, m_softmax, m_divider);
#endif
				return selectGibbsUnexplored();
			}
			// Exploit - just let the default selection handle it
		}
		// Pure Selection Step
		return tiebreakExploited();
	}
	else // Pure Playout Step?
	{
		// Are FAST features active?
		if(m_fuseTiling)
		{
#ifdef PASTRAVE_METHOD_2
			swapInPAST(theory, m_fsoftmax, m_fdivider);
#endif			
			return selectFASTUnexplored();
		}
#ifdef PASTRAVE_METHOD_2
		swapInPAST(theory, m_softmax, m_divider);
#endif
		return selectGibbsUnexplored();
	}
	
	return 0; // Failsafe - return a legal move (should never happen).
}

