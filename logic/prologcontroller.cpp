/*
 *  Built on :
 *  SState.cc
 *  GGP
 *
 *  Created by Yngvi Bjornsson on 3/18/07.
 *  Copyright 2007 CADIA Reykjavík University. All rights reserved.
 *
 */
#include "prologcontroller.h"
using namespace cadiaplayer::logic;
using namespace cadiaplayer::logic::utils;
using namespace cadiaplayer::play;
using namespace cadiaplayer::play::parsing;

PrologController::~PrologController()
{
	// Free up memory used by strored states
	PrologStateStore::iterator it = stateStore.begin();
	while(it!=stateStore.end())
	{
		delete it->second;
		it++;
	}
	stateStore.clear();
	
	// Free up memory reseved for calls buffering effect+ predicate sets.
	discardEffectsPlus();
	
	// Free up memory used for enumerating possible state predicates
	if(factEnumeration != NULL)
		delete factEnumeration;
	
	if(syncInfo != NULL)
		delete syncInfo;
	if(initInfo != NULL)
		delete initInfo;
}
bool PrologController::init( GameTheory* gametheory, std::string gamename)
{
	GameController::init(gametheory, gamename);
	
	syncInfo = NULL;
	initInfo = NULL;
	
	// Convert kif to prolog
	std::string command = GDL_TO_PL_CONVERSION_PROGRAM;
	command += " ";
	command += getGamename();
	
	// Run the conversion program
	int result = system(command.c_str());
	if(result < 0)
		std::cerr << "Warning: system(" << command << ") returned " << result << std::endl;
	
	// Build the prolog filename
	prologfile = GDL_TO_PLSTATE_FOLDER;
	prologfile += getGamename();
	prologfile += GDL_TO_PL_EXTENSION;
	try
	{
#ifdef USE_YAP_FAST_INIT
		// Init YAP with the new game description
		if ( YAP_FastInit( prologfile.c_str() ) == YAP_BOOT_ERROR ) 
		{
			return false;
		}
#else
		YAP_init_args initArgs;
		char savestate[4096];
		strcpy(savestate, prologfile.c_str());
		initArgs.SavedState = savestate;
		initArgs.HeapSize = 0;
		initArgs.StackSize = 0;
		initArgs.TrailSize = 0;
		initArgs.NumberWorkers = 0;
		initArgs.SchedulerLoop = 0;
		initArgs.DelayedReleaseLoad = 0;
		initArgs.Argc = 1;
		char* argv[] = {YAP_INIT_ARG1,YAP_INIT_ARG2};
		initArgs.Argv = argv;
		// For consulting/read only
		//initArgs.ErrorNo (int)
		//initArgs.ErrorCause	(char *)
		// Init YAP with the new game description
		if ( YAP_Init( &initArgs ) == YAP_BOOT_ERROR ) 
		{
			return false;
		}
#endif
	}
	catch(...)
	{
		return false;
	}
    YAP_Reset();
	
	// Workaround - Yap seems to fail sometimes if the first thing asked is getMoves
	this->isTerminal();
	
	initGotoState();
	proveInitState();
	
	initInfo = getRetractInfo();
	initState.clear();
	getCurrentState(initState);
	
    return true;
}
bool PrologController::proveInitState()
{
	// Prolog call: ?- state_initialize.
    int okay = YAP_RunGoal( YAP_MkAtomTerm( YAP_LookupAtom("state_initialize")  ) ) != 0;
    YAP_Reset();   
    return okay;
}
void PrologController::getInitialState(cadiaplayer::play::GameState& state)
{
	state.insert(state.end(), initState.begin(), initState.end());
}
void PrologController::getCurrentState(cadiaplayer::play::GameState& state)
{
	loadStateInfo(state);
}
bool PrologController::isTerminal( )
{
	// Prolog call: ?- is_terminal( L ).
    int okay = YAP_RunGoal( YAP_MkAtomTerm( YAP_LookupAtom("state_is_terminal")  ) ) != 0;
    YAP_Reset();   
    return okay;
}

