/*
 *  tomastraveplayer.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 3/18/10.
 *  Copyright 2010 Reykjavik University. All rights reserved.
 *
 */

#ifndef TOMASTRAVEPLAYER_H
#define TOMASTRAVEPLAYER_H

#include "mastraveplayer.h"

namespace cadiaplayer {
	namespace play {
		namespace players {
			class TOMASTRAVEPlayer : public MASTRAVEPlayer
			{
			public:
				TOMASTRAVEPlayer() : MASTRAVEPlayer(){};
				virtual ~TOMASTRAVEPlayer(){};
				virtual std::string getPlayerName() {return "RAVE/TOMAST player";};
				virtual std::string getPlayerShortName() {return "TORAVEMAST";};
				
				virtual void updateMoveValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::Move* move, double& q);
			};
		}}} // namespaces
#endif //TOMASTRAVEPLAYER_H
