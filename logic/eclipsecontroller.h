/*
 *  eclipsecontroller.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 2/8/11.
 *  Copyright 2011 Reykjavik University. All rights reserved.
 *
 */

#ifndef ECLIPSECONTROLLER_H
#define ECLIPSECONTROLLER_H

#include "gamecontroller.h"
#include "../play/parsing/compound.h"
#include <eclipse/eclipse.h>

namespace cadiaplayer {
	namespace logic {
		
		class EclipseController : public GameController
		{

		public:
			// Constructor
			EclipseController();
			// Destructor
			~EclipseController();
			
			// GameTheory middleware calls this function to allow the controller to do any initialization needed
			// Returns true if the controller is ready to play the game from the path/gamename given. 
			virtual bool init( cadiaplayer::play::GameTheory* gametheory, std::string gamename );
			
			// Returns the initial state of the game as a GameState class (vector of Compound*)
			virtual void getInitialState(cadiaplayer::play::GameState& state);
			// Returns the current state of the game as a GameState class (vector of Compound*)
			virtual void getCurrentState(cadiaplayer::play::GameState& state);
			
			// Loades the available legal moves in the current state for role r (0=first role) into RoleMoves class m (vector of Move* classses)
			virtual void getMoves( cadiaplayer::play::RoleIndex role, cadiaplayer::play::RoleMoves& moves );
			// Creates Does relations from the Move* instances in the PlayMoves(vector) moves and proves the Next relations.
			// Returns the new current state of the game as a GameState class (vector of Compound*).
			virtual void make(cadiaplayer::play::PlayMoves& moves);
			
			// Retract to the previouse state of the current state. 
			virtual void retract( void );
			// Disconnects the work done by retract, used by simulation implementations that do not need to make any more moves
			// once the retract/backpropagation phase begins.  Save the theorem prover needless work as the states played through
			// have been sent to the player and they will not be used to take play any other branch of the tree.
			// This function should retain a snapshot of the current state so it can be restored by calling syncRetract();
			virtual void muteRetract( void );
			// Reconnects the work done by retract and restores the current state to how it was when muteRetract(); was last called. 
			virtual void syncRetract( void );
			
			// Returns the goal for the role indexed (0 = first role)  
			virtual double goal( cadiaplayer::play::RoleIndex role );
			// Returns true if the current state is a terminal one, false otherwise.
			virtual bool isTerminal( );
			
			/**
			* get the StateFactID of the state fact represented by the given Compound
			* (actually this has nothing to do with the prolog system and should be located somewhere else)
			*/
			virtual cadiaplayer::play::StateFactID getStateFact(cadiaplayer::play::parsing::Compound* comp);

			/**
			* save the current state (for retract) and set the given state as new current state
			*/
			virtual void syncState(const cadiaplayer::play::StateFacts& state);

		private:
			// push current state on the stack for later retrieval
			void saveCurrentState();
			
			pword getCurrentStatePWord();
			// Helper functions for transforming Eclipse objects into the CadiaPlayer standard ones and vice versa
			std::vector<cadiaplayer::play::parsing::Compound*> pword_to_compound_list(pword w);
			void pword_to_compound_list(pword w, std::vector<cadiaplayer::play::parsing::Compound*>& list);
			cadiaplayer::play::parsing::Compound* pword_to_compound(pword w);
			pword compound_list_to_pword(const std::vector<cadiaplayer::play::parsing::Compound*> &compoundList);
			pword compound_to_pword(const cadiaplayer::play::parsing::Compound &compound);
			pword moves_list_to_pword(const std::vector<cadiaplayer::play::Move*> movesList);
			pword move_to_pword(const cadiaplayer::play::parsing::Compound &compound);

			std::string termToString(pword term);
			pword parseTermString(const std::string &s);
			
			void cleanUp();
			
			void test_controller();
		}; 
	}} // namespaces
#endif // ECLIPSECONTROLLER_H

