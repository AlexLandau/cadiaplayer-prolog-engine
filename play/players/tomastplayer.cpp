/*
 *  tomastplayer.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 2/27/09.
 *  Copyright 2009 Reykjavik University. All rights reserved.
 *
 */

#include "tomastplayer.h"

using namespace cadiaplayer::play::players;
using namespace cadiaplayer::play::utils;

void TOMASTPlayer::updateMoveValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::Move* move, double& q)
{
	UCTPlayer::updateMoveValue(theory, move, q);
	// MAST update outside tree now disabled 
}
