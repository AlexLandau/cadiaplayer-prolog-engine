#include "../config.h"
#include "gametheory.h"
#include "parsing/gameparser.h"
#include "../logic/gamecontroller.h"

#include "../utils/timer.h"

#include <iostream>
#include <fstream>

using namespace cadiaplayer::play;
using namespace cadiaplayer::play::parsing;
using namespace cadiaplayer::logic;

GameTheory::GameTheory() : 
m_kifFile(""),
m_parser(NULL),
m_mark(1),
m_rolesDone(false),
m_controller(NULL),
m_hashCounterPredicateDetected(false)
{	
	GameState initState;
	addNewState(initState);
	srand((unsigned)time(0)); 
	
	m_binaryRelations = new utils::BinaryRelations();
	m_relationCounters = new utils::RelationCounters();
	m_symbols = new parsing::SymbolTable();	
	
}

GameTheory::~GameTheory()
{
	//std::cerr << "Start of Gametheory destructor\n";
	terminate();
	
	// delete rule lists
	for(size_t n = 0 ; n <  m_rules.size() ; n++)
	{
		delete  m_rules[n];
	}
	for(size_t n = 0 ; n <  m_next.size() ; n++)
	{
		delete  m_next[n];
	}
	for(size_t n = 0 ; n <  m_legal.size() ; n++)
	{
		delete  m_legal[n];
	}
	for(size_t n = 0 ; n <  m_goals.size() ; n++)
	{
		delete  m_goals[n];
	}
	for(size_t n = 0 ; n <  m_terminals.size() ; n++)
	{
		delete  m_terminals[n];
	}
	// delete gamestate handler
	retractAll();
	for(size_t n = 0 ; n <  m_gameline.top().size() ; n++)
	{
		if( m_gameline.top()[n] != NULL)
			delete  m_gameline.top()[n];
	}
	 m_gameline.top().clear();
	 m_gameline.pop();
	// delete symbol table
	delete m_symbols;
	
	StateMoves* sm;
	RoleMoves* rm;
	for(size_t n = 0 ; n < m_statemovesline.size() ; ++n)
	{
		sm = m_statemovesline[n];
		for(size_t i = 0 ; i < sm->size() ; ++i)
		{
			rm = (*(sm))[i];
			for(size_t j = 0 ; j < rm->size() ; ++j)
			{
				delete (*(rm))[j];
			}
			rm->clear();
			delete rm;
		}
		sm->clear();
		delete sm;
	}
	m_statemovesline.clear();
	
	if(m_binaryRelations)
		delete m_binaryRelations;
	if(m_relationCounters)
		delete m_relationCounters;
	
	//std::cerr << "End of Gametheory destructor\n";
}

bool GameTheory::initTheory(cadiaplayer::play::parsing::GameParser* parser, std::string filename, std::string externalpath)
{
	setParser(parser);
	setKifFile(filename);

	std::stringstream kiffile;
	kiffile << KIF_FILE_PATH << m_kifFile << KIF_FILE_EXT;
	std::stringstream externalkiffile;
	if(externalpath.length())
	{
		externalkiffile << externalpath << m_kifFile << KIF_FILE_EXT;
		// copy the game file.
		std::stringstream cpfile;
		cpfile << "cat " << externalkiffile.str() << " | tr [A-Z] [a-z] > " << kiffile.str();
		std::string strCmd = cpfile.str();
		char command[strCmd.size()+1];
		strcpy(command, strCmd.c_str());
		int result = system(command);
		if(result < 0)
			std::cerr << "Warning: system(" << command << ") returned " << result << std::endl;
	}
	
	if(!m_parser->parseFile(this, kiffile.str().c_str()))
		return false;

	buildKeymap();
	detectCounterPredicate();
	
	return true;
}
cadiaplayer::play::parsing::GameParser* GameTheory::getParser()
{
	return m_parser;
};
void GameTheory::setParser(cadiaplayer::play::parsing::GameParser* p)
{
	m_parser = p;
};

