/*
 *  mcmemoryplayer.cc
 *  src
 *
 *  Created by Hilmar Finnsson on 10/10/07.
 *  Copyright 2007 CADIA Reykjavík University. All rights reserved.
 *
 */

#include "mcmemoryplayer.h"
using namespace cadiaplayer::play;
using namespace cadiaplayer::play::players;
using namespace cadiaplayer::play::utils;

MCMemoryPlayer::~MCMemoryPlayer()
{
	if(m_multitree != NULL)
		delete  m_multitree;
}
void MCMemoryPlayer::newGame(cadiaplayer::play::GameTheory& theory)
{
	srand((unsigned)time(0));
	if(m_multitree != NULL)
		delete  m_multitree;
	m_multitree = new MultiTree(&theory);
	determineSinglePlayer(theory);
	m_solved = false;
	setGamma(isSinglePlayer() ? 1.0 : DEFAULT_GAMMA);
	
#ifdef USE_WORKERS	
	clearWork();
	initWorkerSlaves(theory);
#endif
	
	setPlayOutMaxValue(1.0);
	m_lastBestActionNode = NULL;
	m_lastBestQ = 0.0;
	m_roleCount = theory.getRoles()->size();
	expandMovesHolder(MCMEM_INIT_MOVES_HOLDER_DEPTH);
	m_averageNodeExpansions = 0;
	m_averageSimulations = 0;
	m_averageIncrementalDivider = 0;
	
	m_qValueHolder.clear();
	m_qValueHolder.resize(theory.getRoles()->size(), 0.0);
	m_goalValueHolder.clear();
	m_goalValueHolder.resize(theory.getRoles()->size(), 0.0);
}

bool MCMemoryPlayer::exceedsMoveHolder(Depth d)
{
	return m_simMoves.size() <= d;
}

void MCMemoryPlayer::expandMovesHolder(Depth d)
{
	Depth cur = m_simMoves.size();
	++d;
	m_simMoves.resize(d);
	for(; cur < d ; ++cur)
	{
		m_simMoves[cur].resize(m_roleCount);
	}
}

cadiaplayer::play::Move* MCMemoryPlayer::play(cadiaplayer::play::GameTheory& theory)
{
	if(!m_memoryreset)
		m_multitree->resetMemoryInfo();

	m_multitree->newRound(getDepth(theory));
	m_postStartClock = false;
	startTimer();
	m_abort = false;

#ifdef USE_WORKERS
	// Process m_workers that finished beyond last turn - note need to cap the trail fronts as no longer exists
	// Note capping also occurs for late m_workers of selfplay even though still the same state, but just allow to be MIA. 
	for(size_t n = 0 ; n < m_workerTrails.size() ; ++n)
	{
		if(m_workerTrails[n] && !m_workerTrails[n]->empty())
			m_workerTrails[n]->pop_front();
	}
	processWorkers(theory);
	// Clean up work to do
	clearWork();
#endif
	
#ifdef LOG_UCT_TREE
	m_treelog.setRoot(theory.getGameStateID(theory.getStateRef()), getDepth(theory));
#endif

	Move* move = doMCPlanning(theory); 
	m_memoryreset = false;
	return move;
}
void MCMemoryPlayer::stop(cadiaplayer::play::GameTheory& theory) 
{
#ifdef LOG_UCT_TREE
	Depth d = getDepth(theory);
	for( ; d <= m_multitree->size() ; d++) 
	{
		if(d==0)
			continue;
		for(MTStateMapItr map = m_multitree->getStateMap(d-1)->begin() ; map != m_multitree->getStateMap(d-1)->end() ; map++)
		{
			m_treelog.deleteNode(map->first, d-1);
		}
		m_multitree->dropDepth(d-1);
	}
#endif
	delete m_multitree;
	m_multitree = NULL;
#ifdef LOG_UCT_TREE
	m_treelog.setRoot(theory.getGameStateID(theory.getStateRef()), getDepth(theory));
	m_treelog.updateNode(theory.getGameStateID(theory.getStateRef()), getDepth(theory), theory.goal(m_roleindex));
#endif
};

