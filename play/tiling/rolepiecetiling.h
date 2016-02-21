/*
 *  rolepiecetiling.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 1/7/10.
 *  Copyright 2010 Reykjavik University. All rights reserved.
 *
 */

#ifndef ROLEPIECETILING_H
#define ROLEPIECETILING_H

#include "tiling.h"

namespace cadiaplayer {
	namespace play {
		namespace tiling {
			const int ROLEPIECETILING_ID = 5;
			static char ROLEPIECETILING_NAME[] = "Role Piece Tiling";
			
			const TileKey RolePieceSalt = 0xABCDABCD;
			
			class RolePieceTiling : public Tiling
				{
				public:
					unsigned int index;
					RoleIndex role;
					unsigned int roleindex;
					
					RolePieceTiling();
					RolePieceTiling(cadiaplayer::play::parsing::SymbolTable* s, TileArguments& args);
					virtual TileKey getFeature(cadiaplayer::play::parsing::Compound* c);
					TileKey getFeature(cadiaplayer::play::Move* m, cadiaplayer::play::RoleIndex r);
					virtual std::string toString(cadiaplayer::play::parsing::Compound* c);
					virtual int getId(void){return ROLEPIECETILING_ID;};
					virtual char* getName(void){return ROLEPIECETILING_NAME;};
				};
		}}}

#endif // ROLEPIECETILING_H