void GameTheory::detectCounterPredicate()
{
	if(m_hashCounterPredicateDetected)
		return;
	m_hashCounterPredicateDetected = true;
	m_counterPredicate = NO_COUNTER_PREDICATE;
	size_t counterArg = 0;
	size_t factArg = 0;
	std::vector<parsing::SymbolID> factIDs;
	std::vector<parsing::SymbolID> counterIDs;
	for(KBaseListItr itr =  m_next.begin() ; itr !=  m_next.end() ; itr++)
	{
		// Template match the next rule
		if((*itr)->getOperator() != co_IMPLICATION)
			continue;
		if((*itr)->getArguments() == NULL)
			continue;
		if((*itr)->getArguments()->size() != 3)
			continue;
		// Template match the header predicate
		if((*itr)->getArgument(0)->getName() == (*itr)->getArgument(1)->getName())
		{
			factArg = 2;
			counterArg = 1;
		}
		if((*itr)->getArgument(0)->getName() == (*itr)->getArgument(2)->getName())
		{
			factArg = 1;
			counterArg = 2;
		}
		if(counterArg == 0)
			continue;
		// Template match the arguments
		if((*itr)->getArgument(0)->getArguments() == NULL || (*itr)->getArgument(0)->getArguments()->size() != 1 || !((*itr)->getArgument(0)->hasVariable()))
			continue;
		if((*itr)->getArgument(counterArg)->getArguments() == NULL || (*itr)->getArgument(counterArg)->getArguments()->size() != 1 || !((*itr)->getArgument(counterArg)->hasVariable()))
			continue;
		if((*itr)->getArgument(0)->getArgument(0)->getName() == (*itr)->getArgument(counterArg)->getArgument(0)->getName())
			continue;
		if((*itr)->getArgument(factArg)->getArguments() == NULL || (*itr)->getArgument(factArg)->getArguments()->size() != 2 || (*itr)->getArgument(factArg)->getArgument(0) == (*itr)->getArgument(factArg)->getArgument(1))
			continue;
		if((*itr)->getArgument(0)->getArgument(0)->getName() != (*itr)->getArgument(factArg)->getArgument(0)->getName() && 
		   (*itr)->getArgument(0)->getArgument(0)->getName() != (*itr)->getArgument(factArg)->getArgument(1)->getName())
			continue;
		if((*itr)->getArgument(counterArg)->getArgument(0)->getName() != (*itr)->getArgument(factArg)->getArgument(0)->getName() && 
		   (*itr)->getArgument(counterArg)->getArgument(0)->getName() != (*itr)->getArgument(factArg)->getArgument(1)->getName())
			continue;
		counterIDs.push_back((*itr)->getArgument(0)->getName());
		factIDs.push_back((*itr)->getArgument(factArg)->getName());
	}
	if(factIDs.size() == 0)
		return;
	size_t counter;
	size_t topCounter = 0;
	for(size_t n = 0 ; n < factIDs.size() ; n++)
	{
		counter = 0;
		for(KBaseListItr rule =  m_rules.begin() ; rule !=  m_rules.end() ; rule++)
		{
			if((*rule)->getName() == factIDs[n] && (*rule)->getArguments() != NULL && (*rule)->getArguments()->size() == 2)
				counter++;
		}
		if(counter > topCounter && counter >= 5)
		{
			topCounter = counter;
			m_counterPredicate = counterIDs[n];
		}
	}
}

void GameTheory::storeRole(parsing::Compound* c)
{
	Role r = m_symbols->getName(c->getArguments()->front()->getName());
	m_symbols->swap(r,  m_roles.size());
	c->getArguments()->front()->setName( m_roles.size());
	m_roles.push_back(r);
	m_rolelist.push_back(c);
	storeRule(c);
}
void GameTheory::storeInit(parsing::Compound* c)
{
	if(! m_rolesDone)
	{
		prepareMoves();
		prepareMoves();
		 m_rolesDone = true;
	}
	 m_gameline.top().push_back(c);
}
void GameTheory::storeRule(parsing::Compound* c)
{
	 m_rules.push_back(c);
}
void GameTheory::storeNext(parsing::Compound* c)
{
	implodeRelationImplication(*c);
	KBaseList* args = c->getArguments();
	for(size_t n = 0 ; n < args->size() ; n++)
	{
		if((*args)[n]->getOperator() == co_RELATION_DOES)
		{
			(*args)[n] = implodeRelationRole((*args)[n]);
		}
	}
	 m_next.push_back(c);
}
void GameTheory::storeLegal(parsing::Compound* c)
{
	if(c->getOperator() == co_IMPLICATION)
	{
		//getSymbols()->insertMoveClass(c->getArgument(0)->getArgument(1)->getName());
		
		Compound* l = c->getArguments()->front();
		c->removeArgumentFront();
		l = implodeRelationRole(l);
		c->addArgumentFront(l);
	}
	else
	{
		//getSymbols()->insertMoveClass(c->getArgument(1)->getName());
		
		c = implodeRelationRole(c);
	}
	 m_legal.push_back(c);
}
void GameTheory::storeGoal(parsing::Compound* c)
{
	m_goals.push_back(c);
	int g;
	if(c->getArgument(0)->getArguments()) 
		g = getSymbols()->getNum(c->getArgument(0)->getArgument(1)->getName());
	else
		g = getSymbols()->getNum(c->getArgument(1)->getName());
	m_goalValues.push_back(g);
}
void GameTheory::getGoalValues(std::vector<int>& g)
{
	for(int n = 0 ; n <  m_goalValues.size() ; n++)
	{
		g.push_back( m_goalValues[n]);
	}
}
void GameTheory::storeTerminal(parsing::Compound* c)
{
	 m_terminals.push_back(c);
}