void MCMemoryPlayer::asyncplay(cadiaplayer::play::GameTheory& theory)
{
	
	if(!m_memoryreset)
	{
		m_multitree->resetMemoryInfo();
		resetCounters();
	}
	startTimer();
	m_abort = false;
	
#ifdef USE_WORKERS	
	processWorkers(theory);
#endif
	
	doSearch(theory); 
	m_asyncCounter++;
	m_memoryreset = true;
}
cadiaplayer::play::Move* MCMemoryPlayer::doMCPlanning(cadiaplayer::play::GameTheory& theory)
{
	
	Depth d = getDepth(theory);
	if( d > 0) 
	{
#ifdef LOG_UCT_TREE
		for(MTStateMapItr map = m_multitree->getStateMap(d-1)->begin() ; map != m_multitree->getStateMap(d-1)->end() ; map++)
		{
			m_treelog.deleteNode(map->first, d-1);
		}
#endif
		m_multitree->dropDepth(d-1);
	}
	
	if(!m_memoryreset)
		resetCounters();
	
	if(simulationsLimited())
	{
		do
		{
			doSearch(theory);
			++m_gameCounter;
		}
		while(hasTime() && !reachedSimulationsLimit() && !m_solved);
	}
	else if(nodeExpansionsLimited())
	{
		do
		{
			doSearch(theory);
			++m_gameCounter;
		}
		while(hasTime() && !reachedNodeExpansionsLimit() && !m_solved);
	}
	else
	{
		do
		{
			doSearch(theory);
			++m_gameCounter;
		}
		while(hasTime() && !m_solved);
	}
	m_workrole = m_roleindex;
	
	m_averageIncrementalDivider += 1.0;
	double temp;
	temp = ((1.0/m_averageIncrementalDivider) * (m_nodeCounter - m_averageNodeExpansions));
	m_averageNodeExpansions += static_cast<int64_t> (temp);
	temp = ((1.0/m_averageIncrementalDivider) * (m_gameCounter - m_averageSimulations));
	m_averageSimulations += static_cast<int64_t> (temp);
	
	return bestAction(theory);
}
void MCMemoryPlayer::resetCounters()
{
	m_gameCounter = 0;
	m_nodeCounter = 0;
	m_asyncCounter = 0;
	m_selfrandomplay = 0;
	m_memoryplay = 0;
	m_aborted = 0;
	m_borderCrossings = 0;
	
#ifdef USE_WORKERS	
	m_workerAssignedCounter = 0;
	m_workerStartedCounter = 0;
	m_workerDoneCounter = 0;
	m_workerRequestDoneCounter = 0;	
#endif
	
	m_totalcontrol = !isSinglePlayer();
}
void MCMemoryPlayer::newSearch(cadiaplayer::play::GameTheory& theory)
{
	m_multitree->resetTrail();
	
	m_generateWholePath = false;
	
	m_multitree->muteRetract();
	
	m_selfrollout = false;
	m_borderState = NULL;
	m_partialcontrol = !isSinglePlayer();
	m_startingDepth = theory.getRound()-1;
	m_memorysteps = 0;
	m_selfrandomsteps = 0;
	
#ifdef USE_WORKERS
	m_assigned = false;
#endif
}
double MCMemoryPlayer::search(cadiaplayer::play::GameTheory& theory)
{
	search(theory, m_qValueHolder);
	return m_qValueHolder[m_roleindex];
}

