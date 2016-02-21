#ifndef GAMETHEORY_H
#define GAMETHEORY_H

#include "../utils/worker.h"
#include "../utils/tcpserver.h"

#include <vector>
#include <stack> 
#include <algorithm>
#include <cassert>

#include "../utils/longlonghash.h"
#include "../utils/stringhash.h"
#include "move.h"

#include "utils/binaryrelation.h"
#include "utils/relationcounter.h"

namespace cadiaplayer {
	namespace logic
	{
		class GameController;
	}
	namespace play {
		const double RAND_DIV = ((double)(RAND_MAX))+1.0;
		
		namespace parsing 
		{
			class GameParser;
		}
		
		const std::string KIF_FETCH_PATH		=	"../games/";
		const std::string KIF_FILE_PATH			=	"./games/kif/";
		const std::string KIF_FILE_EXT			=	".kif";
		
		const std::string PREDICATE_HAS_SEEN	=	"has_seen";
		
		typedef parsing::CompoundArgs KBaseList;
		typedef KBaseList::iterator KBaseListItr;
		typedef std::stack<parsing::Compound*> KBaseStack;
		typedef __gnu_cxx::hash_map<unsigned int,  KBaseList> KBaseTable;
		typedef KBaseTable::iterator KBaseTableItr;
		typedef KBaseList GameState;
		typedef std::stack<GameState> GameLine;
		typedef std::stack<GameState*> ParentLine;
		typedef std::size_t RoleIndex;
		typedef std::string Role;
		typedef std::vector<Role> Roles;
		
		typedef size_t MoveIndex;
		typedef std::vector<Move*> RoleMoves;
		typedef std::vector<Move*> PlayMoves;
		typedef std::vector<RoleMoves> AllRoleMoves;
		typedef std::vector<AllRoleMoves> AllRoleMovesList;
		typedef std::vector<RoleMoves*> StateMoves;
		typedef std::vector<StateMoves*> StateMovesLine;
		
		const int TERMINAL_TRUE=1;
		const int TERMINAL_FALSE=0;
		const int TERMINAL_UNKNOWN=-1;
		const double GOAL_MIN =  0.0;
		const double GOAL_DRAW = 50.0;
		const double GOAL_MAX =  100.0;
		const double GOAL_NOT =  -1.0;
		const double GOAL_UNKNOWN =  -2.0;
		const std::string STATE_FACT = "true";
		const parsing::SymbolID NO_COUNTER_PREDICATE = 0;
		const size_t ADVERSARY_PLAYER1 = 0;
		const size_t ADVERSARY_PLAYER2 = 1;
		const size_t ADVERSARY_NOT = 2;
		
		typedef unsigned long long GameStateID;
		typedef std::vector<GameStateID> GameStateIDMap;
		typedef std::vector<GameStateIDMap> GameStateIDMapList;
		
		typedef GameStateID StateFactID;
		typedef std::vector<StateFactID> StateFacts;
		typedef __gnu_cxx::hash_map<cadiaplayer::play::StateFactID, cadiaplayer::play::StateFactID, cadiaplayer::utils::LongLongHash> StateFactMap;
		typedef __gnu_cxx::hash_map<cadiaplayer::play::StateFactID, cadiaplayer::play::parsing::Compound*, cadiaplayer::utils::LongLongHash> CompoundFactMap;
		
		struct HistoryHeuristic
		{
			unsigned int n;
			double q;
			std::string action;
		};
		typedef std::vector<HistoryHeuristic> HistoryHeuristics;
		typedef std::vector<HistoryHeuristics> AllHistoryHeuristics;

		typedef __gnu_cxx::hash_map<cadiaplayer::play::GameStateID, cadiaplayer::play::parsing::Compound, cadiaplayer::utils::LongLongHash> CompoundStore;
		typedef CompoundStore::iterator CompoundStoreItr;
		
		typedef __gnu_cxx::hash_map<cadiaplayer::play::GameStateID, cadiaplayer::play::parsing::Compound*, cadiaplayer::utils::LongLongHash> CompoundPtrStore;
		typedef CompoundPtrStore::iterator CompoundPtrStoreItr;
		
