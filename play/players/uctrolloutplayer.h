/*
 *  uctrolloutplayer.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 9/23/12.
 *  Copyright 2012 Reykjavik University. All rights reserved.
 *
 */

/*
 *  mastplayer.h
 *  src
 *
 *  Created by Hilmar Finnsson on 10/10/07.
 *  Copyright 2007 CADIA Reykjavík University All rights reserved.
 *
 */

#ifndef UCTROLLOUTPLAYER_H
#define UCTROLLOUTPLAYER_H

#include <cstdlib>
#include "uctplayer.h" 
namespace cadiaplayer {
	namespace play {
		namespace players {
			const double		DEFAULT_GIBBS_TEMPERATURE		= 10.0;
			
			class UCTRolloutPlayer : public UCTPlayer
			{
			public:
				double					m_temperature;
				std::vector<double>		m_softmax;
				double					m_divider;
				
				virtual bool adjustArrays(std::size_t branch);
			public:
				// Construction and Destruction.
				UCTRolloutPlayer() : UCTPlayer(), m_temperature(DEFAULT_GIBBS_TEMPERATURE){};
				virtual ~UCTRolloutPlayer(){};
				
				// Override of info functions.
				virtual std::string getPlayerName() {return "Template for uct player selecting from Gibbs distribution during rollout";};
				virtual std::string getPlayerShortName() {return "ROLL";};
				
				// Getters and Setters
				inline double getTemperature(){return m_temperature;};
				inline void setTemperature(double temperature){m_temperature = temperature;};
				
				// Override for move selection for 2+ player games (adversary and multiplayer).
				virtual std::size_t selectAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleMoves& moves);
				// Override for move selection for single player games (puzzles).
				virtual std::size_t selectSingleAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleMoves& moves);
				
				// The return values will be selected from with the softmax policy. 
				virtual double		playoutMoveValue(cadiaplayer::play::GameTheory& /*theory*/, cadiaplayer::play::utils::MTState* state, cadiaplayer::play::utils::MTAction* node, cadiaplayer::play::Move* move);
				virtual std::size_t	selectFromDistribution(cadiaplayer::play::GameTheory& theory, std::size_t count, std::vector<double>& values, double& sum);
				
			protected:
				inline std::size_t selectGibbsUnexplored(){return m_foundexp==1?m_randexp[0]:m_randexp[selectGibbAction(m_foundexp, m_softmax, m_divider)];};
				
			};
		}}} // namespaces
#endif //UCTROLLOUTPLAYER_H