void GameTheory::useController(GameController* controller)
{
	if(controller != NULL)
	{
		if(m_controller != NULL)
			delete m_controller;
		m_controller = controller;
	}
	
	if(!m_controller)
		return;
	std::cerr << "m_kifFile: " << m_kifFile << std::endl;
	m_controller->init(this, m_kifFile);
	GameState initState;
	m_controller->getInitialState(initState);
	if(initState.size() != 0)
	{
		retractAll();
		if(! m_gameline.empty())
		{
			for(size_t n = 0 ; n <  m_gameline.top().size() ; n++)
			{
				if( m_gameline.top()[n] != NULL)
					delete  m_gameline.top()[n];
			}
			 m_gameline.top().clear();
			 m_gameline.pop();
		}
		 m_gameline.push(initState);
		
		//std::cerr << GameTheory::listToString(&initState, getSymbols());
	}
}

void GameTheory::ask(parsing::Compound* c, bool interupt, KBaseList& answer)
{
	return m_controller->ask(c, interupt, answer);
}
size_t GameTheory::solutionCardinality(parsing::SymbolID predicate, parsing::SymbolID atom, size_t pos, size_t size)
{
	return m_controller->solutionCardinality(m_symbols->lookup(predicate)->getLexeme(), m_symbols->lookup(atom)->getLexeme(), pos, size);
}
GameState GameTheory::getState(void)
{
	return  m_gameline.top();
}
GameState GameTheory::getStateForced(void)
{
	GameState s;
	m_controller->getCurrentState(s);
	return s;
}
GameState* GameTheory::getStateRef(void)
{
	return &( m_gameline.top());
}
GameState* GameTheory::getParentState(void)
{
	return m_parentLine.top();
}

