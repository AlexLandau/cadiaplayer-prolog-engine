/*
 *  STerm.cpp
 *  GGP
 *
 *  Created by Yngvi Bjornsson on 3/18/07.
 *  Copyright 2007 CADIA Reykjavík University. All rights reserved.
 *
 */
#include <iostream>
#include <sstream>
#include "sterm.h"
using namespace cadiaplayer::logic::utils;
using namespace std;

STerm::STerm( YAP_Term term )
    : m_tag( sTermUnknown ), m_int( 0 ), m_float( 0.0 )
{
    if ( YAP_IsIntTerm( term ) ) { 
        m_tag = sTermInt;
        m_int = YAP_IntOfTerm( term );
    }
    else if ( YAP_IsFloatTerm( term ) ) { 
        m_tag = sTermFloat;
        m_float = YAP_FloatOfTerm( term ); 
    }
    else if ( YAP_IsAtomTerm( term ) ) { 
        m_tag = sTermAtom;
        m_str = YAP_AtomName( YAP_AtomOfTerm( term ) ); 
    }
    else if ( YAP_IsPairTerm(term)  ) {
        m_tag = sTermPair;
        m_comp.args.push_back( STerm( YAP_HeadOfTerm( term ) ) );
        m_comp.args.push_back( STerm( YAP_TailOfTerm( term ) ) );
    }
    else if ( YAP_IsApplTerm(term)  ) {
        m_tag = sTermAppl;
        YAP_Functor fnct = YAP_FunctorOfTerm( term );
        m_comp.name = YAP_AtomName( YAP_NameOfFunctor( fnct ) );
        for ( unsigned int i=1; i<=YAP_ArityOfFunctor( fnct ); ++i ) {
            m_comp.args.push_back( STerm( YAP_ArgOfTerm( i, term ) ) );
        }
    }
    else {
        m_tag = sTermUnknown;
    }
}


STerm::STerm( const STerm& head, const STerm& tail )
: m_tag( sTermUnknown ), m_int( 0 ), m_float( 0.0 )
{
    m_tag = sTermList;
    m_comp.args.push_back( head );
    m_comp.args.push_back( tail );
}


STerm::STerm( const std::vector<STerm>& stermlist )
    : m_tag( sTermUnknown ), m_int( 0 ), m_float( 0.0 )
{
    m_tag = sTermList;
    for ( unsigned int i=0; i<stermlist.size(); ++i ) {
        m_comp.args.push_back( stermlist[i] );
    }
}


STerm::STerm( istream& istr )
    : m_tag( sTermUnknown ), m_int( 0 ), m_float( 0.0 )
{
    unsigned int n = 0;
    char c;    
    istr >> c;
    if ( c != '(' ) return;
    istr >> c;        
    switch ( c ) {
        case 'I':  
            m_tag = sTermInt;
            istr >> m_int;
            break;
        case 'F':
            m_tag = sTermInt;
            istr >> m_float;
            break;
        case 'A':
            m_tag = sTermAtom;
            istr >> m_str;
            break;
        case 'P':
            m_tag = sTermPair;
            m_comp.args.push_back( STerm( istr ) );
            m_comp.args.push_back( STerm( istr ) );
            break;
        case 'L':
            m_tag = sTermList;
            istr >> n;
            for ( unsigned int i=0; i < n; ++i ) {
                m_comp.args.push_back( STerm( istr ) );
            }
            break;
        case 'C':
            m_tag = sTermAppl;
            istr >> m_comp.name;
            istr >> n;
            for ( unsigned int i=0; i < n; ++i ) {
                m_comp.args.push_back( STerm( istr ) );
            }
            break;
    }
        
    istr >> c;  // )
}


void STerm::toStream( ostream& ostr ) const 
{
    ostr << "( ";
    if ( m_tag == sTermInt ) { 
        ostr << "I " << m_int;
    }
    else if ( m_tag == sTermFloat ) { 
        ostr << "F " << m_float;
    }
    else if ( m_tag == sTermAtom ) { 
        ostr << "A " << m_str.c_str();
    }
    else if ( m_tag == sTermPair ) {
        ostr << "P ";
        m_comp.args[0].toStream( ostr );
        m_comp.args[1].toStream( ostr );
    }
    else if ( m_tag == sTermList ) {
        ostr << "L " << m_comp.args.size();
        for ( unsigned int i=0; i < m_comp.args.size(); ++i ) {
            m_comp.args[i].toStream( ostr );
        }
    }
    else if ( m_tag == sTermAppl ) {
        ostr << "C " << m_comp.name.c_str() << ' ' << m_comp.args.size()  << ' ';
        for ( unsigned int i=0; i < m_comp.args.size(); ++i ) {
            m_comp.args[i].toStream( ostr );
        }
    }
    else {
        ostr << "? ";
    }
    ostr << " )";    
}


