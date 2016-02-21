/*
 *  agent.ccp
 *  src
 *
 *  Created by Hilmar Finnsson on 4/16/08.
 *  Copyright 2008 Reykjavik University. All rights reserved.
 *
 */

#include "agent.h"
using namespace cadiaplayer::agent;
using namespace cadiaplayer::utils;
using namespace cadiaplayer::play;
using namespace cadiaplayer::logic;

Agent::Agent(cadiaplayer::play::players::GamePlayer* player, bool external, cadiaplayer::logic::GameController* controller):
m_srcdir("games/kif/"),
m_workdir("games/ggp/"),
m_restorefile("restore_ggp.txt"),
m_restoremode(false),
m_playing(false),
m_isExternal(external),
m_ignoreExternals(false), 
m_logFile("agent.log"),
m_pgnFile(""),
m_externalPlayersFile("externals.txt"),
m_startmessageHandled(false),
m_player(player),
m_exttype(Parallel),
m_playerCount(0),
m_controller(controller),
m_localController(false)
{
}
Agent::~Agent()
{
	Agent::clean();
	
	if(m_localController && m_controller != NULL)
	{
		delete m_controller;		
		m_controller = NULL;
	}
}
void Agent::log(std::string str)
{
	std::ofstream o;
	o.open(m_logFile.c_str(), std::ios::app);
	o << cadiaplayer::utils::Timer::getTimeStamp() << " ";
	o << str.c_str() << "\n";
	o.close();
}

void Agent::startExternals(GMMessage msg)
{
	if(m_ignoreExternals || m_isExternal || !m_externalPlayers.getNumWorkers())
	{
		log("No external players defined");
		return;
	}
	checkExternals();
	Message extmsg;
	extmsg.setID(-1);
	extmsg.setType(Message::msgGameDesc);
	std::stringstream content;
	msg.encodeToStream(content);
	extmsg.setContent(content.str());
	log("Sending game description to external players");
	//std::stringstream ss;
	for(size_t n = 0 ; n < m_externalPlayers.getNumWorkers() ; n++)
	{
		//ss << "Sending game description to external player no " << n;
		//log(ss);
		m_externalPlayers.addWorkToDo(extmsg, n);
	}
}
void Agent::playExternals(GMMessage msg)
{
	if(m_ignoreExternals || m_isExternal || !m_externalPlayers.getNumWorkers())
		return;
	
	Message extmsg;
	if(msg.type == gm_INIT)
		extmsg.setID(1);
	else
		extmsg.setID((int)m_theory->getRound()+1);
		
	if(m_exttype == Parallel)
		extmsg.setType(Message::msgParallel);
	else
		extmsg.setType(Message::msgSearch);
	
	std::stringstream content;
	msg.encodeToStream(content);
	extmsg.setContent(content.str());
	log("Sending play message to external players");
	//std::stringstream ss;
	for(size_t n = 0 ; n < m_externalPlayers.getNumWorkers() ; n++)
	{
		//ss << "Sending play message for id " << extmsg.getID() << " to external player no " << n;
		//log(ss);
		m_externalPlayers.addWorkToDo(extmsg, n);
	}
}
void Agent::stopExternals(GMMessage msg)
{
	if(m_ignoreExternals || m_isExternal || !m_externalPlayers.getNumWorkers())
		return;
	
	Message extmsg;
	extmsg.setID((int)m_theory->getRound()+1);
	
	extmsg.setType(Message::msgFinish);
	
	std::stringstream content;
	msg.encodeToStream(content);
	extmsg.setContent(content.str());
	log("Sending stop message to external players");
	//std::stringstream ss;
	for(size_t n = 0 ; n < m_externalPlayers.getNumWorkers() ; n++)
	{
		//ss << "Sending stop message for id " << extmsg.getID() << " to external player no " << n;
		//log(ss);
		m_externalPlayers.addWorkToDo(extmsg, n);
	}
}
void Agent::checkExternals()
{
	if(!m_ignoreExternals || m_isExternal || !m_externalPlayers.getNumWorkers())
		return;
	std::stringstream ss;
	ss << "Checking external connections returned " << m_externalPlayers.checkConnections(0);
	log(ss);
}
void Agent::restartExternals()
{
	if(!m_externalPlayers.getNumWorkers())
		return;
	log("Sending restart message to external players");
	//std::stringstream ss;
	Message extmsg;
	extmsg.setID(-1);
	extmsg.setType(Message::msgReset);
	extmsg.setContent("(quit)");
	for(size_t n = 0 ; n < m_externalPlayers.getNumWorkers() ; n++)
	{
		//ss << "Sending restart message to external player no " << n;
		//log(ss);
		m_externalPlayers.addWorkToDo(extmsg, n);
	}
}
std::string Agent::compareWithExternals(std::string currentMove)
{
	//if(m_ignoreExternals || m_isExternal || !m_externalPlayers.getNumWorkers())
	//	return currentMove;
	
	std::string returnMove;
	returnMove = currentMove;
	double currentConfidence = m_player->minReachableGoal(*m_theory);
	std::stringstream ss;
	std::stringstream replyParser;
	double replyConfidence;
	char delimiter;
	Message msg;
	ss << "Scanning move recommendations by external players against " << currentMove << " of confidence " << currentConfidence;
	log(ss);
	if(m_externalPlayers.isWorkDone())
	{
		while(m_externalPlayers.isWorkDone())
		{
			m_externalPlayers.popWorkDone( msg );
			if(msg.getID()==-1)
			{
				m_startmessageHandled = true;
				log("Start message handled");
				//externalPlayers.waitForWorkDone();
				continue;
			}
			else
				log("Work done by external " + msg.toStr());
			if(msg.getID() >= 0 && msg.getID() < (int)m_theory->getRound())
			{
				log("Work done by external arrived too late");
				continue;
			}
			replyParser.clear();
			replyParser << msg.getContent();
			replyParser >> replyConfidence;
			if(replyConfidence < currentConfidence)
				continue;
			currentConfidence = replyConfidence;
			replyParser >> delimiter;
			while(delimiter == ':' || delimiter == '\n')
				replyParser >> delimiter;
			replyParser.putback(delimiter);
			getline(replyParser, returnMove);
		}
	}
	else
		log("No work done by externals");
	ss << "Using move " << returnMove << " with confidence " << currentConfidence;
	log(ss);
	return returnMove;
}

