/*
 *  eclipsecontroller.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 2/8/11.
 *  Copyright 2011 Reykjavik University. All rights reserved.
 *
 */

#include <iostream>
#include "eclipsecontroller.h"

using namespace std;
using namespace cadiaplayer::play;
using namespace cadiaplayer::play::parsing;
using namespace cadiaplayer::logic;

void panic(char* what_, char* where_)
{
	std::cerr 
		<< "what: " << what_ << "\n"
		<< "where: " << where_ << std::endl;
}

// Constructor
EclipseController::EclipseController() : GameController()
{
	ec_set_option_ptr(EC_OPTION_ECLIPSEDIR, getenv("ECLIPSE_DIR"));
	
	// set local and global store
	// higher values for the local store may be neccessary the game has very complex rules
	ec_set_option_long(EC_OPTION_LOCALSIZE, 128*1024*1024);
	// higher values for the global store may be neccessary if you store a lot of big EclipseTerms (e.g. states with a lot of fluents)
	ec_set_option_long(EC_OPTION_GLOBALSIZE, 128*1024*1024);

	// set the panic callback function
	ec_set_option_ptr(EC_OPTION_PANIC, (void*)panic);

	int result;
	if(result=ec_init()) {
		std::cerr << "error (" << result << ") initializing eclipse context!" << std::endl;
		exit(-1);
	}
	
	dident compile_1 = ec_did("compile",1);
	ec_post_goal(ec_term(compile_1,ec_string("prolog/main")));
	if((result = ec_resume()) != PSUCCEED){
		std::cerr << "error (" << result << ") starting prolog system!" << std::endl;
	}
}

// Destructor
EclipseController::~EclipseController()
{
	
}



// GameTheory middleware calls this function to allow the controller to do any initialization needed
// Returns true if the controller is ready to play the game from the path/gamename given. 
bool EclipseController::init( cadiaplayer::play::GameTheory* gametheory, std::string gamename )
{
	GameController::init(gametheory, gamename);
	int result;
	std::string kiffile = "games/kif/"+gamename+".kif";
	ec_post_goal(ec_term(ec_did("set_rules_from_file",1),ec_string(kiffile.c_str())));
	if((result = ec_resume()) != PSUCCEED){
		std::cerr << "error (" << result << ") in set_rules_from_file!" << std::endl;
		return false;
	}
// 	test_controller();
	return true;
}

void EclipseController::cleanUp()
{
	ec_post_goal(ec_atom(ec_did("fail",0)));
	while(ec_resume() == PSUCCEED) {
		std:cerr << "error in prolog code: there is an open choicepoint!" << std::endl;
	}
//	ec_post_goal(ec_atom(ec_did("garbage_collect",0)));
//	ec_resume();
}

void EclipseController::test_controller()
{
	std::cerr << std::endl << "getting initial state ..." << std::endl;
	GameState s;
	getInitialState(s);
	std::cerr << "state: " << m_theory->listToString(&s, m_theory->getSymbols()) << std::endl;

	std::cerr << std::endl << "getting current state ..." << std::endl;
	s.clear();
	getCurrentState(s);
	std::cerr << "state: " << m_theory->listToString(&s, m_theory->getSymbols()) << std::endl;
	
	while(! isTerminal()) {
		std::cerr << std::endl << "getting legal moves ..." << std::endl;
		cadiaplayer::play::RoleMoves movesList;
		getMoves(0, movesList);
		std::cerr << "moves: " << m_theory->movesToString(&movesList, m_theory->getSymbols()) << std::endl;
		
		std::cerr << std::endl << "making move ..." << std::endl;
		m_theory->make(0, movesList.front());

		std::cerr << std::endl << "getting current state ..." << std::endl;
		s.clear();
		getCurrentState(s);
		std::cerr << "state: " << m_theory->listToString(&s, m_theory->getSymbols()) << std::endl;
	}

	std::cerr << std::endl << "getting goal value ..." << std::endl;
	double g = goal(0);
	std::cerr << "goal value: " << g << std::endl;
	
	std::cerr << std::endl << "retracting ..." << std::endl;
	retract();
	s.clear();
	getCurrentState(s);
	std::cerr << "state: " << m_theory->listToString(&s, m_theory->getSymbols()) << std::endl;

}

