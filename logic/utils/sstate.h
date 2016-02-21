/*
 *  SState.h
 *  GGP
 *
 *  Created by Yngvi Bjornsson on 3/18/07.
 *  Copyright 2007 CADIA Reykjavík University. All rights reserved.
 *
 */
#ifndef INCL_SSTATE_H
#define INCL_SSTATE_H

#include <string>
#include <vector>
#include <sstream>
#include "sterm.h"
#include "smove.h"

namespace cadiaplayer {
	namespace logic {
		namespace utils {
			
			class SRetract
				{
				public:
					SRetract( const STerm& stClauses ) :
					m_stClauses( stClauses ) {}
					
					const STerm& getSTerm() const { return m_stClauses; } 
					
				private:
					STerm m_stClauses;
				};
			
			
			// TODO: assert in constr that has been initialize (Value/SState).
			class SState 
				{
				public:
					
					static bool init( std::string compPrologFile );
					
					static unsigned int numPlayers() { 
						return m_playerNames.size(); 
					}
					
					static std::string playerName(unsigned int player) {
						if( player < m_playerNames.size() );
							return m_playerNames[ player ];
						return "nil";
					}
					
					static int playerNumber(std::string name) {
						for ( unsigned int i= 0; i< m_playerNames.size(); ++i ) {
							if ( m_playerNames[i] == name ) {
								return i;
							}
						}
						return m_playerNames.size();
					}
					
					class Value 
					{
					public:
						Value( ) 
						: m_val( SState::numPlayers() ) 
						{
							
						}
						
						int getValue( unsigned int player ) const {
							if( player < m_val.size() )
								return m_val[ player ];
							return 0;
						}
						
						void setValue( unsigned int player, int val ) {
							if( player < m_val.size() )
								m_val[ player ] = val;
						}
						
						std::string getStr() const {
							std::stringstream ss;
							ss << '[';
							for ( unsigned int i=0; i< m_val.size(); ++i ) {
								if ( i > 0 ) ss << ',';
								ss << m_val[i];
							}
							ss << ']';
							return ss.str();
						}
						
						unsigned int size() const { 
							return m_val.size(); 
						}
						
					private:
						std::vector<int> m_val;
					};
					
					bool isTerminal( );
					
					void getMoves( SMovelist& ml );
					
					SRetract* getRetractInfo( );
					
					bool make( SMove move, SRetract& ri );
					
					bool make_sim( const SMoveSim& move, SRetract *ri = NULL );
					
					bool retract( SRetract *ri );
					
					bool goal( SState::Value& goals );
					
					int goal() { return 0; }
					
					void display( );
					
					static bool m_debug;
					
				private:
					
					static std::vector<std::string> m_playerNames;    
				}; 
		}}} // namespaces
#endif