RoleMoves* GameTheory::getMoves(RoleIndex role)
{
	RoleMoves* moves = (*(m_statemovesline[ m_gameline.size()]))[role];
	if(moves->size() == 0)
	{
		m_controller->getMoves(role, *moves);
	}
	return moves;
}
void GameTheory::getMoves(std::vector<std::vector<Move*>*>& moves)
{
	for(RoleIndex n = 0 ; n < getRoles()->size() ; n++)
	{
		moves.push_back(getMoves(n));
	}
}
void GameTheory::prepareMoves()
{
	if(m_statemovesline.size() >  m_gameline.size())
		return;
	StateMoves* statemoves = new StateMoves();
	for(size_t n = 0 ; n <  m_roles.size() ; ++n)
	{
		statemoves->push_back(new RoleMoves());
	}
	m_statemovesline.push_back(statemoves);
}
void GameTheory::retractMoves()
{
	//std::cerr << "Retracting moves...\n";
	StateMoves* statemoves = m_statemovesline[ m_gameline.size()];
	RoleMoves* rolemoves = NULL;
	for(size_t n = 0 ; n < statemoves->size() ; ++n)
	{
		rolemoves = (*(statemoves))[n];
		for(size_t i = 0 ; i < rolemoves->size() ; ++i)
		{
			delete (*(rolemoves))[i];
		}
		rolemoves->clear();
	}
	//std::cerr << "Retracting moves done\n";
}
void GameTheory::make(RoleIndex role, Move* m)
{
	m_controller->make(role, m);
	GameState s;
	m_controller->getCurrentState(s);
	addNewState(s);
	prepareMoves();
}
void GameTheory::makeByForce(RoleIndex role, Move* m)
{
	GameState s;
	m_controller->makeByForce(role, m);
	m_controller->getCurrentState(s);
	addNewState(s);
	prepareMoves();
}
//#define GAMETHEORY_DEBUG_MAKE
void GameTheory::make(PlayMoves& moves)
{
#ifdef GAMETHEORY_DEBUG_MAKE
	std::cerr << "PLAYING:\n";
	for(size_t n = 0 ; n < moves.size() ; ++n)
	{
		std::cerr << moves[n]->compound.toString(getSymbols()).c_str() << std::endl;
	}
	std::cerr << "OLD STATE:\n";
	std::cerr << GameTheory::listToString(getStateRef(),getSymbols()).c_str();
	std::cerr << "MAKE STATE:\n";
#endif
	m_controller->make(moves);
	GameState s;
	m_controller->getCurrentState(s);
#ifdef GAMETHEORY_DEBUG_MAKE
	std::cerr << "ADD STATE:\n";
	std::cerr << GameTheory::listToString(&s,getSymbols()).c_str();
#endif
	addNewState(s);
#ifdef GAMETHEORY_DEBUG_MAKE
	std::cerr << "NEW STATE:\n";
	std::cerr << GameTheory::listToString(getStateRef(),getSymbols()).c_str();
#endif
	prepareMoves();
}
void GameTheory::makeRelaxed(PlayMoves& moves)
{
	GameState s;
	m_controller->makeRelaxed(moves);
	m_controller->getCurrentState(s);
	addNewState(s);
	prepareMoves();
}

GameState GameTheory::makeNullMove(void)
{
	GameState s;
	m_controller->makeNullMove(s);
	return s;
}
GameState GameTheory::makeExploratory(RoleIndex role, Move* m)
{
	GameState s;
	m_controller->makeExploratory(role, m, s);
	return s;
}

void GameTheory::makeAllRelaxed(RoleIndex role)
{
	RoleMoves* moves = getMoves(role);
	for(size_t m = 0 ; m < moves->size() ; ++m)
	{
		effectsPlus(role, (*moves)[m]);
	}
	commitEffectsPlus();
	GameState s;
	m_controller->getCurrentState(s);
	addNewState(s);
}


void GameTheory::addNewState(GameState state)
{
	//sort(state.begin(), state.end(), compoundCompareLessThan());
	if( m_gameline.size())
		m_parentLine.push(getStateRef());
	else
		m_parentLine.push(NULL);
	 m_gameline.push(state);
}
void GameTheory::peekNextState(RoleIndex role, Move* m, GameState& state)
{
	m_controller->peekNextState(role, m, state);
}
void GameTheory::peekNextState(PlayMoves& moves, GameState& state)
{
	m_controller->peekNextState(moves, state);
}
parsing::CompoundList GameTheory::effectsPlus(RoleIndex role, Move* m)
{
	//std::cerr << "EffectsPlus for : " << m->compound.toString(getSymbols()) << std::endl;
	//std::cerr << GameTheory::listToString(getStateRef(), getSymbols());
	parsing::CompoundList list;
	m_controller->effectsPlus(role, m, list);
	return list;
}
parsing::CompoundList GameTheory::effectsPlus(PlayMoves& moves)
{
	parsing::CompoundList list;
	m_controller->effectsPlus(moves, list);
	return list;
}
parsing::CompoundList GameTheory::effectsMinus(RoleIndex role, Move* m)
{
	parsing::CompoundList list;
	m_controller->effectsMinus(role, m, list);
	return list;
}
parsing::CompoundList GameTheory::effectsMinus(PlayMoves& moves)
{
	parsing::CompoundList list;
	m_controller->effectsMinus(moves, list);
	return list;
}
void GameTheory::commitEffectsPlus()
{
	GameState s;
	m_controller->commitEffectsPlus();
	m_controller->getCurrentState(s);
	addNewState(s);
	prepareMoves();
	//std::cout << "Committing to state:\n" << GameTheory::listToString( getStateRef(), m_symbols).c_str();
}
void GameTheory::discardEffectsPlus()
{
	m_controller->discardEffectsPlus();
}