std::string STerm::getStr( ) const 
{
    std::stringstream ss;
    if ( m_tag == sTermInt ) { 
        ss << m_int;
    }
    else if ( m_tag == sTermFloat ) { 
        ss << m_float;
    }
    else if ( m_tag == sTermAtom ) { 
        ss << m_str;
    }
    else if ( m_tag == sTermPair ) {
        ss << '[';
        ss << m_comp.args[0].getStr() << ',';
        ss << m_comp.args[1].getStr() << ']';
    }
    else if ( m_tag == sTermList ) {
        ss << '[';
        for ( unsigned int i=0; i<m_comp.args.size(); ++i ) {
            if ( i > 0 ) ss << ',';
            ss << m_comp.args[i].getStr();
        }
        ss << ']';
    }
    else if ( m_tag == sTermAppl ) {
        ss << m_comp.name << '(';
        for ( unsigned int i=0; i<m_comp.args.size(); ++i ) {
            if ( i > 0 ) ss << ',';
            ss << m_comp.args[i].getStr();
        }
        ss << ')';
    }
    else {
        ss << '?';
    }
    
    return ss.str();
}


std::string STerm::getStrKIF( ) const 
{
    std::stringstream ss;
    if ( m_tag == sTermInt ) { 
        ss << m_int;
    }
    else if ( m_tag == sTermFloat ) { 
        ss << m_float;
    }
    else if ( m_tag == sTermAtom ) { 
        ss << m_str;
    }
    else if ( m_tag == sTermPair ) {
        ss << '[';
        ss << m_comp.args[0].getStr() << ',';
        ss << m_comp.args[1].getStr() << ']';
    }
    else if ( m_tag == sTermList ) {
        ss << '[';
        for ( unsigned int i=0; i<m_comp.args.size(); ++i ) {
            if ( i > 0 ) ss << ',';
            ss << m_comp.args[i].getStr();
        }
        ss << ']';
    }
    else if ( m_tag == sTermAppl ) {
        ss << '(' << m_comp.name;
        for ( unsigned int i=0; i<m_comp.args.size(); ++i ) {
            ss << ' ';
            ss << m_comp.args[i].getStr();
        }
        ss << ')';
    }
    else {
        ss << '?';
    }
    
    return ss.str();
}



YAP_Term STerm::mkYAPTerm( STerm::YapTermHolder& holder ) const 
{
    YAP_Term term;
    if ( m_tag == sTermInt ) { 
        term = YAP_MkIntTerm( m_int );
    }
    else if ( m_tag == sTermFloat ) { 
        term = YAP_MkFloatTerm( m_float );
    }
    else if ( m_tag == sTermAtom ) { 
        term = YAP_MkAtomTerm( YAP_LookupAtom( (char*)m_str.c_str() ) );
    }
    else if ( m_tag == sTermPair ) {
        term = YAP_MkPairTerm(
                              m_comp.args[0].mkYAPTerm( holder ),
                              m_comp.args[1].mkYAPTerm( holder )
                              );
    }
    else if ( m_tag == sTermList ) {  // NOTE: need to check better.
        int n = static_cast<int>( m_comp.args.size() );
        term = YAP_MkAtomTerm( YAP_FullLookupAtom( "[]" ) );
        for ( int i=n-1; i>=0; --i ) {
            term = YAP_MkPairTerm( m_comp.args[i].mkYAPTerm( holder ),
                                   term );
        }
    }
    else if ( m_tag == sTermAppl ) {
        unsigned int n = m_comp.args.size();
        YAP_Functor yapFnct = YAP_MkFunctor( YAP_LookupAtom( (char*) m_comp.name.c_str() ), n );
        int pos = holder.reserve( n );
        for ( unsigned int i=0; i<n; ++i ) {
            holder.setTerm( pos, i, m_comp.args[i].mkYAPTerm( holder ) );
        }
        term = YAP_MkApplTerm( yapFnct, n, holder.addrOf(pos) );
    }
    else {
        term = 0;
    }
    
    return term;
}