//#define DEBUG_YAP_NULL_MOVE
void PrologController::getMoves( RoleIndex role, RoleMoves& moves )
{
	//std::cerr << "Calling: getMoves( RoleIndex role, RoleMoves& m )" << std::endl;
	// Prolog call: ?- state_gen_moves( P, M ).
    static YAP_Functor yapFnct = YAP_MkFunctor( YAP_LookupAtom("state_gen_moves"), 2 );
	//static YAP_Functor yapFnct = YAP_MkFunctor( YAP_LookupAtom("legal"), 2 );

    std::vector<YAP_Term> args;
	YAP_Term player = YAP_MkAtomTerm(YAP_LookupAtom(getTheory()->getRole(role).c_str()));
	args.push_back(player);
	args.push_back(YAP_MkVarTerm());
		
	YAP_Term yapGoal = YAP_MkApplTerm( yapFnct, 2, &args[0] );
	long slot = YAP_InitSlot(yapGoal);
    unsigned int okay = YAP_RunGoal( yapGoal );
	Compound* c = NULL;
    YAP_Term resAction;
#ifdef DEBUG_YAP_NULL_MOVE
	std::stringstream ssMoves;
	bool failMove = false;
#endif
	while ( okay != 0 ) 
	{
        yapGoal = YAP_GetFromSlot(slot);
		resAction = YAP_ArgOfTerm( 2, yapGoal );
		c = buildCompound(resAction);
		if(c==NULL) // || !(clock()%50))
		{
			std::cerr << "NULL move" << std::endl;
#ifdef DEBUG_YAP_NULL_MOVE
			char nulbuf[4096];
			YAP_WriteBuffer(resAction, nulbuf, 4096, 0);
			ssMoves << "Move failed to generate : " << nulbuf << std::endl;
			//std::cerr << ssMoves.str();
			failMove = true;
#else
			break;
#endif
		}
#ifdef DEBUG_YAP_NULL_MOVE
		else
		{
			char nulbuf[4096];
			YAP_WriteBuffer(resAction, nulbuf, 4096, 0);
			ssMoves << "Move generated successfully : " << nulbuf << std::endl;
		}
#endif
		
		moves.push_back(new Move(*c));
		delete c;
		okay = YAP_RestartGoal();
		
		//char buf[4096];
		//YAP_WriteBuffer(resAction, buf, 4096, 0);
		//std::cerr << "Move generated : " << buf << std::endl;
    }
    
    YAP_Reset();
	YAP_RecoverSlots(1);
	if(moves.empty())
	{
		std::cerr << "Empty Movelist generated for" << std::endl;
		SRetract* infoRI = getRetractInfo();
		if(infoRI == NULL)
			std::cerr << "No statefacts in knowledge base" << std::endl;
		else
		{
			std::cerr << infoRI->getSTerm().getStr() << std::endl;
			delete infoRI;
		}
    }
	//std::cerr << "Ending: getMoves( RoleIndex role, RoleMoves& m )" << std::endl;
#ifdef DEBUG_YAP_NULL_MOVE
	if(failMove)
	{
		GameState state;
		getCurrentState(state);
		ssMoves << "Simulation path size : " << getTheory()->getRound() << std::endl;
		ssMoves << "Current state is : " << std::endl << GameTheory::listToString(&state, getTheory()->getSymbols()); 
		
		std::cerr << "FAIL!" << ssMoves.str();
	}
	//else
	//	std::cerr << "OK!" << ssMoves.str();
#endif
}

//#define PROLOGCONTROLLER_DEBUG_MAKE
void PrologController::make(PlayMoves& moves)
{
#ifdef PROLOGCONTROLLER_DEBUG_MAKE
	std::cerr << getRetractInfo()->getSTerm().getStr() << std::endl;
#endif
	// Store state for retraction
	if(!isMute())
		retractInfos.push(getRetractInfo());
	
	// Prolog call: ?- state_make_sim_moves( [ [<p>,<m>], ... ] ).
    static YAP_Functor fnct = YAP_MkFunctor( YAP_LookupAtom("state_make_sim_moves"), 1 );
	
	int okay;
	std::vector<YAP_Term> args;
	unsigned long int arity = 0;
	for(size_t n = 0 ; n < moves.size() ; ++n)
	{
		YAP_Term player = YAP_MkAtomTerm(YAP_LookupAtom(getTheory()->getRole(n).c_str()));
		
		YAP_Term move = makeTerm(moves[n], getTheory()->getSymbols());
		args.push_back(YAP_MkPairTerm(player, YAP_MkPairTerm(move, YAP_MkAtomTerm( YAP_FullLookupAtom( "[]" )))));
		++arity;
	}
	if(arity)
	{
		YAP_Term arg = YAP_MkAtomTerm( YAP_FullLookupAtom( "[]" ) );
		for ( int n = static_cast<int>(arity-1) ; n >= 0; --n ) 
		{
			arg = YAP_MkPairTerm( args[n], arg);
		}
		YAP_Term yapGoal = YAP_MkApplTerm( fnct, 1, &arg );
#ifdef PROLOGCONTROLLER_DEBUG_MAKE	
		char buf[4096];
		YAP_WriteBuffer(yapGoal, buf, 4096, 0);
		std::cerr << "Make generated : " << buf << std::endl;
#endif
		
    	okay = YAP_RunGoal( yapGoal );
    
		YAP_Reset();
	}
#ifdef PROLOGCONTROLLER_DEBUG_MAKE
	std::cerr << "Asserting to : " << getRetractInfo()->getSTerm().getStr() << std::endl;
#endif
	
	// Old return type
    //GameState state;
	//loadStateInfo(state);
    //return state;
}
void PrologController::makeRelaxed(PlayMoves& moves)
{
	// Store state for retraction
	if(!isMute())
		retractInfos.push(getRetractInfo());
	
	// Prolog call: ?- state_make_sim_moves( [ [<p>,<m>], ... ] ).
    static YAP_Functor fnct = YAP_MkFunctor( YAP_LookupAtom("state_make_rel_moves"), 1 );

	int okay;
	std::vector<YAP_Term> args;
	unsigned long int arity = 0;
	for(size_t n = 0 ; n < moves.size() ; ++n)
	{
		YAP_Term player = YAP_MkAtomTerm(YAP_LookupAtom(getTheory()->getRole(n).c_str()));
		YAP_Term move = makeTerm(moves[n], getTheory()->getSymbols());
		args.push_back(YAP_MkPairTerm(player, YAP_MkPairTerm(move, YAP_MkAtomTerm( YAP_FullLookupAtom( "[]" )))));
		++arity;
	}
	if(arity)
	{
		YAP_Term arg = YAP_MkAtomTerm( YAP_FullLookupAtom( "[]" ) );
		for ( int n = static_cast<int>(arity-1) ; n >= 0; --n ) 
		{
			arg = YAP_MkPairTerm( args[n], arg);
		}
		YAP_Term yapGoal = YAP_MkApplTerm( fnct, 1, &arg );
    	okay = YAP_RunGoal( yapGoal );
    
		YAP_Reset();
	}
}

