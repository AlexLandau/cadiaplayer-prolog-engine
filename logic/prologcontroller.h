#ifndef PROLOGCONTROLLER_H
#define PROLOGCONTROLLER_H

#include "gamecontroller.h"
#include "Yap/YapInterface.h"
#include "prologutils.h"
#include <stack>
#include <fstream>
#include "utils/domainanalyzer.h"

#define USE_YAP_FAST_INIT

#define GDL_TO_PLSTATE_FOLDER "games/ggp/"
#define GDL_TO_PL_CONVERSION_PROGRAM "./kif2ggp"
#define GDL_TO_PL_EXTENSION ".ggp"

namespace cadiaplayer {
	namespace logic {
		
		// Move SymbolID and StateFactID classes to this namespace
		typedef cadiaplayer::play::parsing::SymbolID SymbolID;
		typedef cadiaplayer::play::StateFactID StateFactID;
		typedef cadiaplayer::play::StateFacts StateFacts;
		
		typedef __gnu_cxx::hash_map<cadiaplayer::play::GameStateID, utils::SRetract*, cadiaplayer::utils::LongLongHash> PrologStateStore;
		typedef __gnu_cxx::hash_map<cadiaplayer::play::GameStateID, std::string, cadiaplayer::utils::LongLongHash> PrologPredicateStore;
		
		class PrologController : public GameController
			{
			private:
				std::string prologfile;
				utils::SRetract* initInfo;
				cadiaplayer::play::GameState initState;
				std::stack<utils::SRetract*> retractInfos;
				std::stack<utils::SRetract*> effectInfos;
				utils::SRetract* syncInfo;
				PrologStateStore stateStore;
				PrologPredicateStore predicateStore;
				utils::FactEnumeration* factEnumeration;
				
			public:
				PrologController(){factEnumeration = NULL;};
				~PrologController();
				
				virtual bool init( cadiaplayer::play::GameTheory* gametheory, std::string gamename );
				bool proveInitState();
				virtual bool isTerminal( );
				
				virtual void getMoves( cadiaplayer::play::RoleIndex role, cadiaplayer::play::RoleMoves& moves );
				
				virtual void getInitialState(cadiaplayer::play::GameState& state);
				virtual void getCurrentState(cadiaplayer::play::GameState& state);
				
				virtual void make(cadiaplayer::play::PlayMoves& moves);
				virtual void makeRelaxed(cadiaplayer::play::PlayMoves& moves);
				virtual void makeNullMove( cadiaplayer::play::GameState& state );
				virtual void makeExploratory( cadiaplayer::play::RoleIndex role, cadiaplayer::play::Move* move, cadiaplayer::play::GameState& state );
				
				virtual void peekNextState(cadiaplayer::play::PlayMoves& moves, cadiaplayer::play::GameState& nextState);
				virtual void effectsPlus(cadiaplayer::play::PlayMoves& moves, cadiaplayer::play::parsing::CompoundList& effects);
				virtual void effectsMinus(cadiaplayer::play::PlayMoves& moves, cadiaplayer::play::parsing::CompoundList& effects);
				virtual void commitEffectsPlus();
				void addToState(utils::SRetract* p);
				void removeDuplicates();
				virtual void discardEffectsPlus();
				virtual void storeCurrentStateAs(cadiaplayer::play::GameStateID sid);
				virtual void jumpToStoredState(cadiaplayer::play::GameStateID sid);
				virtual void deleteStoredState(cadiaplayer::play::GameStateID sid);
				
				virtual void retract( void );
				void retract( utils::SRetract* ri );
				virtual void retractByForce( void );
				virtual void muteRetract( void );
				virtual void syncRetract( void );
				
			private:
				void initGotoState();
				cadiaplayer::play::StateFactID getFact(cadiaplayer::play::parsing::Compound* comp, std::stringstream& istr);
				void getState(const cadiaplayer::play::StateFacts& state, std::stringstream& istr);
			public:
				virtual void gotoStart();
				virtual void gotoState(const cadiaplayer::play::StateFacts& state);
				virtual void gotoStateByForce(const cadiaplayer::play::StateFacts& state);
				virtual void syncState(const cadiaplayer::play::StateFacts& state);
				virtual cadiaplayer::play::StateFactID getStateFact(cadiaplayer::play::parsing::Compound* comp);
				virtual std::string stateFactToString(const StateFactID& stateFact);
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
			private:
				utils::SRetract* getRetractInfo(  int attempt=0 );
				void loadStateInfo( cadiaplayer::play::GameState& state );
				cadiaplayer::play::parsing::Compound* buildMove(cadiaplayer::play::RoleIndex role, YAP_Term action);
				cadiaplayer::play::parsing::Compound* buildCompound(YAP_Term term);
				// Create a YAP_Term from this move
				YAP_Term makeTerm(cadiaplayer::play::Move* move, cadiaplayer::play::parsing::SymbolTable* s);
				// Create a YAP_Term from this compound
				YAP_Term makeTerm(cadiaplayer::play::parsing::Compound* move, cadiaplayer::play::parsing::SymbolTable* s);
				inline std::string termToString(YAP_Term term)
				{
					char buf[4096];
					YAP_WriteBuffer(term, buf, 4096, 0);
					return std::string(buf);
				};
#ifndef USE_YAP_FAST_INIT
				static char YAP_INIT_ARG1[] = {'y','a','p','\0'};
				static char YAP_INIT_ARG2[] = {'\0'};
#endif
				
			}; 
	}} // namespaces
#endif // PROLOGCONTROLLER_H
