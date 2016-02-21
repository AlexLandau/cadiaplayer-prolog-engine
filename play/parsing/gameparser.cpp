#include "gameparser.h"
#include "lex.yy.cc"

using namespace cadiaplayer::play::parsing;

GameParser::GameParser(bool quiet) :
variableName(1),
quietmode(quiet),
error(false),
binaryRelations(NULL),
relationCounters(NULL)
{
}

GameParser::~GameParser()
{
}
Token* GameParser::getNextToken()
{
	return nextToken();
}
bool GameParser::parseFile(cadiaplayer::play::GameTheory* t, const char* filename)
{
	if(t== NULL)
		return false;
	stored.clear();
	// Check if file exists
	struct stat stFileInfo; 
	int intStat; 
	// Attempt to get the file attributes 
	intStat = stat(filename,&stFileInfo); 
	if(intStat)
	{
		std::cerr << "File not found for parsing: " << filename << std::endl;
		return false;
	}
	theory = t;
	theory->setParser(this);
	table = t->getSymbols();
	binaryRelations = t->getBinaryRelations();
	if(binaryRelations) 
		binaryRelations->clear();
	relationCounters = t->getRelationCounters();
	if(relationCounters) 
		relationCounters->clear();
	
	allocateLexerToFile(filename);
	bool ok = parse();
	/*if(ok)
	{
		theory->setKifFile(filename);
		theory->initTheory();
	}*/
	freeLexer();
	
	return ok;
}

bool GameParser::parseString(cadiaplayer::play::GameTheory* t, const std::string gdl)
{
	if(t == NULL)
		return false;
	theory = t;
	theory->setParser(this);
	table = t->getSymbols();
	
	std::stringbuf sbuf(gdl);
	allocateLexer(new std::istream(&sbuf), &std::cout);
	bool ok = parse();
	//if(ok)
	//	theory->initTheory();
	freeStringLexer();
	return ok;
}

bool GameParser::parseString(cadiaplayer::play::GameTheory* t, const std::string gdl, CompoundList* result)
{
	if(t == NULL)
		return false;
	//bool q = quietmode;
	//quietmode = true;
	theory = t;
	theory->setParser(this);
	table = t->getSymbols();
	
	std::stringbuf sbuf(gdl);
	std::istream* is = new std::istream(&sbuf);
	allocateLexer(is, &std::cout);
	bool ok = parse(result);
	freeStringLexer();
	delete is;
	//quietmode = q;
	return ok;
}
bool GameParser::parse(CompoundList* result)
{
	Token* token = getNextToken();
	if(token->getTokenCode() == tc_EOF)
		return false;
	variableName = 1;
	variableTable.clear();
	Compound* sentence = NULL;
	depth = 0;
	int sentenceCount = 0;
	int commentCount = 0;
	bool ok = true;
	while(token->getTokenCode() != tc_EOF)
	{
		if(token->getTokenCode() == tc_COMMENT)
		{
			token = getNextToken();
			commentCount++;
			continue;
		}
		if(token->getTokenCode() == tc_LPAREN)
		{
			sentence = parseSentence(NULL);
			std::flush(std::cout);
			standardizeApart(sentence);
			if(result == NULL)
				store(sentence);
			else
				result->push_back(sentence);
			sentenceCount++;
		}
		else if(token->getTokenCode() == tc_CONSTANT)
		{
			sentence = new Compound(ct_VALUE, table->insertConstant(token->getLexeme()), co_ATOM);
			if(token->getOpType() == op_INT)
				sentence->setOperator(co_INT);
			sentence->setParent(NULL);
			
			if(result == NULL)
				store(sentence);
			else
				result->push_back(sentence);
			sentenceCount++;
		}
		else
		{
			
			if(!quietmode)
			{
				std::cerr << "Parse Error!  Kif sentence not well formed at " 
				<< token->tokenCodeToString()
				<< " \""
				<< token->getLexeme().c_str()
				<< "\""
				<< " at depth "
				<< depth
				<< "line no " 
				<< getLineNo()
				<< std::endl;
			}
			ok = false;
		}
		if(error)
		{
			ok = false;
			break;
		}
		token = getNextToken();
	}
	if(ok)
	{
		if(!quietmode)
			std::cerr << "\nTotal of "
			<< getLineNo()
			<< " lines parsed into " 
			<< sentenceCount 
			<< " sentences and "
			<< commentCount
			<< " comments.\n\n";
	}
	token = NULL;
	return ok;
}

