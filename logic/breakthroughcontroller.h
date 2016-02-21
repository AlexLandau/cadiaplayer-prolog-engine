/*
 *  breakthroughcontroller.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 6/18/12.
 *  Copyright 2012 Reykjavik University. All rights reserved.
 *
 */

#ifndef BREAKTHROUGHCONTROLLER_H
#define BREAKTHROUGHCONTROLLER_H

#include "gamecontroller.h"
#include "../play/gametheory.h"
#include "games/breakthrough.h"

namespace cadiaplayer {
	namespace logic {
		
		
		static std::string BTC_CONTROL =	"control";
		static std::string BTC_CELL =		"cell";
		static std::string BTC_MOVE =		"move";
		static std::string BTC_NOOP =		"noop";
		static std::string BTC_NUM[] =		{"1","2","3","4","5","6","7","8"};
		static std::string BTC_WHITE =		"white";
		static std::string BTC_BLACK =		"black";

		class BreakthroughController : public GameController
		{
		private:
			cadiaplayer::play::parsing::SymbolTable* m_symbols;
			games::BTState* m_breakthrough;
			games::BTMoves m_moves;
			
			cadiaplayer::play::GameState m_initstate;
			cadiaplayer::play::GameState m_state;
			
			// Vector of all predicate compounds, 64 white pos, 64 black pos, 2 control
			cadiaplayer::play::GameState m_omnistate;
			// Vector of all StateFactID(GameStateID) index matched to m_omnistate
			cadiaplayer::play::StateFacts m_omnifacts;
			// Map of all indexes to m_omnistate, keyed on StateFactID(GameStateID)
			cadiaplayer::play::StateFactMap m_factmap;
			cadiaplayer::play::Move* m_noopWhite;
			cadiaplayer::play::Move* m_noopBlack;
			// Map of all white moves keyed on the numerical value of the breakthrough move
			cadiaplayer::play::MovePtrStore m_omniwhitemoves;
			// Map of all black moves keyed on the numerical value of the breakthrough move
			cadiaplayer::play::MovePtrStore m_omniblackmoves;
			// Map of all moves keyed on the GameStateID of the move compound
			cadiaplayer::play::StateFactMap m_omnibtmoves;
			
			games::BTStateID	m_syncState;
			games::History		m_gamestack;
			
			void loadGDL(cadiaplayer::play::GameTheory* t);
			cadiaplayer::play::Move* moveToGDL(games::BTMove& move, games::BTPiece turn);
			void BTStateToGameState(games::BTState& btstate, cadiaplayer::play::GameState& state);
			void BTStateToStateFacts(games::BTState& btstate, cadiaplayer::play::StateFacts& facts);
			void StateFactsToBTState(cadiaplayer::play::StateFacts& facts, games::BTState* btstate);
			void addStateFactToBTState(cadiaplayer::play::StateFactID fact, games::BTState* btstate);
			void removeStateFactFromBTState(cadiaplayer::play::StateFactID fact, games::BTState* btstate);
			
		public:
			BreakthroughController();
			
			virtual ~BreakthroughController();
			
			virtual bool init( cadiaplayer::play::GameTheory* gametheory, std::string path, std::string gamename );
			
			virtual bool isTerminal( );
			
			virtual void getMoves( cadiaplayer::play::RoleIndex role, cadiaplayer::play::RoleMoves& moves );
			
			virtual void getInitialState(cadiaplayer::play::GameState& state);
			virtual void getCurrentState(cadiaplayer::play::GameState& state);
			virtual void make(cadiaplayer::play::RoleIndex role, cadiaplayer::play::Move* move );
			virtual void makeByForce(cadiaplayer::play::RoleIndex role, cadiaplayer::play::Move* move );
			virtual void make(cadiaplayer::play::PlayMoves& moves);
			virtual void makeRelaxed(cadiaplayer::play::PlayMoves& moves);
			virtual void makeNullMove( cadiaplayer::play::GameState& state );
			virtual void makeExploratory( cadiaplayer::play::RoleIndex role, cadiaplayer::play::Move* move, cadiaplayer::play::GameState& state);
			
			virtual void peekNextState(cadiaplayer::play::RoleIndex role, cadiaplayer::play::Move* move, cadiaplayer::play::GameState& nextState);
			virtual void peekNextState(cadiaplayer::play::PlayMoves& moves, cadiaplayer::play::GameState& nextState);
			virtual void effectsPlus(cadiaplayer::play::PlayMoves& moves, cadiaplayer::play::parsing::CompoundList& effects);
			virtual void effectsMinus(cadiaplayer::play::PlayMoves& moves, cadiaplayer::play::parsing::CompoundList& effects);
			virtual void commitEffectsPlus();
			virtual void discardEffectsPlus();
			virtual void storeCurrentStateAs(cadiaplayer::play::GameStateID sid);
			virtual void jumpToStoredState(cadiaplayer::play::GameStateID sid);
			virtual void deleteStoredState(cadiaplayer::play::GameStateID sid);
			
			virtual void retract( void );
			virtual void retractByForce( void );
			virtual void muteRetract( void );
			virtual void syncRetract( void );
			
			virtual void gotoStart();
			virtual void gotoState(const cadiaplayer::play::StateFacts& state);
			virtual void gotoStateByForce(const cadiaplayer::play::StateFacts& state);
			/**
			 * save the current state (for retract) and set the given state as new current state
			 */
			virtual void syncState(const cadiaplayer::play::StateFacts& state);
			virtual cadiaplayer::play::StateFactID getStateFact(cadiaplayer::play::parsing::Compound* comp);
			virtual void getStateFacts(const cadiaplayer::play::GameState& state, cadiaplayer::play::StateFacts& stateFacts);
			virtual std::string stateFactToString(const cadiaplayer::play::StateFactID& stateFact);
			// Assert one statefact into the current state.
			virtual void assertStateFact(const cadiaplayer::play::StateFactID& stateFact);
			// Retract one statefact from the current state.
			virtual void retractStateFact(const cadiaplayer::play::StateFactID& stateFact);
			// Recalculate the fact id when enclosed in a has_seen predicate.
			virtual cadiaplayer::play::StateFactID getHasSeenID(const cadiaplayer::play::StateFactID& stateFact, unsigned long long salt);
			
			virtual double goal( cadiaplayer::play::RoleIndex role );
			virtual double incGoals( cadiaplayer::play::RoleIndex role );
			virtual double allGoals( cadiaplayer::play::RoleIndex role );
			virtual bool reachedGoal( cadiaplayer::play::RoleIndex role, int goal);
			
			virtual void ask(cadiaplayer::play::parsing::Compound* c, bool interupt, cadiaplayer::play::KBaseList& answer);
			virtual size_t solutionCardinality(std::string predicate, std::string atom, size_t pos, size_t size);
			
			// String interface implementation
			virtual std::string getGameFileName();
			virtual std::string getStringState();
			virtual void gotoStringState(std::string state);
		};
	}
} // namespaces
#endif // BREAKTHROUGHCONTROLLER_H
