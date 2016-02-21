/*
 *  SMove.h
 *  GGP
 *
 *  Created by Yngvi Bjornsson on 3/23/07.
 *  Copyright 2007 CADIA Reykjavík University. All rights reserved.
 *
 */

#ifndef INCL_SMOVE_H
#define INCL_SMOVE_H

#include <cstdlib>
#include <string>
#include <vector>
#include <sstream>
#include <math.h>
#include "sterm.h"
#include "smovetable.h"

namespace cadiaplayer 
{
	namespace logic 
	{
		namespace utils 
		{
			typedef unsigned int Player;
			
			const double SM_TEMPERATURE = 10.0;
			const double rand_div = ((double)(RAND_MAX))+1.0;
			
			class SMove
				{
				public:
					
					SMove(  ) : 
					m_player( 0 ), m_stAction( STerm() ) { }
					
					SMove( Player player, STerm stAction ) : 
					m_player( player ), m_stAction( stAction ) { }
					
					std::string getStr() const { 
						std::stringstream ss;
						ss << m_stAction.getStrKIF();
						return ss.str();
					}
					
					Player getPlayer() const {
						return m_player;
					}
					
					STerm getActionTerm() const {  //Note: return by ref?
						return m_stAction;
					}
					
				private:
					Player m_player;
					STerm  m_stAction;
				};
			
			
			class SMoveSim
				{
				public:
					SMoveSim( const std::vector<SMove>& moves ) 
					: m_moves( moves ) 
					{
					}
					
					SMoveSim( unsigned int numElem ) 
					: m_moves( numElem ) 
					{
					}
					
					unsigned int size() const {
						return m_moves.size();
					}
					
					std::string getStr() const {
						std::stringstream ss;
						ss << "(";
						for ( unsigned int i=0 ; i<m_moves.size(); ++i ) {
							if ( i > 0 ) 
								ss << ' ';
							ss << m_moves[ i ].getStr();
						}
						ss << ")";
						return ss.str();
					}
					
					std::string getTransferStr() const {
						std::stringstream ss;
						for ( unsigned int i=0 ; i<m_moves.size(); ++i ) {
							if ( i > 0 ) ss << ' ';
							ss << m_moves[ i ].getStr();
						}
						return ss.str();
					}
					
					SMove getMove( unsigned int player ) const {
						return m_moves[ player ];
					}
					
				private:
					std::vector<SMove> m_moves;
				};
			
			
			class SMovelist
				{
				public:
					
					class SMIterator 
					{
					public:
						SMIterator( SMovelist* pml, bool atEnd = false ) 
						: m_pml( pml ), m_curr( pml->numPlayers() ), m_atEnd( atEnd )
						{
						}
						
						~SMIterator() 
						{
						}
						
						SMIterator& operator=( const SMIterator& other )
						{
							m_curr = other.m_curr;
							m_pml  = other.m_pml;
							return ( *this );
						}
						
						bool operator==( const SMIterator& other )
						{
							return ( m_pml  == other.m_pml  &&
									m_curr == other.m_curr &&
									m_atEnd == other.m_atEnd );
						}
						
						bool operator!=(const SMIterator& other)
						{
							return !( *this == other );
						}
						
						SMIterator& operator++()
						{
							for ( int p= static_cast<int>(m_pml->numPlayers())-1; p>=0; --p ) {
								m_curr[p]++;
								if ( m_curr[p] >= static_cast<int>(m_pml->numMoves(p)) ) {
									m_curr[p] = 0;
									if ( p == 0 ) m_atEnd = true;
								}
								else break;
							}
							return ( *this );
						}
						
						SMIterator operator++(int)
						{
							SMIterator tmp( *this );
							++(*this);
							return ( tmp );
						}
						
						const SMoveSim curr() 
						{
							std::vector<SMove> arr;
							arr.reserve( m_pml->numPlayers() );
							for ( Player p=0; p<m_pml->numPlayers(); ++p ) {
								arr.push_back( m_pml->getMove( p, m_curr[p] ) );
							}
							return ( SMoveSim( arr ) );
						}
						
					private:
						SMovelist        *m_pml;
						std::vector<int>  m_curr;
						bool              m_atEnd;
					};
					
					
					SMovelist( unsigned int numPlayers ) 
					: m_numElem(0), m_players( numPlayers )
					{
					}
					
					unsigned int numPlayers() const 
					{
						return m_players.size();
					}
					
					unsigned int numMoves( Player p ) const 
					{
						return m_players[ p ].size();
					}    
					
					bool empty() const 
					{
						return m_numElem == 0;
					}
					
					unsigned int size() const 
					{
						return m_numElem;
					}
					
					void add( Player p, const STerm& sterm ) {
						m_players[p].push_back( sterm );
						m_numElem++;
					}
					
					SMove getMove( Player p, unsigned int n ) const {
						return SMove( p, m_players[p][n] );
					} 
					
					SMove operator[] ( unsigned int n ) { 
						Player p;
						for ( p=0; p < m_players.size() ; ++p ) {
							unsigned int num = m_players[p].size();
							if ( num > n ) break;
							n -= num;
						}
						return this->getMove( p, n );
					}
					
					SMove getLastMove( Player p ) const {
						return SMove( p, m_players[p][m_players[p].size()-1] );
					} 
					
					SMoveSim getRandomSimMove() const {
						std::vector<SMove> arr;
						arr.reserve( this->numPlayers() );
						for ( Player p=0; p<this->numPlayers(); ++p ) {
							arr.push_back( this->getMove( p, rand() % m_players[p].size() ) );
						}
						return ( SMoveSim( arr ) );
					}
					
					SMoveSim getGibbsSimMove(SMoveTables& tables) const 
					{
						std::vector<SMove> arr;
						arr.reserve( this->numPlayers() );
						for ( Player p=0; p<this->numPlayers(); ++p ) 
						{
							if( m_players[p].size() == 1)
							{
								arr.push_back( this->getMove( p, 0 ));
								tables[p].addToPath(this->getMove( p, 0 ).getStr());
							}
							else
							{
								double softmax[ m_players[p].size()];
								size_t count = 0;
								double divider = 0;
								for(size_t n = 0 ; n < m_players[p].size() ; n++)
								{
									softmax[count] = exp(tables[p].get(this->getMove( p, n ).getStr())/SM_TEMPERATURE);
									divider+=softmax[count++];
								}
								double sel = rand()/rand_div;
								double pos = 0;
								size_t action = 0;
								for(size_t n = 0 ; n < count ; n++)
								{
									pos += softmax[n]/divider;
									if(pos > sel)
									{
										action = n;
										break;
									}
								}
								arr.push_back( this->getMove( p, action ));
								tables[p].addToPath(this->getMove( p, action ).getStr());
							}
						}
						return ( SMoveSim( arr ) );
					}
					
					SMIterator begin() 
					{
						return ( SMIterator( this ) ) ;
					}
					
					SMIterator end() 
					{
						return ( SMIterator( this, true /* at end */ ) ) ;
					}
					
				private:
					unsigned int                      m_numElem;
					std::vector< std::vector<STerm> > m_players;
				};
		}
	}
} // namespaces
#endif