std::string Agent::consolidateExternals(std::string currentMove)
{
	//if(m_ignoreExternals || m_isExternal || !m_externalPlayers.getNumWorkers())
	//	return currentMove;
	
	std::string returnMove;
	returnMove = currentMove;
	std::stringstream ss;
	Message msg;
	ss << "Scanning move ratings by external players against " << currentMove;
	log(ss);
	if(m_externalPlayers.isWorkDone())
	{
		MoveRatings extRatings;
		MoveRatings intRatings;
		m_player->parallelInfo(*m_theory, intRatings);
		MoveRatingMap ratingsmap;
		MoveRatingMapItr finder;
		
		//log("Internal move ratings:");
		//log(MoveRating::toString(intRatings));
		
		for(int n = 0 ; n < intRatings.size() ; n++)
		{
			finder = ratingsmap.find(intRatings[n].getMove());
			if(finder == ratingsmap.end())
			{
				if(playerCount() == 1 && intRatings[n].getScore() == GOAL_MAX)
				{
					std::stringstream ssOk;
					returnMove = intRatings[n].getMove();
					ssOk << "Solved internally with move " << returnMove;
					log(ssOk);
					return returnMove;
				}
				ratingsmap[intRatings[n].getMove()] = intRatings[n];
			}
			else
			{
				if(playerCount() == 1)
				{
					if(intRatings[n].getScore() == GOAL_MAX)
					{
						std::stringstream ssOk;
						returnMove = intRatings[n].getMove();
						ssOk << "Solved internally with move " << returnMove;
						log(ssOk);
						return returnMove;
					}
					finder->second.max(intRatings[n]);
				}
				else
					finder->second.merge(intRatings[n]);
			}
		}
		log("Processing work done by external players");
		while(m_externalPlayers.isWorkDone())
		{
			m_externalPlayers.popWorkDone( msg );
			if(msg.getID()==-1)
			{
				m_startmessageHandled = true;
				//log("Start message handled");
				//externalPlayers.waitForWorkDone();
				continue;
			}
			/*else
			{
				//log("Work done by external " + msg.toStr());
				log("Processing work done by external");
			}*/
			if(msg.getID() >= 0 && msg.getID() < (int)m_theory->getRound())
			{
				log("Work done by external arrived too late");
				continue;
			}
			
			extRatings.clear();
			MoveRating::fromString(msg.getContent(), extRatings);
			
			//log("Merging external move ratings");
			//log(MoveRating::toString(extRatings));
			
			for(int n = 0 ; n < extRatings.size() ; n++)
			{
				finder = ratingsmap.find(extRatings[n].getMove());
				if(finder == ratingsmap.end())
				{
				   ratingsmap[extRatings[n].getMove()] = extRatings[n];
				}
				else
				{
					if(playerCount() == 1)
						finder->second.max(extRatings[n]);
					else
						finder->second.merge(extRatings[n]);
				}
			}
		}
		double score = -1.0;
		std::stringstream merged;
		merged << "Merged move ratings:";
		for(MoveRatingMapItr itr = ratingsmap.begin() ; itr != ratingsmap.end() ; itr++)
		{
			merged << std::endl;
			itr->second.info(merged);
			if(itr->second.getScore() > score)
			{
				finder = itr;
				score = finder->second.getScore();
			}
		}
		log(merged.str());
		returnMove = finder->second.getMove();
		log("Return move:");
		log(returnMove);
	}
	else
		log("No work done by externals");
	ss << "Using move " << returnMove;
	log(ss);
	return returnMove;
}

