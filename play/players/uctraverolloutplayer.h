/*
 *  uctraverolloutplayer.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 9/23/12.
 *  Copyright 2012 Reykjavik University. All rights reserved.
 *
 */

/*
 *  uctraveplayer.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 2/26/09.
 *  Copyright 2009 Reykjavik University. All rights reserved.
 *
 */

#ifndef UCTRAVEROLLOUTPLAYER_H
#define UCTRAVEROLLOUTPLAYER_H

#include "uctraveplayer.h"
#include "../utils/movetrails.h"

namespace cadiaplayer {
	namespace play {
		namespace players {
			const double		DEFAULT_RAVE_GIBBS_TEMPERATURE		= 10.0;
			
			class UCTRAVERolloutPlayer : public UCTRAVEPlayer
			{
			public:
				double					m_temperature;
				std::vector<double>		m_softmax;
				double					m_divider;
				
				virtual bool adjustArrays(std::size_t branch);
			public:
				// Construction and Destruction.
				UCTRAVERolloutPlayer() : UCTRAVEPlayer(), m_temperature(DEFAULT_RAVE_GIBBS_TEMPERATURE){};
				virtual ~UCTRAVERolloutPlayer(){};
				
				// Override of info functions.
				virtual std::string getPlayerName() {return "Template for uct player with RAVE selecting from Gibbs distribution during rollout";};
				virtual std::string getPlayerShortName() {return "RAVEROLL";};
				
				// Getters and Setters
				inline double getTemperature(){return m_temperature;};
				inline void setTemperature(double temperature){m_temperature = temperature;};
				
				// UCTRolloutPlayer
				// Getters and Setters
				// Override for move selection for 2+ player games (adversary and multiplayer).
				virtual std::size_t selectAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleMoves& moves);
				// Override for move selection for single player games (puzzles).
				virtual std::size_t selectSingleAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleMoves& moves);
				// The return values will be selected from with the softmax policy. 
				virtual double		playoutMoveValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTState* state, cadiaplayer::play::utils::MTAction* node, cadiaplayer::play::Move* move);
				virtual std::size_t	selectFromDistribution(cadiaplayer::play::GameTheory& theory, std::size_t count, std::vector<double>& values, double& sum);
				
			protected:
				inline std::size_t selectGibbsUnexplored(){return m_foundexp==1?m_randexp[0]:m_randexp[selectGibbAction(m_foundexp, m_softmax, m_divider)];};

				// UCTRAVEPlayer - by inheritance
			/*public:
				virtual void newSearch(cadiaplayer::play::GameTheory& theory);
				virtual double selectionNodeValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTState* state, cadiaplayer::play::utils::MTAction* node);
				virtual cadiaplayer::play::Move* bestAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTState* state, double &reward);
				virtual cadiaplayer::play::Move* controlAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTState* state, double &reward);
				virtual void bestActionRatings(RoleIndex role, cadiaplayer::play::utils::MTState* state,cadiaplayer::play::MoveRatings& ratings);
				virtual void controlActionRatings(RoleIndex role, cadiaplayer::play::utils::MTState* state,cadiaplayer::play::MoveRatings& ratings);
				virtual void updateMoveValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::Move* move, double& q);
				virtual void updateNodeValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTNode* node, cadiaplayer::play::utils::MTAction* action, double& q);
				 */
			};
		}}} // namespaces
#endif //UCTRAVEROLLOUTPLAYER_H
