/*
 *  tilingplayerkit.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 3/18/10.
 *  Copyright 2010 Reykjavik University. All rights reserved.
 *
 */

#ifndef TILINGPLAYERKIT_H
#define TILINGPLAYERKIT_H

#include <cstdlib>
#include "../tiling/tilecodinglearner.h"
#include "../tiling/valuetiling.h"
#include "../tiling/rolepiecetiling.h"
#include "../tiling/piecevaluetiling.h"

#define TILING_KIT_VALID_AVERAGE 2
#define TILING_KIT_VALID_SAMPLES 1000
#define TILING_KIT_IGNORE_VALUE -100
#define TILING_KIT_POSITION_TUNER 5.0
#define TILING_KIT_MARKER_WEIGHT 5.0
//#define TILING_KIT_DISABLE_MARKER_POSITIONS

namespace cadiaplayer {
	namespace play {
		namespace kits {
			class TilingPlayerKit
			{
				// Holders for implementing players if the use softmax 
			public:
				std::vector<double> m_softmax_tiling;
				double				m_divider_tiling;
				//double				m_tilingscore;
				bool				m_useTiling;
				bool				m_void;
				
			public:
				cadiaplayer::play::tiling::TileCodingLearner*	m_learner;
				cadiaplayer::play::utils::Episode				m_episode;
				cadiaplayer::play::tiling::PieceValueTiling*	m_pvt;
				cadiaplayer::play::tiling::RolePieceTiling*		m_rptPlace;
				cadiaplayer::play::tiling::RolePieceTiling*		m_rptMove;
				std::size_t										m_isValid;
			public:
				TilingPlayerKit();
				virtual ~TilingPlayerKit();
				
				// Segments
				void newGameTilingEndSegment(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleIndex playerindex);
				void newSearchTilingEndSegment(cadiaplayer::play::GameTheory& theory);
				void atTerminalTilingEndSegment(cadiaplayer::play::GameTheory& /*theory*/, std::vector<double>& qValues);
				void makeActionTilingEndSegment(cadiaplayer::play::GameTheory& theory);
				std::string getLastPlayInfoTilingEndSegment(cadiaplayer::play::GameTheory& /*theory*/);
				
				// Helper functions
				cadiaplayer::play::GameStateID getMoveFeature(GameTheory& theory, Move* move, RoleIndex role);
				cadiaplayer::play::GameStateID getCaptureFeature(GameTheory& theory, Move* move, RoleIndex role);
				double getTilingMoveEvaluation(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleIndex roleindex, cadiaplayer::play::RoleIndex playerindex, cadiaplayer::play::Move* move);
				void createLearner(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleIndex playerindex);
				bool isValid(){return m_isValid>0;};
				std::size_t validMax(){return m_isValid;};
				
				bool isVoid(){
					return m_void;
				};
				void setVoid(bool v){
					m_void = v;
				};
			};
		}}} // namespaces
#endif //TILINGPLAYERKIT_H