// Returns the initial state of the game as a GameState class (vector of Compound*)
void EclipseController::getInitialState(cadiaplayer::play::GameState& state)
{
	int result;
	ec_ref stateRef=ec_ref_create_newvar();
	pword gameState;
	ec_post_goal(ec_term(ec_did("initial_state",1),ec_ref_get(stateRef)));
	if(((result = ec_resume()) != PSUCCEED)){
		std::cerr << "error (" << result << ") while getting initial state!" << std::endl;
		gameState=ec_nil();
	} else {
		gameState = ec_ref_get(stateRef);
	}
	pword_to_compound_list(gameState, state);
}

// Returns the current state of the game as a GameState class (vector of Compound*)
void EclipseController::getCurrentState(cadiaplayer::play::GameState& state)
{
	pword_to_compound_list(getCurrentStatePWord(), state);
}

// Loades the available legal moves in the current state for role r (0=first role) into RoleMoves class m (vector of Move* classses)
void EclipseController::getMoves( cadiaplayer::play::RoleIndex role, cadiaplayer::play::RoleMoves& moves )
{
	int result;
// 	std::cerr << "computing legal moves for role " << role << " ..." << std::endl;
	ec_ref movesRef=ec_ref_create_newvar();
	ec_post_goal(ec_term(ec_did("legal_moves",2),ec_long(role+1),ec_ref_get(movesRef)));
	if((result = ec_resume()) != PSUCCEED){
		std::cerr << "error (" << result << ") while computing legal moves for role " << role << " in state " << termToString(getCurrentStatePWord()) << std::endl;
	}else{
		pword movesList=ec_ref_get(movesRef);
		pword move, tailList;
		while(ec_get_list(movesList,&move,&tailList)==PSUCCEED){
			Compound *c=pword_to_compound(move);
			//c->addArgumentFront(new Compound(ct_VALUE, r, co_ATOM));
			moves.push_back(new Move(*c));
			delete c;
			movesList=tailList;
		}
	}
	ec_ref_destroy(movesRef);
	// cleanUp();
}

// Creates Does relations from the Move* instances in the PlayMoves(vector) moves and proves the Next relations.
// Returns the new current state of the game as a GameState class (vector of Compound*).
// cadiaplayer::play::GameState (old return type)
void EclipseController::make(cadiaplayer::play::PlayMoves& moves)
{
	int result;
	if(!mute) {
		saveCurrentState();
	}
	pword movesList=moves_list_to_pword(moves);
	pword state;
	ec_ref stateRef=ec_ref_create_newvar();
	ec_post_goal(ec_term(ec_did("make_move",2),movesList,ec_ref_get(stateRef)));
	if(((result = ec_resume()) != PSUCCEED)){
		std::cerr << "error (" << result << ") while making move " << termToString(movesList) << " in state " << termToString(getCurrentStatePWord()) << std::endl;
		state=ec_nil();
	} else {
		state = ec_ref_get(stateRef);
	}
	//cadiaplayer::play::GameState gameState = pword_to_compound_list(state);
	cleanUp();
	//return gameState;
}

// Retract to the previouse state of the current state. 
void EclipseController::retract( void )
{
	if(!mute) {
		int result;
		ec_post_goal(ec_term(ec_did("pop_current_state_from_stack",0)));
		if(((result = ec_resume()) != PSUCCEED)){
			std::cerr << "error (" << result << ") while retracting state!" << std::endl;
		}
	}
}

// Disconnects the work done by retract, used by simulation implementations that do not need to make any more moves
// once the retract/backpropagation phase begins.  Save the theorem prover needless work as the states played through
// have been sent to the player and they will not be used to take play any other branch of the tree.
// This function should retain a snapshot of the current state so it can be restored by calling syncRetract();
void EclipseController::muteRetract( void )
{
	mute = true;
	saveCurrentState();
}
// Reconnects the work done by retract and restores the current state to how it was when muteRetract(); was last called. 
void EclipseController::syncRetract( void )
{
	int result;
	ec_post_goal(ec_term(ec_did("pop_current_state_from_stack",0)));
	if(((result = ec_resume()) != PSUCCEED)){
		std::cerr << "error (" << result << ") while retracting state!" << std::endl;
	}

	mute = false;
}

