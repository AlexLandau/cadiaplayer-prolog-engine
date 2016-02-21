/*
 *  pastraveplayer.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 5/25/09.
 *  Copyright 2009 Reykjavik University. All rights reserved.
 *
 */

#ifndef PASTRAVEPLAYER_H
#define PASTRAVEPLAYER_H

//#define PASTRAVE_METHOD_1
#define PASTRAVE_METHOD_2
#define DEFAULT_PASTRAVE_TEMPERATURE 8.0

#include "mastraveplayer.h"
#include "../utils/movepredicatefeature.h"

namespace cadiaplayer {
	namespace play {
		namespace players {
			class PASTRAVEPlayer : public MASTRAVEPlayer
				{
				protected:
					cadiaplayer::play::utils::MovePredicateFeaturesList m_featuresList;
#ifdef PASTRAVE_METHOD_2
					cadiaplayer::play::utils::ActionIndexMap m_actionIndex;
#endif
					unsigned int m_borderCounter;
				public:
					PASTRAVEPlayer();
					virtual ~PASTRAVEPlayer(){};
					virtual std::string getPlayerName() {return "PAST/RAVE player";};
					virtual std::string getPlayerShortName() {return "PASTRAVE";};
					
					virtual void newGame(cadiaplayer::play::GameTheory& theory);
#ifdef PASTRAVE_METHOD_1
					virtual double		playoutMoveValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTState* state, cadiaplayer::play::utils::MTAction* node, cadiaplayer::play::Move* move);
#endif
#ifdef PASTRAVE_METHOD_2
					virtual void		swapInPAST(cadiaplayer::play::GameTheory& theory, std::vector<double>& values, double& sum);
#endif
					virtual std::size_t selectAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleMoves& moves);
					virtual std::size_t selectSingleAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleMoves& moves);
					virtual void updateMoveValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::Move* move, double& q);
					virtual void updateNodeValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTNode* node, cadiaplayer::play::utils::MTAction* action, double& q);
					virtual void postplay(cadiaplayer::play::GameTheory& theory);
					
				};
		}}} // namespaces
#endif //RAVEPASTPLAYER_H
