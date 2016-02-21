/*
 *  uctraveplayer.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 2/26/09.
 *  Copyright 2009 Reykjavik University. All rights reserved.
 *
 */

#ifndef UCTRAVEPLAYER_H
#define UCTRAVEPLAYER_H

#include "uctplayer.h"
#include "../utils/movetrails.h"

namespace cadiaplayer {
	namespace play {
		namespace players {
			
			class UCTRAVEPlayer : public UCTPlayer
				{
				protected:
					// Holder move ids for RAVE
					cadiaplayer::play::utils::MoveTrails m_moveTrails;
					double m_equiv;
				public:
					UCTRAVEPlayer() : UCTPlayer(), m_equiv(EQUIV_PARAM){};
					virtual ~UCTRAVEPlayer();
					double getEquivalenceParameter(void){return m_equiv;};
					void   setEquivalenceParameter(double equiv){m_equiv=equiv;};
					virtual std::string getPlayerName() {return "UCT RAVE player";};
					virtual std::string getPlayerShortName() {return "UCTRAVE";};
					virtual void newSearch(cadiaplayer::play::GameTheory& theory);
					virtual double selectionNodeValue(cadiaplayer::play::GameTheory& /*theory*/, cadiaplayer::play::utils::MTState* state, cadiaplayer::play::utils::MTAction* node);
					virtual cadiaplayer::play::Move* bestAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTState* state, double &reward);
					virtual cadiaplayer::play::Move* controlAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTState* state, double &reward);
					virtual void bestActionRatings(RoleIndex role, cadiaplayer::play::utils::MTState* state,cadiaplayer::play::MoveRatings& ratings);
					virtual void controlActionRatings(RoleIndex role, cadiaplayer::play::utils::MTState* state,cadiaplayer::play::MoveRatings& ratings);
					double rave(cadiaplayer::play::utils::MTAction* node, RoleIndex role, double basevalue);
					virtual void updateMoveValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::Move* move, double& q);
					virtual void updateNodeValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTNode* node, cadiaplayer::play::utils::MTAction* action, double& q);
				};
		}}} // namespaces
#endif //UCTRAVEPLAYER_H
