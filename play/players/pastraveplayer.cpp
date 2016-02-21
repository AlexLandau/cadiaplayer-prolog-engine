/*
 *  pastraveplayer.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 5/25/09.
 *  Copyright 2009 Reykjavik University. All rights reserved.
 *
 */

#include "pastraveplayer.h"

using namespace cadiaplayer::play::players;
using namespace cadiaplayer::play::utils;

PASTRAVEPlayer::PASTRAVEPlayer() : 
MASTRAVEPlayer()
{
	setTemperature(DEFAULT_PASTRAVE_TEMPERATURE);
}

void PASTRAVEPlayer::newGame(cadiaplayer::play::GameTheory& theory)
{
	MASTRAVEPlayer::newGame(theory);
	
	m_featuresList.clear();
	m_featuresList.resize(theory.getRoles()->size());
	for(cadiaplayer::play::utils::MovePredicateFeaturesListItr itr = m_featuresList.begin() ; itr != m_featuresList.end() ; ++itr)
	{
		itr->setTheory(&theory);
	}
}

#ifdef PASTRAVE_METHOD_1
double PASTRAVEPlayer::playoutMoveValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTState* state, cadiaplayer::play::utils::MTAction* node, cadiaplayer::play::Move* move)
{
	move = node ? node->move : m_multitree->getBufferedMove(m_workrole, move);
	return m_featuresList[m_workrole].getMaxConf(move, *theory.getStateRef());
}
#endif

size_t PASTRAVEPlayer::selectAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleMoves& moves)
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
#ifdef PASTRAVE_METHOD_2
	m_actionIndex.clear();
#endif	
	
	for(size_t n = 0 ; n < moves.size() ; n++)
	{
		node = m_multitree->lookupAction(m_workrole, state, moves[n]);
		actionAvailable(theory, moves[n], node, t, m_workrole);
		if(node == NULL || node->n == 0)
		{
			m_softmax[m_foundexp] = exp(playoutMoveValue(theory, state, node, moves[n])/getTemperature());
			m_divider+=m_softmax[m_foundexp];
			m_randexp[m_foundexp] = n;
#ifdef PASTRAVE_METHOD_2
			m_actionIndex[theory.getGameStateID(moves[n])] = m_foundexp;
#endif
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
	{
#ifdef PASTRAVE_METHOD_2
		swapInPAST(theory, m_softmax, m_divider);
#endif
		m_action = selectGibbsUnexplored();
	}
	else if(m_foundexp && m_val < unexploredNodeValue(theory, state, moves, m_foundexp))
	{
#ifdef PASTRAVE_METHOD_2
		swapInPAST(theory, m_softmax, m_divider);
#endif
		m_action = selectGibbsUnexplored();
	}
	else
		m_action = tiebreakExploited();
	
	return m_action;
}
#ifdef PASTRAVE_METHOD_2
void PASTRAVEPlayer::swapInPAST(cadiaplayer::play::GameTheory& theory, std::vector<double>& values, double& sum)
{
	if(values.size() < 2)
		return;
	cadiaplayer::play::utils::MaxPredicateAction* a;
	cadiaplayer::play::utils::ActionIndexMapItr aitr;
	for(size_t n = 0 ; n < theory.getStateRef()->size() ; n++)
	{
		a = m_featuresList[m_workrole].getMaxPredicateAction(theory.getGameStateID((*(theory.getStateRef()))[n]));
		if(a != NULL)
		{
			aitr = m_actionIndex.find(a->action);
			if(aitr != m_actionIndex.end())
			{
				double maxVal = exp(a->qVal/getTemperature());
				if(maxVal > values[aitr->second])
				{
					sum -= values[aitr->second];
					values[aitr->second] = maxVal; 
					sum += maxVal; 
				}
			}
		}
	}	
}
#endif

size_t PASTRAVEPlayer::selectSingleAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleMoves& moves)
{
	return selectAction(theory, moves);
}

void PASTRAVEPlayer::updateMoveValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::Move* move, double& q)
{
	MASTRAVEPlayer::updateMoveValue(theory, move, q);
	Move* m = m_multitree->getBufferedMove(m_workrole, move);
	m_featuresList[m_workrole].addQ(m, theory.getStateRef(), q);
}
void PASTRAVEPlayer::updateNodeValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTNode* node, cadiaplayer::play::utils::MTAction* action, double& q)
{
	MASTRAVEPlayer::updateNodeValue(theory, node, action, q);
	m_featuresList[m_workrole].addQ(action->move, theory.getStateRef(), q);
}

void PASTRAVEPlayer::postplay(cadiaplayer::play::GameTheory& theory)
{
	MASTRAVEPlayer::postplay(theory);
#ifdef DUMP_ACTIONBUFFER
	std::stringstream ss;
	ss << getPlayerShortName() << "_" << getDepth(theory) << ".txt";
	std::ofstream file;
	file.open(ss.str().c_str());
	m_featuresList[m_roleindex].getInfo(file);
	file.flush();
	file.close();
#endif
};