void Agent::newRestorePoint(GMMessage& msg)
{
	std::ofstream os;
	os.open(m_restorefile.c_str(), std::fstream::trunc);
	msg.writeToStream(os);
	os.close();
}
void Agent::addRestoreAction(GMMessage& msg)
{
	std::ofstream os;
	os.open(m_restorefile.c_str(), std::fstream::app);
	msg.writeToStream(os);
	os.close();
}

bool Agent::load()
{
	if(m_parser != NULL)
		delete m_parser;
	m_parser = new cadiaplayer::play::parsing::GameParser(true);
	if(m_theory != NULL)
		delete m_theory;
	m_theory = new cadiaplayer::play::GameTheory();
	bool ok = true;
	
	try
	{
		log("Running parser");
		ok = m_theory->initTheory(m_parser, m_id);
	}
	catch(...)
	{
		log("Could not parse file");
		ok = false;
	}
	if(ok && m_theory != NULL)
	{
		// Make 3 tries
		for(int n = 1 ; n <= 3 ; n++)
		{
			ok = configureController();
			if(ok)
				break;
			if(n < 3)
				log("Configure failed, retring...");
			else
				log("Configure failed");
		}
	}
	if(ok)
		log("Configure suceeded");
	ok = m_theory == NULL ? false:ok;
	if(ok)
		log("Theory ready after configure process");
	else
		log("Controller failiure");
	return ok;
}

bool Agent::configureController()
{
	bool ok = true;
	try
	{
		log("Configuring controller");
		if(!m_controller)
		{
			log("Local controller");
#ifdef USE_ECLIPSE
			m_controller = new cadiaplayer::logic::EclipseController();
#else
			m_controller = new cadiaplayer::logic::PrologController();
#endif
			m_localController = true;
		}
		else
			log("External controller");
		m_theory->useController(m_controller);
		log("Controller configured");
	}
	catch(...)
	{
		log("Unable to start controller");
		ok = false;
	}
	return ok;
}

