/*
 *  worker.h
 *  server
 *
 *  Created by Yngvi Bjornsson on 7/12/07.
 *  Copyright 2007 CADIA Reykjavík University. All rights reserved.
 *
 */

#ifndef INCL_WORKER
#define INCL_WORKER

#include <string>
#include <vector>
#include <queue>
#include "tcpserver.h"

namespace cadiaplayer {
	namespace utils {
		typedef unsigned int WorkerId;
		
		typedef enum {
			wNotConnected,
			wReady,
			wBusy,
			wRestarting,
			wUnknown,
			wError
		} WorkerStatus;
		
		
		typedef struct {
			std::string   name;
			std::string   host;
			unsigned int  port;
			WorkerStatus  status;
			int           msgId;
			unsigned int noRequestsServed;
		} WorkerInfo;
		
		
		class Workers
			{
			public:
				Workers( );
				~Workers( );
				
				bool startUp( std::string fileName );
				unsigned int getNumWorkers() const; 
				
				WorkerStatus      getWorkerStatus( WorkerId wid );
				const WorkerInfo* getWorkerInfo( WorkerId wid ) const;
				bool              tryToConnect( WorkerId wid, int msecMaxWait = 200 );
				int               checkConnections( int msecMaxWait = 200 );
				
				unsigned int getNumWorksToDo() const;
				bool addWorkToDo( const Message& msg );
				bool addWorkToDo( const Message& msg, WorkerId wid );
				
				bool isWorkDone();
				unsigned int getNumWorksDone() const;
				bool popWorkDone( Message& msg );
				bool waitForWorkDone(  );
				bool wait( int msec );   
				bool process();
				
			private:
				
				typedef struct {
					WorkerInfo           info;
					TCPClient           *connection;
					std::queue<Message>  workToDo;        
				} Worker;
				
				std::vector<Worker> m_workers;
				std::queue<Message> m_workDone;
				std::queue<Message> m_workToDo;
			};
	}} // namespaces
#endif

