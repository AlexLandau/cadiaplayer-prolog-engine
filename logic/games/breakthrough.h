/*
 *  breakthrough.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 6/18/12.
 *  Copyright 2012 Reykjavik University. All rights reserved.
 *
 */

#ifndef BT_STATE_H
#define BT_STATE_H

#include <vector>
#include <stack>
#include <iostream>
#include <sstream>
#include <stdint.h>

namespace cadiaplayer 
{
	namespace logic 
	{
		namespace games 
		{

typedef unsigned short BTPiece;
const BTPiece BT_EMPTY_CELL		= 0x00;  // 00
const BTPiece BT_WHITE_PAWN		= 0x01;  // 01
const BTPiece BT_BLACK_PAWN		= 0x02;  // 10
const BTPiece BT_CAPTURE_CELL	= 0x03;  // 11
const BTPiece BT_PIECE_MASK		= 0x03;  // 11

typedef int BTPos;
const BTPos BT_CHAR_OFFSET = 'a';

typedef uint64_t BTBoard;
struct BTStateID
{
	BTBoard white;
	BTBoard black;
	BTPiece turn;
};
typedef BTBoard BTKey;
typedef BTBoard BTMove;
const BTMove NullMove = 0;
typedef std::vector<BTMove> BTMoves;
typedef std::vector<BTStateID> History;

typedef unsigned short BTScore;
const BTScore BT_NOT_OVER		= 0x00;  // 00
const BTScore BT_WHITE_WINS		= 0x01;  // 01
const BTScore BT_BLACK_WINS		= 0x02;  // 10
const BTScore BT_DRAW			= 0x03;  // 11

class BTState
{
public:
	BTBoard whiteboard;
	BTBoard blackboard;
private:
	BTPiece turn;
	History history;
	void getLineMovesWhite(BTMoves& moves, BTMove& from);
	void getLineMovesBlack(BTMoves& moves, BTMove& from);
public:
	BTState();
	~BTState();
	
	BTPiece getSquare(const BTPos& col, const BTPos& row);
	void setSquare(const BTPos& col, const BTPos& row, const BTPiece& piece);
	void clear();
	void reset();
	BTStateID getID();
	BTBoard getWhiteboard(){return whiteboard;};
	BTBoard getBlackboard(){return blackboard;};
	BTPiece getTurn(){return turn;};
	void setTurn(const BTPiece& piece){turn = piece;};
	
	void getMoves(BTMoves& moves);
	bool isPieceAt(BTBoard& board, BTPos& pos);
	void make(const BTMove& move);  // Returns true if a piece was captured;
	void retract(const BTStateID& id);
	void syncState(const BTStateID& id);
	BTScore isTerminal();
	bool isCapture(const BTMove& move);
	
	
	bool strToMove(std::string strMove, BTMove& move); 
	bool setPosition(std::string pos);
	
	std::string toString();
	
	static void getPieceName(const BTPiece& piece, std::ostream& istr)
	{
		if(piece == BT_WHITE_PAWN)
			istr << "White";
		else if(piece == BT_BLACK_PAWN)
			istr << "Black";
		else
			istr << "n/a";
	};
	
	static std::string moveToString(const BTMove& move, BTPiece turn, bool capture)
	{
		std::stringstream ss;
		BTMove mask = 1;
		BTMove temp = move;
		char firstcol = 'a';
		int firstrow = 0;
		char secondcol = 'a';
		int secondrow = 0;
		int i;
		for(i = 0 ; i < 64 ; i++)
		{
			if(temp & mask)
			{
				firstcol = (char)(i%8+BT_CHAR_OFFSET);
				firstrow = i/8 + 1;
				break;
			}
			temp = temp >> 1;
		}
		temp = temp >> 1;
		for(i+=1; i < 64 ; i++)
		{
			if(temp & mask)
			{
				secondcol = (char)(i%8+BT_CHAR_OFFSET); 
				secondrow = i/8 + 1;
				break;
			}
			temp = temp >> 1;
		}
		if(turn == BT_WHITE_PAWN)
		{
			ss << firstcol << firstrow;
			if(capture)
				ss << "x";
			else
				ss << "-";
			ss << secondcol << secondrow;
			
		}
		else
		{
			ss << secondcol << secondrow;
			if(capture)
				ss << "x";
			else
				ss << "-";
			ss << firstcol << firstrow;
		}
		return ss.str();
	};
	static std::string moveToString(const BTMove& move, BTState &state)
	{
		return moveToString(move, state.getTurn(), state.isCapture(move));
	}
	static std::string moveToPlayString(const BTMove& move, BTPiece turn)
	{
		std::stringstream ss;
		BTMove mask = 1;
		BTMove temp = move;
		char firstcol = 'a';
		int firstrow = 0;
		char secondcol = 'a';
		int secondrow = 0;
		int i;
		for(i = 0 ; i < 64 ; i++)
		{
			if(temp & mask)
			{
				firstcol = (char)(i%8+BT_CHAR_OFFSET);
				firstrow = i/8 + 1;
				break;
			}
			temp = temp >> 1;
		}
		temp = temp >> 1;
		for(i+=1; i < 64 ; i++)
		{
			if(temp & mask)
			{
				secondcol = (char)(i%8+BT_CHAR_OFFSET); 
				secondrow = i/8 + 1;
				break;
			}
			temp = temp >> 1;
		}
		if(turn == BT_WHITE_PAWN)
		{
			ss << firstcol << firstrow;
			ss << secondcol << secondrow;
		}
		else
		{
			ss << secondcol << secondrow;
			ss << firstcol << firstrow;
		}
		return ss.str();
	}
	static std::string boardToBitString(const BTBoard& board)
	{
		BTBoard temp = board;
		BTBoard mask = 1;
		std::stringstream ss;
		for(int n = 0 ; n < 63 ; n++)
		{
			if(temp & mask)
				ss << "1";
			else
				ss << "0";
			
			temp = temp >> 1;	
		}
		return ss.str();
	};
	static std::string moveToBitString(const BTMove& move)
	{
		return boardToBitString(move);
	}
};
		} // namespace cadiaplayer
	} // namespace games
} // namespace logic

#endif // BT_STATE_H