void Agent::clean()
{
	restartExternals();
	if(m_theory != NULL)
	{
		delete m_theory;
		m_theory = NULL;
	}
	if(m_parser != NULL)
	{
		delete m_parser;
		m_parser = NULL;
	}
}
void Agent::init()
{
	m_error = 0;
	m_theory = NULL;
	m_parser = NULL;
	m_move = NULL;
	m_killme = false;
	m_externalPlayers.startUp(m_externalPlayersFile);
}
bool Agent::setupGame(GMMessage& msg)
{
	m_startTimer.startTimer();
	log("Starting agent");	
	m_moveLog.clear();
	startExternals(msg);
	checkExternals();
	
	// save kif to file
	m_id = msg.matchId;
	if(m_isExternal)
		m_id = "ext_" + m_id;
	m_filename = m_srcdir;
	m_filename += m_id;
	m_filename += ".kif";
	
	// Make sure srcdir exists
	mkdir(m_srcdir.c_str(), 0777);
	// Make sure workdir exists
	mkdir(m_workdir.c_str(), 0777);
	std::ofstream file;
	file.open(m_filename.c_str());
	file << msg.kif.c_str();
	file.close();
	log("Parsing GDL");
	// Parse kif into theory
	if(!load())
	{
		m_error = GDL_PARSE_ERROR;
		log("Parsing error occurred");
		return false;
	}
	
	/*if(m_theory->getRoles()->size() == 1)
		m_ignoreExternals = false;
	else
		m_ignoreExternals = true;*/
	
	// Create a restore point
	if(!m_restoremode)
		newRestorePoint(msg);
	// Create player
#ifdef USE_WORKERS
	log("Resetting external workers");
	m_theory->clearCheckWorkers();
	log("Workers reset");
	if(!m_isExternal)
	{
		log("Adding available workers to message queue");
		m_theory->addCheckWorkers(&m_externalPlayers);
		log("Workers added");
	}
	else
	{
		log("Using externally defined player");
		if(m_ignoreExternals)
		{
			log("Externally defined player not applicable");
			return false;
		}
	}	
#else
	log("Using externally defined player");
	if(m_ignoreExternals)
	{
		log("Externally defined player not applicable");
		return false;
	}
#endif
	try
	{
		log("Querying player name and setup info");
		log(m_player->getPlayerName());
		log(m_player->getSetupInfo());
	}
	catch(...)
	{
		log("Error caught when trying to query player name");
	}
	// role must be set before announcing a new game
	double starttime = msg.startclock;
	// take off already spent time
	//starttime = m_startTimer.remaining(starttime);
	if(m_isExternal)
		starttime -= EXTERNAL_START_DELAY;
	else 
		starttime-=START_DELAY;
	if(starttime < 1)
		starttime = 1;
	m_player->setThinkTime(starttime);
	std::stringstream timing;
	timing << "Using start clock as " << starttime;
	log(timing);
	m_playerCount = m_theory->getRoles()->size();
	m_player->setRole(m_theory->getRoleIndex(msg.role), msg.role);
	log("Player initializing game space");
	m_player->newGame(*m_theory);
	return true;
}
void Agent::setPlayClock(double playtime)
{
	if(m_isExternal)
		playtime -= EXTERNAL_PLAY_DELAY;
	else
		playtime -= PLAY_DELAY;
	m_player->setThinkTime(playtime);
	std::stringstream timing;
	timing << "Using play clock as " << playtime;
	log(timing);
	
}
bool Agent::start(GMMessage msg)
{
	
	if(!setupGame(msg))
		return false;
	
	m_playing = true;
	
	// selfplay on start clock
	log("Thinking on start clock...");
	m_player->prepare(*m_theory);
	log("... done");
	logPlay("");
	
	// send READY reply
	GMProtocol::sendMessage(gm_START, "");
	log("READY reply sent");
	
	setPlayClock(msg.playclock);
	
#if USE_WORKERS
	// Check on workers real quick before the GM server replies if needed
	if(!m_isExternal)
	{
		std::stringstream ss;
		ss << "Checking worker connections returned " << m_theory->checkWorkers() << std::endl;
		//ss << "Checking worker connections returned " << "?" << std::endl;
		ss << m_theory->getWorkersInfo();
		log(ss);
	}
#endif
	
	checkExternals();
	m_player->postplay(*m_theory);
	return true;
}
void Agent::logPlay(std::string m)
{
	std::stringstream o;
	o << std::endl;
	o << m_player->getPlayerName() << std::endl;
	o << m_player->getLastPlayInfo(*m_theory).c_str();
	log(o.str());
	if(!m.empty())
		log("Sending " + m);
}
void Agent::play()
{	
	if(m_isExternal && m_ignoreExternals)
	{
		log("Externally defined player not applicable");
		return;
	}
	log("Thinking...");
	m_move = m_player->play(*m_theory);
	log("...done thinking");
	// send the move in reply
	std::string strMove = m_move->compound.toPlayString(m_theory->getSymbols());
	
	bool skipExternals = m_ignoreExternals || m_isExternal || !m_externalPlayers.getNumWorkers();
	if(!skipExternals)
	{
		if(m_exttype == Parallel)
		{
			strMove = consolidateExternals(strMove);
		}
		else
		{	
			strMove = compareWithExternals(strMove);			
		}
	}
	
	logPlay(strMove);
	GMProtocol::sendMessage(gm_PLAY, strMove);
	
#ifdef USE_WORKERS
	// Check on workers real quick before the GM server replies if needed
	if(!m_isExternal)
	{
		std::stringstream ss;
		ss << "Checking worker connections returned " << m_theory->checkWorkers() << std::endl;
		//ss << "Checking worker connections returned " << "?" << std::endl;
		ss << m_theory->getWorkersInfo();
		log(ss);
	}
#endif
	
	m_player->postplay(*m_theory);
}
void Agent::asyncplay()
{	
	m_player->asyncplay(*m_theory);
}
void Agent::conf()
{	
	std::stringstream ss;
	ss << m_player->minReachableGoal(*m_theory);
	GMProtocol::sendMessage(gm_CONFIDENCE, ss.str());
}
void Agent::mcparallel()
{	
	cadiaplayer::play::MoveRatings ratings;
	m_player->parallelInfo(*m_theory, ratings);
	//std::cerr << cadiaplayer::play::MoveRating::toString(ratings) << std::endl;
	GMProtocol::sendMessage(gm_MCPARALLEL, cadiaplayer::play::MoveRating::toString(ratings));
}
void Agent::randomplay()
{	
	log("Playing at random");
	m_move = m_theory->generateRandomMove(m_player->getRoleIndex());
	// send the move in reply
	log("Random play : " + m_move->compound.toPlayString(m_theory->getSymbols()));
	GMProtocol::sendMessage(gm_PLAY, m_move->compound.toPlayString(m_theory->getSymbols()));
}
void Agent::logMove()
{
	std::stringstream o;
	o << std::endl << "Moves in round " << (m_theory->getRound()-1) << std::endl << m_theory->playInfo(&m_pm).c_str();
	log(o.str());
}
bool Agent::make(GMMessage& msg)
{
	if(msg.type == gm_STOP)
		stopExternals(msg);
	else
		playExternals(msg);
	checkExternals();
	// parse the moves into the theory
	//log("Move kif is:");
	//log(msg.kif);
	if(!m_parser->parseString(m_theory, msg.kif, &m_moves))
	{
		log("Move kif caused parse error");
		m_error = GDL_PARSE_ERROR;
		return false;
	}
	//log(cadiaplayer::play::GameTheory::listToString(&m_moves, m_theory->getSymbols()));
	m_moveLog.push_back(MoveLogItem(msg.kif, m_player->getLastPlayConfidence()));
	for(size_t n = 0 ; n < m_moves.size() ; ++n)
	{
		/*m_moves[n]->addArgumentFront(
			new cadiaplayer::play::parsing::Compound(	
					cadiaplayer::play::parsing::ct_VALUE, 
					n, 
					cadiaplayer::play::parsing::co_ATOM));*/
		cadiaplayer::play::Move* m = new cadiaplayer::play::Move(*(m_moves[n]));
		m_pm.push_back(m);
		//log("Move changed from:");
		//log(m_moves[n]->toString(m_theory->getSymbols()));
		//log("Move changed to:");
		//log(m->toString(m_theory->getSymbols()));
		delete m_moves[n];
	}
	log("Calling make for agent");
	m_theory->make(m_pm);
	logMove();
	for(size_t n = 0 ; n < m_pm.size() ; ++n)
	{
		delete m_pm[n];
	}
	m_pm.clear();
	m_moves.clear();
	
	// Add to restore point
	if(!m_restoremode)
		addRestoreAction(msg);
	
	return true;
}
bool Agent::load(GMMessage msg)
{	
	if(!setupGame(msg))
		return false;
	setPlayClock(msg.playclock);
	m_playing = true;
	return true;
}
bool Agent::loadFile(GMMessage msg)
{	
	//std::cerr << "Loading...\n";
	std::ifstream kiffile;
	kiffile.open(msg.kif.c_str());
	if(!kiffile.is_open() || kiffile.eof())
	{
	//	std::cerr << "...failed\n";
		return false;
	}
	std::string temp;
	msg.kif = "";
	while(std::getline(kiffile, temp))
	{
	//	std::cerr << temp << std::endl;
		if(temp[0] != ';')
			msg.kif+=temp;
	}
	//std::cerr << "Done\n";
	return start(msg);
}
void Agent::state() 
{	
	std::string str = cadiaplayer::play::GameTheory::listToString(m_theory->getStateRef(), m_theory->getSymbols());
	if(str[str.size()-1] == '\n')
		str.erase(--str.end());
	GMProtocol::sendMessage(gm_STATE, str);
}
void Agent::avail()
{	
	std::stringstream ss;
	cadiaplayer::play::RoleMoves* moves;
	for(int n = 0 ; n < m_theory->getRoles()->size() ; n++)
	{
		moves = m_theory->getMoves(n);
		ss << (*(m_theory->getRoles()))[n] << std::endl;
		for(int m = 0 ; m < moves->size() ; m++)
		{
			ss << (*(moves))[m]->compound.toString(m_theory->getSymbols()) << std::endl;
		}
	}
	GMProtocol::sendMessage(gm_AVAIL, ss.str());
}
void Agent::term()
{	
	std::stringstream ss;
	if(m_theory->isTerminal())
		GMProtocol::sendMessage(gm_TERM, "1");
	else
		GMProtocol::sendMessage(gm_TERM, "0");
}
void Agent::goal()
{	
	std::stringstream ss;
	for(int n = 0 ; n < m_theory->getRoles()->size() ; n++)
	{
		ss << m_theory->goal(n) << std::endl;
	}
	GMProtocol::sendMessage(gm_GOAL, ss.str());
}
void Agent::info(GMMessage msg)
{	
	m_player->writeMemoryInfo(*m_theory, msg.kif.c_str());
	GMProtocol::sendMessage(gm_GOAL, "done");
}
void Agent::move(GMMessage& msg)
{	
	make(msg);
}
void Agent::undo()
{	
	m_theory->retract();
	log("Move retracted");
}
void Agent::stop()
{	
	GMProtocol::sendMessage(gm_STOP, "");
	std::stringstream o;
	if(m_player != NULL)
		o << "Goal reached : " << m_theory->goal(m_player->getRoleIndex());
	else
		o << "Stopping an unparsable game";
	log(o);
	
	try
	{
		generatePGN(m_pgnFile);
	}
	catch(...)
	{
		log("Error generating PGN file");
	}
	log("Stopping player...");
	m_player->stop(*m_theory);
	log("...player stopped");
	m_playing = false;
	//restartExternals();
}
void Agent::ping()
{	
	std::string status; 
	if(m_playing)
		status = "busy";
	else
		status = "available";
	
	GMProtocol::sendMessage(gm_PING, status);
	log("Answearing PING with status: " + status);
}
void Agent::abort()
{	
	log("Aborting player");
	m_playing = false;
	
	GMProtocol::sendMessage(gm_ABORT, "aborted");
	log("Answearing ABORT with aborted");
}

