/*
 *  breakthrough.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 6/18/12.
 *  Copyright 2012 Reykjavik University. All rights reserved.
 *
 */

#include "breakthrough.h"

using namespace cadiaplayer::logic::games;

BTState::BTState()
{
	reset();
}
BTState::~BTState()
{
}

BTPiece BTState::getSquare(const BTPos& col, const BTPos& row)
{
	if(col > 7 || col < 0)
		return BT_EMPTY_CELL;
	if(row > 7 || row < 0)
		return BT_EMPTY_CELL;
	
	BTBoard mask = static_cast<BTBoard>(1) << (row*8+col);
	if(whiteboard & mask)
		return BT_WHITE_PAWN;
	if(blackboard & mask)
		return BT_BLACK_PAWN;
	return BT_EMPTY_CELL;
}

void BTState::setSquare(const BTPos& col, const BTPos& row, const BTPiece& piece)
{
	BTBoard mask = static_cast<BTBoard>(1) << (row*8+col);
	if(!piece)
	{
		mask = ~mask;
		whiteboard = whiteboard & mask;
		blackboard = blackboard & mask;
		return;
	}
	if(piece == BT_WHITE_PAWN)
	{
		whiteboard = whiteboard | mask;
		blackboard = blackboard & (~mask);
		return;
	}
	blackboard = blackboard | mask;
	whiteboard = whiteboard & (~mask);
}
void BTState::clear()
{
	whiteboard = 0x0000000000000000ULL;
	blackboard = 0x0000000000000000ULL;

	turn = BT_WHITE_PAWN;
}
void BTState::reset()
{
	whiteboard = 0x000000000000FFFFULL;
	blackboard = 0xFFFF000000000000ULL;
	
	turn = BT_WHITE_PAWN;
	
	BTStateID sid;
	sid.white = whiteboard;
	sid.black = blackboard;
}

BTStateID BTState::getID()
{
	BTStateID id;
	id.white = whiteboard;
	id.black = blackboard;
	id.turn = turn;
	return id;
}

void BTState::getMoves(BTMoves& moves)
{
	BTMove from = 1;
	if(turn == BT_WHITE_PAWN)
	{
		from = from << 55;
		getLineMovesWhite(moves, from);
		getLineMovesWhite(moves, from);
		getLineMovesWhite(moves, from);
		getLineMovesWhite(moves, from);
		getLineMovesWhite(moves, from);
		getLineMovesWhite(moves, from);
		getLineMovesWhite(moves, from);
		
	}
	else
	{
		from = from << 8;
		getLineMovesBlack(moves, from);
		getLineMovesBlack(moves, from);
		getLineMovesBlack(moves, from);
		getLineMovesBlack(moves, from);
		getLineMovesBlack(moves, from);
		getLineMovesBlack(moves, from);
		getLineMovesBlack(moves, from);
		
	}
}

void BTState::getLineMovesWhite(BTMoves& moves, BTMove& from)
{
	BTMove to = 0;
	if(whiteboard & from)
	{
		to = from << 7;
		if(!(whiteboard & to))
			moves.push_back(from | to);
		to <<= 1;
		if(!((whiteboard|blackboard) & to))
			moves.push_back(from | to);
	}
	from >>= 1;
	for(int n = 0 ; n < 6 ; n++)
	{
		if(!(whiteboard & from))
		{
			from >>= 1;
			continue;
		}
		to = from << 7;
		if(!(whiteboard & to))
			moves.push_back(from | to);
		to <<= 1;
		if(!((whiteboard|blackboard) & to))
			moves.push_back(from | to);
		to <<= 1;
		if(!(whiteboard & to))
			moves.push_back(from | to);			
		from >>= 1;
	}
	if(whiteboard & from)
	{
		to = from << 8;
		if(!((whiteboard|blackboard) & to))
			moves.push_back(from | to);
		to <<= 1;
		if(!(whiteboard & to))
			moves.push_back(from | to);		
	}
	from >>= 1;
}

void BTState::getLineMovesBlack(BTMoves& moves, BTMove& from)
{
	BTMove to = 0;
	if(blackboard & from)
	{
		to = from >> 7;
		if(!(blackboard & to))
			moves.push_back(from | to);
		to >>= 1;
		if(!((whiteboard|blackboard) & to))
			moves.push_back(from | to);
	}
	from <<= 1;
	for(int n = 0 ; n < 6 ; n++)
	{
		if(!(blackboard & from))
		{
			from <<= 1;
			continue;
		}
		to = from >> 7;
		if(!(blackboard & to))
			moves.push_back(from | to);
		to >>= 1;
		if(!((whiteboard|blackboard) & to))
			moves.push_back(from | to);
		to >>= 1;
		if(!(blackboard & to))
			moves.push_back(from | to);			
		from <<= 1;
	}
	if(blackboard & from)
	{
		to = from >> 8;
		if(!((whiteboard|blackboard) & to))
			moves.push_back(from | to);
		to >>= 1;
		if(!(blackboard & to))
			moves.push_back(from | to);		
	}
	from <<= 1;
}