GameStateID GameTheory::storeCurrentState(void)
{
	GameStateID sid = getGameStateID(getStateRef());
	//printf("ID received %u\n", sid);
	m_controller->storeCurrentStateAs(sid);
	return sid;
}
bool GameTheory::jumpToStoredState(GameStateID sid)
{
	GameState s;
	m_controller->jumpToStoredState(sid);
	m_controller->getCurrentState(s);
	if(s.size() == 0)
		return false;
	addNewState(s);
	prepareMoves();
	return true;
}
void GameTheory::deleteStoredState(GameStateID sid)
{
	return m_controller->deleteStoredState(sid);
}
bool GameTheory::gotoStart()
{
	retractAll();
	if(! m_gameline.empty())
	{
		for(size_t n = 0 ; n <  m_gameline.top().size() ; n++)
		{
			if( m_gameline.top()[n] != NULL)
				delete  m_gameline.top()[n];
		}
		 m_gameline.top().clear();
		 m_gameline.pop();
	}
	GameState startState;
	m_controller->gotoStart();
	m_controller->getCurrentState(startState);
	 m_gameline.push(startState);
	return true;
}
bool GameTheory::gotoState(const StateFacts& state, bool skipController)
{
	if(!skipController)
	{
		GameState s;
		m_controller->gotoState(state);
		m_controller->getCurrentState(s);
		addNewState(s);
		prepareMoves();
		return true;
	}
	GameState temp;
	CompoundStoreItr itr;
	for(size_t n = 0 ; n < state.size() ; n++)
	{
		if(!state[n])
			continue;
		itr = m_compoundStore.find(state[n]);
		if(itr == m_compoundStore.end())
		{
			std::cerr << "None-existing compound requested from gamtheory compound store" << std::endl;
			return false;
		}
		temp.push_back(new cadiaplayer::play::parsing::Compound(itr->second));
	}
	addNewState(temp);
	prepareMoves();
	return true;
}
bool GameTheory::gotoStateByForce(const StateFacts& state)
{
	GameState s; 
	m_controller->gotoStateByForce(state);
	m_controller->getCurrentState(s);
	addNewState(s);
	prepareMoves();
	return true;
}
bool GameTheory::gotoStringState(std::string state)
{
	GameState s;
	m_controller->gotoStringState(state);
	m_controller->getCurrentState(s);
	addNewState(s);
	prepareMoves();
	return true;
}
void GameTheory::syncController(StateFacts& stateFacts)
{
	m_controller->syncState(stateFacts);	
}
StateFactID GameTheory::getStateFact(Compound* comp)
{
	StateFactID id = m_controller->getStateFact(comp);
	CompoundStoreItr itr = m_compoundStore.find(id);
	if(itr == m_compoundStore.end())
		m_compoundStore[id] = *(comp);
	return id;
}
void GameTheory::getStateFacts(StateFacts& stateFacts)
{
	getStateFacts(*getStateRef(), stateFacts);
}
void GameTheory::getStateFacts(const GameState& state, StateFacts& stateFacts)
{
	m_controller->getStateFacts(state, stateFacts);
	CompoundStoreItr itr;
	for(size_t n = 0 ; n < state.size() ; n++)
	{
		itr = m_compoundStore.find(stateFacts[n]);
		if(itr == m_compoundStore.end())
			m_compoundStore[stateFacts[n]] = *(state[n]);
	}
}
std::string GameTheory::stateFactToString(const StateFactID& stateFact)
{
	return m_controller->stateFactToString(stateFact);
}

std::string GameTheory::stateFactsToString(StateFacts& stateFacts)
{
	std::stringstream ss;
	for(std::size_t n = 0 ; n < stateFacts.size() ; n++)
	{
		ss << stateFactToString(stateFacts[n]) << std::endl;
	}
	return ss.str();
}
// Assert one statefact into the current state.
void GameTheory::assertStateFact(const StateFactID& stateFact)
{
	GameState s;
	m_controller->assertStateFact(stateFact);
	m_controller->getCurrentState(s);
	addNewState(s);
	prepareMoves();
}
// Retract one statefact from the current state. 
void GameTheory::retractStateFact(const StateFactID& stateFact)
{
	GameState s;
	m_controller->retractStateFact(stateFact);
	m_controller->getCurrentState(s);
	addNewState(s);
	prepareMoves();
}