void Agent::getMessage(GMMessage& msg)
{
	msg = GMProtocol::getMessage();
	
}
void Agent::logMessage(GMMessage& msg)
{
	log(msg.toString().c_str());
}

void Agent::restore(cadiaplayer::play::players::GamePlayer* player)
{
	log("Restore begins");
	init();
	std::ifstream is;
	is.open(m_restorefile.c_str());
	if(is.fail() || is.eof())
		return;
	
	GMMessage msg;
	msg.readFromStream(is);
	
	// save kif to file
	m_id = msg.matchId;
	m_filename = m_srcdir;
	m_filename += m_id;
	m_filename += ".kif";
	
	// make sure the directory exists
	mkdir(m_srcdir.c_str(), 0777);
	std::ofstream file;
	file.open(m_filename.c_str());
	file << msg.kif.c_str();
	file.close();
	
	// Parse kif into theory
	if(!load())
	{
		m_error = GDL_PARSE_ERROR;
		return;
	}
	
	// Create player
	m_player = player;
	// role must be set before announcing a new game
	m_player->setRole(m_theory->getRoleIndex(msg.role), msg.role);
	m_player->newGame(*m_theory);
	m_player->setThinkTime(msg.playclock-PLAY_DELAY);
	
	while(!is.eof())
	{
		if(msg.readFromStream(is))
			make(msg);
	}
	// Quickly play a random move since we have no idea of how much playtime has passed
	randomplay();
	std::cerr << cadiaplayer::play::GameTheory::listToString(m_theory->getStateRef(),m_theory->getSymbols());
	log("Restore ends");
}

