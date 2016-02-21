/*
 *  agent.h
 *  src
 *
 *  Created by Hilmar Finnsson on 5/29/08.
 *  Copyright 2008 Reykjavik University. All rights reserved.
 *
 */

#ifndef AGENT_H
#define AGENT_H

#include "../config.h"

#include "../utils/worker.h"

#include <string>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>

#include "gmprotocol.h"
#include "../utils/tcpserver.h"
#include "../utils/timer.h"

#include "../play/parsing/gameparser.h"
#include "../play/players/gameplayer.h"

#ifdef USE_ECLIPSE
#include "../logic/eclipsecontroller.h"
#else
#include "../logic/prologcontroller.h"
#endif


//#define USE_ASYNC

namespace cadiaplayer {
	namespace agent {

		const int START_ERROR = -1;
		const int FIRST_PLAY_ERROR = -2;
		const int PLAY_ERROR = -3;
		const int GDL_PARSE_ERROR = -4;
		
		//const double START_DELAY = 0;
		//const double PLAY_DELAY = 0;
		const double START_DELAY = 0.50;
		const double PLAY_DELAY = 0.25;
		//const double START_DELAY = 1;
		//const double PLAY_DELAY = 1;
		const double EXTERNAL_START_DELAY = 1;
		const double EXTERNAL_PLAY_DELAY = 1;
	
		typedef enum {
			Parallel,
			Confidence
		} ExternalsType;
		
		struct MoveLogItem
		{
			std::string move;
			double confidence;
			MoveLogItem(std::string m, double c):move(m),confidence(c){};
		};
		typedef std::vector<MoveLogItem> MoveLog;
		
		class Agent
			{
			private:
				cadiaplayer::utils::Timer					m_startTimer;
				cadiaplayer::play::GameTheory*				m_theory;
				cadiaplayer::play::parsing::GameParser*		m_parser;
				cadiaplayer::logic::GameController*			m_controller;
				bool										m_localController;
				std::string									m_srcdir;
				std::string									m_workdir;
				std::string									m_filename;
				std::string									m_id;
				cadiaplayer::play::players::GamePlayer*		m_player;
				bool										m_isExternal;
				bool										m_ignoreExternals;
				bool										m_playing;
				cadiaplayer::play::Move*					m_move;
				cadiaplayer::play::parsing::CompoundList	m_moves;
				cadiaplayer::play::PlayMoves				m_pm;
				MoveLog										m_moveLog;
				int											m_error;
				bool										m_killme;
				ExternalsType								m_exttype;
				
				bool			m_restoremode;
				std::string		m_restorefile;
				std::string		m_logFile;
				std::string		m_pgnFile;
			public:
				Agent(cadiaplayer::play::players::GamePlayer* player, bool external = false, cadiaplayer::logic::GameController* controller = NULL);
				virtual ~Agent();
				void log(std::string str);
				void log(std::stringstream& str){log(str.str());str.str("");};
				void setLogFile(std::string strLog){m_logFile = strLog;};
				void setPGNFile(std::string strPGN){m_pgnFile = strPGN;};

				void generatePGN(std::string filename ="");	
				
				// Run the agent, the player is only needed if a crashed game should be restored 
				int run(cadiaplayer::play::players::GamePlayer* restorePlayer = NULL);
				
				// External players section begins 
			private:
				cadiaplayer::utils::Workers m_externalPlayers;
				std::string m_externalPlayersFile;
				bool m_startmessageHandled;
				int  m_playerCount;
				
				void startExternals(GMMessage msg);
				void playExternals(GMMessage msg);
				void stopExternals(GMMessage msg);
				void checkExternals();
				void restartExternals();
				std::string compareWithExternals(std::string currentMove);
				std::string consolidateExternals(std::string currentMove);
			public:
				void setExternalsType(ExternalsType type){m_exttype = type;};
				void setExternalsFile(std::string str){m_externalPlayersFile = str;};

				// Logging and restoring
			private:		
				void newRestorePoint(GMMessage& msg);
				void addRestoreAction(GMMessage& msg);
				bool load();
				void clean();
				void init();
				bool setupGame(GMMessage& msg);
				bool configureController();
				void setPlayClock(double playtime);
				bool start(GMMessage msg);
				void logPlay(std::string m);
				void play();
				void asyncplay();
				void conf();
				void mcparallel();
				bool load(GMMessage msg);
				bool loadFile(GMMessage msg);
				void state();
				void avail();
				void term();
				void goal();
				void info(GMMessage msg);
				void randomplay();
				void logMove();
				bool make(GMMessage& msg);
				void move(GMMessage& msg);
				void undo();
				void stop();
				void ping();
				void abort();
				int playerCount(){return m_playerCount;};
				void getMessage(GMMessage& msg);
				void logMessage(GMMessage& msg);
			public:
				void restore(cadiaplayer::play::players::GamePlayer* player);
				
			};
	}} // namespaces
#endif // AGENT_H
