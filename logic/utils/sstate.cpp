/*
 *  SState.cpp
 *  GGP
 *
 *  Created by Yngvi Bjornsson on 3/18/07.
 *  Copyright 2007 CADIA Reykjavík University. All rights reserved.
 *
 */
#include <iostream>
#include <utility>
#include <vector>
#include "sstate.h"

using namespace cadiaplayer::logic::utils;
using namespace std;

vector<string> SState::m_playerNames;
bool SState::m_debug = false;


bool SState::init( string compiledPrologFile )
{
#ifdef USE_YAPINIT
    YAP_init_args y_args;  // For YAP_Init if used.
    memset( &y_args, 0, sizeof( y_args ) ); 
    y_args.SavedState = (char*) compiledPrologFile.c_str();
    if ( YAP_Init( &y_args ) == YAP_BOOT_ERROR ) {
        return false;
    }
#else
    if ( YAP_FastInit( (char*)compiledPrologFile.c_str() ) == YAP_BOOT_ERROR ) {
        return false;
    }
#endif
    
    // Prolog call: ?- state_init( Players ).
    static YAP_Functor yapFnct = YAP_MkFunctor( YAP_LookupAtom("state_init"), 1 );
    
    YAP_Term    yapGoal = YAP_MkNewApplTerm( yapFnct, 1 );
    int okay = YAP_RunGoal( yapGoal );
    while ( okay != 0 ) {
        YAP_Term result = YAP_ArgOfTerm( 1, yapGoal );
        m_playerNames.push_back( STerm( result ).getStr() );
        okay = YAP_RestartGoal();
    }
    YAP_Reset();
    return m_playerNames.size() > 0;
}


void SState::display( )
{
    if ( m_debug )
        cout << "-> SState::display" << endl;

    // Prolog call: ?- state( Clauses ).
    static YAP_Functor yapFnct = YAP_MkFunctor( YAP_LookupAtom("state"), 1 );

    YAP_Term    yapGoal = YAP_MkNewApplTerm( yapFnct, 1 );
    int okay = YAP_RunGoal( yapGoal );
    while ( okay != 0 ) {
        YAP_Term result = YAP_ArgOfTerm( 1, yapGoal );
        cout <<  "\tState: '" << STerm( result ).getStr() << '\'' << endl;
        okay = YAP_RestartGoal();
    }
    YAP_Reset();

    if ( m_debug )
        cout << "<- SState::display" << endl;
}


bool SState::isTerminal()
{
    if ( m_debug )
        cout << "-> SState::isTerminal" << endl;

    // Prolog call: ?- is_terminal( L ).
    int okay = YAP_RunGoal( YAP_MkAtomTerm( YAP_LookupAtom("state_is_terminal")  ) ) != 0;
    YAP_Reset();   
    
    if ( m_debug )
        cout << "<- SState::isTerminal" << endl;

    return okay;
}


SRetract* SState::getRetractInfo(  )
{
    if ( m_debug )
        cout << "-> SState::getRetractInfo" << endl;

    // Prolog call: ?- state_clauses( L ).
    static YAP_Functor yapFnct = YAP_MkFunctor( YAP_LookupAtom("state_retract_info"), 1 );
    SRetract *pRI = NULL;
    
    YAP_Term    yapGoal = YAP_MkNewApplTerm( yapFnct, 1 );
    int okay = YAP_RunGoal( yapGoal );
    if ( okay != 0 ) {
      if ( m_debug )
	cout << STerm( YAP_ArgOfTerm( 1, yapGoal ) ).getStr() << endl;
      pRI =  new SRetract( STerm( YAP_ArgOfTerm( 1, yapGoal ) ) );
    }
    YAP_Reset();

    if ( m_debug )
        cout << "<- SState::getRetractInfo" << endl;

    //assert( okay != 0 );
    return pRI;
}


void SState::getMoves( SMovelist& ml )
{
    if ( m_debug )
        cout << "-> SState::getMoves" << endl;

    // Prolog call: ?- state_gen_moves( P, M ).
    static YAP_Functor yapFnct = YAP_MkFunctor( YAP_LookupAtom("state_gen_moves"), 2 );
    
    YAP_Term yapGoal = YAP_MkNewApplTerm( yapFnct, 2 );
    int      okay    = YAP_RunGoal( yapGoal );
    while ( okay != 0 ) {
        YAP_Term resPlayer = YAP_ArgOfTerm( 1, yapGoal );
        YAP_Term resAction = YAP_ArgOfTerm( 2, yapGoal );
        unsigned int player = SState::playerNumber( YAP_AtomName( YAP_AtomOfTerm( resPlayer ) ) );
        ml.add( player, resAction );
        if ( m_debug )
            cout << "\tMove:  '" << ml.getLastMove(player).getStr() << '\'' << endl;
        okay = YAP_RestartGoal();
    }    
    YAP_Reset();
    
    if ( m_debug )
        cout << "<- SState::getMoves" << endl;

    //assert( !ml.empty() );
}