int Agent::run(cadiaplayer::play::players::GamePlayer* restorePlayer)
{   
	m_restoremode = false;  
	if(restorePlayer)
		m_restoremode = true;
	init();
	log("Starting player:");
	log(m_player->getPlayerName());
	log(m_player->getSetupInfo());
	while(true)
	{
		try
		{
			if(m_restoremode)
			{
				restore(restorePlayer);
				m_restoremode = false;
			}
			else // normal
			{
				// Get Start message
				log("Waiting for start message");
				GMMessage msg;
				getMessage(msg);
				while(msg.type == gm_ERROR)
				{
					log("Got an erroneous message.\n");
					getMessage(msg);
				}
				logMessage(msg);
				if(msg.type == gm_START) 
				{
					if(!start(msg))
					{
						log("Start game failed once\n");
						if(!start(msg))
						{
							log("Start game failed twice\n");
							if(!start(msg))
							{
								log("Could not start game from START message\n");
								stop();
								clean();
								continue;
							}
						}
					}
				}
				else if(msg.type == gm_LOAD) 
				{
					if(!load(msg))
					{
						log("Could not start game from LOAD message\n");
						clean();
						continue;
					}
					state();
				}
				else if (msg.type == gm_FILE)
				{
					log("File request");
					if(!loadFile(msg))
					{
						log("Failed to load kif file " + msg.kif);
						continue;
					}
				}
				else if(msg.type == gm_PING)
				{
					log("Ping request");
					ping();
					continue;
				}
				else if(msg.type == gm_ABORT)
				{
					log("Abort request");
					abort();
					continue;
				}
				else
				{
					if(msg.type == gm_KILL)
					{
						m_killme = true;
						break;
					}
					
					if (msg.type == gm_CONFIDENCE)
					{
						log("Confidence request");
						conf();
						continue;
					}
					if (msg.type == gm_MCPARALLEL)
					{
						log("Parallel request");
						mcparallel();
						continue;
					}
					log("Unexpected message type when expecting START or KILL\n");
					GMProtocol::sendMessage(gm_NIL, "");
					continue;
				}
				
			}
			while(true)
			{
				// get next message
#ifdef USE_ASYNC
				if(!m_theory->isTerminal())
				{
					
					log("Doing async play while waiting for message");
					
					while(!GMProtocol::hasMessage())
					{
						//log("Calling asyncplay");
						asyncplay();
					}
				}
				else
					log("Waiting for message (async)");
				
#else
				log("Waiting for message (blocked)");
#endif
				GMMessage msg;
				getMessage(msg);
				log("Message received");
				if(msg.type == gm_ERROR)
				{
					log("Got an erroneous message.\n");
					continue;
				}
				logMessage(msg);
				if(msg.type == gm_START)
				{
					clean();
					if(!start(msg))
					{
						log("Start game failed once\n");
						if(!start(msg))
						{
							log("Start game failed twice\n");
							if(!start(msg))
							{
								log("Could not start game from START message after three tries\n");
								stop();
								clean();
								break;
							}
						}
					}
					continue;
				}
				else if (msg.type == gm_INIT)
				{
					//msg.kif = "nil";
					playExternals(msg);
					play();
					continue;
				}
				else if (msg.type == gm_PLAY)
				{
					if(!make(msg))
					{
						stop();
						clean();
						break;
					}
					play();
					continue;
				}
				else if (msg.type == gm_STOP)
				{
					make(msg);
					stop();
					clean();
					break;
				}
				else if (msg.type == gm_ABORT)
				{
					abort();
					clean();
					break;
				}
				else if (msg.type == gm_KILL)
				{
					clean();
					m_killme = true;
					break;
				}
				else if (msg.type == gm_CONFIDENCE)
				{
					log("Confidence request");
					conf();
					continue;
				}
				else if (msg.type == gm_MCPARALLEL)
				{
					log("Parallel request");
					mcparallel();
					continue;
				}
				else if(msg.type == gm_PING)
				{
					log("Ping request");
					ping();
					continue;
				}
				else if (msg.type == gm_LOAD)
				{
					log("Load request");
					load(msg);
					state();
					continue;
				}
				else if (msg.type == gm_STATE)
				{
					log("State request");
					state();
					continue;
				}
				else if (msg.type == gm_MOVE)
				{
					log("Move request");
					move(msg);
					state();
					continue;
				}
				else if (msg.type == gm_UNDO)
				{
					log("Undo request");
					undo();
					state();
					continue;
				}
				else if (msg.type == gm_FILE)
				{
					log("File request");
					if(!loadFile(msg))
						log("Failed to load kif file " + msg.kif);
					continue;
				}
			    else if (msg.type == gm_THINK)
			    {
					log("Think request");
					play();
					continue;
				}
				else if (msg.type == gm_AVAIL)
			    {
					log("Moves available request");
					avail();
					continue;
				}
				else if (msg.type == gm_TERM)
			    {
					log("Terminal request");
					term();
					continue;
				}
				else if (msg.type == gm_GOAL)
			    {
					log("Goals request");
					goal();
					continue;
				}
				else if (msg.type == gm_HALT)
			    {
					log("Halt request");
					stop();
					clean();
					break;
				}
				else if (msg.type == gm_INFO)
			    {
					log("Info request");
					info(msg);
					continue;
				}
				else
				{
					stop();
					clean();
					fflush(stdin);
					fflush(stdout);
					break;
				}
			}
			if(m_killme)
			{
				log("Kill switch triggered\n");
				break;
			}
		}
		catch(const std::exception & e)
		{
			std::stringstream ss;
			ss << "Unexpected error caught: " << e.what() << std::endl;
			log(ss.str());
			clean();
		}
		catch(...)
		{
			log("Unexpected error caught\n");
			clean();
		}
	}
	log("Ending agent run execution\n");
	clean();
	m_playing = false;
	return m_error;
}


