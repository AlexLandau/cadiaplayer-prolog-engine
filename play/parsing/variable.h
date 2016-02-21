#ifndef VARIABLE_H
#define VARIABLE_H

#include "compound.h"
namespace cadiaplayer{
	namespace play{
		namespace parsing{
			
			class Variable : public Compound
				{
				public:
					Variable(Compound& c);	
					Variable(unsigned int name);
					Variable(CompoundOperator o);
					Variable(unsigned int name, CompoundOperator o);
				};
		}}} // namespaces
#endif // VARIABLE_H
