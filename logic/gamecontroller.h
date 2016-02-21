#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include "../play/gametheory.h"

namespace cadiaplayer {
namespace logic {

class GameController
{
protected:
	// Basic members of this interface
	cadiaplayer::play::GameTheory* m_theory;	// The GameTheory object using thes controller
	std::string m_gamename;						// Name of the game being played
	bool m_mute;								// Modifier for the mute/sync functionality for bypassing
												// controller execution if not needed (f.ex. when retracting simulation paths)
	
	// Accessor functions for the GameTheory object, Game name string and the Mute (mute/sync) modifier
	inline cadiaplayer::play::GameTheory* getTheory(){return m_theory;};
	inline std::string getGamename(){return m_gamename;};
	inline bool isMute(){return m_mute;}; 
	inline void overrideMute(bool mute){m_mute = mute;};
public:
	GameController();
	virtual ~GameController();
	
	// Do all initialization
	// If you override this function start by calling:
	//		GameController::init(gametheory, gamename);
	// to get all the default initialization done for you (setting GameTheory, Gamename and Mute members). 
	virtual bool init( cadiaplayer::play::GameTheory* gametheory, std::string gamename );
	
	//***********************************************************************
	// Mandatory functions when implementing the GameController interface  //
	//***********************************************************************
	// Get the predicate list describing the initial state of the game.
	virtual void getInitialState(cadiaplayer::play::GameState& state)=0;
	// Get the predicate list describing the current state of the game.
	virtual void getCurrentState(cadiaplayer::play::GameState& state)=0;
	// Load available moves to role with index r into the RoleMoves vector (byRef)
	virtual void getMoves( cadiaplayer::play::RoleIndex /*role*/, cadiaplayer::play::RoleMoves& /*moves*/ )=0;
	// Normal make - moves holds a move for every player in the correct role order
	virtual void make(cadiaplayer::play::PlayMoves& moves)=0;
	// Is the game in a terminal state
	virtual bool isTerminal( )=0;
	// Score the the game in the current state for a certain role 
	virtual double goal( cadiaplayer::play::RoleIndex role )=0;
	
	//***********************************************************************
	// Optional functions when implementing the GameController interface   //
	//***********************************************************************
	// Normal make - random moves selected automatically for other roles
	virtual void make(cadiaplayer::play::RoleIndex role, cadiaplayer::play::Move* move);
	// Make that overrides the mute/sync retract functionality and forces the current state onto the gamestack for retraction later
	virtual void makeByForce(cadiaplayer::play::RoleIndex role, cadiaplayer::play::Move* move );
	// Make that add the effects of the moves made to the current state (i.e. current state not cleared before assert)
	virtual void makeRelaxed(cadiaplayer::play::PlayMoves& moves);
	// Make that proves and asserts the next state without the knowledge base containing any does clauses
	// Returns the resulting state, but does not make permanent changes to the underlying knowledgebase.
	virtual void makeNullMove(cadiaplayer::play::GameState& state);
	// Make that fetches the next state if only one role makes a move in the current state.  The knowledgebase should be 
	// in the exact same state after calling this fuction as it was before.
	virtual void makeExploratory( cadiaplayer::play::RoleIndex role, cadiaplayer::play::Move* move, cadiaplayer::play::GameState& state );
	// Fetches the next state given the moves of all roles without moving to that state.
	// The knowledgebase should be in the exact same state after calling this fuction as it was before.
	virtual void peekNextState(cadiaplayer::play::PlayMoves& moves, cadiaplayer::play::GameState& nextState );
	// Same as the above peeNextState except random moves are selected automatically for other roles than r.  
	// The knowledgebase should be in the exact same state after calling this fuction as it was before.
	virtual void peekNextState(cadiaplayer::play::RoleIndex role, cadiaplayer::play::Move* move, cadiaplayer::play::GameState& nextState );
	virtual void effectsPlus(cadiaplayer::play::RoleIndex role, cadiaplayer::play::Move* move, cadiaplayer::play::parsing::CompoundList& effects);
	virtual void effectsPlus(cadiaplayer::play::PlayMoves& moves, cadiaplayer::play::parsing::CompoundList& effects);
	virtual void effectsMinus(cadiaplayer::play::RoleIndex role, cadiaplayer::play::Move* move, cadiaplayer::play::parsing::CompoundList& effects);
	virtual void effectsMinus(cadiaplayer::play::PlayMoves& moves, cadiaplayer::play::parsing::CompoundList& effects);
	virtual void commitEffectsPlus();
	virtual void discardEffectsPlus();
	virtual void storeCurrentStateAs(cadiaplayer::play::GameStateID /*sid*/);
	virtual void jumpToStoredState(cadiaplayer::play::GameStateID /*sid*/);
	virtual void deleteStoredState(cadiaplayer::play::GameStateID /*sid*/);

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
	virtual void syncState(const cadiaplayer::play::StateFacts& /*state*/);
	virtual cadiaplayer::play::StateFactID getStateFact(cadiaplayer::play::parsing::Compound* /*comp*/);
	virtual void getStateFacts(const cadiaplayer::play::GameState& state, cadiaplayer::play::StateFacts& stateFacts);
	virtual std::string stateFactToString(const cadiaplayer::play::StateFactID& /*stateFact*/);
	// Assert one statefact into the current state.
	virtual void assertStateFact(const cadiaplayer::play::StateFactID& stateFact);
	// Retract one statefact from the current state.
	virtual void retractStateFact(const cadiaplayer::play::StateFactID& stateFact);
	// Recalculate the fact id when enclosed in a has_seen predicate.
	virtual cadiaplayer::play::StateFactID getHasSeenID(const cadiaplayer::play::StateFactID& stateFact, unsigned long long salt);

	// Additional goal handling functions
	virtual double incGoals( cadiaplayer::play::RoleIndex role );
	virtual double allGoals( cadiaplayer::play::RoleIndex role );
	virtual bool reachedGoal( cadiaplayer::play::RoleIndex role, int goal);

	virtual void ask(cadiaplayer::play::parsing::Compound* /*c*/, bool /*interupt*/, cadiaplayer::play::KBaseList& /*answer*/);
	virtual size_t solutionCardinality(std::string /*predicate*/, std::string /*atom*/, size_t /*pos*/, size_t /*size*/);

	// String interface implementation
	virtual std::string getGameFileName();
	virtual std::string getStringState();
	virtual void gotoStringState(std::string state);
};
}
} // namespaces
#endif // GAMECONTROLLER_H
