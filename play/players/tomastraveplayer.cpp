/*
 *  tomastraveplayer.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 3/18/10.
 *  Copyright 2010 Reykjavik University. All rights reserved.
 *
 */

#include "tomastraveplayer.h"

using namespace cadiaplayer::play;
using namespace cadiaplayer::play::players;
using namespace cadiaplayer::play::utils;

void TOMASTRAVEPlayer::updateMoveValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::Move* move, double& q)
{
	UCTRAVEPlayer::updateMoveValue(theory, move, q);	
	// Skip UCTImprove (MAST) player
}
