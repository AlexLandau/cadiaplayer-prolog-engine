/*
 *  mcmemoryplayer.h
 *  src
 *
 *  Created by Hilmar Finnsson on 10/10/07.
 *  Copyright 2007 CADIA Reykjavík University. All rights reserved.
 *
 */

#ifndef MCMEMORYPLAYER_H
#define MCMEMORYPLAYER_H

//#define LOG_UCT_TREE
//#define DUMP_TREES

#define MCMEM_INIT_MOVES_HOLDER_DEPTH 200
#define MCMEM_EXPAND_MOVES_HOLDER_DEPTH 50

#include <list>
#include "mcplayer.h"
#include "../utils/multitree.h"

#ifdef LOG_UCT_TREE
#include "../../utils/treelog.h"
#endif

namespace cadiaplayer {
	namespace play {
		namespace players {
			
			typedef std::vector<cadiaplayer::play::utils::GameStateSpace*> MultiStateSpace;
			
			class MCMemoryPlayer : public MCPlayer
				{
				protected:
					bool m_abort;
					bool m_selfrollout;
					bool m_solved;
					cadiaplayer::play::utils::MTState* m_borderState;
					cadiaplayer::play::GameStateID m_pathChild;
					bool m_partialcontrol;
					bool m_totalcontrol;
					AllRoleMovesList m_simMoves;
					bool exceedsMoveHolder(cadiaplayer::play::utils::Depth d);
					void expandMovesHolder(cadiaplayer::play::utils::Depth d);
#ifdef LOG_UCT_TREE
					cadiaplayer::utils::TreeLog m_treelog;
#endif
				public:
					// Id of the role being searched for
					cadiaplayer::play::RoleIndex m_workrole;
					cadiaplayer::play::utils::MultiTree* m_multitree;
					unsigned int m_selfrandomplay;
					unsigned int m_memoryplay;
					unsigned int m_selfrandomsteps;
					unsigned int m_memorysteps;
					unsigned int m_aborted;
					unsigned int m_borderCrossings;
					std::vector<double> m_qValueHolder;
					std::vector<double> m_goalValueHolder;
					cadiaplayer::play::utils::MTAction* m_lastBestAction;
					bool m_memoryreset;
					unsigned int m_startingDepth;
					