void MCMemoryPlayer::search(GameTheory& theory, std::vector<double>& qValues)
{
	m_nodeCounter++;
	m_memorysteps++;
	size_t d = getDepth(theory);
	MTStateID sid = theory.getGameStateID(theory.getStateRef());
	if(this->cutoff(theory, d))
	{
		calcTerminal(theory, qValues);
		atTerminal(theory, qValues);
		return;
	}
	if(m_multitree->isTerminal())
	{
		calcTerminal(theory, qValues);
		m_multitree->terminateTrail(sid, d);
		atTerminal(theory, qValues);
		return;
	}
	if(!hasTime())
	{
		m_abort = true;
		m_aborted++;
		return;
	}
	PlayMoves pm;
	size_t move;
	if(exceedsMoveHolder(d))
		expandMovesHolder(d+MCMEM_EXPAND_MOVES_HOLDER_DEPTH);
	for(size_t n  = 0 ; n < theory.getRoles()->size() ; ++n)
	{
		// Setup workrole and moves holder
		m_workrole = n;
		RoleMoves& moves = m_simMoves[d][m_workrole];
		moves.clear();
		
		// Get moves and select action
		if(!getMoves(n, sid, d, moves))	
			return calcTerminal(theory, qValues);
		m_borderState = m_multitree->prepareState(n, sid, moves, d, theory.getStateRef());
		if(isSinglePlayer())
			move = selectSingleAction(theory, moves);
		else
		{
			if(m_partialcontrol && n != m_roleindex && moves.size() > 1)
				m_partialcontrol = false;
			move = selectAction(theory, moves);
		}
		pm.push_back(moves[move]);
	}
	m_multitree->pushTrail(makeAction(theory, sid, d, pm));
	if(m_borderState)
	{
		m_borderCrossings++;
#ifdef USE_WORKERS
		if(assignWorkerSlave())
			m_assigned = true;
		else
		{
			m_selfrollout = true;
			selfRollout(theory, qValues);
		}
#else
		m_selfrollout = true;
		selfRollout(theory, qValues);
#endif
	}
	else
		search(theory, qValues);
	// Retract when recursion returns to maintain game integrity
	retractAction(theory);
	// If recursion returned due to abort indicator, no need to do more calculations
	if(m_abort)
		return;

#ifdef USE_WORKERS
	if( m_assigned )
		return;
#endif

	// Discount return values and update q values.
	updateValues(theory, pm, qValues);
}

void MCMemoryPlayer::selfRollout(GameTheory& theory, std::vector<double>& qValues)
{
	m_nodeCounter++;
	m_selfrandomsteps++;
	size_t d = getDepth(theory);
	MTStateID sid = theory.getGameStateID(theory.getStateRef());
	if(m_multitree->isTerminal())
	{
		calcSelfTerminal(theory, qValues);
		atTerminal(theory, qValues);
		return;
	}
	if(this->cutoff(theory, d))
	{
		calcSelfTerminal(theory, qValues);
		atTerminal(theory, qValues);
		return;
	}
	if(!hasTime())
	{
		m_abort = true;
		m_aborted++;
		return;
	}
	PlayMoves pm;
	size_t move;
	if(exceedsMoveHolder(d))
		expandMovesHolder(d+MCMEM_EXPAND_MOVES_HOLDER_DEPTH);
	for(size_t n  = 0 ; n < theory.getRoles()->size() ; ++n)
	{
		// Setup workrole and moves holder
		m_workrole = n;
		RoleMoves& moves = m_simMoves[d][m_workrole];
		moves.clear();
		
		// Get moves and select action
		if(!getMoves(n, sid, d, moves))
			return calcSelfTerminal(theory, qValues);
		if(isSinglePlayer())
			move = selectSingleAction(theory, moves);
		else
		{
			if(m_partialcontrol && n != m_roleindex && moves.size() > 1)
				m_partialcontrol = false;
			move = selectAction(theory, moves);
		}
		pm.push_back(moves[move]);
	}
	m_multitree->pushTrail(makeAction(theory, sid, d, pm));
	selfRollout(theory, qValues);
	// Retract when recursion returns to maintain game integrity
	retractAction(theory);
	// If recursion returned due to abort indicator, no need to do more calculations
	if( m_abort )
		return;
	// Discount return values and update q values.
	updateValues(theory, pm, qValues);
}