// Recalculate the fact id when enclosed in a has_seen predicate.
StateFactID GameTheory::getHasSeenID(const StateFactID& stateFact)
{
	return m_controller->getHasSeenID(stateFact, m_hasSeenSalt);
}
bool GameTheory::isHasSeen(parsing::Compound* c)
{
	return c == NULL ? false : c->getName() == m_hasSeenSymbolID; 
}
void GameTheory::retract(void)
{
	if( m_gameline.size() <= 1)
		return;
	
	retractMoves();
	m_parentLine.pop();
	//std::cerr << "deleting  m_gameline top with  m_gameline size as " <<  m_gameline.size() << " and the top state size as " <<  m_gameline.top().size() << "\n";
	for(size_t n = 0 ; n <  m_gameline.top().size() ; n++)
	{
		if( m_gameline.top()[n] != NULL)
		{
			try
			{
				//std::cerr << " m_gameline top[" << n <<"] is " <<  m_gameline.top()[n]->toString(getSymbols()) << "\n";
				delete  m_gameline.top()[n];
			}
			catch(...) 
			{
				std::cerr << " m_gameline top[" << n <<"] failed to be deleted\n";
				std::cerr << " m_gameline top[" << n <<"] is " <<  m_gameline.top()[n]->toString(getSymbols()) << "\n";
				std::cerr << " m_gameline top[" << n <<"] deleted\n";
			}
		}
	}
	//std::cerr << "clearing  m_gameline top\n";
	 m_gameline.top().clear();
	//std::cerr << "popping  m_gameline top\n";
	 m_gameline.pop();
	//std::cerr << "Calling controller retract\n";
	m_controller->retract();
	//std::cerr << "Done calling controller retract\n";
}
void GameTheory::retractByForce(void)
{
	if( m_gameline.size() <= 1)
		return;
	
	retractMoves();
	m_parentLine.pop();
	//std::cerr << "deleting  m_gameline top with  m_gameline size as " <<  m_gameline.size() << " and the top state size as " <<  m_gameline.top().size() << "\n";
	for(size_t n = 0 ; n <  m_gameline.top().size() ; n++)
	{
		if( m_gameline.top()[n] != NULL)
		{
			try
			{
				//std::cerr << " m_gameline top[" << n <<"] is " <<  m_gameline.top()[n]->toString(getSymbols()) << "\n";
				delete  m_gameline.top()[n];
			}
			catch(...)
			{
				std::cerr << " m_gameline top[" << n <<"] failed to be deleted\n";
				std::cerr << " m_gameline top[" << n <<"] is " <<  m_gameline.top()[n]->toString(getSymbols()) << "\n";
				std::cerr << " m_gameline top[" << n <<"] deleted\n";
			}
		}
	}
	//std::cerr << "clearing  m_gameline top\n";
	 m_gameline.top().clear();
	//std::cerr << "popping  m_gameline top\n";
	 m_gameline.pop();
	//std::cerr << "Calling controller retract\n";
	m_controller->retractByForce();
	//std::cerr << "Done calling controller retract\n";
}
void GameTheory::muteRetract(void)
{
	m_controller->muteRetract();
}
void GameTheory::syncRetract(void)
{
	m_controller->syncRetract();
}

void GameTheory::retractAll(void)
{
	while( m_gameline.size() > 1)
	{
		retract();
	}
}
void GameTheory::markGameState(void)
{
	m_mark =  m_gameline.size();
}
void GameTheory::retractToMark(void)
{
	while(m_mark <  m_gameline.size())
	{
		retract();
	}
}

unsigned int GameTheory::getRound(void)
{
	return  m_gameline.size();
}

bool GameTheory::isTerminal(void)
{
	return m_controller->isTerminal();
}
void GameTheory::terminate(void)
{
}
double GameTheory::goal(RoleIndex role)
{
	return m_controller->goal(role);
}

double GameTheory::incrementingGoal(RoleIndex role)
{
	return m_controller->incGoals(role);
}

double GameTheory::topGoal(RoleIndex role)
{
	return m_controller->allGoals(role);
}

bool GameTheory::reachedGoal(RoleIndex role, int goal)
{
	return m_controller->reachedGoal(role, goal);
}

