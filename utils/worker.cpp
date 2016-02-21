/*
 *  worker.cpp
 *  server
 *
 *  Created by Yngvi Bjornsson on 7/12/07.
 *  Copyright 2007 CADIA Reykjavík University. All rights reserved.
 *
 */

#include <iostream>
#include <fstream>
#include <unistd.h>
#include "worker.h"
using namespace cadiaplayer::utils;
using namespace std;

Workers::Workers( )
{
}


Workers::~Workers( )
{
    for ( WorkerId w=0 ; w<m_workers.size(); ++w ) {
        if ( m_workers[w].connection != 0 ) {
            delete m_workers[w].connection;
        }
    }
}
 

bool Workers::startUp( std::string fileName )
{
    fstream in;
    
    in.open( fileName.c_str() );
    if ( !in.is_open() ) // No file - treat as disable (normal execution)
		return true;
    
    while ( !in.eof() ) {
        std::string   name, host;
        int      port, enabled;
        in >> name >> host >> port >> enabled;
        if ( !in.fail() && enabled ) {
            Worker worker;
            worker.info.name = name;
            worker.info.host = host;
            worker.info.port = port;
            worker.info.status = wNotConnected;
            worker.info.noRequestsServed = 0;
            worker.connection = new TCPClient( worker.info.host, worker.info.port);
            if ( worker.connection != 0 ) {
                if ( worker.connection->connect( 500 ) ) 
				//if ( worker.connection->connectWithTimeout( 100 ) ) 
				{
                    worker.info.status = wReady;
                }
                m_workers.push_back( worker );
            }
        }
    }
    in.close();                    
    
    return true;
}


unsigned int Workers::getNumWorkers() const
{
    return m_workers.size();
}


WorkerStatus Workers::getWorkerStatus( WorkerId wid )
{
    if ( wid >= m_workers.size() ) return wUnknown;

    if ( !m_workers[wid].connection->isConnected() ) {
        m_workers[wid].info.status = wNotConnected;
    }        
    return m_workers[ wid ].info.status; 
}


const WorkerInfo* Workers::getWorkerInfo( WorkerId wid ) const
{
    if ( wid >= m_workers.size() ) return 0;
    
    return &m_workers[ wid ].info; 
}


bool Workers::tryToConnect( WorkerId wid, int msecMaxWait )
{
	//if(m_workers[ wid ].info.status == wError)
	//	return false;
    if ( ! m_workers[ wid ].connection->isConnected() ) {
        m_workers[ wid ].info.status = wNotConnected;
        if ( m_workers[ wid ].connection->connect( msecMaxWait ) ) 
		{
            m_workers[ wid ].info.status = wReady;
        }
		else
			m_workers[ wid ].info.status = wNotConnected; //wError;
    }
    else if ( m_workers[ wid ].info.status == wNotConnected ) {
        m_workers[ wid ].info.status = wReady;
    }
    return m_workers[ wid ].info.status != wNotConnected;
}



int Workers::checkConnections( int msecMaxWait )
{
    int n = 0;
    for ( WorkerId w=0 ; w<m_workers.size(); ++w ) {
        if ( this->tryToConnect( w, msecMaxWait ) ) n++;
    }
    return n;
}

    
unsigned int Workers::getNumWorksToDo() const
{
    unsigned int n = m_workToDo.size();
    for ( unsigned int i=0; i<m_workers.size(); ++i ) {
        n += m_workers[i].workToDo.size();
    }
    return n;
}


bool Workers::addWorkToDo( const Message& msg )
{
    m_workToDo.push( msg );
    this->process();

    return true;
}


bool Workers::addWorkToDo( const Message& msg, WorkerId wid )
{
    if ( wid >= m_workers.size() ) return 0;

    m_workers[ wid ].workToDo.push( msg );
    this->process();

    return true;
}


bool Workers::isWorkDone()
{
    this->process();
    return !m_workDone.empty();
}

unsigned int Workers::getNumWorksDone() const
{
    return m_workDone.size();
}


bool Workers::popWorkDone( Message& msg )
{
    this->process();
    if ( ! m_workDone.empty() ) {
         msg = m_workDone.front();
         m_workDone.pop();
         return true;
    }
    return false;
}


bool Workers::waitForWorkDone( )
{
    this->process();
    while ( m_workDone.empty() ) {
        usleep( 2000 ); //microseconds
        this->process();
    }
    return true;
}


bool Workers::wait( int millisec )
{
    this->process();
    for ( unsigned int i=0; i<100; ++i ) {
      usleep( 10 * millisec );  // = #microsec
      this->process();  //to make sure requests are processed.
    }
    return true;
}

bool Workers::process(  )
{
    Message msg;
    
    for ( WorkerId w=0 ; w<m_workers.size(); ++w ) {
       if ( m_workers[w].info.status == wBusy ) {
            if ( m_workers[w].connection->read( msg ) ) {
                m_workDone.push( msg );
                m_workers[w].info.noRequestsServed++;
                m_workers[w].info.status = wReady;                
            }
            else if ( !m_workers[w].connection->isConnected() ) {
                m_workers[w].info.status = wNotConnected;
            }            
        }
        if ( m_workers[w].info.status == wReady ) {
            if ( !m_workers[w].workToDo.empty() ) {
                msg = m_workers[w].workToDo.front();
                if ( m_workers[w].connection->send( msg ) ) {
                    m_workers[w].workToDo.pop();
					if(msg.getType()==Message::msgReset)
						m_workers[w].info.status = wRestarting;
					else
						m_workers[w].info.status = wBusy;
                }
                else if ( !m_workers[w].connection->isConnected() ) {
                    m_workers[w].info.status = wNotConnected;
                }
            }
            else if ( !m_workToDo.empty() ) {
                msg = m_workToDo.front();
                if ( m_workers[w].connection->send( msg ) ) {
                    m_workToDo.pop();
                    if(msg.getType()==Message::msgReset)
						m_workers[w].info.status = wRestarting;
					else
						m_workers[w].info.status = wBusy;
                }
                else if ( !m_workers[w].connection->isConnected() ) {
                    m_workers[w].info.status = wNotConnected;
                }                
            }
        }
    }
    return false;
}