// Returns the goal for the role indexed (0 = first role)  
double EclipseController::goal( cadiaplayer::play::RoleIndex role )
{
	int result;
	ec_ref valueRef=ec_ref_create_newvar();
	long longValue;
	ec_post_goal(ec_term(ec_did("get_goal_value",2),ec_long(role+1),ec_ref_get(valueRef)));
	if((result = ec_resume()) != PSUCCEED){
		// std::cerr << "error (" << result << ") while computing goal value for role " << role << " in state " << termToString(getCurrentStatePWord()) << std::endl;
		longValue=GOAL_DRAW;
	}else{
		if ((result=ec_get_long(ec_ref_get(valueRef), &longValue)) != PSUCCEED) {
			std::cerr << "error (" << result << ") computing goal value (value is not a number)!" << std::endl;
			longValue=GOAL_DRAW;
		}
	}
	ec_ref_destroy(valueRef);
	return longValue;
}
// Returns true if the current state is a terminal one, false otherwise.
bool EclipseController::isTerminal( )
{
	int result;
	long is_terminal=0;
	ec_ref ref=ec_ref_create_newvar();
	ec_post_goal(ec_term(ec_did("is_terminal",1),ec_ref_get(ref)));
	if(((result = ec_resume()) != PSUCCEED) || (ec_get_long(ec_ref_get(ref),&is_terminal) != PSUCCEED)){
		std::cerr << "error (" << result << ") while checking terminality of state " << termToString(getCurrentStatePWord()) << std::endl;
	}
	ec_ref_destroy(ref);
	return is_terminal!=0;
}

pword EclipseController::parseTermString(const std::string &s) {
	int result;
	pword term;
	ec_ref ref=ec_ref_create_newvar();
	ec_post_goal(ec_term(ec_did("parse_gdl_term_string",2),ec_string(s.c_str()),ec_ref_get(ref)));
	if((result = ec_resume()) != PSUCCEED){
		std::cerr << "error (" << result << ") while parsing term: \"" << s << "\"" << std::endl;
		term=ec_nil();
	} else{
		term=ec_ref_get(ref);
	}
	ec_ref_destroy(ref);
	return term;
}

string EclipseController::termToString(pword term) {
	int result;
	char *cstr;
	string s;
	ec_ref ref=ec_ref_create_newvar();
	ec_post_goal(ec_term(ec_did("convert_to_gdl_string",2),term,ec_ref_get(ref)));
	if((result = ec_resume()) != PSUCCEED){
		std::cerr << "error (" << result << ") while transforming term to string!" << std::endl;
		s="error";
	} else{
		ec_get_string(ec_ref_get(ref), &cstr);
		s=cstr;
	}
	ec_ref_destroy(ref);
	return s;
}

cadiaplayer::play::StateFactID EclipseController::getStateFact(cadiaplayer::play::parsing::Compound* comp)
{
	StateFactID fid = m_theory->getGameStateID(comp);
	PrologPredicateStore::iterator itr = predicateStore.find(fid);
	if(itr == predicateStore.end()) {
		pword p=compound_to_pword(*comp);
		// TODO: store term on the eclipse side
		predicateStore[fid] = termToString(p);
	}
	return fid;
}

void EclipseController::syncState(const cadiaplayer::play::StateFacts& state)
{
	int result;
	// store current state for retraction
	if(!mute) {
		saveCurrentState();
	}
	// construct prolog list stateTerm from stored strings for the single StateFactIDs
	pword stateTerm = ec_nil();
	cadiaplayer::play::StateFacts::const_reverse_iterator i;
	for(i = state.rbegin() ; i != state.rend() ; ++i) {
		stateTerm = ec_list(parseTermString(predicateStore[*i]), stateTerm);
	}
	// set the constructed state as current state
	ec_post_goal(ec_term(ec_did("set_state_as_current_state",1), stateTerm));
	if(((result = ec_resume()) != PSUCCEED)){
		std::cerr << "error (" << result << ") while setting state as current state: " << termToString(stateTerm) << std::endl;
	}
}