		typedef __gnu_cxx::hash_map<cadiaplayer::play::GameStateID, cadiaplayer::play::Move*, cadiaplayer::utils::LongLongHash> MovePtrStore;
		typedef MovePtrStore::iterator MovePtrStoreItr;
		
		class GameTheory
			{
			private:
				parsing::SymbolID m_counterPredicate;
				//bool m_hashCounterPredicate;
				bool m_hashCounterPredicateDetected;
				parsing::SymbolTable* m_symbols;
				utils::BinaryRelations* m_binaryRelations;
				utils::RelationCounters* m_relationCounters;
				parsing::GameParser* m_parser;
				bool m_rolesDone;
				KBaseList m_rolelist;
				KBaseList m_rules;
				KBaseList m_next;
				KBaseList m_legal;
				KBaseList m_goals;
				KBaseList m_terminals;
				std::vector<int> m_goalValues;
				GameLine m_gameline;
				ParentLine m_parentLine;
				Roles m_roles;
				std::size_t m_mark;
				cadiaplayer::logic::GameController* m_controller;
				StateMovesLine m_statemovesline;
				std::string m_kifFile;
				//GameStateIDMap gameStateIdMap;
				GameStateIDMapList m_gameStateIdMapList;
				unsigned long long m_hasSeenSalt;
				parsing::SymbolID m_hasSeenSymbolID;
				CompoundStore m_compoundStore;

				bool m_createSharedMemory;
			public:
				GameTheory();
				~GameTheory();
				
				bool initTheory(cadiaplayer::play::parsing::GameParser* parser, std::string filename, std::string externalpath = "");
				
				// Functions for determining if the game step counter is used in the GameStateID hash keys.
				void detectCounterPredicate();
				/*void enableCounterHashing(){m_hashCounterPredicate = true;};
				 void disableCounterHashing(){m_hashCounterPredicate = false;};
				 bool counterHashed(){return m_hashCounterPredicate;};*/
				parsing::SymbolID getCounterPredicate(){return m_counterPredicate;};
				
				
				parsing::SymbolTable* getSymbols(void){return m_symbols;};
				utils::BinaryRelations* getBinaryRelations(void){return m_binaryRelations;};
				utils::RelationCounters* getRelationCounters(void){return m_relationCounters;};
				
				cadiaplayer::play::parsing::GameParser* getParser();
				void setParser(cadiaplayer::play::parsing::GameParser* p);
				
				void storeRole(parsing::Compound* c);
				void storeInit(parsing::Compound* c);
				void storeRule(parsing::Compound* c);
				void storeNext(parsing::Compound* c);
				void storeLegal(parsing::Compound* c);
				void storeGoal(parsing::Compound* c);
				void storeTerminal(parsing::Compound* c);
				void getGoalValues(std::vector<int>& g);
				
				void useController(cadiaplayer::logic::GameController* controller);
				
				void ask(parsing::Compound* c, bool interupt, KBaseList& answer);
				size_t solutionCardinality(parsing::SymbolID predicate, parsing::SymbolID atom, size_t pos, size_t size);
				
				GameState getState(void);
				GameState* getStateRef(void);
				GameState* getParentState(void);
				GameState getStateForced(void);
				Roles* getRoles(void){return &m_roles;};
				Role getRole(RoleIndex r){return m_roles[r];};
				RoleIndex getRoleIndex(Role r){for(size_t n = 0 ; n < m_roles.size() ; ++n){if(m_roles[n] == r)return n;}return 0;};
				KBaseList* getRules(void){return &m_rules;};
				KBaseList* getNexts(void){return &m_next;};
				KBaseList* getLegals(void){return &m_legal;};
				KBaseList* getGoals(void){return &m_goals;};
				KBaseList* getTerminals(void){return &m_terminals;};
				
				// Get available moves in the current state
				RoleMoves*	getMoves(RoleIndex r);  // per role
				void		getMoves(std::vector<std::vector<Move*>*>& moves); // for all roles
				// Prepare inner structure for buffering all move branches of the current state
				// to speed up MiniMax like algorithms
				void prepareMoves();
				// Discard buffered moves at the current depth
				void retractMoves();
				