void PrologController::makeNullMove( cadiaplayer::play::GameState& state )
{
	// Store state for retraction
	retractInfos.push(getRetractInfo());
	
	// Prolog call: ?- state_make_sim_moves( [ [<p>,<m>], ... ] ).
    static YAP_Functor fnct = YAP_MkFunctor( YAP_LookupAtom("state_make_sim_moves"), 1 );

	int okay;
	std::vector<YAP_Term> args;
	
	YAP_Term arg = YAP_MkAtomTerm( YAP_FullLookupAtom( "[]" ) );
	
	YAP_Term yapGoal = YAP_MkApplTerm( fnct, 1, &arg );
	okay = YAP_RunGoal( yapGoal );
    
	YAP_Reset();
	
	loadStateInfo(state);
	// Back up as exploratory state has been extracted
	retract();
}

void PrologController::makeExploratory( RoleIndex role, Move* move, cadiaplayer::play::GameState& state )
{
	// Store state for retraction
	retractInfos.push(getRetractInfo());
	
	// Prolog call: ?- state_make_sim_moves( [ [<p>,<m>], ... ] ).
    static YAP_Functor fnct = YAP_MkFunctor( YAP_LookupAtom("state_make_sim_moves"), 1 );

	int okay;
	std::vector<YAP_Term> args;
	
	YAP_Term player = YAP_MkAtomTerm(YAP_LookupAtom(getTheory()->getRole(role).c_str()));
		
	YAP_Term yap_move = makeTerm(move, getTheory()->getSymbols());
	args.push_back(YAP_MkPairTerm(player, YAP_MkPairTerm(yap_move, YAP_MkAtomTerm( YAP_FullLookupAtom( "[]" )))));
	
	YAP_Term arg = YAP_MkAtomTerm( YAP_FullLookupAtom( "[]" ) );
	arg = YAP_MkPairTerm( args[0], arg);
	
	YAP_Term yapGoal = YAP_MkApplTerm( fnct, 1, &arg );
	okay = YAP_RunGoal( yapGoal );
    
	YAP_Reset();
	
	loadStateInfo(state);
	// Back up as exploratory state has been extracted
	retract();
}

