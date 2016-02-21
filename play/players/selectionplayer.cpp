/*
 *  selectionplayer.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 10/11/11.
 *  Copyright 2011 Reykjavik University. All rights reserved.
 *
 */

#include "selectionplayer.h"

using namespace cadiaplayer::play::players;
using namespace cadiaplayer::play::utils;

SelectionPlayer::SelectionPlayer() : MCMemoryPlayer()
{
};
SelectionPlayer::~SelectionPlayer()
{
};

void SelectionPlayer::enableEarlyCutoff(int minimumThreshold)
{
	m_useEarlyCutoff = true;
	this->setMinimumTerminalSpan(minimumThreshold);
};
void SelectionPlayer::disableEarlyCutoff()
{
	m_useEarlyCutoff = false;
};


size_t SelectionPlayer::selectAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleMoves& moves)
{
	if(moves.size() == 1)
		return 0;
	unsigned int t = getDepth(theory);
	MTState* state = m_multitree->lookupState(theory.getStateRef(), t);
	MTAction* node;
	
	unsigned int i = 0;
	double val = NEG_INF;
	double curVal = NEG_INF;
	size_t randmax[moves.size()];
	size_t foundmax = 0;
	
	for(size_t n = 0 ; n < moves.size() ; n++)
	{
		node = m_multitree->lookupAction(m_workrole, state, moves[n]);
		actionAvailable(theory, moves[n], node, t, m_workrole);
		if(node == NULL || node->n == 0)
			curVal = unexploredNodeValue(theory, state, moves, moves.size());//POS_INF;
		else
			curVal = selectionNodeValue(theory, state, node);
		if(curVal == val)
			randmax[foundmax++] = n;
		else if(curVal > val)
		{
			val = curVal;
			i = n;
			foundmax=1;
			randmax[0] = n;
		}
	}
	if(foundmax > 1)
		i = randmax[rand() % foundmax];
	return i;
}

double SelectionPlayer::selectionNodeValue(cadiaplayer::play::GameTheory& /*theory*/, cadiaplayer::play::utils::MTState* state, cadiaplayer::play::utils::MTAction* node)
{
	return node->q;
}

double SelectionPlayer::unexploredNodeValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTState* state, cadiaplayer::play::RoleMoves& moves, std::size_t unexplored)
{
	return POS_INF;
}

//****************************************************
// Early cutoff, inclusion of the GoalStability kit.
//****************************************************
void SelectionPlayer::newGame(cadiaplayer::play::GameTheory& theory)
{
	MCMemoryPlayer::newGame(theory);
	if(isEarlyCutoffEnabled())
		newGameGoalStabilityEndSegment(theory, this);
}
void SelectionPlayer::newSearch(cadiaplayer::play::GameTheory& theory)
{
	MCMemoryPlayer::newSearch(theory);
	if(isEarlyCutoffEnabled())
		newSearchGoalStabilityEndSegment(theory);
}
bool SelectionPlayer::cutoff(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::Depth depth)
{
	bool cut = MCMemoryPlayer::cutoff(theory, depth);
	if(cut || !isEarlyCutoffEnabled())
		return cut;
	
	return cutoffGoalStabilityEndSegment(theory, m_selfrandomsteps);
}
void SelectionPlayer::makeAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::PlayMoves& pm, std::vector<double>& rewards)
{
	MCMemoryPlayer::makeAction(theory, pm, rewards);
	makeActionGoalStabilityEndSegment(theory);
}
cadiaplayer::play::utils::MTNode* SelectionPlayer::makeAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTStateID sid, cadiaplayer::play::utils::Depth depth, cadiaplayer::play::PlayMoves& moves)
{
	MTNode* node = MCMemoryPlayer::makeAction(theory, sid, depth, moves);
	if(isEarlyCutoffEnabled())
		makeActionGoalStabilityEndSegment(theory);
	return node;
}
void SelectionPlayer::atTerminal(cadiaplayer::play::GameTheory& theory, std::vector<double>& qValues)
{
	MCMemoryPlayer::atTerminal(theory, qValues);
	if(isEarlyCutoffEnabled())
		atTerminalGoalStabilityEndSegment(theory, qValues);
}
std::string SelectionPlayer::getSetupInfo()
{
	std::stringstream ss;
	ss << MCMemoryPlayer::getSetupInfo() << SETUP_INFO_MESSAGE_DELIMITER;
	if(isEarlyCutoffEnabled())
		ss << EC_ACTIVE_LOG_MESSAGE;
	else
		ss << EC_INACTIVE_LOG_MESSAGE;
	return ss.str();
}
std::string SelectionPlayer::getLastPlayInfo(cadiaplayer::play::GameTheory& theory)
{
	std::stringstream ss;
	ss << MCMemoryPlayer::getLastPlayInfo(theory);
	if(isEarlyCutoffEnabled())
		ss << getLastPlayInfoGoalStabilityEndSegment(theory);
	else
		ss << EC_INACTIVE_LOG_MESSAGE;

	ss << std::endl;
	return ss.str();
}

