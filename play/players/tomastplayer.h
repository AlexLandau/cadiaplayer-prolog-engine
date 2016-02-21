/*
 *  tomastplayer.h  
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 2/27/09.
 *  Copyright 2009 Reykjavik University. All rights reserved.
 *
 */

#ifndef TOMASTPLAYER_H
#define TOMASTPLAYER_H

#include "mastplayer.h"

namespace cadiaplayer {
	namespace play {
		namespace players {
			class TOMASTPlayer : public MASTPlayer
				{
				private:
				public:
					TOMASTPlayer() : MASTPlayer(){};
					virtual ~TOMASTPlayer(){};
					
					virtual std::string getPlayerName() {return "Tree-Only Move-Average Sampling Technique (TOMAST) player";};
					virtual std::string getPlayerShortName() {return "TOMAST";};
					virtual void updateMoveValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::Move* move, double& q);
				};
		}}} // namespaces
#endif //UCTTREEPLAYER_H