void EclipseController::saveCurrentState()
{
	int result;
	ec_post_goal(ec_term(ec_did("push_current_state_on_stack",0)));
	if(((result = ec_resume()) != PSUCCEED)){
		std::cerr << "error (" << result << ") while saving state!" << std::endl;
	}
}

pword EclipseController::getCurrentStatePWord()
{
	int result;
	ec_ref stateRef=ec_ref_create_newvar();
	pword state;
	ec_post_goal(ec_term(ec_did("get_current_state",1),ec_ref_get(stateRef)));
	if(((result = ec_resume()) != PSUCCEED)){
		std::cerr << "error (" << result << ") while getting current state!" << std::endl;
		state=ec_nil();
	} else {
		state = ec_ref_get(stateRef);
	}
	return state;
}


// Helper functions for transforming Eclipse objects into the CadiaPlayer standard ones.


std::vector<cadiaplayer::play::parsing::Compound*> EclipseController::pword_to_compound_list(pword w) {
// 	std::cerr << "pword_to_compound_list ..." << std::endl;
	pword head, tail;
	std::vector<cadiaplayer::play::parsing::Compound*> list;
	if (ec_get_list(w,&head,&tail)==PSUCCEED) {
		list.push_back(pword_to_compound(head));
// 		std::cerr << "add element, new list: " << m_theory->listToString(list) << std::endl;
		while(ec_get_list(tail,&head,&tail)==PSUCCEED){
			list.push_back(pword_to_compound(head));
// 			std::cerr << "add element, new list: " << m_theory->listToString(list) << std::endl;
		}
	}
	return list;
}

void EclipseController::pword_to_compound_list(pword w, std::vector<cadiaplayer::play::parsing::Compound*>& list) {
	// 	std::cerr << "pword_to_compound_list ..." << std::endl;
	pword head, tail;
	if (ec_get_list(w,&head,&tail)==PSUCCEED) {
		list.push_back(pword_to_compound(head));
		// 		std::cerr << "add element, new list: " << m_theory->listToString(list) << std::endl;
		while(ec_get_list(tail,&head,&tail)==PSUCCEED){
			list.push_back(pword_to_compound(head));
			// 			std::cerr << "add element, new list: " << m_theory->listToString(list) << std::endl;
		}
	}
}

cadiaplayer::play::parsing::Compound* EclipseController::pword_to_compound(pword w) {
	Compound* c = NULL;
	long l;
	dident d;
	char *c_name;
	string name;
	if (ec_get_long(w,&l) == PSUCCEED) { // integer value
		ostringstream ss;
		ss << l;
		c = new Compound(ct_VALUE, m_theory->getSymbols()->lookup(ss.str())->getId(), co_INT);		
	} else if (ec_get_atom(w,&d) == PSUCCEED) { // atom
		c_name = DidName(d);
		name = c_name;
		c = new Compound(ct_VALUE, m_theory->getSymbols()->lookup(name)->getId(), co_ATOM);

	} else if (ec_get_functor(w,&d) == PSUCCEED) { // function or predicate with arity > 0
		c_name = DidName(d);
		name = c_name;
		cadiaplayer::play::parsing::SymbolID id = m_theory->getSymbols()->lookup(name)->getId();
		c = new Compound(ct_COMPOUND, id, co_PREDICATE);
		pword p=w,p2;
		int arity=ec_arity(p);
		for(int i=0;i<arity;++i){
			ec_get_arg(i+1,p,&p2);
			c->addArgument(pword_to_compound(p2));
		}
	}
	return c;
}

pword EclipseController::compound_list_to_pword(const std::vector<cadiaplayer::play::parsing::Compound*> &compoundList) {
	pword list=ec_nil();
	std::vector<cadiaplayer::play::parsing::Compound*>::const_reverse_iterator i;
	for(i=compoundList.rbegin(); i!=compoundList.rend(); ++i){
		list=ec_list(compound_to_pword(*(*i)), list);
	}
	return list;
}