bool GameTheory::isTrue(parsing::Compound* c)
{
	KBaseList ans;
	
	//m_controller->ask(c, true, ans);
	//return ans.size() > 0;
	
	GameState* curState = getStateRef();
	for(size_t n = 0 ; n < curState->size() ; ++n)
	{
		if(!((*curState)[n] == c))
			return true;
	}
	return false;
}

GameStateID GameTheory::getGameStateID(parsing::Compound* c, std::size_t parampos)
{
	SymbolID name = c->getName();
	if(SymbolTable::isVariable(name))
		return 0;
	if(m_gameStateIdMapList[parampos].size() <= name)
		return 0;
	GameStateID key = m_gameStateIdMapList[parampos][name];
	if(c->getArguments() == NULL)
		return key;
	parsing::CompoundArgs* args = c->getArguments();
	for(size_t n = 0 ; n <  args->size() ; n++)
	{
		key ^= getGameStateID((*args)[n], name+n);
	}
	return key;
}
GameStateID GameTheory::getGameStateID(Move* m)
{
	if(m == NULL)
		return 0;
	return getGameStateID(&(m->compound));
}
GameStateID GameTheory::getGameStateID(GameState* s)
{
	if(s == NULL)
		return 0;
	GameStateID key = 0;
	GameStateID upper = 0;
	GameStateID lower = 0;
	size_t pos = 0;
	for(size_t n = 0 ; n <  s->size() ; n++)
	{
		if(pos > 63)
			pos = 0;
		upper = getGameStateID((*s)[n]);
		lower = upper >> pos;
		upper = upper << (64-pos);
		key ^= (lower | upper);
		++pos;
	}
	return key;
}
GameStateID GameTheory::getGameStateIDNoCounter(GameState* s)
{
	if(s->size() == 1)
		return getGameStateID((*s)[0]);
	GameStateID key = 0;
	GameStateID upper = 0;
	GameStateID lower = 0;
	size_t pos = 0;
	for(size_t n = 0 ; n <  s->size() ; n++)
	{
		if(m_counterPredicate != NO_COUNTER_PREDICATE && ((*s)[n])->getName() == m_counterPredicate)
			continue;
		if(pos > 63)
			pos = 0;
		upper = getGameStateID((*s)[n]);
		lower = upper >> pos;
		upper = upper << (64-pos);
		key ^= (lower | upper);
		++pos;
	}
	return key;
}
void GameTheory::buildKeymap(void)
{
	m_gameStateIdMapList.clear();
	m_hasSeenSymbolID = m_symbols->insertConstant(PREDICATE_HAS_SEEN);
	unsigned int mapsize = getSymbols()->getConstantCount();
	unsigned int paramCount = getSymbols()->getParameterMax() + 1;
	unsigned int listSize = mapsize + paramCount;
	m_gameStateIdMapList.resize(listSize);
	std::cerr << "building keymap...";
	for(unsigned int i = 0 ; i < listSize ; i++)
	{
		for(unsigned int j = 0 ; j < mapsize ; j++)
		{
			m_gameStateIdMapList[i].push_back((((GameStateID)rand()) << 32) | rand());
		}
	}
	std::cerr << "done\n";
	m_hasSeenSalt = m_gameStateIdMapList[0][m_symbols->lookup(PREDICATE_HAS_SEEN)->getId()];
}
Move* GameTheory::generateRandomMove(RoleIndex role)
{
	RoleMoves* moves = getMoves(role);
	if(moves->size() == 0)
	{
		std::cerr << "No moves found when trying to generate a random move." << std::endl;
		return NULL;
	}
	size_t r;
	if(moves->size() == 1)
		r = 0;
	else
	{
		r = rand() % moves->size(); 
	}
	
	return (*(moves))[r];
}

void GameTheory::generateRandomMove(RoleIndex role, Move* move, PlayMoves& pm)
{
	for(size_t n = 0 ; n < getRoles()->size() ; n++)
	{
		if(n == role)
			pm.push_back(move);
		else
			pm.push_back(generateRandomMove(n));
	}
}

void GameTheory::generateRandomMove(PlayMoves& pm)
{
	for(size_t n = 0 ; n <  m_roles.size() ; n++)
	{
		pm.push_back(generateRandomMove(n));
	}
}

void GameTheory::playRandomMove(void)
{
	PlayMoves pm;
	generateRandomMove(pm);
	make(pm);
}
void GameTheory::playRandomMove(RoleIndex role, Move* move, PlayMoves& pm)
{
	generateRandomMove(role, move, pm);
	make(pm);
}
void GameTheory::playRandomMove(PlayMoves& pm)
{
	generateRandomMove(pm);
	make(pm);
}