Compound* GameParser::parseSentence(Token* token)
{
	Compound* sentence = NULL;
	depth++;
	if(token == NULL)
		token = getNextToken();
	bool eos = false;
	Compound* temp = NULL;
	while(!eos)
	{
		switch(token->getTokenCode())
		{
			case tc_RPAREN : 
				eos = true;
			case tc_COMMENT :
				break;
			case tc_LPAREN :
				temp = parseSentence(NULL);
				if(temp != NULL)
					temp->setParent(sentence);
				if(sentence != NULL)
					sentence->addArgument(temp);
				else
					sentence = temp;
				break;
			case tc_RELATION : 
				if(token->getOpType() == op_TRUE)
					break;
				if(sentence == NULL)
					sentence = new Compound(ct_COMPOUND, table->insertConstant(token->getLexeme()), mapRelation(token));
				else
				{
					if(token->getOpType() != op_TERMINAL)
					{
						temp = parseSentence(token);
						temp->setParent(sentence);
						sentence->addArgument(temp);
						eos = true;
					}
					else
					{
						temp = new Compound(ct_COMPOUND, table->insertConstant(token->getLexeme()), mapRelation(token));
						temp->setParent(sentence);
						sentence->addArgument(temp);
					}
				}
				break;
			case tc_OP : 
				sentence = mapOperator(token);
				break;
			case tc_PREDICATE : 
				sentence= new Compound(ct_COMPOUND, table->insertConstant(token->getLexeme()), co_PREDICATE);
				break;
			case tc_VARIABLE : 
			{
				if(sentence == NULL)
				{
					sentence= new Compound(ct_VARIABLE, table->insertVariable(token->getLexeme()), co_PREDICATE);
				}
				else
				{
					Variable* v = new Variable(table->insertVariable(token->getLexeme()));
					v->setParent(sentence);
					sentence->addArgument(v);
				}
				break;
			}
			case tc_CONSTANT : 
			{
				Compound* c = new Compound(ct_VALUE, table->insertConstant(token->getLexeme()), co_ATOM);
				if(token->getOpType() == op_INT)
					c->setOperator(co_INT);
				if(sentence != NULL)
				{
					c->setParent(sentence);
					sentence->addArgument(c);
				}
				else
					sentence = c;
				break;
			}
			default : 
				break;
		}
		if(!eos)
		{
			if(token->getTokenCode() == tc_EOF)
			{
				error = true;
				if(sentence->getOperator() != co_IMPLICATION && sentence->getArguments())
					table->updateParameterMax(sentence->getArguments()->size());
				return sentence;
			}
			token = getNextToken();
		}
	}
	depth--;
	if(sentence->getOperator() != co_IMPLICATION && sentence->getArguments())
		table->updateParameterMax(sentence->getArguments()->size());
	return sentence;
}

Compound* GameParser::mapOperator(Token* token)
{
	Compound* c = new Compound();
	switch(token->getOpType())
	{
		case op_IMPLICATION : 
			c->setOperator(co_IMPLICATION);
			break;
		case op_OR : 
			c->setOperator(co_OR);
			break;
		case op_NOT : 
			c->setOperator(co_NOT);
			break;
		case op_DISTINCT :
			c->setOperator(co_DISTINCT);
			break;
		default:
			c->setOperator(co_NONE);
			break;
	}
	return c;
}