bool SState::make( SMove move, SRetract& /* ri */ )
{
    if ( m_debug )
        cout << "-> SState::make" << endl;

    // Prolog call: ?- state_make_move( <player>, <move_term> ).
    static YAP_Functor   yapFnct = YAP_MkFunctor( YAP_LookupAtom("state_make_move"), 2 );

    STerm::YapTermHolder holder( HOLDER_LIMIT );
    int pos = holder.reserve( 2 );

    YAP_Term playerTerm = YAP_MkAtomTerm( 
                             YAP_LookupAtom( (char*)SState::playerName(move.getPlayer()).c_str() ) );
    holder.setTerm( pos, 0, playerTerm ); 
    holder.setTerm( pos, 1, move.getActionTerm().mkYAPTerm( holder ) ); 
    
    YAP_Term yapGoal = YAP_MkApplTerm( yapFnct, 2, holder.addrOf(pos) );
    int      okay    = YAP_RunGoal( yapGoal );
    YAP_Reset();

    if ( m_debug )
        cout << "<- SState::make" << endl;

    //assert( okay != 0 );
    return okay != 0;
}


bool SState::make_sim( const SMoveSim& sm, SRetract* /* ri */ )
{
    if ( m_debug )
        cout << "-> SState::make_sim" << endl;

    // Prolog call: ?- state_make_sim_moves( [ [<p>,<m>], ... ] ).
    static YAP_Functor yapFnct = YAP_MkFunctor( YAP_LookupAtom("state_make_sim_moves"), 1 );
    
    vector<STerm> sterms;
    for ( unsigned int i=0; i<sm.size(); ++i ) {
        YAP_Term playerTerm = YAP_MkAtomTerm( 
                YAP_LookupAtom( (char*)SState::playerName(sm.getMove(i).getPlayer()).c_str() ) );
        sterms.push_back( STerm( playerTerm, sm.getMove(i).getActionTerm() ) );
    }
    //cout << STerm( sterms ).getStr() << endl;
    STerm::YapTermHolder holder( HOLDER_LIMIT );
    YAP_Term argTerm = STerm( sterms ).mkYAPTerm( holder );
    YAP_Term yapGoal = YAP_MkApplTerm( yapFnct, 1, &argTerm );
    int okay = YAP_RunGoal( yapGoal );    
    YAP_Reset();

    if ( m_debug )
        cout << "<- SState::make_sim" << endl;

    //assert( okay != 0 );
    return okay;
}


bool SState::retract( SRetract* ri )
{
    if ( m_debug )
        cout << "-> SState::retract" << endl;
    
    // Prolog call: ?- state_retract_move( <clause_list> ).
    static YAP_Functor yapFnct = YAP_MkFunctor( YAP_LookupAtom("state_retract_move"), 1 );

    STerm::YapTermHolder holder( HOLDER_LIMIT);
    YAP_Term argTerm = ri->getSTerm().mkYAPTerm( holder );
    YAP_Term yapGoal = YAP_MkApplTerm( yapFnct, 1, &argTerm );
    
    int okay = YAP_RunGoal( yapGoal );    
    YAP_Reset();

    if ( m_debug )
        cout << "<- SState::retract" << endl;

    //assert( okay != 0 );
    return okay;
}


bool SState::goal( SState::Value& goals )
{
    if ( m_debug )
        cout << "-> SState::goal" << endl;

    // Prolog call: ?- state_goal( Player, Value ).
    static YAP_Functor   yapFnct = YAP_MkFunctor( YAP_LookupAtom("state_goal"), 2 );
    
    YAP_Term yapGoal = YAP_MkNewApplTerm( yapFnct, 2 );
    int      okay    = YAP_RunGoal( yapGoal );
    while ( okay != 0 ) {
        YAP_Term resPlayer = YAP_ArgOfTerm( 1, yapGoal );
        YAP_Term resValue  = YAP_ArgOfTerm( 2, yapGoal );
//        cout << "Goal:  '" << STerm(resPlayer).getStr() << ' ' 
//                           << STerm(resValue).getStr()  << '\'' << endl;    
        unsigned int player = SState::playerNumber( YAP_AtomName( YAP_AtomOfTerm( resPlayer ) ) );
        if( player < SState::numPlayers() )
			goals.setValue( player, STerm(resValue).getInt() );
        okay = YAP_RestartGoal();
    }    
    YAP_Reset();

    if ( m_debug )
        cout << "<- SState::goal" << endl;

    return true;
}
