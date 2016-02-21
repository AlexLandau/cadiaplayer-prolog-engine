#ifndef GAMEPLAYER_H
#define GAMEPLAYER_H

#include <ctime>
#include <string>
#include <iostream>
#include <limits>

#include "../gametheory.h"
#include "../../utils/timer.h"

namespace cadiaplayer {
	namespace play {
		namespace players {
			
//			const double NEG_INF = static_cast<double>(LONG_MIN);
//			const double POS_INF = static_cast<double>(LONG_MAX);
			const double POS_INF = std::numeric_limits<double>::max();
			const double NEG_INF = -POS_INF;
			typedef double Cost;
			
			class GamePlayer
				{
				protected:
					double							m_thinktime;
					cadiaplayer::utils::Timer		m_timer;
					cadiaplayer::play::Role			m_role;
					cadiaplayer::play::RoleIndex	m_roleindex;
					int64_t							m_gameCounter;
					int64_t							m_nodeCounter;
					int64_t							m_asyncCounter;
					bool							m_solved;
				public:
					GamePlayer() : m_thinktime(0), m_role("None"), m_roleindex(0), m_gameCounter(0), m_nodeCounter(0), m_asyncCounter(0), m_solved(false){};
					virtual ~GamePlayer(){};
					
					virtual void newGame(cadiaplayer::play::GameTheory& /*theory*/){};
					
					virtual cadiaplayer::play::Move* play(cadiaplayer::play::GameTheory& /*theory*/) {return NULL;};
					
					virtual void prepare(cadiaplayer::play::GameTheory& /*theory*/) {};
					
					virtual void postplay(cadiaplayer::play::GameTheory& /*theory*/) {};
					
					virtual void asyncplay(cadiaplayer::play::GameTheory& /*theory*/) {usleep(100);};
					
					virtual void stop(cadiaplayer::play::GameTheory& /*theory*/) {};
					
					virtual double getLastPlayConfidence() {return 0.0;};
					
					virtual std::string getSetupInfo() {return "No setup info available.";};
					
					virtual std::string getLastPlayInfo(cadiaplayer::play::GameTheory& /*theory*/) {return "No play info available.";};
					
					virtual std::string getPlayerName() {return "John Doe";};
					
					virtual std::string getPlayerShortName() {return "NN";};
					
					virtual bool isSolved(){return m_solved;};
					
					virtual double minReachableGoal(cadiaplayer::play::GameTheory& /*theory*/){return 0.0;};
					
					virtual void parallelInfo(cadiaplayer::play::GameTheory& /*theory*/, cadiaplayer::play::MoveRatings& /*ratings*/){return;};
					
					virtual void setThinkTime(double seconds) {m_thinktime = seconds;};
					
					virtual double getThinkTime() {return m_thinktime;};
					
					void startTimer() 
					{
						m_timer.startTimer();
					};
					
					bool hasTime()
					{
						return m_timer.hasTime(m_thinktime);
					};	
					
					void setRole(cadiaplayer::play::RoleIndex ri, cadiaplayer::play::Role r) {m_roleindex = ri;m_role = r;};
					
					cadiaplayer::play::Role getRole(void){return m_role;};
					cadiaplayer::play::RoleIndex getRoleIndex(void){return m_roleindex;};
					
					virtual void writeMemoryInfo(cadiaplayer::play::GameTheory& theory, std::string file){};
				private:
				};
			
			typedef std::vector<GamePlayer*> GamePlayers; 
			
		}}} // namespaces
#endif // GAMEPLAYER_H
