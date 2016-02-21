/*
 *  mastraveplayer.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 5/5/09.
 *  Copyright 2009 Reykjavik University. All rights reserved.
 *
 */

#ifndef MASTRAVEPLAYER_H
#define MASTRAVEPLAYER_H

#include "uctraverolloutplayer.h"

namespace cadiaplayer {
	namespace play {
		namespace players {
			class MASTRAVEPlayer : public UCTRAVERolloutPlayer
				{
				public:
					MASTRAVEPlayer() : UCTRAVERolloutPlayer(){};
					virtual ~MASTRAVEPlayer(){};
					virtual std::string getPlayerName() {return "MAST/RAVE player";};
					virtual std::string getPlayerShortName() {return "MASTRAVE";};
					
					virtual void updateMoveValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::Move* move, double& q);
					virtual void updateNodeValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTNode* node, cadiaplayer::play::utils::MTAction* action, double& q);
					
					// The return values will be selected from with the softmax policy. 
					virtual double playoutMoveValue(cadiaplayer::play::GameTheory& /*theory*/, cadiaplayer::play::utils::MTState* state, cadiaplayer::play::utils::MTAction* node, cadiaplayer::play::Move* move);
				};
		}}} // namespaces
#endif //RAVEMASTPLAYER_H