void GameParser::store(Compound* c)
{
	if(isBinaryRelation(c))
		storeBinaryRelation(c);
	else
		storeRelationCounter(c);
		
	if(c->getOperator() >= co_RELATION)
		storeRelation(c, c->getOperator());
	else if(c->getOperator() == co_IMPLICATION && c->getArguments()->front()->getOperator() >= co_RELATION)
		storeRelation(c, c->getArguments()->front()->getOperator());
	else
		theory->storeRule(c);
	relation = op_NONE;
	
	
}
void GameParser::storeRelation(Compound* c, CompoundOperator relation)
{
	switch(relation)
	{
		case co_RELATION_ROLE :
			theory->storeRole(c);
			break;
		case co_RELATION_INIT :
			if(c->getOperator() == co_IMPLICATION)
			{
				Compound* init = c->getArguments()->front();
				if(init == NULL)
					break;
				Compound* temp = init->getArguments()->front();
				if(temp == NULL)
					break;
				c->removeArgumentFront();
				init->getArguments()->clear();
				delete init;
				c->addArgumentFront(temp);
				theory->storeInit(c);
			}
			else
			{
				theory->storeInit(c->getArguments()->front());
				c->getArguments()->clear();
				delete c;
			}
			//theory->storeInit(c);
			break;
		case co_RELATION_LEGAL :
			theory->storeLegal(c);
			break;
		case co_RELATION_NEXT :
			theory->storeNext(c);
			break;
		case co_RELATION_GOAL :
			theory->storeGoal(c);
			break;
		case co_RELATION_TERMINAL :
			theory->storeTerminal(c);
			break;
		default :
			theory->storeRule(c);
			break;
	}
}

bool GameParser::isBinaryRelation(Compound* c)
{
	//if(!binaryRelations)
	//	return false;
	if(c->getOperator() == co_PREDICATE && c->getArguments() && c->getArguments()->size() == 2)
	{
		if(c->getArgument(0)->getOperator() == co_ATOM || c->getArgument(0)->getOperator() == co_INT)
		{
			if(c->getArgument(1)->getOperator() == co_ATOM || c->getArgument(1)->getOperator() == co_INT)
				return true;
		}
	}
	return false;
}
void GameParser::storeBinaryRelation(Compound* c)
{
	//std::cerr << "Storing " << table->getName(c->getName()) << "(" <<  table->getName(c->getArgument(0)->getName()) << ", " << table->getName(c->getArgument(1)->getName()) << ")" << std::endl;
	this->binaryRelations->addBinaryRelation(c->getName())->add(c->getArgument(0)->getName(), c->getArgument(1)->getName());
}

void GameParser::storeRelationCounter(Compound* c)
{
	if(!relationCounters)
		return;
	
	if(!c->getArguments())
		return;
	for(size_t n = 0 ; n < c->getArguments()->size() ; n++)
	{
		if(c->getArgument(n)->getArguments())
		{
			storeRelationCounter(c->getArgument(n));
			continue;
		}
		if(c->getArgument(n)->getType() == ct_VARIABLE)
			continue;
		relationCounters->add(c->getArgument(n)->getName(), c->getName(), n);
	}
}

CompoundOperator GameParser::mapRelation(Token* token)
{
	CompoundOperator op = co_RELATION;
	switch(token->getOpType())
	{
		case op_ROLE :
			op = co_RELATION_ROLE;
			break;
		case op_INIT :
			op = co_RELATION_INIT;
			break;
		case op_NEXT :
			op = co_RELATION_NEXT;
			break;
		case op_DOES :
			op = co_RELATION_DOES;
			break;
		case op_TRUE :
			op = co_RELATION_TRUE;
			break;
		case op_LEGAL :
			op = co_RELATION_LEGAL;
			break;
		case op_GOAL :
			op = co_RELATION_GOAL;
			break;
		case op_TERMINAL :
			op = co_RELATION_TERMINAL;
			break;
		default :
			op = co_RELATION;
			break;
	}
	return op;
}

void GameParser::standardizeApart(Compound* c)
{
	variableTable.clear();
	standardizeApartTraverse(c);
}

void GameParser::standardizeApartTraverse(Compound* c)
{
	if(c == NULL)
		return;
	if(c->getType() == ct_VARIABLE)
	{
		__gnu_cxx::hash_map<unsigned int, unsigned int>::iterator mapping = variableTable.find(c->getName());
		if(mapping != variableTable.end())
			c->setName((*mapping).second);
		else
		{
			char buffer[12];
			sprintf(buffer, "?%d", variableName++);
			std::string name = buffer;
			unsigned int temp = table->insertVariable(name);
			variableTable[c->getName()] = temp;
			c->setName(temp);
		}
	}
	
	if(c->getArguments() == NULL)
		return;
	for(CompoundArgsItr args = c->getArguments()->begin() ; args != c->getArguments()->end() ; args++)
	{
		standardizeApartTraverse(*args);
	}	
}
