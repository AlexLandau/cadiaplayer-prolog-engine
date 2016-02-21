/*
 *  mastplayer.cc
 *  src
 *
 *  Created by Hilmar Finnsson on 10/25/07.
 *  Copyright 2007 CADIA Reykjavík University. All rights reserved.
 *
 */

#include "mastplayer.h"
using namespace cadiaplayer::play::players;
using namespace cadiaplayer::play::utils;

double MASTPlayer::playoutMoveValue(cadiaplayer::play::GameTheory& /*theory*/, MTState* state, MTAction* node, Move* move)
{
#ifndef MAST_LEVELS
	//std::cerr << "playoutMoveValue for role " << m_workrole << std::endl;
	return node ? node->move->getQ() : m_multitree->getBufferedMove(m_workrole, move)->getQ();
#else
	unsigned int d = getDepth(theory);
	return node ? node->move->getQ(d) : m_multitree->getBufferedMove(m_workrole, move)->getQ(d);
#endif
}

void MASTPlayer::updateMoveValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::Move* move, double& q)
{
#ifndef MAST_LEVELS
	//std::cerr << "updateMoveValue (non-node) for role " << m_workrole << std::endl;
	m_multitree->getBufferedMove(m_workrole, move)->addQ(q);
#else
	m_multitree->getBufferedMove(m_workrole, move)->addQ(q, getDepth(theory));
#endif
}
void MASTPlayer::updateNodeValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTNode* node, cadiaplayer::play::utils::MTAction* action, double& q)
{
#ifndef MAST_LEVELS
	//std::cerr << "updateMoveValue (node) for role " << m_workrole << std::endl;
	action->move->addQ(q);
#else
	action->move->addQ(q, getDepth(theory));
#endif
}
