/*
 *  tomastcomboplayer.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 3/18/10.
 *  Copyright 2010 Reykjavik University. All rights reserved.
 *
 */

#ifndef FASTTOMASTRAVEPLAYER_H
#define FASTTOMASTRAVEPLAYER_H

#include "fastmastraveplayer.h"
#include "../kits/tilingplayerkit.h"

namespace cadiaplayer {
	namespace play {
		namespace players {
			class FASTTOMASTRAVEPlayer : public FASTMASTRAVEPlayer
			{
			public:
				FASTTOMASTRAVEPlayer():FASTMASTRAVEPlayer(){};
				virtual ~FASTTOMASTRAVEPlayer(){};
				virtual std::string getPlayerName() {return "FAST/TOMAST/RAVE player";};
				virtual std::string getPlayerShortName() {return "FASTTOMASTRAVE";};
				virtual void updateMoveValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::Move* move, double& q);
			};
		}}} // namespaces
#endif //FASTTOMASTRAVEPLAYER_H