				// Make a move in the current state for a given role
				// Other roles are assigned a random move.
				void make(RoleIndex role, Move* m);
				// Same as above, but ignore mute settings.
				void makeByForce(RoleIndex role, Move* m);
				// Make moves for some or all roles in the current state
				void make(PlayMoves& moves);
				// Make a move without deleting any old predicates from the game state 
				void makeRelaxed(PlayMoves& moves);
				// Prove the next game state with no moves supplied (exposes constants).  Automatically retracted
				GameState makeNullMove(void);
				// Prove the next game state with only one move supplied.  Automatically retracted
				GameState makeExploratory(RoleIndex role, Move* m);
				// Place an empty state on the gameline to keep depth measures in sync 
				// when an outside class is managing state transitions.
				void makeEmpty(){GameState s; m_gameline.push(s);};
				// Make all moves available with a single role without deleting any old predicates from the game state 
				void makeAllRelaxed(RoleIndex role);
				// Check next state without asserting it.  
				void peekNextState(RoleIndex role, Move* m, GameState& state);	
				void peekNextState(PlayMoves& moves, GameState& state);
				// Check effects+ without asserting anything. Stores the effects+ set if it needs to be asserted later.
				// User is responsible for calling discardEffectsPlus() to clear stored sets.
				parsing::CompoundList effectsPlus(RoleIndex role, Move* m);	
				parsing::CompoundList effectsPlus(PlayMoves& moves);
				// Check effects- without asserting anything.
				parsing::CompoundList effectsMinus(RoleIndex role, Move* m);	
				parsing::CompoundList effectsMinus(PlayMoves& moves);
				// Commits all stored effect+ sets and pushes the new state onto the gamestack.
				void commitEffectsPlus();
				// Discards all currently stored effect+ states.
				void discardEffectsPlus();
				// Store current state for later use (for A*).  Use GameStateID returned to reference later.  
				// Deletion from storage is the responsibility of the programmer by calling deleteStoredState(GameStateID).
				GameStateID storeCurrentState(void);
				// Jump to a previously stored state by GameStateID.  The stored state is pushed onto the game stack. 
				// Does not remove the state from storage.
				bool jumpToStoredState(GameStateID sid);
				// Delete a previously stored state by GameStateID.
				void deleteStoredState(GameStateID sid);
				// Retract the current state
				void retract(void);
				// Retract the current state even if muted
				void retractByForce( void );
				// Turn off retraction code of controller to save CPU time
				void muteRetract(void);
				// Re-enable retraction in the controller. Should by done with the gameline 
				// exactly like it was when muteRetract was called.
				void syncRetract(void);
				// Retract all moves (go back to the initial state)
				void retractAll(void);
				// Remember the current depth in the game stack (one marker allowed)
				void markGameState(void);
				// Retract to the marker set when markGameState was called last.
				void retractToMark(void);
				
				// Bactrack everything to the init state.
				bool gotoStart();
				// Rebuild and assert gamestate represented by enumeration values of the predicates describing the state.
				bool gotoState(const StateFacts& state, bool skipController=false);
				bool gotoStateByForce(const StateFacts& state);
				bool gotoStringState(std::string state);
				// Push the current state onto the controller -- use with caution --
				void syncController(StateFacts& stateFacts);
				// Get an predicate enumeration of a gamestate which can be used to rebuild it by calling gotoState.
				StateFactID getStateFact(cadiaplayer::play::parsing::Compound* comp);
				void getStateFacts(StateFacts& stateFacts);
				void getStateFacts(const GameState& state, StateFacts& stateFacts);
				std::string stateFactToString(const StateFactID& stateFact);
				std::string stateFactsToString(StateFacts& stateFacts);
				// Assert one statefact into the current state.
				void assertStateFact(const StateFactID& stateFact);
				// Retract one statefact from the current state.
				void retractStateFact(const StateFactID& stateFact);
				// Recalculate the fact id when enclosed in a has_seen predicate.
				StateFactID getHasSeenID(const StateFactID& stateFact);
				bool isHasSeen(parsing::Compound* c);
				