void PrologController::peekNextState(PlayMoves& moves, GameState& nextState)
{
	
	// Prolog call: ?- peek_next_state( ML, SL ).
    static YAP_Functor fnct = YAP_MkFunctor( YAP_LookupAtom("state_peek_next"), 2 );

	int okay;
	std::vector<YAP_Term> args;
	unsigned long int arity = 0;
	for(size_t n = 0 ; n < moves.size() ; ++n)
	{
		YAP_Term player = YAP_MkAtomTerm(YAP_LookupAtom(getTheory()->getRole(n).c_str()));
		
		YAP_Term move = makeTerm(moves[n], getTheory()->getSymbols());
		args.push_back(YAP_MkPairTerm(player, YAP_MkPairTerm(move, YAP_MkAtomTerm( YAP_FullLookupAtom( "[]" )))));
		++arity;
	}
	if(arity)
	{
		YAP_Term arg = YAP_MkAtomTerm( YAP_FullLookupAtom( "[]" ) );
		for ( int n = static_cast<int>(arity-1) ; n >= 0; --n ) 
		{
			arg = YAP_MkPairTerm( args[n], arg);
		}
		args.clear();
		args.push_back(arg);
		args.push_back(YAP_MkVarTerm());
		YAP_Term yapGoal = YAP_MkApplTerm( fnct, 2, &args[0] );
    	long slot = YAP_InitSlot( yapGoal );
		okay = YAP_RunGoal( yapGoal );
		yapGoal = YAP_GetFromSlot(slot);
		if(okay)
		{
			YAP_Term head = YAP_ArgOfTerm( 2, yapGoal );
			okay = YAP_IsPairTerm(head);
			while( okay != 0 ) 
			{
				YAP_Term item = YAP_HeadOfTerm( head );
				nextState.push_back(buildCompound( item ));
				head = YAP_TailOfTerm( head );
				okay = YAP_IsPairTerm(head);
			}
		}
		YAP_Reset();
		YAP_RecoverSlots(1);
	}
}
void PrologController::effectsPlus(PlayMoves& moves, CompoundList& effects)
{
	// Prolog call: ?- get_effects_plus( ML, PS ).
    static YAP_Functor fnct = YAP_MkFunctor( YAP_LookupAtom("state_effects_plus"), 2 );

	int okay;
	std::vector<YAP_Term> args;
	unsigned long int arity = 0;
	for(size_t n = 0 ; n < moves.size() ; ++n)
	{
		YAP_Term player = YAP_MkAtomTerm(YAP_LookupAtom(getTheory()->getRole(n).c_str()));
		
		YAP_Term move = makeTerm(moves[n], getTheory()->getSymbols());
		args.push_back(YAP_MkPairTerm(player, YAP_MkPairTerm(move, YAP_MkAtomTerm( YAP_FullLookupAtom( "[]" )))));
		++arity;
	}
	if(arity)
	{
		YAP_Term arg = YAP_MkAtomTerm( YAP_FullLookupAtom( "[]" ) );
		for ( int n = static_cast<int>(arity-1) ; n >= 0; --n ) 
		{
			arg = YAP_MkPairTerm( args[n], arg);
		}
		args.clear();
		args.push_back(arg);
		args.push_back(YAP_MkVarTerm());
		
		YAP_Term yapGoal = YAP_MkApplTerm( fnct, 2, &args[0] );
		long slot = YAP_InitSlot( yapGoal );
		okay = YAP_RunGoal( yapGoal );
		yapGoal = YAP_GetFromSlot(slot);
		if(okay)
		{
			// collect effects
			YAP_Term head = YAP_ArgOfTerm( 2, yapGoal );
			okay = YAP_IsPairTerm(head);
			if( okay != 0 )
				effectInfos.push(new SRetract( STerm( YAP_ArgOfTerm( 2, yapGoal ) ) ));
			while( okay != 0 ) 
			{
				YAP_Term item = YAP_HeadOfTerm( head );
				effects.push_back(buildCompound( item ));
				head = YAP_TailOfTerm( head );
				okay = YAP_IsPairTerm(head);
			}
		}
		YAP_Reset();
		YAP_RecoverSlots(1);
	}
}
void PrologController::effectsMinus(PlayMoves& moves, CompoundList& effects)
{
	// Prolog call: ?- get_effects_minus( ML, MS ).
    static YAP_Functor fnct = YAP_MkFunctor( YAP_LookupAtom("state_effects_minus"), 2 );

	int okay;
	std::vector<YAP_Term> args;
	unsigned long int arity = 0;
	for(size_t n = 0 ; n < moves.size() ; ++n)
	{
		YAP_Term player = YAP_MkAtomTerm(YAP_LookupAtom(getTheory()->getRole(n).c_str()));
		
		YAP_Term move = makeTerm(moves[n], getTheory()->getSymbols());
		args.push_back(YAP_MkPairTerm(player, YAP_MkPairTerm(move, YAP_MkAtomTerm( YAP_FullLookupAtom( "[]" )))));
		++arity;
	}
	if(arity)
	{
		YAP_Term arg = YAP_MkAtomTerm( YAP_FullLookupAtom( "[]" ) );
		for ( int n = static_cast<int>(arity-1) ; n >= 0; --n ) 
		{
			arg = YAP_MkPairTerm( args[n], arg);
		}
		args.clear();
		args.push_back(arg);
		args.push_back(YAP_MkVarTerm());
		YAP_Term yapGoal = YAP_MkApplTerm( fnct, 2, &args[0] );
    	long slot = YAP_InitSlot( yapGoal );
		okay = YAP_RunGoal( yapGoal );
		yapGoal = YAP_GetFromSlot(slot);
		if(okay)
		{
			YAP_Term head = YAP_ArgOfTerm( 2, yapGoal );
			okay = YAP_IsPairTerm(head);
			while( okay != 0 ) 
			{
				YAP_Term item = YAP_HeadOfTerm( head );
				effects.push_back(buildCompound( item ));
				head = YAP_TailOfTerm( head );
				okay = YAP_IsPairTerm(head);
			}
		}
		YAP_Reset();
		YAP_RecoverSlots(1);
	}
}
void PrologController::commitEffectsPlus()
{
	// Store state for retraction
	if(!isMute())
		retractInfos.push(getRetractInfo());
	SRetract* temp;
	while(!effectInfos.empty())
	{
		temp = effectInfos.top();
		effectInfos.pop();
		addToState(temp);
		delete temp;
	}
	removeDuplicates();
}
void PrologController::addToState(SRetract* p)
{
	if(p == NULL)
		return;
	// Prolog call: ?- state_append_move( <clause_list> ).
    static YAP_Functor yapFnct = YAP_MkFunctor( YAP_LookupAtom("state_append_move"), 1 );

    STerm::YapTermHolder holder( HOLDER_LIMIT );
	YAP_Term argTerm = p->getSTerm().mkYAPTerm( holder );
    YAP_Term yapGoal = YAP_MkApplTerm( yapFnct, 1, &argTerm );
    
    YAP_RunGoal( yapGoal );    
    YAP_Reset();
}
void PrologController::removeDuplicates()
{
	// Prolog call: ?- state_remove_duplicates.
    static YAP_Functor yapFnct = YAP_MkFunctor( YAP_LookupAtom("state_remove_duplicates"), 0 );

     YAP_Term yapGoal = YAP_MkApplTerm( yapFnct, 0, 0 );
    
    YAP_RunGoal( yapGoal );    
    YAP_Reset();
}
void PrologController::discardEffectsPlus()
{
	SRetract* temp;
	while(!effectInfos.empty())
	{
		temp = effectInfos.top();
		effectInfos.pop();
		delete temp;
	}
}

void PrologController::storeCurrentStateAs(GameStateID sid)
{
	PrologStateStore::iterator it = stateStore.find(sid);
	if(it!=stateStore.end())
		return;
	stateStore[sid] = getRetractInfo();
}
void PrologController::jumpToStoredState(GameStateID sid)
{
	PrologStateStore::iterator it = stateStore.find(sid);
	if(it==stateStore.end())
		return;
	SRetract* jump = it->second;
	retractInfos.push(jump);
	retract();
}
void PrologController::deleteStoredState(GameStateID sid)
{
	PrologStateStore::iterator it = stateStore.find(sid);
	if(it==stateStore.end())
		return;
	stateStore.erase(sid);
}


