/*
 *  mastplayer.h
 *  src
 *
 *  Created by Hilmar Finnsson on 10/10/07.
 *  Copyright 2007 CADIA Reykjavík University All rights reserved.
 *
 */

#ifndef MASTPLAYER_H
#define MASTPLAYER_H

#include <cstdlib>
#include "uctrolloutplayer.h" 
namespace cadiaplayer {
	namespace play {
		namespace players {
			class MASTPlayer : public UCTRolloutPlayer
				{
				public:
					// Construction and Destruction.
					MASTPlayer() : UCTRolloutPlayer(){};
					virtual ~MASTPlayer(){};
					
					// Override of info functions.
					virtual std::string getPlayerName() {return "Move-Average Sampling Technique (MAST) player";};
					virtual std::string getPlayerShortName() {return "MAST";};
					
					// MAST update values overrides.
					virtual double		playoutMoveValue(cadiaplayer::play::GameTheory& /*theory*/, cadiaplayer::play::utils::MTState* state, cadiaplayer::play::utils::MTAction* node, cadiaplayer::play::Move* move);
					//virtual void updateValue(cadiaplayer::play::GameTheory& theory, Move* move, double& q, PlayMoves& pm);
					virtual void updateMoveValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::Move* move, double& q);
					virtual void updateNodeValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTNode* node, cadiaplayer::play::utils::MTAction* action, double& q);
				};
		}}} // namespaces
#endif //MASTPLAYER_H
