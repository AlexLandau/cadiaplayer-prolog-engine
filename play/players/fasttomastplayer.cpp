/*
 *  fasttomastplayer.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 10/4/12.
 *  Copyright 2012 Reykjavik University. All rights reserved.
 *
 */

#include "fasttomastplayer.h"

using namespace cadiaplayer::play;
using namespace cadiaplayer::play::players;
using namespace cadiaplayer::play::utils;
using namespace cadiaplayer::play::kits;

void FASTTOMASTPlayer::updateMoveValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::Move* move, double& q)
{
	UCTPlayer::updateMoveValue(theory, move, q);
	// MAST update outside tree now disabled 
}