void BTState::make(const BTMove& move)
{
	if(move == NullMove)
	{	
		turn = turn == BT_BLACK_PAWN ? BT_WHITE_PAWN : BT_BLACK_PAWN;
		return;
	}
	
	if(turn == BT_WHITE_PAWN)
	{
		whiteboard ^= move;
		if(blackboard & move)
			blackboard &= (~move);
		turn = BT_BLACK_PAWN;
	}
	else
	{
		blackboard ^= move;
		if(whiteboard & move)
			whiteboard &= (~move);
		turn = BT_WHITE_PAWN;
	}
}

void BTState::retract(const BTStateID& id)
{
	syncState(id);
}

void BTState::syncState(const BTStateID& id)
{
	whiteboard = id.white;
	blackboard = id.black;
	turn = id.turn;
}


BTScore BTState::isTerminal()
{
	if(!whiteboard)
		return BT_BLACK_WINS;
	if(!blackboard)
		return BT_WHITE_WINS;
	if(whiteboard & 0xFF00000000000000ULL)
		return BT_WHITE_WINS;
	if(blackboard & 0x00000000000000FFULL)
		return BT_BLACK_WINS;
	
	return BT_NOT_OVER;
}

bool BTState::isCapture(const BTMove& move)
{
	if(turn == BT_WHITE_PAWN)
		return (whiteboard^move)&blackboard;
	else
		return (blackboard^move)&whiteboard;
}

bool BTState::strToMove(std::string strMove, BTMove& move)
{
	if(strMove.length() < 4)
		return false;
	BTPos fromCol = strMove[0] - BT_CHAR_OFFSET;
	if(fromCol < 0 || fromCol > 7)
		return false;
	BTPos fromRow = strMove[1] - '1';
	if(fromRow < 0 || fromRow > 7)
		return false;
	BTPos toCol = strMove[2] - BT_CHAR_OFFSET;
	if(toCol < 0 || toCol > 7)
		return false;
	BTPos toRow = strMove[3] - '1';
	if(toRow < 0 || toRow > 7)
		return false;
	if(fromRow == toRow)
		return false;
	if(fromRow < toRow && turn != BT_WHITE_PAWN)
		return false;
	if(fromRow > toRow && turn != BT_BLACK_PAWN)
		return false;
	if(fromCol - toCol > 1 || toCol - fromCol > 1)
		return false;
	
	BTMove mask = 1;
	move = (mask << (fromCol+fromRow*8)) | (mask << (toCol+toRow*8));
	
	return true;
}
bool BTState::setPosition(std::string pos)
{
	if(pos.length() < 65)
		return false;
	// First validate
	for(int n = 0 ; n < 64 ; n++)
	{
		if(pos[n] != 'w' && pos[n] != 'b' && pos[n] != '.')
			return false;
	}
	if(pos[64] != 'W' && pos[64] != 'B' ) 
		return false;
	for(int n = 0 ; n < 64 ; n++)
	{
		if(pos[n] == 'w')
			setSquare(n%8, n/8, BT_WHITE_PAWN);
		else if(pos[n] == 'b')
			setSquare(n%8, n/8, BT_BLACK_PAWN);
		else
			setSquare(n%8, n/8, BT_EMPTY_CELL);
	}
	turn = BT_WHITE_PAWN;
	if(pos[64] == 'B' ) 
		turn = BT_BLACK_PAWN;
	
	return true;
}

std::string BTState::toString()
{
	BTPiece piece;
	std::stringstream ss;
	ss << "     a b c d e f g h" << std::endl << std::endl;
	for(BTPos n = 7 ; n >= 0 ; n--)
	{
		ss << (n+1) << "    ";
		for(BTPos m = 0 ; m < 8 ; m++)
		{
			piece = getSquare(m, n);
			if(piece == BT_WHITE_PAWN)
				ss << "w ";
			else if(piece == BT_BLACK_PAWN)
				ss << "b ";
			else
				ss << ". ";
		}
		ss << "   " << (n+1) << std::endl;
	}
	ss << std::endl <<"     a b c d e f g h" << std::endl;
	return ss.str();
}