void MCMemoryPlayer::calcTerminal(GameTheory& theory, std::vector<double>& qValues)
{
	size_t d = getDepth(theory);
	MTStateID sid = theory.getGameStateID(theory.getStateRef());
	RoleMoves moves;
	for(size_t n  = 0 ; n < theory.getRoles()->size() ; ++n)
	{
		if(m_multitree->isTerminal())
		{
			//MTStateID pid = theory.getGameStateID(theory.getParentState());
			m_multitree->prepareState(n, sid, moves, d, theory.getStateRef());
			
		}
		qValues[n] = m_multitree->goal(n);
		m_goalValueHolder[n] = qValues[n];
	}
	processGoals(qValues);
	m_memoryplay++;
	if(isSinglePlayer() && getPlayOutMaxValue() <= qValues[m_roleindex])
	{
		setPlayOutMaxValue(qValues[m_roleindex]);
		m_generateWholePath = true;
		if(qValues[m_roleindex] == GOAL_MAX)
		{	
#ifdef USE_WORKERS
			 if(!theory.usesWorkers())
#endif
				 m_solved = true;
		}
		m_pathChild = 0;
#ifdef LOG_UCT_TREE
		m_treelog.bufferTrail(theory.getGameStateID(theory.getStateRef()), qValues[m_roleindex]);
#endif
	}
	else if(!isSinglePlayer())
	{
		if(!m_partialcontrol)
			m_totalcontrol = false;
	}
	if(m_simulationCount) m_averageSimulationLength += (1/++m_simulationCount)*(theory.getRound()-m_averageSimulationLength);
	else{ m_averageSimulationLength = theory.getRound(); ++m_simulationCount;}
	
	return;
}
void MCMemoryPlayer::calcSelfTerminal(GameTheory& theory, std::vector<double>& qValues)
{
	for(size_t n  = 0 ; n < theory.getRoles()->size() ; ++n)
	{
		qValues[n] = m_multitree->goal(n);
		m_goalValueHolder[n] = qValues[n];
	}
	processGoals(qValues);
	m_selfrandomplay++;
	if(isSinglePlayer() && getPlayOutMaxValue() <= qValues[m_roleindex])
	{
		setPlayOutMaxValue(qValues[m_roleindex]);
		m_generateWholePath = true;
		if(qValues[m_roleindex] == GOAL_MAX)
		{	
#ifdef USE_WORKERS
			if(!theory.usesWorkers())
#endif
				m_solved = true;
		}
		
		m_pathChild = 0;
#ifdef LOG_UCT_TREE
		m_treelog.bufferTrail(theory.getGameStateID(theory.getStateRef()), qValues[m_roleindex]);
#endif
	}
	else if(!isSinglePlayer())
	{
		if(!m_partialcontrol)
			m_totalcontrol = false;
	}
	if(m_simulationCount) m_averageSimulationLength += (1/++m_simulationCount)*(theory.getRound()-m_averageSimulationLength);
	else{ m_averageSimulationLength = theory.getRound(); ++m_simulationCount;}
}
void MCMemoryPlayer::endSearch(cadiaplayer::play::GameTheory& theory)
{
	// The gamestate has retracted back to the state present when search began
	// Tell the controller that it is back in sync
#ifdef USE_WORKERS
	// Start the m_workers that have been assigned work
	startWorkers(theory);
#endif
	
	m_multitree->syncRetract();
	
#ifdef USE_WORKERS
	if(m_selfrollout)
		theory.syncWorkers();
	if(!m_abort)
	{
		// Process m_workers done
		processWorkers(theory);
	}
#endif
	
#ifdef LOG_UCT_TREE
	m_treelog.createNodes(getDepth(theory), theory.getGameStateID(theory.getParentState()));
#endif
}
bool MCMemoryPlayer::getMoves(RoleIndex role, cadiaplayer::play::utils::MTStateID id, cadiaplayer::play::utils::Depth depth, RoleMoves& moves)
{
	return m_multitree->getMoves(role, id, depth, moves);
}
size_t MCMemoryPlayer::selectAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleMoves& moves)
{
	size_t i = rand() % moves.size();
	return i;
}
cadiaplayer::play::Move* MCMemoryPlayer::bestAction(cadiaplayer::play::GameTheory& theory)
{
	double q = 0;
	Move* move = NULL; 
	MTState* state = m_multitree->lookupState(theory.getStateRef(), getDepth(theory));
	if(isSinglePlayer())
	{
		move = maxAction(theory, state, q);
		if(q < GOAL_MAX)
			m_solved = false;
	}
	else
	{
		if(m_totalcontrol && m_simulationCount && !m_solved)
		   move = controlAction(theory, state, q);
		else
		   move = bestAction(theory, state, q);
	}
		   
	if(move == NULL || q < 0)
	{
		move = theory.generateRandomMove(m_roleindex);
		q = 0;
	}
	m_lastBestAction = m_multitree->lookupAction(m_roleindex, state, move);
	m_lastBestQ = q;
	return move;
}
cadiaplayer::play::Move* MCMemoryPlayer::bestAction(cadiaplayer::play::GameTheory& theory, MTState* state, double &reward)
{
	return m_multitree->bestAction(m_roleindex, state, reward);
}
cadiaplayer::play::Move* MCMemoryPlayer::controlAction(cadiaplayer::play::GameTheory& theory, MTState* state, double &reward)
{
	return m_multitree->controlAction(m_roleindex, state, reward);
}
cadiaplayer::play::Move* MCMemoryPlayer::maxAction(cadiaplayer::play::GameTheory& theory, MTState* state, double &reward)
{
	return m_multitree->maxAction(m_roleindex, state, reward);
}

