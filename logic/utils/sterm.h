/*
 *  STerm.h
 *  GGP
 *
 *  Created by Yngvi Bjornsson on 3/18/07.
 *  Copyright 2007 CADIA Reykjavík University. All rights reserved.
 *
 */
#ifndef INCL_STERM_H
#define INCL_STERM_H

#include <string>
#include <vector>
#include <iostream>
#include <cassert>
#include "Yap/YapInterface.h"

namespace cadiaplayer {
	namespace logic {
		namespace utils {
			
			const std::size_t HOLDER_LIMIT = 1000;
			const std::size_t HOLDER_LIMIT_SMALL = 100;
			
			class STerm 
				{
				public:
					
					class YapTermHolder {
					public:
						YapTermHolder( unsigned int maxTerms ) : m_pos(0), m_terms( maxTerms ) {
						}
						
						unsigned int reserve( unsigned int n ) {
							//assert( m_pos + n < m_terms.size() );
							accommodate( m_pos + n );
							int old = m_pos;
							m_pos += n;
							return old;
						}
						
						void setTerm( unsigned int pos, unsigned int i, YAP_Term term ) {
							//assert( pos+i < m_terms.size() );
							accommodate( pos+i );
							m_terms[pos+i] = term;
						}
						
						YAP_Term *addrOf( unsigned int pos ) {
							//assert( pos < m_terms.size() );
							accommodate( pos );
							return &m_terms[pos];
						}
						void accommodate( unsigned int pos ) {
							while( pos >= m_terms.size()){
								m_terms.resize(m_terms.size()*2);
								//std::cerr << "Resizing holder to " << m_terms.size() << std::endl; 
							}
						}
						
					private:
						unsigned int          m_pos;
						std::vector<YAP_Term> m_terms;
					};
					
					typedef enum { sTermInt, sTermFloat, sTermAtom, sTermPair,
					sTermAppl, sTermList, sTermUnknown } STermTag;
					
					STerm( YAP_Term term = 0 );
					
					STerm( const STerm& head, const STerm& tail );
					
					STerm( const std::vector<STerm>& stermlist );
					
					STerm( std::istream& istr );
					
					STermTag getTag() const {
						return m_tag;
					}
					
					std::string getStr() const;
					
					std::string getStrKIF() const;
					
					int getInt() const {
						return m_int;
					}
					
					YAP_Term mkYAPTerm( YapTermHolder& holder ) const;
					
					void toStream( std::ostream& ostr ) const; 
					
				private:
					
					STermTag    m_tag;
					
					int         m_int;
					float       m_float;
					std::string m_str;
					struct {
						std::string        name;
						std::vector<STerm> args;
					} m_comp;
				};
		}}} // namespaces
#endif
