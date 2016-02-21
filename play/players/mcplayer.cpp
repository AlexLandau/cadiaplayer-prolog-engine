/*
 *  mcplayer.cc
 *  src
 *
 *  Created by Hilmar Finnsson on 10/10/07.
 *  Copyright 2007 CADIA Reykjavík University. All rights reserved.
 *
 */

#include "mcplayer.h"
using namespace cadiaplayer::play::players;

void MCPlayer::newGame(GameTheory& theory)
{
	if(m_statespace != NULL)
		delete m_statespace;
	m_statespace = new cadiaplayer::play::utils::GameStateSpace();
	
	determineSinglePlayer(theory);
	setGamma(isSinglePlayer() ? 1.0 : DEFAULT_GAMMA);
	setPlayOutMaxValue(1.0);	
	m_lastBestActionNode = NULL;
	m_lastBestQ = 0.0;
	m_roleCount = theory.getRoles()->size();
	m_averageNodeExpansions = 0;
	m_averageSimulations = 0;
	m_averageIncrementalDivider = 0;
}
cadiaplayer::play::Move* MCPlayer::play(cadiaplayer::play::GameTheory& theory)
{
	m_postStartClock = false;
	if(!m_statespace->isTheorySet())
		m_statespace->setTheory(&theory, getRoleIndex());
	Move* move = doMCPlanning(theory); 
	return move;
}

void MCPlayer::asyncplay(cadiaplayer::play::GameTheory& theory)
{
	doSearch(theory); 
	m_asyncCounter++;
}
void MCPlayer::prepare(cadiaplayer::play::GameTheory& theory)
{
	play(theory);
	m_postStartClock = true;
}
cadiaplayer::play::Move* MCPlayer::doMCPlanning(cadiaplayer::play::GameTheory& theory)
{
	size_t d = getDepth(theory);
	if( d > 0)
		m_statespace->dropDepth(d-1);
	m_gameCounter = 0;
	startTimer();
	if(simulationsLimited())
	{
		do
		{
			doSearch(theory);
			++m_gameCounter;
		}
		while(hasTime() && !reachedSimulationsLimit());
	}
	else
	{
		do
		{
			doSearch(theory);
			++m_gameCounter;
		}
		while(hasTime());
	}
	double temp = ((1.0/d) * (m_nodeCounter - m_averageNodeExpansions));
	m_averageNodeExpansions += static_cast<int64_t> (temp);
	temp = ((1.0/d) * (m_gameCounter - m_averageSimulations));
	m_averageSimulations += static_cast<int64_t> (temp);
	
	return bestAction(theory);
}
void MCPlayer::newSearch(cadiaplayer::play::GameTheory& theory)
{
	m_curdepth = getDepth(theory);
	m_statespace->prepareState(theory.getStateRef(), *theory.getMoves(m_roleindex), m_curdepth);
	if(m_curdepth >1)
		m_statespace->dropDepth(m_curdepth-1);
	
	m_generateWholePath = false;
}
void MCPlayer::doSearch(cadiaplayer::play::GameTheory& theory)
{
	newSearch(theory);
	search(theory);
	endSearch(theory);	
}
double MCPlayer::gamma(void)
{
	return m_gammaValue;
}

double MCPlayer::search(cadiaplayer::play::GameTheory& theory)
{
	if(theory.isTerminal())
	{
		if(m_simulationCount) m_averageSimulationLength += (1/++m_simulationCount)*(theory.getRound()-m_averageSimulationLength);
		else{ m_averageSimulationLength = theory.getRound(); ++m_simulationCount;}
		double reward = theory.goal(m_roleindex);
		return reward;
	}
	RoleMoves moves;
	m_statespace->getMoves(theory, m_roleindex, getDepth(theory), moves);
	size_t move = selectAction(theory, moves);
	double reward = makeAction(theory, moves[move]);
	
	double q = search(theory);
	if( q < 0.000000001 )
		q = 0;
	q = reward + gamma() * q;
	retractAction(theory);
	updateValue(theory, moves[move], q);
	return q;
}
size_t MCPlayer::selectAction(cadiaplayer::play::GameTheory& /*theory*/, cadiaplayer::play::RoleMoves& moves)
{
	size_t i = rand() % moves.size();
	return i;
}
cadiaplayer::play::Move* MCPlayer::bestAction(cadiaplayer::play::GameTheory& theory)
{
	double q;
	Move* move = NULL;
	if(isSinglePlayer())
		move = m_statespace->maxAction(theory.getStateRef(), m_curdepth, q);
	else
		move = m_statespace->bestAction(theory.getStateRef(), m_curdepth, q);
	if(move == NULL || q <= 0)
		move = theory.generateRandomMove(m_roleindex);
	
	m_lastBestActionNode = m_statespace->getNode(theory.getStateRef(), move, getDepth(theory));
	m_lastBestQ = q;
	
	return move;
}
double MCPlayer::makeAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::Move* move)
{
	theory.make(m_roleindex, move);
	return getReward(theory, m_roleindex);
}
void MCPlayer::retractAction(cadiaplayer::play::GameTheory& theory)
{
	theory.retract();
}
double MCPlayer::getReward(cadiaplayer::play::GameTheory& /*theory*/, cadiaplayer::play::RoleIndex /*r*/)
{
	// No rewards untill terminal conditions as is so leave out for now
	double reward = 0.0; //theory.goal(r);
	return reward;
}
void MCPlayer::updateValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::Move* move, double& q)
{
	QNode* a = NULL;
	if(getDepth(theory) == m_curdepth)
		a = m_statespace->updateValue(theory.getStateRef(), move, m_curdepth, q);
}
double MCPlayer::minReachableGoal(GameTheory& theory)
{
	if(m_lastBestActionNode == NULL)
		return 0.0;
	if(isSinglePlayer())
		return m_lastBestActionNode->max;
	return m_lastBestQ;
}

std::string MCPlayer::getSetupInfo() 
{
	std::ostringstream s;
	if(simulationsLimited())
		s << SETUP_INFO_MESSAGE_SIMULATIONS_LIMITED << "(" << getSimulationsLimit() << ")";
	else
		s << SETUP_INFO_MESSAGE_SIMULATIONS_UNLIMITED;
	s << SETUP_INFO_MESSAGE_DELIMITER;
	if(nodeExpansionsLimited())
		s << SETUP_INFO_MESSAGE_EXPANSIONS_LIMITED << "(" << getNodeExpansionsLimit() << ")";
	else
		s << SETUP_INFO_MESSAGE_EXPANSIONS_UNLIMITED;
	return s.str();
}

std::string MCPlayer::getLastPlayInfo(cadiaplayer::play::GameTheory& /*theory*/) 
{
	std::ostringstream s;
	s << "Game counter          : " << m_gameCounter << std::endl;
	s << "Actions available     :\n" << m_statespace->getBestActionInfo() << std::endl;
	return s.str();
}

std::size_t MCPlayer::selectGibbAction(std::size_t count, std::vector<double>& softmax, double divider)
{
	size_t action = count-1;
	if(count == 1)
		return action;		
	if(divider == 0)
		return rand() % count;
	double sel = rand()/cadiaplayer::play::RAND_DIV;
	double pos = 0;
	//std::cerr << "*** Gibbs ****\n";
	for(size_t n = 0 ; n < count ; n++)
	{
		pos += softmax[n]/divider;
		if(pos > sel)
		{
	//		std::cerr << (n+1) << ".\t" << softmax[n] << " - break" << std::endl;
			action = n;
			break;
		}
	//	std::cerr << (n+1) << ".\t" << softmax[n] << std::endl;
	}
	return action;
}