void Agent::generatePGN(std::string filename)
{
	if(filename == "")
		filename = m_id + ".pgn";
	std::ofstream file;
	file.open(filename.c_str());
	if(file.fail())
	{
		log("PGN failiure");
		return;
	}
	file << "[Event \"Agent logged event\"]" << std::endl;
	file << "[Site \"Agent\"]" << std::endl;
	file << "[Date \"";
	file << cadiaplayer::utils::Timer::getPGNDate();
	file << "\"]" << std::endl;
	file << "[Round \"0\"]" << std::endl;
	file << "[White \"";
	if(m_player->getRoleIndex() == 0)
		file << m_player->getPlayerName();
	else
		file << m_theory->getRoles()->at(0);
	file << "\"]" << std::endl;
	file << "[Black \"";
	if(m_player->getRoleIndex() == 1)
		file << m_player->getPlayerName();
	else
		file << m_theory->getRoles()->at(1);
	file << "\"]" << std::endl;
	file << "[Result \"";
	file << m_theory->goal(0);
	for(size_t n = 1 ; n < m_theory->getRoles()->size() ; n++)
	{
		file << "," << m_theory->goal(n);
	}
	file << "\"]" << std::endl;
	file << std::endl;
	for(size_t n = 0 ; n < m_moveLog.size() ; n++)
	{
		file << (n+1) << ". " << m_moveLog[n].move << " {" << m_moveLog[n].confidence << "} ";  
	}
	file << std::endl;
	file.flush();
	file.close();
}
