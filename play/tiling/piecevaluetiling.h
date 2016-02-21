/*
 *  piecevaluetiling.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 1/13/10.
 *  Copyright 2010 Reykjavik University. All rights reserved.
 *
 */

#ifndef PIECEVALUETILING_H
#define PIECEVALUETILING_H

#include "valuetiling.h"

namespace cadiaplayer {
	namespace play {
		namespace tiling {
			const int PIECEVALUETILING_ID = 12;
			static char PIECEVALUETILING_NAME[] = "Piece Value Tiling";
			class PieceValueTiling : public ValueTiling
				{
				public:
					unsigned int index;
					unsigned int roleCount;
					PieceValueTiling(){};
					PieceValueTiling(cadiaplayer::play::parsing::SymbolTable* s, TileArguments& args);
					virtual TileKey getFeature(cadiaplayer::play::parsing::Compound* c);
					virtual std::string toString(cadiaplayer::play::parsing::Compound* c)
					{return index < c->getArguments()->size() ? c->getArgument(index)->toString(symbols) : "[n/a]" ;};
					virtual int getId(void){return PIECEVALUETILING_ID;};
					virtual char* getName(void){return PIECEVALUETILING_NAME;};
				};
		}}}

#endif // PIECEVALUETILING_H
