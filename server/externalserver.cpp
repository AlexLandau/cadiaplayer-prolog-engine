/*
 *  externalserver.cpp
 *  src
 *
 *  Created by Hilmar Finnsson on 5/28/08.
 *  Copyright 2008 Reykjavik University. All rights reserved.
 *
 */

#include <iostream>
#include <string>
#include "../utils/childprocess.h"
#include "../utils/tcpserver.h"
#include "../utils/timer.h"

using namespace std;
using namespace cadiaplayer::utils;

const int EXT_DEFAULT_PORT = 15050;
const int START_ERROR = -1;
const int FIRST_PLAY_ERROR = -2;
const int PLAY_ERROR = -3;
const int GDL_PARSE_ERROR = -4;

string filename;
int error;
string id;
bool gameDescriptionReceived;
string logfile = "extagent.log";
ChildProcess* m_program;
int m_cntProcessedRequests;

bool startProgram()
{
    m_cntProcessedRequests = 0;
    m_program = new ChildProcess( filename.c_str() );    
    if ( m_program != NULL ) 
	{
        cerr << Timer::getTimeStamp() << "Trying to start program '" << filename << "'" << endl;
        if ( m_program->start(  ) ) 
		{
            cerr << Timer::getTimeStamp() << "External GGP Program started sucessfully!" << endl;
            return true;
        }
    }
    cerr << Timer::getTimeStamp() << "External GGP Program did not start successfully! NOTE" << endl;
    return false;
}


void stopProgram()
{
	try 
	{
		if ( m_program != 0 ) 
		{
			m_program->writeToStdIn( "(quit)\n" );
			m_program->stop();
			delete m_program;
			m_program = 0;
		}
		return;
	}
	catch (...) 
	{
		cerr << "Failed to stop program, assuming it has crashed\n";
	}
	try 
	{
		if(m_program != 0)
			delete m_program;
	}
	catch (...) 
	{
		cerr << "Failed to delete program object, assuming it has been flush from memory\n";
	}
    m_program = 0;
}

void sendToProgram(string str)
{
	m_program->writeToStdIn(str);
}
string readFromProgram()
{
	string msg = m_program->readFromStdOut();
	cerr << Timer::getTimeStamp() << "RCVI-FROM-PRG:\n[" << msg <<"]" << endl;
	return msg;
}
int 
main ( int argc, char * const argv[] )
{
    int    port = EXT_DEFAULT_PORT; // Default port.
    filename = "./externalastar";
    int    returnCode = 0;
    
    // Usage: program [ggp executeable] [port]
    if ( argc < 3 ) {
        cerr << "Usage: " << argv[0] << " [ggp executeable] [port]" << endl;
        return -1;
    }
	
	filename = argv[1];   // Use filename that was provided as an argument.
	port = atoi( argv[2] );  // Use port that was provided as an argument.
	
	// Start TCP server.
    TCPServer server( port );
    srand( getpid() );
	
	gameDescriptionReceived = false;
	if(!startProgram())
		return START_ERROR;
	
    // Listening on port for incoming request.
    bool doQuit = false;
	stringstream ssConf;
	while ( !doQuit ) {
		
        Message msg( Message::msgUnknown );
        string  strReply;
		if(returnCode == 1)
		{
			server.accept();
			returnCode = 0;
		}
		// Wait for message (blocking).
		cerr << "Waiting for server message" << endl;
		
		bool gotMsg = false;
		int tries = 1;
		while(!gotMsg)
		{
			try 
			{
				gotMsg = server.wait(msg);
			}
			catch (...) 
			{
				cerr << "server.wait threw an error on try " << tries++ << endl;
				gotMsg = false;
				sleep(0);
				continue;
			}
			if(!gotMsg)
			{
				cerr << "server.wait returned false on try " << tries++ << endl;
				sleep(0);
			}
			/*if(!server.isConnected())
			{
				returnCode = 1;
				gotMsg = false;
				break;
			}*/
			
		}
		if(!gotMsg)
			continue;
        //while ( !server.wait( msg ) ) {
        //    cerr << "server.wait returned false" << endl;
        //}
		m_cntProcessedRequests++;
        // Process request, and send reply.
        int startupProgram = 1;
		switch ( msg.getType() ) 
		{
			case Message::msgGameDesc:
				cerr << Timer::getTimeStamp() << "Received game description message [" << msg.getID() << "]" << endl;
				stopProgram();
				if(!startProgram())
					return START_ERROR;
				sendToProgram(msg.getContent());
				strReply = readFromProgram();
				while(strReply[0] != 'R')
				{
					cerr << Timer::getTimeStamp() << "Did not get READY reply, restarting agent : " << startupProgram++ << std::endl;
					//stopProgram();
					if(!startProgram())
						return START_ERROR;
					sendToProgram(msg.getContent());
					strReply = readFromProgram();
					/*if(strReply[0] != 'R')
					{
						cerr << Timer::getTimeStamp() << "Did not get READY reply again, restarting agent again\n";
						stopProgram();
						if(!startProgram())
							return START_ERROR;
						sendToProgram(msg.getContent());
						strReply = readFromProgram();
						if(strReply[0] != 'R')
						{
							cerr << Timer::getTimeStamp() << "Did not get READY reply third time, restarting agent anyway (cross your fingers!)\n";
						}
					}*/
				}
				msg.setContent(strReply);
				server.send(msg);
				break;
                
				// New game position, search it.
				case Message::msgSearch:
				cerr << Timer::getTimeStamp() << "Received search message [" << msg.getID() << "]" << endl;
                sendToProgram(msg.getContent());
				strReply = readFromProgram();
				sendToProgram("(conf)");
				strReply = readFromProgram() + ":" + strReply;
				msg.setContent(strReply);
				server.send( msg );
				break;
				
				// New game position, search it and contribute to root parallelization.
				case Message::msgParallel:
				cerr << Timer::getTimeStamp() << "Received parallel message [" << msg.getID() << "]" << endl;
                sendToProgram(msg.getContent());
				strReply = readFromProgram();
				sendToProgram("(mcpar)");
				strReply = readFromProgram();
				msg.setContent(strReply);
				server.send( msg );
				break;
				
				// Last game position, finish up.
			case Message::msgFinish:
				cerr << Timer::getTimeStamp() << "Received finish message [" << msg.getID() << "]" << endl;
                sendToProgram(msg.getContent());
				strReply = readFromProgram();
				server.send( msg );
				break;
				
				// Ping message, send a pong.
				case Message::msgPing:
				cerr << Timer::getTimeStamp() << "Received ping message [" << msg.getID() << "]" << endl;
                msg.setReturnCode( 0 ); 
                msg.clearContent();
                server.send( msg );
                break;
                
				// Quit the program,
				case Message::msgReset:
				cerr << Timer::getTimeStamp() << "Received reset message [" << msg.getID() << "]" << endl;
                stopProgram();
				if(!startProgram())
				{
					std::cerr << "Unable to start program.\n"; 
					return START_ERROR;
				}
				server.send(msg);
				returnCode = 1;
                //doQuit = true;
                break;
                
				case Message::msgQuit:
				cerr << Timer::getTimeStamp() << "Received quit message [" << msg.getID() << "]" << endl;
                stopProgram();
				//doQuit = true;
                break;
				// Unknown message, output an error message.
				default:
                cerr << "Unknown message type: " << msg.toStr() << endl;
                break;
        }
        
    }
    return returnCode;
}
