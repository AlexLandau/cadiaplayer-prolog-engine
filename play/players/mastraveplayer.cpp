/*
 *  mastraveplayer.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 5/5/09.
 *  Copyright 2009 Reykjavik University. All rights reserved.
 *
 */

#include "mastraveplayer.h"

using namespace cadiaplayer::play;
using namespace cadiaplayer::play::players;
using namespace cadiaplayer::play::utils;

double MASTRAVEPlayer::playoutMoveValue(cadiaplayer::play::GameTheory& /*theory*/, MTState* state, MTAction* node, Move* move)
{
	return node ? node->move->getQ() : m_multitree->getBufferedMove(m_workrole, move)->getQ();
}

void MASTRAVEPlayer::updateMoveValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::Move* move, double& q)
{
	UCTRAVERolloutPlayer::updateMoveValue(theory, move, q);
	// MAST
	m_multitree->getBufferedMove(m_workrole, move)->addQ(q);
}
void MASTRAVEPlayer::updateNodeValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTNode* node, cadiaplayer::play::utils::MTAction* action, double& q)
{
	UCTRAVERolloutPlayer::updateNodeValue(theory, node, action, q);
	// MAST
	action->move->addQ(q);
}

