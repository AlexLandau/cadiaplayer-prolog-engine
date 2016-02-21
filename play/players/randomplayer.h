#ifndef RANDOMPLAYER_H
#define RANDOMPLAYER_H

#include "gameplayer.h"
#include <sstream>

namespace cadiaplayer {
	namespace play {
		namespace players {
			
			
			class RandomPlayer : public GamePlayer
				{
				public:
					RandomPlayer() : GamePlayer() {};
					
					virtual Move* play(cadiaplayer::play::GameTheory& theory);
					
					virtual std::string getPlayerName() {std::stringstream ss;ss << "Random player (" << m_roleindex+1 << ")";return ss.str();};
					virtual std::string getPlayerShortName() {return "Random";};
					
				};
		}}} // namespaces
#endif // RANDOMPLAYER_H
