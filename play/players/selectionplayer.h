/*
 *  selectionplayer.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 10/11/11.
 *  Copyright 2011 Reykjavik University. All rights reserved.
 *
 */

#ifndef SELECTIONPLAYER_H
#define SELECTIONPLAYER_H

#include "mcmemoryplayer.h"
#include "../kits/goalstabilitykit.h"

#define EC_ACTIVE_LOG_MESSAGE "Early Cutoff enabled"
#define EC_INACTIVE_LOG_MESSAGE "Early Cutoff disabled"

namespace cadiaplayer {
	namespace play {
		namespace players {
						
			class SelectionPlayer : public MCMemoryPlayer, cadiaplayer::play::kits::GoalStabilityKit
			{
			protected:
				bool m_useEarlyCutoff;
			public:
				SelectionPlayer();
				virtual ~SelectionPlayer();
				virtual std::string getPlayerName() {return "Selection player";};
				virtual std::string getPlayerShortName() {return "SELECT";};
				
				// Overrides and added override functions
				virtual size_t selectAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleMoves& moves);
				// The return value will determine move selection with the greedy policy.
				virtual double selectionNodeValue(cadiaplayer::play::GameTheory& /*theory*/, cadiaplayer::play::utils::MTState* state, cadiaplayer::play::utils::MTAction* node);
				virtual double unexploredNodeValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTState* state, cadiaplayer::play::RoleMoves& moves, std::size_t unexplored);
				virtual void actionAvailable(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::Move* move, cadiaplayer::play::utils::MTAction* node, std::size_t depth, cadiaplayer::play::RoleIndex role){};
			
				// Early cutoff Getter and Setter
				void enableEarlyCutoff(int minimumThreshold = cadiaplayer::play::kits::DEFAULT_MINIMUM_TERMINAL_SPAN);
				void disableEarlyCutoff();
				inline bool isEarlyCutoffEnabled(){return m_useEarlyCutoff;};
				// Early cutoff, inclusion of the GoalStability kit.
				virtual void newGame(cadiaplayer::play::GameTheory& theory);
				virtual void newSearch(cadiaplayer::play::GameTheory& theory);
				virtual bool cutoff(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::Depth depth);
				virtual void makeAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::PlayMoves& pm, std::vector<double>& rewards);
				virtual cadiaplayer::play::utils::MTNode* makeAction(cadiaplayer::play::GameTheory& /*theory*/, cadiaplayer::play::utils::MTStateID sid, cadiaplayer::play::utils::Depth depth, cadiaplayer::play::PlayMoves& moves);
				virtual void atTerminal(cadiaplayer::play::GameTheory& theory, std::vector<double>& qValues);
				virtual std::string getSetupInfo();
				virtual std::string getLastPlayInfo(cadiaplayer::play::GameTheory& theory);
				
			};
		}}} // namespaces
#endif // SELECTIONPLAYER_H