void MCMemoryPlayer::makeAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::PlayMoves& pm, std::vector<double>& rewards)
{
	theory.make(pm);
	rewards.resize(theory.getRoles()->size(), 0.0);
	for(size_t n  = 0 ; n < theory.getRoles()->size() ; ++n)
	{
		rewards[n] = getReward(theory, n);
	}
	return;
}
cadiaplayer::play::utils::MTNode* MCMemoryPlayer::makeAction(cadiaplayer::play::GameTheory& /*theory*/, cadiaplayer::play::utils::MTStateID sid, cadiaplayer::play::utils::Depth depth, cadiaplayer::play::PlayMoves& moves)
{
	return m_multitree->makeAction(sid, depth, moves);
}
void MCMemoryPlayer::retractAction(cadiaplayer::play::GameTheory& theory)
{
	m_multitree->retractAction();
}
MTNode* MCMemoryPlayer::updateValues(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::PlayMoves& pm, std::vector<double>& qValues)
{
	for(size_t n = 0 ;  n < qValues.size(); ++n)
	{
		m_workrole = n;
		qValues[m_workrole] *= gamma();
		updateValue(theory, pm[m_workrole], qValues[m_workrole], pm);
	}
#ifdef LOG_UCT_TREE
	if(m_generateWholePath || m_multitree->peekTrail() )
		m_treelog.bufferTrail(theory.getGameStateID(theory.getStateRef()));
#endif
	return m_multitree->popTrail();
}
void MCMemoryPlayer::updateValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::Move* move, double& q, PlayMoves& pm)
{
	MTNode* node = m_multitree->peekTrail();
	if(node == NULL)
	{
		if(m_generateWholePath)
		{
			Depth d = getDepth(theory);
			MTState* state = m_multitree->setupState(theory.getStateRef(), d, m_simMoves[d]);
			m_multitree->updateValue(m_workrole, state, d, q, pm, m_pathChild);
			m_pathChild = state->id;
		}
		updateMoveValue(theory, move, q);
		return;
	}
	MTAction* a = m_multitree->updateValue(m_workrole, node, q);
	updateNodeValue(theory, node, a, q);
}
void MCMemoryPlayer::updateMoveValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::Move* move, double& q)
{
	return;
}
void MCMemoryPlayer::updateNodeValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTNode* node, cadiaplayer::play::utils::MTAction* action, double& q)
{
	return;
}

void MCMemoryPlayer::processGoals(std::vector<double>& goals)
{
	// Disabled for now
	if(goals.size() != 2)
		return;
	
	double temp = goals[0];
	goals[0] = ((goals[0] - goals[1]) + 100)/2;
	goals[1] = ((goals[1] - temp    ) + 100)/2;
	
	m_goalValueHolder[0] = goals[0];
	m_goalValueHolder[1] = goals[1];
}

void MCMemoryPlayer::postplay(cadiaplayer::play::GameTheory& theory)
{
#ifdef DUMP_TREES
	std::stringstream ss;
	Depth d = getDepth(theory);
	if(!m_postStartClock)
		ss << "multitree_" << (d+1) << ".txt";
	m_multitree->dumpToFile(ss.str(), theory.getStateRef(), d);
#endif
#ifdef LOG_UCT_TREE
	m_treelog.updateNode(theory.getGameStateID(theory.getStateRef()), getDepth(theory), getLastPlayConfidence());
#endif
}

double MCMemoryPlayer::getLastPlayConfidence()
{
	return m_lastBestQ;
}

