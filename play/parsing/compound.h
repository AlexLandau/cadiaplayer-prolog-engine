#ifndef COMPOUND_H
#define COMPOUND_H

#include <vector>
#include <string>
#include <cassert>

#include "symtab.h"

namespace cadiaplayer{
	namespace play{
		namespace parsing{

			const SymbolID NO_NAME = 0;
			
			enum CompoundType
			{
				ct_COMPOUND=0,
				ct_VARIABLE,
				ct_VALUE
			};
			
			enum CompoundOperator
			{
				co_NONE=0,
				co_NOT,
				co_AND,
				co_OR,
				co_DISTINCT,
				co_PREDICATE,
				co_IMPLICATION,
				co_ATOM,
				co_INT,
				co_RELATION,
				co_RELATION_ROLE,
				co_RELATION_INIT,
				co_RELATION_NEXT,
				co_RELATION_DOES,
				co_RELATION_TRUE,
				co_RELATION_LEGAL,
				co_RELATION_GOAL,
				co_RELATION_TERMINAL
			};
			static const char* CompoundOperatorStrings[] =
			{
			"[none]",
			"not",
			"and",
			"or",
			"distinct",
			"[pred]",
			"<=",
			"[atom]",
			"[int]",
			"[rel]",
			"role",
			"init",
			"next",
			"does",
			"true",
			"legal",
			"goal",
			"terminal"
			};
			
			class Compound;
			typedef std::vector<Compound*> CompoundArgs;
			typedef std::vector<Compound*> CompoundList;
			typedef CompoundArgs::iterator CompoundArgsItr;
			
			class Compound
				{
				private:
					SymbolID name;
					CompoundOperator op;
					CompoundArgs* args;
					CompoundType type;
					Compound* parent;
				public:
					Compound();
					Compound(CompoundType t, SymbolID n, CompoundOperator o);
					Compound(const Compound &c);
					virtual ~Compound(void);
					
					friend int operator ==(const Compound& c1, const Compound& c2)
					{
						if(c1.op != c2.op)
							return c1.op - c2.op;
						if(c1.type != c2.type)
							return c1.type - c2.type;
						if(c1.name != c2.name)
							return c1.name - c2.name;
						if(c1.args == NULL)
						{
							if(c2.args == NULL)
								return 0;
							return -1;
						}
						if(c2.args == NULL)
							return 1;
						
						if(c1.args->size() != c2.args->size())
							return c1.args->size() - c2.args->size();
						Compound *arg1, *arg2; 
						int argc = 0;
						for(size_t n = 0 ; n < c1.args->size() ; n++)
						{
							arg1 = (*(c1.args))[n];
							arg2 = (*(c2.args))[n];
							argc = ((*arg1) == (*arg2));
							if(argc == 0)
								continue;
							return argc;
						}
						return 0;
					};
					int operator ==(const Compound& c);
					int operator <(const Compound& c);
					Compound& operator =(const Compound& c);
					
					CompoundType getType(void) const;
					SymbolID  getName(void) const;
					CompoundOperator getOperator(void) const;
					CompoundArgs* getArguments(void) const;
					Compound* getArgument(size_t i) const;
					Compound *getParent(void) const;
					
					void setType(CompoundType t);
					void setName(SymbolID  n);
					void setOperator(CompoundOperator o);
					void addArgument(Compound* c);
					void addArgumentFront(Compound* c);
					void removeArgumentFront(void);
					void setParent(Compound* c);
					
					bool hasVariable(void) const;
					bool isGround(void) const;
					std::string toString(SymbolTable* table) const;
					std::string toPlayString(SymbolTable* table);
					long sizeInBytes(void);
					
					static std::string operatorToString(CompoundOperator op)
					{
						return CompoundOperatorStrings[op];
					}
					
					static std::string toSaveString(Compound* c);
					static Compound* fromSaveString(const std::string& str, std::size_t& index);
				};
			
			struct compoundCompare
			{
				bool operator()(const Compound* s1, const Compound* s2) const
				{
					return *s1 == *s2;
				}
			};
			
			struct compoundCompareLessThan
			{
				bool operator()(const Compound* s1, const Compound* s2) const
				{
					return (*s1 == *s2) < 0;
				}
			};
		}}} // namespaces
#endif // COMPOUND_H
