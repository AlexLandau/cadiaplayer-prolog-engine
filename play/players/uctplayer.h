#ifndef UCTPLAYER_H
#define UCTPLAYER_H

#include "selectionplayer.h"

#define UAU_ACTIVE_LOG_MESSAGE "Unexplored Action Urgency enabled"
#define UAU_INACTIVE_LOG_MESSAGE "Unexplored Action Urgency disabled"

namespace cadiaplayer {
	namespace play {
		namespace players {
			
			const double DEFAULT_C = 40.0;
			
			class UCTPlayer : public SelectionPlayer
				{
				protected:
					std::vector<double> m_cValues;
					double m_c;
					
					std::size_t			m_action;
					double				m_val;
					double				m_curVal;
					std::vector<size_t> m_randmax;
					size_t				m_foundmax;
					std::vector<std::size_t>	m_randexp;
					std::size_t					m_foundexp;
					bool				m_expanded;
			
					bool m_useUAU;
					double m_scoreUAU;
					
					virtual bool adjustArrays(std::size_t branch);
					inline std::size_t tiebreakExploited(){return m_foundmax < 2 ? m_randmax[0] : m_randmax[rand() % m_foundmax];};
					inline std::size_t selectRandomUnexplored(){return m_foundexp < 2 ? m_randexp[0] : m_randexp[rand() % m_foundexp];};
				public:
					UCTPlayer();
					virtual ~UCTPlayer();
					virtual void newGame(cadiaplayer::play::GameTheory& theory);
					virtual void setCValues(double c);
					virtual std::string getPlayerName() {return "UCT player";};
					virtual std::string getPlayerShortName() {return "UCT";};
					
					// UCT formula
					virtual double uct(cadiaplayer::play::utils::MTAction* node);
					virtual void tuneCValues(double maxAvailable);
					
					// UAU Getter and Setter
					void enableUnexploredActionUrgency(double score = GOAL_DRAW);
					void disableUnexploredActionUrgency();
					inline bool isUnexploredActionUrgencyEnabled(){return m_useUAU;};
					double  getUnexploredActionUrgencyValue(){return m_scoreUAU;};
					
					// Overrides
					virtual size_t selectAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleMoves& moves);
					virtual size_t selectSingleAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleMoves& moves);
					virtual double selectionNodeValue(cadiaplayer::play::GameTheory& /*theory*/, cadiaplayer::play::utils::MTState* state, cadiaplayer::play::utils::MTAction* node);
					virtual double unexploredNodeValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTState* state, cadiaplayer::play::RoleMoves& moves, std::size_t unexplored);
					virtual std::string getSetupInfo();
					virtual std::string getLastPlayInfo(cadiaplayer::play::GameTheory& theory);
				};
		}}} // namespaces
#endif // UCTPLAYER_H