void PrologController::retract( void )
{
	if(isMute() || retractInfos.size() == 0)
		return;
	// Prolog call: ?- state_retract_move( <clause_list> ).
    static YAP_Functor yapFnct = YAP_MkFunctor( YAP_LookupAtom("state_retract_move"), 1 );

    STerm::YapTermHolder holder( HOLDER_LIMIT );
	SRetract* ri = retractInfos.top();
	retractInfos.pop();
    YAP_Term argTerm = ri->getSTerm().mkYAPTerm( holder );
    YAP_Term yapGoal = YAP_MkApplTerm( yapFnct, 1, &argTerm );
    
	YAP_RunGoal( yapGoal );    
    YAP_Reset();
    
	//std::cerr << "Retracting to : " << ri->getSTerm().getStr() << std::endl;
	delete ri;
}
void PrologController::retract( SRetract* ri )
{
	// Prolog call: ?- state_retract_move( <clause_list> ).
    static YAP_Functor yapFnct = YAP_MkFunctor( YAP_LookupAtom("state_retract_move"), 1 );
	
    STerm::YapTermHolder holder( HOLDER_LIMIT );
	YAP_Term argTerm = ri->getSTerm().mkYAPTerm( holder );
    YAP_Term yapGoal = YAP_MkApplTerm( yapFnct, 1, &argTerm );
    
	YAP_RunGoal( yapGoal );    
    YAP_Reset();
}
void PrologController::retractByForce( void )
{
	if(isMute())
	{
		//std::cerr << "Overriding mute with retractinfos at size " << retractInfos.size() << std::endl;
		overrideMute(false);
		retract();
		overrideMute(true);
	}
	else
		retract();
	
}
void PrologController::muteRetract( void )
{
	GameController::muteRetract();
	if(syncInfo != NULL)
		delete syncInfo;
	syncInfo = getRetractInfo();
}
void PrologController::syncRetract( void )
{
	GameController::syncRetract();
	retractInfos.push(syncInfo);
	retract();
	syncInfo = NULL;
}
void PrologController::initGotoState()
{
	// Build enumeration structure for possible state predicates
	//DomainAnalyzer analyzer;
	//PredicateDomains pd = analyzer.makeStateDomains(*t);
	//factEnumeration = analyzer.getFactEnumeration(*t, pd);
}
cadiaplayer::play::StateFactID PrologController::getFact(Compound* comp, std::stringstream& istr)
{
	StateFactID fid = getTheory()->getGameStateID(comp);
	PrologPredicateStore::iterator itr = predicateStore.find(fid);
	if(itr != predicateStore.end())
	{
		istr << itr->second;
		return fid;
	}
	std::stringstream factstr;
	SymbolTableEntry* entry = getTheory()->getSymbols()->lookup(comp->getName());
	if(comp->getArguments() == NULL || comp->getArguments()->size() == 0)
	{
		if(entry->getType() == st_NUM)
			factstr << "( I" << entry->getLexeme() << " )";
		else
			factstr << "( A" << entry->getLexeme() << " )";
		
		predicateStore[fid] = factstr.str();
		istr << factstr.str();
		return fid;
	}
	factstr << "( C " << entry->getLexeme() << " " << comp->getArguments()->size();
	for(size_t n = 0 ; n < comp->getArguments()->size() ; n++)
	{
		entry = getTheory()->getSymbols()->lookup(comp->getArgument(n)->getName());
		if(entry->getType() == st_NUM)
			factstr << " ( I " << entry->getLexeme() << " )";
		else
			factstr << " ( A " << entry->getLexeme() << " )";
	}
	factstr << " ) ";
	predicateStore[fid] = factstr.str();
	istr << factstr.str();
	return fid;
}
void PrologController::getState(const StateFacts& state, std::stringstream& istr)
{
	for(size_t n = 0 ; n < state.size() ; n++)
	{
		if(!state[n])
			continue;
		istr << "( P ";
		istr << predicateStore[state[n] ];
	};
	istr << " ( A [] )";
	for(size_t n = 0 ; n < state.size() ; n++)
	{
		if(!state[n])
			continue;
		istr << ") ";
	}
}
void PrologController::gotoStart()
{
	while(retractInfos.size())
	{
		delete retractInfos.top();
		retractInfos.pop();
	}
	retract(initInfo);
}
void PrologController::gotoState(const StateFacts& state)
{
	syncState(state);
}
void PrologController::gotoStateByForce(const StateFacts& state)
{
	if(isMute())
	{
		overrideMute(false);
		syncState(state);
		overrideMute(true);
	}
	else
		syncState(state);
	
}
void PrologController::syncState(const StateFacts& state)
{
	// Store state for retraction
	if(!isMute())
		retractInfos.push(getRetractInfo());
	std::stringstream ss;
	getState(state, ss);
	SRetract* ri = new SRetract(ss);
	
	static YAP_Functor yapFnct = YAP_MkFunctor( YAP_LookupAtom("state_retract_move"), 1 );
	
	STerm::YapTermHolder holder( HOLDER_LIMIT );
	YAP_Term argTerm = ri->getSTerm().mkYAPTerm( holder );
	YAP_Term yapGoal = YAP_MkApplTerm( yapFnct, 1, &argTerm );
    
	YAP_RunGoal( yapGoal );    
	YAP_Reset();
	
	delete ri;
}
cadiaplayer::play::StateFactID PrologController::getStateFact(cadiaplayer::play::parsing::Compound* comp)
{
	std::stringstream istr;
	return getFact(comp, istr);
}
std::string PrologController::stateFactToString(const StateFactID& stateFact)
{
	PrologPredicateStore::iterator itr = predicateStore.find(stateFact);
	if(itr == predicateStore.end())
		return "n/a";
	return predicateStore[stateFact];
}
// Assert one statefact into the current state.
void PrologController::assertStateFact(const StateFactID& stateFact)
{
	// Store state for retraction
	if(!isMute())
		retractInfos.push(getRetractInfo());
	
	std::stringstream ss;
	ss << predicateStore[stateFact];
	// Prolog call: ?- state_goal( Player, Value ).
    static YAP_Functor   yapFnct = YAP_MkFunctor( YAP_LookupAtom("state_assert_clause"), 1 );
	STerm term(ss);
	STerm::YapTermHolder holder( HOLDER_LIMIT_SMALL );
    YAP_Term clause = term.mkYAPTerm(holder);
	std::vector<YAP_Term> args;
	args.push_back(clause);
	
    YAP_Term yapGoal = YAP_MkApplTerm( yapFnct, 1, &args[0] );
    YAP_RunGoal( yapGoal );
	YAP_Reset();
}
// Retract one statefact from the current state.
void PrologController::retractStateFact(const StateFactID& stateFact)
{
	// Store state for retraction
	if(!isMute())
		retractInfos.push(getRetractInfo());
	
	std::stringstream ss;
	ss << predicateStore[stateFact];
	// Prolog call: ?- state_goal( Player, Value ).
    static YAP_Functor   yapFnct = YAP_MkFunctor( YAP_LookupAtom("state_retract_clause"), 1 );
	STerm term(ss);
	STerm::YapTermHolder holder( HOLDER_LIMIT_SMALL );
    YAP_Term clause = term.mkYAPTerm(holder);
	std::vector<YAP_Term> args;
	args.push_back(clause);
	
    YAP_Term yapGoal = YAP_MkApplTerm( yapFnct, 1, &args[0] );
	YAP_RunGoal( yapGoal );
	YAP_Reset();
}
cadiaplayer::play::StateFactID PrologController::getHasSeenID(const StateFactID& stateFact, unsigned long long salt)
{
	StateFactID hasSeenID = stateFact^salt;
	PrologPredicateStore::iterator itr = predicateStore.find(hasSeenID);
	if(itr != predicateStore.end())
		return hasSeenID;
	std::stringstream hasSeenClause;
	hasSeenClause << "( C " << PREDICATE_HAS_SEEN << " 1 " << predicateStore[stateFact] << " )";
	predicateStore[hasSeenID] = hasSeenClause.str();
	return hasSeenID;
}
double PrologController::goal( RoleIndex role )
{
	// Prolog call: ?- state_goal( Player, Value ).
    static YAP_Functor   yapFnct = YAP_MkFunctor( YAP_LookupAtom("state_goal"), 2 );
    YAP_Term player = YAP_MkAtomTerm(YAP_LookupAtom(getTheory()->getRole(role).c_str()));
	std::vector<YAP_Term> args;
	args.push_back(player);
	args.push_back(YAP_MkVarTerm());
	
    YAP_Term yapGoal = YAP_MkApplTerm( yapFnct, 2, &args[0] );
    long slot = YAP_InitSlot( yapGoal );
	int okay = YAP_RunGoal( yapGoal );
    yapGoal = YAP_GetFromSlot(slot);
	double goal = GOAL_NOT;
    if( okay != 0 ) 
		goal = static_cast<double>(YAP_IntOfTerm(YAP_ArgOfTerm( 2, yapGoal )));
	YAP_Reset();
    YAP_RecoverSlots(1);
	return goal;
}
double PrologController::incGoals( RoleIndex role )
{
	// Prolog call: ?- state_goal( Player, Value ).
    static YAP_Functor   yapFnct = YAP_MkFunctor( YAP_LookupAtom("state_goal"), 2 );
    YAP_Term player = YAP_MkAtomTerm(YAP_LookupAtom(getTheory()->getRole(role).c_str()));
	std::vector<YAP_Term> args;
	args.push_back(player);
	args.push_back(YAP_MkVarTerm());
	
    YAP_Term yapGoal = YAP_MkApplTerm( yapFnct, 2, &args[0] );
    long slot = YAP_InitSlot( yapGoal );
	int okay = YAP_RunGoal( yapGoal );
    yapGoal = YAP_GetFromSlot(slot);
	double value = GOAL_NOT;
    double goal = GOAL_NOT;
    while( okay != 0 ) 
	{
    	goal = static_cast<double>(YAP_IntOfTerm(YAP_ArgOfTerm( 2, yapGoal )));
		if(goal > value)
			value = goal;
		else
			break;
		okay = YAP_RestartGoal();
    }    
    YAP_Reset();
    YAP_RecoverSlots(1);
	return value;
}
double PrologController::allGoals( RoleIndex role )
{
	// Prolog call: ?- state_goal( Player, Value ).
    static YAP_Functor   yapFnct = YAP_MkFunctor( YAP_LookupAtom("state_goal"), 2 );
    YAP_Term player = YAP_MkAtomTerm(YAP_LookupAtom(getTheory()->getRole(role).c_str()));
	std::vector<YAP_Term> args;
	args.push_back(player);
	args.push_back(YAP_MkVarTerm());
	
    YAP_Term yapGoal = YAP_MkApplTerm( yapFnct, 2, &args[0] );
    long slot = YAP_InitSlot( yapGoal );
	int okay = YAP_RunGoal( yapGoal );
    yapGoal = YAP_GetFromSlot(slot);
	double value = GOAL_NOT;
    double goal = GOAL_NOT;
    while( okay != 0 ) 
	{
    	goal = static_cast<double>(YAP_IntOfTerm(YAP_ArgOfTerm( 2, yapGoal )));
		if(goal > value)
			value = goal;
		okay = YAP_RestartGoal();
    }    
    YAP_Reset();
    YAP_RecoverSlots(1);
	return value;
}
bool PrologController::reachedGoal( RoleIndex role, int goal )
{
	// Prolog call: ?- state_goal( Player, Value ).
    static YAP_Functor   yapFnct = YAP_MkFunctor( YAP_LookupAtom("state_goal"), 2 );
    YAP_Term player = YAP_MkAtomTerm(YAP_LookupAtom(getTheory()->getRole(role).c_str()));
	std::vector<YAP_Term> args;
	args.push_back(player);
	args.push_back(YAP_MkIntTerm(goal));
	
    YAP_Term yapGoal = YAP_MkApplTerm( yapFnct, 2, &args[0] );
    int okay = YAP_RunGoal( yapGoal );
    YAP_Reset();
    return okay;
}