				// Get the current round according to the gameline stack
				unsigned int getRound(void);
				// Check if the game is at an terminal state
				bool isTerminal(void);
				// Call game termination processing
				void terminate(void);
				// Check if there is a goal available for a certain role in the current state.
				// returns GOAL_NOT if no goal available.
				double goal(RoleIndex role);
				// Checks if there are more than one goal as long as the next goal is higher than the previous
				double incrementingGoal(RoleIndex role);
				// Checks if there are more than one goal and returns the highest one
				double topGoal(RoleIndex role);
				// Checks if the player has reached a specific goal (e.g. 100)
				bool reachedGoal(RoleIndex role, int goal);
				// Check if a predicate/compound is true in the current state (only for state predicates)
				bool isTrue(parsing::Compound* c);
				
				// Calculate an unique hash id for a compound (non-reversible)
				GameStateID getGameStateID(parsing::Compound* c, std::size_t parampos=0);
				// Calculate an unique hash id for a move compound (non-reversible)
				GameStateID getGameStateID(Move* m);
				// Calculate an unique hash id for a gamestate (non-reversible)
				GameStateID getGameStateID(GameState* s);
				// Calculate an unique hash id for a gamestate wo/counter predicate if any (non-reversible)
				GameStateID getGameStateIDNoCounter(GameState* s);
				// Build a map of Zobrist keys to help generate the unique hash ids 
				void buildKeymap(void);
				
				// Generate a random move for a player
				Move* generateRandomMove(RoleIndex role);
				// Generate a random move for all but one player and collect in pm
				void generateRandomMove(RoleIndex role, Move* move, PlayMoves& pm);
				// Generate random moves for all players and collect in pm
				void generateRandomMove(PlayMoves& pm);
				// Play a random move for all players
				void playRandomMove(void);
				// Play a random move for all but one player and collect in pm
				void playRandomMove(RoleIndex role, Move* move, PlayMoves& pm);
				// Generate and play random moves for all players (loaded into pm parameter)
				void playRandomMove(PlayMoves& pm);
				
				// Random slave control
				int randomSlave(std::vector<std::string>& roles, std::vector<double>& goals);
				
				// Generate a psudo state from a predicate and score it as if were a terminal and return the difference in goal values
				// with paranoid assumption from the perspective of a specific player.
				double getPredicateGain(cadiaplayer::play::parsing::Compound* c, RoleIndex player);
								
				// Info functions
				std::string getKifFile(void){return m_kifFile;};
				void setKifFile(std::string kif){m_kifFile = kif;};
				std::string info(void);
				std::string playInfo(PlayMoves* moves);
				std::string roleInfo(RoleIndex r, RoleMoves* moves);
				
				// returns ADVERSARY_NOT if false, roleindex of first player to move else.
				// noop1 and noop2 are initialized with passive moves when not players turn
				//size_t isAdversaryGame(Move& noop1, Move& noop2);
				size_t getAdversary(std::string myRole);

			private:
				void addNewState(GameState state);
				void implodeRelationImplication(parsing::Compound& c);
				parsing::Compound* implodeRelationRole(parsing::Compound* c);
				std::string kbaseListToString(KBaseList* kb);
				//Move checkForNoop(unsigned int roleIndex);
				
			public:
				static std::string movesToString(RoleMoves* moves, parsing::SymbolTable* symbols)
				{
					std::stringstream str;
					RoleMoves::iterator it = moves->begin();
					for(;it != moves->end();it++)
					{
						str << (*it)->compound.toString(symbols) << std::endl;
					}
					return str.str();
				};
				static std::string listToString(KBaseList* list, parsing::SymbolTable* symbols)
				{
					std::stringstream str;
					KBaseListItr it = list->begin();
					int count = 0;
					for(;it != list->end();it++)
					{
						str << (++count) << ":";
						str << (*it)->toString(symbols) << std::endl;
					}
					return str.str();
				};
				static void deleteStateCompounds(KBaseList& list)
				{
					for(size_t n = 0 ; n < list.size() ; n++ )
					{
						delete list[n];
					}
				};
			};
		
		struct moveCompareLessThan
		{
			bool operator()(const Move& m1, const Move& m2) const
			{
				return (m1.compound == m2.compound) < 0;
			}
		};
		
		struct GameStateIDCompare
		{
			bool operator()(GameStateID id1, GameStateID id2)
			{
				return id1 < id2;
			}
		};
		
	}} // namespaces
#endif // GAMETHEORY_H
