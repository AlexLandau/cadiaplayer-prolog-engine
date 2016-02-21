/*
 *  fastplayer.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 3/3/12.
 *  Copyright 2012 Reykjavik University. All rights reserved.
 *
 */

#ifndef FASTPLAYER_H
#define FASTPLAYER_H

#include "uctrolloutplayer.h"
#include "../kits/tilingplayerkit.h"

namespace cadiaplayer {
	namespace play {
		namespace players {
			class FASTPlayer : public UCTPlayer, cadiaplayer::play::kits::TilingPlayerKit 
			{
			protected:
				// FAST functionality wrapping 
				Cost						m_score;
				Cost						m_fscore;
				std::vector<double>			m_fsoftmax;
				std::vector<std::size_t>	m_frandexp;
				size_t						m_fcount;
				double						m_fdivider;
				bool						m_fuseTiling;
				bool						m_fvalid;
				inline std::size_t			selectFASTUnexplored()
				{return m_fcount < 2 ? m_frandexp[0] : 
					m_frandexp[selectGibbAction(m_fcount, m_fsoftmax, m_fdivider)];};

			public:
				FASTPlayer():UCTPlayer(), cadiaplayer::play::kits::TilingPlayerKit(){};
				virtual ~FASTPlayer(){};
				virtual std::string getPlayerName() {return "FAST player";};
				virtual std::string getPlayerShortName() {return "FAST";};
				
				virtual bool adjustArrays(std::size_t branch);
				virtual std::size_t selectAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleMoves& moves);
				
				// Tiling segment functions
				virtual void newGame(cadiaplayer::play::GameTheory& theory);
				virtual void newSearch(cadiaplayer::play::GameTheory& theory);
				virtual void atTerminal(cadiaplayer::play::GameTheory& theory, std::vector<double>& qValues);
				virtual void makeAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::PlayMoves& pm, std::vector<double>& rewards);
				virtual cadiaplayer::play::utils::MTNode* makeAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTStateID sid, cadiaplayer::play::utils::Depth depth, cadiaplayer::play::PlayMoves& moves);
				virtual std::string getLastPlayInfo(cadiaplayer::play::GameTheory& theory);
							};
		}}} // namespaces
#endif //FASTPLAYER_H