void MCMemoryPlayer::parallelInfo(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::MoveRatings& ratings)
{
	MTState* state = m_multitree->lookupState(theory.getStateRef(), getDepth(theory));
	if(isSinglePlayer())
		maxActionRatings(getRoleIndex(), state, ratings);
	else
	{
		if(m_totalcontrol && m_simulationCount)
			controlActionRatings(getRoleIndex(), state, ratings);
		else
			bestActionRatings(getRoleIndex(), state, ratings);
	}
}

void MCMemoryPlayer::bestActionRatings(RoleIndex role, MTState* state, cadiaplayer::play::MoveRatings& ratings)
{
	m_multitree->bestActionRatings(role, state, ratings);
}
void MCMemoryPlayer::controlActionRatings(RoleIndex role, MTState* state,cadiaplayer::play::MoveRatings& ratings)
{
	m_multitree->controlActionRatings(role, state, ratings);
}
void MCMemoryPlayer::maxActionRatings(RoleIndex role, cadiaplayer::play::utils::MTState* state, cadiaplayer::play::MoveRatings& ratings)
{
	m_multitree->maxActionRatings(role, state, ratings);
}

std::string MCMemoryPlayer::getLastPlayInfo(cadiaplayer::play::GameTheory& /*theory*/) 
{
	std::ostringstream s;
	s << "Node expansions : " << m_nodeCounter << std::endl;
#ifdef USE_WORKERS
	s << "Worker Simulations" << std::endl;
	s << "-> Assigned  : "    << m_workerAssignedCounter << std::endl;
	s << "-> Started   : "    << m_workerStartedCounter << std::endl;
	s << "-> Completed : "    << m_workerDoneCounter << std::endl;
#endif
	s << "Simulations Overview"        << std::endl;
	s << "-> Model simulations     : " << m_memoryplay << std::endl;
	s << "-> Self random partials  : " << m_selfrandomplay << std::endl;
	s << "-> Total selfsimulations : " << m_memoryplay+m_selfrandomplay << std::endl;
	s << "-> Avg simulation length : " << m_averageSimulationLength << std::endl;
	s << "-> Avg simulation count  : " << m_averageSimulations << std::endl;
	s << "-> Avg node expasions    : " << m_averageNodeExpansions << std::endl;
#ifdef USE_WORKERS
	s << "-> Worker simulations    : " << m_workerRequestDoneCounter << std::endl;
	s << "-> Total simulations     : " << m_memoryplay+m_selfrandomplay+m_workerRequestDoneCounter << std::endl;
#else
	s << "-> Total simulations     : " << m_memoryplay+m_selfrandomplay << std::endl;
#endif
	s << "-> Aborted simulations   : " << m_aborted << std::endl;
	s << "-> Border crossed        : " << m_borderCrossings << std::endl;
	s << "-> Game counter          : " << m_gameCounter << std::endl;
	s << "-> ASync counter         : " << m_asyncCounter << std::endl;
	s << "Actions available :\n"	   <<  m_multitree->getBestActionInfo() << std::endl;
	s << "Memory info :" << std::endl;
	s << m_multitree->getMemoryInfo() << std::endl;
	return s.str();
}

#ifdef USE_WORKERS
/*************************************************************/
/******************* Worker Functions ************************/
/*************************************************************/