pword EclipseController::compound_to_pword(const cadiaplayer::play::parsing::Compound &compound) {
	pword p;
	if (compound.getOperator() == co_INT) {
		std::string s = m_theory->getSymbols()->getName(compound.getName());
		p = ec_long(atoi(s.c_str()));
	} else {
		int arity = (compound.getArguments()==NULL ? 0 : compound.getArguments()->size());
		if (arity == 0) {
			string name = m_theory->getSymbols()->getName(compound.getName());
			p=ec_atom(ec_did(name.c_str(), arity));
		} else {
			pword *args = new pword[arity];
			for(int i=0; i<arity; ++i) {
				args[i] = compound_to_pword(*compound.getArgument(i));
			}
			string name = theory->getSymbols()->getName(compound.getName());
			p = ec_term_array(ec_did(name.c_str(), arity), args);
			delete[] args;
		}
	}
	return p;
}

pword EclipseController::move_to_pword(const cadiaplayer::play::parsing::Compound &compound) {
	pword p;
	int arity = (compound.getArguments()==NULL ? 0 : compound.getArguments()->size());
	if (arity-1 == 0) {
		string name = m_theory->getSymbols()->getName(compound.getName());
		p=ec_atom(ec_did(name.c_str(), arity-1));
	} else {
		pword *args = new pword[arity-1];
		for(int i=1; i<arity; ++i) { // skip the first argument because it contains the role
			args[i-1] = compound_to_pword(*compound.getArgument(i));
		}
		string name = m_theory->getSymbols()->getName(compound.getName());
		p = ec_term_array(ec_did(name.c_str(), arity-1), args);
		delete[] args;
	}
	return p;
}

pword EclipseController::moves_list_to_pword(const std::vector<cadiaplayer::play::Move*> moveList) {
	pword list=ec_nil();
	std::vector<cadiaplayer::play::Move*>::const_reverse_iterator i;
	for(i=moveList.rbegin(); i!=moveList.rend(); ++i){
		list=ec_list(move_to_pword((*i)->compound), list);
	}
	return list;
}