void PrologController::ask(Compound* /*c*/, bool /*interupt*/, KBaseList& /*answer*/)
{
	// need to convert arbitrary compounds to YAP_Terms.
}
size_t PrologController::solutionCardinality(std::string predicate, std::string atom, size_t pos, size_t size)
{
	// Prolog call: ?- state( Compound ).
    static YAP_Functor   yapFnct = YAP_MkFunctor( YAP_LookupAtom("state"), 1 );
    std::vector<YAP_Term> stateArgs;
	
	static YAP_Functor   predFnct = YAP_MkFunctor( YAP_LookupAtom(predicate.c_str()), size);
    
	YAP_Term atomTerm = YAP_MkAtomTerm(YAP_LookupAtom(atom.c_str()));
	
	std::vector<YAP_Term> args;
	for(size_t n = 0 ; n < size ; n++)
	{
		if(n==pos)
			args.push_back(atomTerm);
		else
			args.push_back(YAP_MkVarTerm());
	}
	YAP_Term yapInner = YAP_MkApplTerm( predFnct, size, &args[0] );
	
	stateArgs.push_back(yapInner);
	YAP_Term yapGoal = YAP_MkApplTerm( yapFnct, 1, &stateArgs[0] );
	
    long slot = YAP_InitSlot( yapGoal );
	int okay = YAP_RunGoal( yapGoal );
	size_t cardinality = okay ? 1 : 0;
    yapGoal = YAP_GetFromSlot(slot);
	while( okay != 0 ) 
	{
    	okay = YAP_RestartGoal();
		if(okay)
			++cardinality;
    }    
    YAP_Reset();
    YAP_RecoverSlots(1);
	return cardinality;
}
SRetract* PrologController::getRetractInfo( int attempt )
{
    // Prolog call: ?- state_clauses( L ).
    static YAP_Functor yapFnct = YAP_MkFunctor( YAP_LookupAtom("state_retract_info"), 1 );
    SRetract *pRI = NULL;
    
    YAP_Term    yapGoal = YAP_MkNewApplTerm( yapFnct, 1 );
    long slot = YAP_InitSlot( yapGoal );
	int okay = YAP_RunGoal( yapGoal );
    yapGoal = YAP_GetFromSlot(slot);
	if ( okay != 0 ) {
        pRI =  new SRetract( STerm( YAP_ArgOfTerm( 1, yapGoal ) ) );
    }
    YAP_Reset();
    // YAP bugfix, state extraction returned something messed up - retry
	if(pRI!= NULL && pRI->getSTerm().getTag() == STerm::sTermUnknown)
	{
		delete pRI;
		pRI = NULL;
		if(attempt <= 3)
		{
			int nextAttempt = attempt+1;
			std::cerr << "Retrying getRetractInfo - attempt " << nextAttempt << std::endl;
			pRI = getRetractInfo(nextAttempt);
		}
	}
    YAP_RecoverSlots(1);
	return pRI;
}
void PrologController::loadStateInfo( GameState& state )
{
    // Prolog call: ?- state_clauses( L ).
    static YAP_Functor yapFnct = YAP_MkFunctor( YAP_LookupAtom("state"), 1 );
    
    YAP_Term    yapGoal = YAP_MkNewApplTerm( yapFnct, 1 );
    long slot = YAP_InitSlot( yapGoal );
	int okay = YAP_RunGoal( yapGoal );
    yapGoal = YAP_GetFromSlot(slot);
	Compound* temp = NULL;
	while ( okay != 0 ) 
	{
		temp = buildCompound( YAP_ArgOfTerm( 1, yapGoal ) );
        state.push_back( temp );
		okay = YAP_RestartGoal();
    }
	YAP_Reset();
	YAP_RecoverSlots(1);
}

