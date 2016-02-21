/*
 *  pastplayer.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 10/6/08.
 *  Copyright 2008 Reykjavik University. All rights reserved.
 *
 */

#ifndef PASTPLAYER_H
#define PASTPLAYER_H

#include "mastplayer.h"
#include "../utils/movepredicatefeature.h"

//#define PAST_METHOD_1
#define PAST_METHOD_2
//#define DUMP_ACTIONBUFFER
#define DEFAULT_PAST_TEMPERATURE 8.0

namespace cadiaplayer {
	namespace play {
		namespace players {
			class PASTPlayer : public MASTPlayer
				{
				protected:
					cadiaplayer::play::utils::MovePredicateFeaturesList m_featuresList;
#ifdef PAST_METHOD_2
					cadiaplayer::play::utils::ActionIndexMap m_actionIndex;
#endif
					unsigned int m_borderCounter;
				public:
					PASTPlayer();
					virtual ~PASTPlayer(){};
					
					//virtual std::string getPlayerName() {return "UCT move-predicate features player";};
					//virtual std::string getPlayerShortName() {return "UCTMPF";};
					virtual std::string getPlayerName() {return "Predicate-Average Sampling Technique (PAST) player";};
					virtual std::string getPlayerShortName() {return "PAST";};
					virtual void newGame(cadiaplayer::play::GameTheory& theory);
#ifdef PAST_METHOD_1
					virtual double		playoutMoveValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTState* state, cadiaplayer::play::utils::MTAction* node, cadiaplayer::play::Move* move);
#endif
#ifdef PAST_METHOD_2
					virtual void		swapInPAST(cadiaplayer::play::GameTheory& theory, std::vector<double>& values, double& sum);
#endif
					virtual std::size_t selectAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleMoves& moves);
					virtual std::size_t selectSingleAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleMoves& moves);
					virtual void updateMoveValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::Move* move, double& q);
					virtual void updateNodeValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTNode* node, cadiaplayer::play::utils::MTAction* action, double& q);
					virtual void postplay(cadiaplayer::play::GameTheory& theory);
				};
		}}} // namespaces
#endif //PASTPLAYER_H