// // Take a GameState instance (vector) and load it with Compound* created from the current state, one for eact state predicate.
// void EclipseController::loadStateInfo( GameState& state )
// {
// 	EC_ref refPred;
//     // Fetch all state predicates and turn them into Compound*
// 	Compound* temp = NULL;
// 	// for all predicates in state
// 	{
// 		// Connect predicate to refPred
// 		temp = buildCompound( refPred );
//         state.push_back( temp );
// 	}
// }
// 
// EC_ref EclipseController::buildEC_ref(cadiaplayer::play::Move* move)
// {
// 	// Build EC_ref for move
// 	EC_word word = buildEC_word(move->items);
// 	EC_ref ref(word);
// 	return ref;
// }
// EC_word EclipseController::buildEC_word(std::vector<cadiaplayer::play::MoveTermItem>& items)
// {
// 	if(items[0].type == tt_INT)
// 		return EC_word((long)items[0].id);
// 	char* tmp = new char[1024];
// 	strcpy(tmp, m_theory->getSymbols()->getOriginalName(items[0].id).c_str());
// 	if(items[0].type == tt_ATOM)
// 		return EC_word(EC_atom(tmp));
// 	int arity = items.size()-1;
// 	EC_word args[arity];
// 	EC_functor fnct(tmp, arity);
// 	if(arity)
// 	{
// 		for ( size_t n=1; n<items.size(); ++n ) 
// 		{
// 			args[n-1] = buildEC_word(items[n]);
// 		}
// 		return term( fnct, args );
// 	}
// 	else
// 	{
// 		if(m_theory->getSymbols()->lookup(items[0].id)->getType() == parsing::st_NUM)
// 			return EC_word((long)items[0].id);
// 		return term( fnct, args );
// 	}
// }
// EC_word EclipseController::buildEC_word(cadiaplayer::play::MoveTermItem& item)
// {
// 	if(item.type == tt_INT)
// 		return EC_word((long)item.id);
// 	char* tmp = new char[1024];
// 	strcpy(tmp, m_theory->getSymbols()->getOriginalName(item.id).c_str());
// 	return EC_word(EC_atom(tmp));
// }
// 
// Move* EclipseController::buildMove(RoleIndex role, EC_ref action)
// {
// 	Compound* compound = buildCompound(action);
// 	if(!compound)
// 		return NULL;
// 	
// 	Move* move = new Move(*compound, m_theory->getSymbols(), true);
// 	delete compound;
// 	return move;
// }
// Compound* EclipseController::buildCompound(EC_ref term)
// {
// 	Compound* c = NULL;
// 	char buffer[64];
// 	EC_word word = EC_word(term);
// 	EC_atom*	atm;
// 	long*		lng;
// 	double*		dbl;
// 	EC_functor* fnct;
// 	if(word.is_atom(atm) == EC_succeed)
// 		c = new Compound(ct_COMPOUND, m_theory->getSymbols()->lookup(atm->name())->getId(), co_PREDICATE);		
// 	else if(word.is_long(lng))
// 	{
// 		sprintf(buffer, "%d" , (int)*lng);
// 		c = new Compound(ct_VALUE, m_theory->getSymbols()->lookup(buffer)->getId(), co_INT);		
// 	}
// 	else if(word.functor(fnct) == EC_succeed)
// 	{		
// 		std::string name = fnct->name();
// 		c = new Compound(ct_COMPOUND, m_theory->getSymbols()->lookup(name)->getId(), co_PREDICATE);
// 		int argsize = fnct->arity();
// 		EC_word wordInner;
// 		EC_functor* fnctInner;
// 		for(int n = 1; n <= argsize; ++n)
// 		{
// 			if(word.arg(n, wordInner) != EC_succeed)
// 				return NULL;
// 				
// 			CompoundOperator op = co_ATOM;
// 			if ( wordInner.is_long( lng ) == EC_succeed ) 
// 			{
// 				sprintf(buffer, "%d" , (int)*lng);
// 				op = co_INT;
// 			}
// 			else if ( wordInner.is_double( dbl ) ) 
// 			{
// 				sprintf(buffer, "%f" , *dbl);
// 			}
// 			else if ( wordInner.is_atom( atm ) )
// 			{
// 				sprintf(buffer, "%s", atm->name() );
// 			}
// 			else if(wordInner.functor(fnctInner) == EC_succeed)
// 			{
// 				name = fnctInner->name();
// 				Compound* cInner = new Compound(ct_COMPOUND, m_theory->getSymbols()->lookup(name)->getId(), co_PREDICATE);
// 				int argsizeInner = fnctInner->arity();
// 				for(int m = 1; m <= argsizeInner; ++m)
// 				{
// 					CompoundOperator op = co_ATOM;
// 					EC_word tempInner;
// 					if(wordInner.arg( m, tempInner ) != EC_succeed)
// 						return NULL;
// 					if ( tempInner.is_long( lng ) ) 
// 					{
// 						sprintf(buffer, "%d" , (int)*lng);
// 						op = co_INT;
// 					}
// 					else if ( tempInner.is_double( dbl ) ) 
// 					{
// 						sprintf(buffer, "%f" , *dbl);
// 					}
// 					else if (  tempInner.is_atom( atm ) )
// 					{
// 						sprintf(buffer, "%s", atm->name());
// 					}
// 					else
// 					{
// 						sprintf(buffer, "panic");
// 						return NULL;
// 					}
// 					std::string innername = buffer;
// 					cInner->addArgument(new Compound(ct_VALUE, m_theory->getSymbols()->lookup(innername)->getId(), op));
// 				}
// 				c->addArgument(cInner);
// 				continue;
// 			}
// 			else
// 			{
// 				sprintf(buffer, "panic");
// 				return NULL;
// 			}
// 			name = buffer;
// 			c->addArgument(new Compound(ct_VALUE, m_theory->getSymbols()->lookup(name)->getId(), op));
// 		}
// 	}
// 	
// 	return c;
// }
// 