std::string PrologController::getGameFileName()
{
	return prologfile;
}
std::string PrologController::getStringState()
{
	std::stringstream ss;
	SRetract *ri = getRetractInfo();
	ri->getSTerm().toStream( ss );
	delete ri;
	return ss.str();
}
void PrologController::gotoStringState(std::string state)
{
	std::stringstream ss;
	ss << state;
	STerm    term( ss ); 
	SRetract retractInfo( term );
	retract(&retractInfo);
}

Compound* PrologController::buildCompound(YAP_Term term)
{
	Compound* c = NULL;
	char buffer[64];
	if(YAP_IsAtomTerm(term))
		c = new Compound(ct_COMPOUND, getTheory()->getSymbols()->lookup(YAP_AtomName(YAP_AtomOfTerm(term)))->getId(), co_PREDICATE);		
	else if(YAP_IsIntTerm(term))
	{
		sprintf(buffer, "%d" , static_cast<int>(YAP_IntOfTerm( term )));
		c = new Compound(ct_VALUE, getTheory()->getSymbols()->lookup(buffer)->getId(), co_INT);		
	}
	else
	{		
		YAP_Functor fnct =  YAP_FunctorOfTerm( term );
		std::string name = YAP_AtomName( YAP_NameOfFunctor( fnct ) );
		c = new Compound(ct_COMPOUND, getTheory()->getSymbols()->lookup(name)->getId(), co_PREDICATE);
		int argsize = YAP_ArityOfFunctor(fnct);
		for(int n = 1; n <= argsize; ++n)
		{
			CompoundOperator op = co_ATOM;
			YAP_Term temp = YAP_ArgOfTerm( n, term );
			if ( YAP_IsIntTerm( temp ) ) 
			{
				sprintf(buffer, "%d" , static_cast<int>(YAP_IntOfTerm( temp )));
				op = co_INT;
			}
			else if ( YAP_IsFloatTerm( temp ) ) 
			{
				sprintf(buffer, "%f" , YAP_FloatOfTerm( temp ));
			}
			else if ( YAP_IsAtomTerm( temp ) )
			{
				sprintf(buffer, "%s", YAP_AtomName( YAP_AtomOfTerm( temp ) ));
			}
			else if(YAP_IsApplTerm(temp))
			{
				YAP_Functor fnctInner =  YAP_FunctorOfTerm( temp );
				name = YAP_AtomName( YAP_NameOfFunctor( fnctInner ) );
				Compound* cInner = new Compound(ct_COMPOUND, getTheory()->getSymbols()->lookup(name)->getId(), co_PREDICATE);
				int argsizeInner = YAP_ArityOfFunctor(fnctInner);
				for(int n = 1; n <= argsizeInner; ++n)
				{
					CompoundOperator op = co_ATOM;
					YAP_Term tempInner = YAP_ArgOfTerm( n, temp );
					if ( YAP_IsIntTerm( tempInner ) ) 
					{
						sprintf(buffer, "%d" , static_cast<int>(YAP_IntOfTerm( tempInner )));
						op = co_INT;
					}
					else if ( YAP_IsFloatTerm( tempInner ) ) 
					{
						sprintf(buffer, "%f" , YAP_FloatOfTerm( tempInner ));
					}
					else if ( YAP_IsAtomTerm( tempInner ) )
					{
						sprintf(buffer, "%s", YAP_AtomName( YAP_AtomOfTerm( tempInner ) ));
					}
					else
					{
						sprintf(buffer, "panic");
						return NULL;
					}
					std::string innername = buffer;
					cInner->addArgument(new Compound(ct_VALUE, getTheory()->getSymbols()->lookup(innername)->getId(), op));
				}
				c->addArgument(cInner);
				continue;
			}
			else
			{
				sprintf(buffer, "panic");
				return NULL;
			}
			name = buffer;
			c->addArgument(new Compound(ct_VALUE, getTheory()->getSymbols()->lookup(name)->getId(), op));
		}
	}
	return c;
}

YAP_Term PrologController::makeTerm(Move* move, parsing::SymbolTable* s)
{
	return makeTerm(&(move->compound), s);
}
YAP_Term PrologController::makeTerm(Compound* move, parsing::SymbolTable* s)
{
	if(move->getType() == ct_VALUE)
	{
		if(move->getOperator() == co_INT)
			return YAP_MkIntTerm(s->getNum(move->getName()));
		
		return YAP_MkAtomTerm(YAP_LookupAtom(s->getName(move->getName()).c_str()));
	}
		
	YAP_Term term;
	size_t arity = move->getArguments() ? move->getArguments()->size() : 0;
	YAP_Functor fnct = YAP_MkFunctor(YAP_LookupAtom(s->getName(move->getName()).c_str()), arity);
	std::vector<YAP_Term> args;
	if(arity)
	{
		for ( size_t n=0; n<arity; ++n ) 
		{
			args.push_back(makeTerm(move->getArgument(n), s));
		}
		term = YAP_MkApplTerm( fnct, arity, &args[0] );
	}
	else
		term = YAP_MkApplTerm( fnct, arity, NULL );
	
	return term;
}
