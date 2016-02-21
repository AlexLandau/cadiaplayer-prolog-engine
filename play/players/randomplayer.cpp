#include "randomplayer.h"
using namespace cadiaplayer::play::players;

cadiaplayer::play::Move* RandomPlayer::play(cadiaplayer::play::GameTheory& theory)
{
	cadiaplayer::play::Move* move = theory.generateRandomMove(m_roleindex);
	return move;
}