					MCMemoryPlayer() : MCPlayer(), m_multitree(NULL), m_selfrandomplay(0), m_memoryplay(0), m_lastBestAction(NULL), m_borderState(NULL), m_pathChild(0),m_memoryreset(false) {};
					virtual~MCMemoryPlayer();
					virtual std::string getPlayerName() {return "MC Memory player";};	
					virtual std::string getPlayerShortName() {return "MCMem";};
					virtual void newGame(cadiaplayer::play::GameTheory& theory);
					virtual cadiaplayer::play::Move* play(cadiaplayer::play::GameTheory& theory);
					virtual void asyncplay(cadiaplayer::play::GameTheory& /*theory*/);
					virtual void stop(cadiaplayer::play::GameTheory& theory);
					virtual cadiaplayer::play::Move* doMCPlanning(cadiaplayer::play::GameTheory& theory);
					virtual void resetCounters();
					virtual void newSearch(cadiaplayer::play::GameTheory& theory);
					virtual double search(cadiaplayer::play::GameTheory& theory);
					virtual void search(cadiaplayer::play::GameTheory& theory, std::vector<double>& qValues);
					virtual void calcTerminal(cadiaplayer::play::GameTheory& theory, std::vector<double>& qValues);
					virtual void selfRollout(cadiaplayer::play::GameTheory& theory, std::vector<double>& qValues);
					virtual void calcSelfTerminal(cadiaplayer::play::GameTheory& theory, std::vector<double>& qValues);
					virtual bool cutoff(cadiaplayer::play::GameTheory& /*theory*/, cadiaplayer::play::utils::Depth /*depth*/){return false;};
					virtual void atTerminal(cadiaplayer::play::GameTheory& /*theory*/, std::vector<double>& /*qValues*/){};
					virtual void endSearch(cadiaplayer::play::GameTheory& theory);
					virtual bool getMoves(RoleIndex role, cadiaplayer::play::utils::MTStateID id, cadiaplayer::play::utils::Depth depth, RoleMoves& moves);
					virtual size_t selectAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleMoves& moves);
					virtual size_t selectSingleAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleMoves& moves){return selectAction(theory, moves);};
					virtual cadiaplayer::play::Move* bestAction(cadiaplayer::play::GameTheory& theory);
					virtual cadiaplayer::play::Move* bestAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTState* state, double &reward);
					virtual cadiaplayer::play::Move* controlAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTState* state, double &reward);
					virtual cadiaplayer::play::Move* maxAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTState* state, double &reward);
					virtual void makeAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::PlayMoves& pm, std::vector<double>& rewards);
					virtual cadiaplayer::play::utils::MTNode* makeAction(cadiaplayer::play::GameTheory& /*theory*/, cadiaplayer::play::utils::MTStateID sid, cadiaplayer::play::utils::Depth depth, cadiaplayer::play::PlayMoves& moves);
					virtual void retractAction(cadiaplayer::play::GameTheory& theory);
					virtual cadiaplayer::play::utils::MTNode* updateValues(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::PlayMoves& pm, std::vector<double>& qValues);
					virtual void updateValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::Move* move, double& q, PlayMoves& pm);
					virtual void updateMoveValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::Move* move, double& q);
					virtual void updateNodeValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTNode* node, cadiaplayer::play::utils::MTAction* action, double& q);
					virtual void processGoals(std::vector<double>& goals);
					virtual unsigned int getDepth(cadiaplayer::play::GameTheory& theory){return theory.getRound()-1;};
					virtual std::string getLastPlayInfo(cadiaplayer::play::GameTheory& /*theory*/);
					virtual double getLastPlayConfidence();
					virtual void parallelInfo(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::MoveRatings& ratings);
					virtual void bestActionRatings(cadiaplayer::play::RoleIndex role, cadiaplayer::play::utils::MTState* state, cadiaplayer::play::MoveRatings& ratings);
					virtual void controlActionRatings(cadiaplayer::play::RoleIndex role, cadiaplayer::play::utils::MTState* state, cadiaplayer::play::MoveRatings& ratings);
					virtual void maxActionRatings(cadiaplayer::play::RoleIndex role, cadiaplayer::play::utils::MTState* state, cadiaplayer::play::MoveRatings& ratings);
					virtual void postplay(cadiaplayer::play::GameTheory& theory);
					virtual void writeMemoryInfo(cadiaplayer::play::GameTheory& theory, std::string file)
					{
						m_multitree->writeTreeToFile(file, theory.getStateRef());
					};

#ifdef USE_WORKERS
				protected:
					/*********** Master - Slave (parallelzation) ***************/
					// Stores trails that slave workers are working on
					typedef std::vector<cadiaplayer::play::utils::MTTrail*> WorkerTrails;
					static const int WORKER_SIMULATIONS_COUNT	=	3;
					static const int WORKER_THINK_MSEC_MAX		=	50;
					
					/*********** Master - Slave (parallelzation) ***************/
					// Stores id of idle slaves
					std::queue<cadiaplayer::utils::WorkerId> m_idleWorkers;
					// Stores id of slaves that have work assigned
					std::queue<cadiaplayer::utils::WorkerId> m_readyWorkers;
					// Holder for simulation assigned per play to workers
					unsigned int m_workerAssignedCounter;
					// Holder for simulation started per play by workers
					unsigned int m_workerStartedCounter;
					// Holder for simulation count per play made by workers
					unsigned int m_workerDoneCounter;
					// Holder for request completed count per play made by workers
					unsigned int m_workerRequestDoneCounter;
					// Indicator to abort recursion if worker has taken over
					bool m_assigned;
					// Stores trails that slave workers are working on
					WorkerTrails m_workerTrails;
					
					// Initialize slave work
					void initWorkerSlaves(GameTheory& theory);
					// Assign work to slave, returns true if one takes on the work
					bool assignWorkerSlave();
					// Start slaves
					void startWorkers(GameTheory& theory);
					// Process work done from slaves
					void processWorkers(GameTheory& theory);
					// Clear work to do (for game advancement)
					void clearWork();
#endif
				};
		}}} // namespaces
#endif //MCMEMORYPLAYER_H
