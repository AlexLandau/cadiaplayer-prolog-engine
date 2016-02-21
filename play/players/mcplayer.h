/*
 *  mcplayer.h
 *  src
 *
 *  Created by Hilmar Finnsson on 10/10/07.
 *  Copyright 2007 CADIA Reykjavík University. All rights reserved.
 *
 */

#ifndef MCPLAYER_H
#define MCPLAYER_H

#include <sstream>
#include "gameplayer.h"
#include "../utils/gamestatespace.h"

#define SETUP_INFO_MESSAGE_SIMULATIONS_LIMITED		"Simulations limited"
#define SETUP_INFO_MESSAGE_SIMULATIONS_UNLIMITED	"Simulations not limited"
#define SETUP_INFO_MESSAGE_EXPANSIONS_LIMITED		"Expansions limited"
#define SETUP_INFO_MESSAGE_EXPANSIONS_UNLIMITED		"Expansions not limited"
#define SETUP_INFO_MESSAGE_DELIMITER				", ";

namespace cadiaplayer {
	namespace play {
		namespace players {			
			
			const double DEFAULT_GAMMA = 0.999;
			typedef	cadiaplayer::play::utils::QNode QNode;
			
			class MCPlayer : public GamePlayer
				{
				protected:
					size_t		m_curdepth;
					bool		m_singleplayer;
					double		m_playOutMaxvalue;
					double		m_gammaValue;
					cadiaplayer::play::utils::GameStateSpace* m_statespace;
					unsigned int m_limitSimulations;
					unsigned int m_limitNodeExpansions;
				protected:
					RoleIndex		m_roleCount;
					bool			m_generateWholePath;
					QNode*			m_lastBestActionNode;
					double			m_lastBestQ;
					bool			m_postStartClock;
					int64_t		m_averageSimulationLength;
					int64_t		m_simulationCount;
					double			m_averageIncrementalDivider;
					int64_t		m_averageNodeExpansions;
					int64_t		m_averageSimulations;
				public:
					MCPlayer() : GamePlayer(), m_postStartClock(true), m_statespace(NULL), m_limitSimulations(0), m_limitNodeExpansions(0), m_averageSimulationLength(100), m_simulationCount(0){};
					virtual ~MCPlayer() {if(m_statespace != NULL) delete m_statespace;};
					virtual void newGame(cadiaplayer::play::GameTheory& theory);
					void determineSinglePlayer(cadiaplayer::play::GameTheory& theory){m_singleplayer = theory.getRoles()->size() == 1;};
					bool isSinglePlayer(){return m_singleplayer;};
					virtual cadiaplayer::play::Move* play(cadiaplayer::play::GameTheory& theory);
					virtual void asyncplay(cadiaplayer::play::GameTheory& /*theory*/);
					virtual void prepare(cadiaplayer::play::GameTheory& theory);
					virtual std::string getPlayerName() {return "MC player";};
					virtual std::string getPlayerShortName() {return "MC";};
					virtual cadiaplayer::play::Move* doMCPlanning(cadiaplayer::play::GameTheory& /*theory*/);
					virtual void newSearch(cadiaplayer::play::GameTheory& theory);
					virtual void doSearch(cadiaplayer::play::GameTheory& theory);
					virtual double search(cadiaplayer::play::GameTheory& theory);
					virtual void endSearch(cadiaplayer::play::GameTheory& /*theory*/){};
					virtual double gamma(void);
					virtual void setGamma(double g){m_gammaValue = g;};
					double getPlayOutMaxValue(){return m_playOutMaxvalue;};
					void setPlayOutMaxValue(double p){m_playOutMaxvalue = p;};
					virtual std::size_t selectAction(cadiaplayer::play::GameTheory& theory,cadiaplayer::play::RoleMoves& moves);
					virtual cadiaplayer::play::Move* bestAction(cadiaplayer::play::GameTheory& theory);
					virtual double makeAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::Move* move);
					virtual void retractAction(cadiaplayer::play::GameTheory& theory);
					virtual double getReward(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleIndex r);
					virtual void updateValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::Move* move, double& q);
					virtual double minReachableGoal(cadiaplayer::play::GameTheory& theory);
					virtual unsigned int getDepth(cadiaplayer::play::GameTheory& theory){return theory.getRound()-1;};
					void limitSimulations(unsigned int limit){m_limitSimulations=limit;};
					void limitlessSimulations(){m_limitSimulations=0;};
					bool simulationsLimited(){return m_limitSimulations;};
					bool reachedSimulationsLimit(){return m_limitSimulations <= m_gameCounter;};
					unsigned int getSimulationsLimit(){return m_limitSimulations;};
					void limitNodeExpansions(unsigned int limit){m_limitNodeExpansions=limit;};
					void limitlessNodeExpansions(){m_limitNodeExpansions=0;};
					bool nodeExpansionsLimited(){return m_limitNodeExpansions;};
					bool reachedNodeExpansionsLimit(){return m_limitNodeExpansions <= m_nodeCounter;};
					unsigned int getNodeExpansionsLimit(){return m_limitNodeExpansions;};

					virtual std::string getSetupInfo();
					virtual std::string getLastPlayInfo(cadiaplayer::play::GameTheory& /*theory*/);
					virtual double getLastPlayConfidence(){return m_lastBestQ;};
					
					// Helper function for all inheriting players 
					std::size_t selectGibbAction(std::size_t count, std::vector<double>& softmax, double divider);
				};
		}}} // namespaces
#endif //MCPLAYER_H