double GameTheory::getPredicateGain(Compound* c, RoleIndex player)
{
	//std::cerr << "BEFORE:\n" << GameTheory::listToString(getStateRef(), getSymbols());
	GameState state;
	state.push_back(c);
	StateFacts facts;
	getStateFacts(state, facts);
	state.clear();
	gotoStateByForce(facts);
	double score = 0;
	for(RoleIndex r = 0 ; r < getRoles()->size() ; r++)
	{
		if(r == player)
			score += goal(r);
		else
			score -= goal(r);
	}
	//std::cerr << "DURING:\n" << GameTheory::listToString(getStateRef(), getSymbols());
	retractByForce();
	//std::cerr << "AFTER:\n" << GameTheory::listToString(getStateRef(), getSymbols());
	//std::cerr << "Returning score of " << score << std::endl;
	return score;
}

void GameTheory::implodeRelationImplication(parsing::Compound& c)
{
	if(c.getOperator() == co_IMPLICATION)
	{
		parsing::Compound* n = c.getArguments()->front();
		parsing::Compound* r = n->getArguments()->front();
		c.removeArgumentFront();
		n->removeArgumentFront();
		delete n;
		c.addArgumentFront(r);		
	}
}
parsing::Compound* GameTheory::implodeRelationRole(parsing::Compound* c)
{
	parsing::Compound* role = c->getArguments()->front();
	parsing::Compound* pred = (*(c->getArguments()))[1];
	if(pred->getType() == ct_VALUE)
	{
		pred->setType(ct_COMPOUND);
		pred->setOperator(co_PREDICATE);
	}
	else if(pred->getType() == ct_VARIABLE)
	{
		parsing::Compound* t = new parsing::Compound(ct_COMPOUND, m_symbols->insertConstant("true"), co_PREDICATE);
		t->addArgument(pred);
		pred = t;
	}		
	pred->addArgumentFront(role);
	c->removeArgumentFront();
	c->removeArgumentFront();
	delete c;
	return pred;		
}
std::string GameTheory::info(void)
{
	std::string str = "";
	str += "Roles:\n";
	for(size_t n = 0 ; n <  m_roles.size() ; n++)
	{
		str +=  m_roles[n];
		str += "\n";
	}
	str += "\nState:\n";
	str += kbaseListToString(& m_gameline.top());
	str += "\nRules:\n";
	str += kbaseListToString(& m_rules);
	str += "\nNext:\n";
	str += kbaseListToString(& m_next);
	str += "\nLegal:\n";
	str += kbaseListToString(& m_legal);
	str += "\nGoals:\n";
	str += kbaseListToString(& m_goals);
	str += "\nTerminal:\n";
	str += kbaseListToString(& m_terminals);
	return str;
}

std::string GameTheory::playInfo(PlayMoves* moves)
{
	std::string str = "[empty]";
	if(moves->empty())
		return str;
	str = "";
	for(size_t n = 0 ; n < moves->size() ; n++)
	{
		str +=  m_roles[n];
		str += " plays ";
		str += (*(moves))[n]->compound.toPlayString (m_symbols);
		str += ".\n";
	}
	return str;
}
std::string GameTheory::roleInfo(RoleIndex r, RoleMoves* moves)
{
	std::string str = "[empty]";
	if(moves->empty())
		return str;
	str = "";
	for(size_t n = 0 ; n < moves->size() ; n++)
	{
		str +=  m_roles[r];
		str += " can play ";
		str += (*(moves))[n]->compound.toString (m_symbols);
		str += ".\n";
	}
	return str;
}
std::string GameTheory::kbaseListToString(KBaseList* kb)
{
	std::string str = "";
	for(KBaseListItr c = kb->begin() ; c != kb->end() ; c++)
	{
		str += (*c)->toString (m_symbols);
		str += "\n";
	}
	return str;
}


size_t GameTheory::getAdversary(std::string myRole)
{
	if( m_roles.size() != 2)
		return ADVERSARY_NOT;
	if(! m_roles[ADVERSARY_PLAYER1].compare(myRole))
		return ADVERSARY_PLAYER2;
	return ADVERSARY_PLAYER1;
}
