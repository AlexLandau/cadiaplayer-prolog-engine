/*
 *  fasttomastplayer.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 10/4/12.
 *  Copyright 2012 Reykjavik University. All rights reserved.
 *
 */

#ifndef FASTTOMASTPLAYER_H
#define FASTTOMASTPLAYER_H

#include "fastmastplayer.h"
#include "../kits/tilingplayerkit.h"

namespace cadiaplayer {
	namespace play {
		namespace players {
			class FASTTOMASTPlayer : public FASTMASTPlayer
			{
			public:
				FASTTOMASTPlayer():FASTMASTPlayer(){};
				virtual ~FASTTOMASTPlayer(){};
				virtual std::string getPlayerName() {return "FAST/TOMAST player";};
				virtual std::string getPlayerShortName() {return "FASTTOMAST";};
				virtual void updateMoveValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::Move* move, double& q);
			};
		}}} // namespaces
#endif //FASTTOMASTPLAYER_H