void MCMemoryPlayer::initWorkerSlaves(GameTheory& theory)
{
	// set the slave count at ratio 2/1
	int slaveCount = theory.initWorkers() * 2;
	while(!m_readyWorkers.empty())
	{m_readyWorkers.pop();}
	while(!m_idleWorkers.empty())
	{m_idleWorkers.pop();}
	for(int n = 0 ; n < slaveCount ; ++n)
	{
		m_idleWorkers.push(n+1);
	}
	// make room for all m_worker m_trails and a "do-it-yourself" NULL at the front;
	clearWork();
	m_workerTrails.clear();
	m_workerTrails.resize(slaveCount+1, NULL);
	theory.checkWorkers();
}
bool MCMemoryPlayer::assignWorkerSlave()
{
	// If no m_worker idle send back a "do-it-yourself" false
	// std::cerr << "Requesting m_worker form idle pool of " << m_idleWorkers.size() << " workers" << std::endl;
	if(m_idleWorkers.size() == 0)
	{
		return false;
	}
	// Get the id of next idle m_worker
	cadiaplayer::utils::WorkerId id = m_idleWorkers.front();
	m_idleWorkers.pop();
	// Transfer current m_trails to the m_workers update slot
	m_workerTrails[id] = m_multitree->hijackTrail();
	// Mark this m_worker as ready to do some work
	m_readyWorkers.push(id);
	m_workerAssignedCounter+=WORKER_SIMULATIONS_COUNT;
	return true;
}
void MCMemoryPlayer::startWorkers(GameTheory& theory)
{
	cadiaplayer::utils::WorkerId id = 0;
	while(!m_readyWorkers.empty())
	{
		id = m_readyWorkers.front();
		theory.startWorker(id, m_roleindex, WORKER_SIMULATIONS_COUNT, WORKER_THINK_MSEC_MAX, gamma());
		m_readyWorkers.pop();
		m_workerStartedCounter+=WORKER_SIMULATIONS_COUNT;
	}
}
void MCMemoryPlayer::processWorkers(GameTheory& theory)
{
#ifdef DEBUG_WORKERS
	std::cerr << cadiaplayer::utils::Timer::getTimeStamp() << " Begin processWorkers(GameTheory& theory) " << std::endl;
#endif
	int plys;
	int sims;
	std::vector<double> goals;
	MTNode* currentNode = NULL;
	cadiaplayer::utils::WorkerId id = theory.getWorkDone(plys, sims, goals);
	while(id != 0)
	{		
		// Very late m_worker, his update data was out of sync and was destroyed 
		if(m_workerTrails[id] == NULL)
		{
			m_idleWorkers.push(id);
			id = theory.getWorkDone(plys, sims, goals);
			goals.clear();
			//std::cerr << "Worker not used" << std::endl; 
			continue;
		}
		if(m_workerTrails[id]->empty())
		{
			m_idleWorkers.push(id);
			id = theory.getWorkDone(plys, sims, goals);
			goals.clear();
			delete m_workerTrails[id];
			m_workerTrails[id] = NULL;
			//std::cerr << "Worker not used" << std::endl; 
			continue;
		}
		// Manipulate goals if appropriate
		if(isSinglePlayer())
		{
		   processGoals(goals);
		   for(size_t n = 0 ; n < goals.size() ; ++n)
		   {
				if(getPlayOutMaxValue() <= goals[n])
					setPlayOutMaxValue(goals[n]);
		   }
		}
		while(!m_workerTrails[id]->empty())
		{
			// Get a pointer to the node to update
			currentNode = m_workerTrails[id]->back();
			// Pop the node off the trail
			m_workerTrails[id]->pop_back();
			// Update
			for(size_t n = 0 ; n < goals.size() ; ++n)
			{
				if(currentNode != NULL)
				{
					MTAction* a = m_multitree->updateValue(n, currentNode, goals[n]);
					m_workrole = n;
					updateNodeValue(theory, currentNode, a, goals[n]);
				}
				else
					std::cerr << "Worker tried to update NULL node" << std::endl; 
				// Discount
				goals[n] *= gamma();
			}
		}
		delete m_workerTrails[id];
		m_workerTrails[id] = NULL;
		goals.clear();
		// Mark this m_worker as idle
		m_idleWorkers.push(id);
		//std::cerr << "Worker used" << std::endl; 
		m_workerDoneCounter+=sims;//WORKER_SIMULATIONS_COUNT;
		m_workerRequestDoneCounter++;
		id = theory.getWorkDone(plys, sims, goals);
	}
#ifdef DEBUG_WORKERS
	std::cerr << cadiaplayer::utils::Timer::getTimeStamp() << " End processWorkers(GameTheory& theory) " << std::endl;
#endif
}

void MCMemoryPlayer::clearWork()
{
	for(size_t n = 0 ; n < m_workerTrails.size() ; ++n)
	{
		if(!m_workerTrails[n])
			continue;
		if(m_workerTrails[n]->empty())
		{
			delete m_workerTrails[n];
			m_workerTrails[n] = NULL;
			continue;
		}
		m_workerTrails[n]->clear();
		delete m_workerTrails[n];
		m_workerTrails[n] = NULL;
	}
	cadiaplayer::utils::WorkerId id = 0;
	while(!m_readyWorkers.empty())
	{
		id = m_readyWorkers.front();
		m_idleWorkers.push(id);
		m_readyWorkers.pop();
	}
}
#endif
