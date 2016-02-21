//
// This programs converts .kif format to Prolog.
//
// Author: Yngvi Bjornsson 2007
//
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <deque>
#include <string>
#include <cctype>
#include <cstdlib>
#include <map>

std::map<std::string,std::string> map_wordsub;

class Node
	{
	public:
		Node( ) {};   
		virtual std::string getName() = 0;
		virtual void setName(std::string name){};
		virtual std::string toStr() = 0;
		virtual ~Node() { };
	private:
	};

class AtomNode : public Node
	{
	public:
		AtomNode( std::string name ) : Node( ), m_name(name) { }
		virtual std::string getName() { return m_name; }
		virtual void setName(std::string name) { m_name = name; }
		virtual std::string toStr() { return m_name; }
	protected:
		std::string m_name;
	};

class NameNode : public AtomNode
	{
	public:
		NameNode( std::string name ) : AtomNode( name ) {}
	};

class NumberNode : public AtomNode
	{
	public:
		NumberNode( std::string name ) : AtomNode( name ) {}
	};

class VariableNode : public AtomNode
	{
	public:
		VariableNode( std::string name ) : AtomNode( name ) {}
		virtual std::string toStr() { return std::string("_") + m_name; }
	};

class ClauseNode : public Node
	{
	public:
		ClauseNode( std::string name ) : Node( ), m_name(name) { }
		virtual void setName(std::string name) { m_name = name; }
		void addArg( Node* node ) {
			m_argument.push_back( node );
		}
		
		virtual std::string getName() { return m_name; }
		
		virtual std::string toStr() { 
			std::string str;
			if ( m_name == "<=" ) {
				//assert( m_argument.size() > 0 );
				str = m_argument[0]->toStr() + ":-\n";
				if ( m_argument.size()  == 1 ) {
					str += "\t[]";
				}
				else {
					for ( unsigned int i=1; i<m_argument.size(); ++i ) {
						if ( i > 1 ) str += ",\n";
						str += '\t';
						str += m_argument[i]->toStr();
					}
				}
			}
			else {
				if ( map_wordsub.find(m_name) != map_wordsub.end() )
					str = map_wordsub[ m_name ];
				else
					str = m_name;
				if ( !m_argument.empty() ) {
					str += "( ";
					for ( unsigned int i=0; i<m_argument.size(); ++i ) {
						if ( i > 0 ) str += ", ";
						str += m_argument[i]->toStr();
					}
					str += " )";
				}
			}
			return str;
		}
		
		Node* getArg(size_t index){return m_argument[index];}
	private:
		std::string        m_name;
		std::vector<Node*> m_argument;
	};


// Delete



typedef enum {
	tokParOpen,
	tokParClose,
	tokInfer,
	tokVariable,
	tokNumber,
	tokName,
	tokEOF,
	tokUnknown,
	NUMtokENUMS
} Tokentag;

static std::string TokenStr[NUMtokENUMS] = 
{
"(",
")",
"<=",
"?<var>",
"<number>",
"<name>",
"EOF",
"Unknown"
};


struct Token
{
	Tokentag    m_tag;
	std::string m_str;
	int         m_lineno;
};

class Lexical
	{
	public:
		Lexical( std::istream& is );
		
		Token getNext();
		Token peek();
	private:
		
		std::istream& m_is;
		char          m_char;
		int           m_lineno;
		std::deque<Token> m_lookahead;
	};


using namespace std;

Lexical::Lexical( istream& is )
: m_is( is ) 
{
	m_is.get(m_char);
	m_lineno = 1;
}

Token Lexical::peek()
{
	if ( m_lookahead.empty() ) {
		m_lookahead.push_back( Lexical::getNext() );
	}
	return m_lookahead.back();
}


Token Lexical::getNext()
{
	Token token;
	
	if ( !m_lookahead.empty() ) {
		token = m_lookahead.front( );
		m_lookahead.pop_front();
		return token;
	}
	
	token.m_tag = tokUnknown;
	
	while ( !m_is.eof() && isspace( m_char ) ) {
		if ( m_char == '\n' ) m_lineno++;
		m_is.get( m_char );
	}
	
	// Strip comments -- add block comments?
	while ( m_char == ';' && !m_is.eof() ) {
		while ( !m_is.eof() && m_char != '\n' ) {
			m_is.get( m_char );
		}
		if ( m_char == '\n' ) m_lineno++;
		m_is.get( m_char );
		while ( !m_is.eof() && isspace( m_char ) ) {
			if ( m_char == '\n' ) m_lineno++;
			m_is.get( m_char );
		}
	}
	token.m_lineno = m_lineno;
	
	if ( m_is.eof() ) {
		token.m_tag = tokEOF;
	}
	else {
		switch ( m_char ) {
				
			case '(': 
				token.m_tag = tokParOpen;
				token.m_str.push_back( m_char );
				m_is.get( m_char );
				break;
				
			case ')': 
				token.m_tag = tokParClose;
				token.m_str.push_back( m_char );
				m_is.get( m_char );
				break;
				
			case '<':
				if ( m_is.peek() == '=' ) {
					token.m_tag = tokName;
					token.m_str.push_back( m_char );
					m_is.get( m_char );
				}
				else if ( m_is.peek() == ' ' || m_is.peek() == '\t' || m_is.peek() == '(' ) {
					token.m_tag = tokName;
					m_char = 'l';
				}
				else {
					token.m_tag = tokUnknown;
				}
				token.m_str.push_back( m_char );
				m_is.get( m_char );
				break;
				
			case '>':
				token.m_tag = tokName;
				token.m_str.push_back( 'g' );
				m_is.get( m_char );
				break;
				
			case '?':
				token.m_tag = tokVariable;
				//token.m_str.push_back( m_char ); skip '?'
				m_is.get( m_char );
				while ( !m_is.eof() && (isalnum( m_char ) || m_char=='_' || m_char=='-' || m_char=='+' )  ) {
					if(m_char == '+') m_char = 'p';
					else if(m_char == '-') m_char = 'm';
					token.m_str.push_back( m_char );
					m_is.get( m_char );
				}
				break;
				
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				token.m_tag = tokNumber;
				while( !m_is.eof() && isdigit( m_char ) ) {
					token.m_str.push_back(m_char);
					m_is.get( m_char );
				}
				break;
				
			default:
				if ( isalnum( m_char ) ) {
					token.m_tag = tokName;
					while ( !m_is.eof() && (isalnum( m_char )||m_char=='_'||m_char=='-'||m_char=='+' ) ) {
						if ( m_char=='-')
							m_char='m';
						else if ( m_char=='+')
							m_char='p';
						token.m_str.push_back( m_char );
						m_is.get( m_char );
					}
				}
				else if ( m_char == '-' || m_char == '+' ) {
					token.m_tag = tokName;
					while ( !m_is.eof() && (isalnum( m_char )||m_char=='_'||m_char=='-'||m_char=='+' ) ) {
						if ( m_char=='-')
							m_char='m';
						else if ( m_char=='+')
							m_char='p';
						token.m_str.push_back( m_char );
						m_is.get( m_char );
					}
				}
				else {
					token.m_tag = tokUnknown;
					token.m_str.push_back( m_char );
					m_is.get( m_char );
				} 
				break;
		}
		
	}
	
	//  cout << "[" << token.m_tag << "," << token.m_str << "]\n";
	return token;
}


class Parser
	{
	public:
		Parser( Lexical& lex );
		Node* parseItem();
		Node* parseClause();
		void  parseProgram( vector<Node*>& statements );
		
	private:
		void  matchToken( Tokentag tag );
		bool  matchIfToken( Tokentag tag );
		
		Lexical& m_lex;
		Token    m_token;
	};


void Parser::matchToken( Tokentag tag )
{
	if ( m_token.m_tag == tag ) {
		m_token = m_lex.getNext();
	}
	else {
		cerr << "Syntax problem line " << m_token.m_lineno << ": expected token '" 
		<< TokenStr[tag] << "', but received '" << TokenStr[m_token.m_tag] << "+ for " << m_token.m_str << ".\n";
		exit(1);
	}
}


bool Parser::matchIfToken( Tokentag tag )
{
	if ( m_token.m_tag == tag ) {
		matchToken( tag );
		return true;
	}
	return false;
}


Parser::Parser( Lexical& lex )
: m_lex( lex )
{
	m_token = m_lex.getNext();
}


Node* Parser::parseItem()
{
	Node *node = NULL;
	if ( m_token.m_tag == tokName ) {
		node = new NameNode( m_token.m_str );
		matchToken( tokName );
	}
	else if ( m_token.m_tag == tokNumber ) {
		node = new NumberNode( m_token.m_str );
		matchToken( tokNumber );
	}
	else if ( m_token.m_tag == tokVariable ) {
		node = new VariableNode( m_token.m_str );
		matchToken( tokVariable );
	}
	else {
		node = parseClause();
	}
	return node;
}

//#define MOVE_NOT_AND_DISTINCT_TO_BACK
Node* Parser::parseClause()
{
	Node *node = NULL;
	if ( matchIfToken( tokParOpen ) ) 
	{
		ClauseNode *clauseNode  = new ClauseNode( m_token.m_str );
		matchToken( tokName );
		Node* item = NULL;
#ifdef MOVE_NOT_AND_DISTINCT_TO_BACK
		std::vector<Node*> toback;
#endif
		while ( m_token.m_tag != tokParClose ) 
		{
			item = parseItem() ;
#ifdef MOVE_NOT_AND_DISTINCT_TO_BACK
			if(item->getName() == "not")
				toback.push_back(item);
			else if(item->getName() == "distinct")
				toback.push_back(item);
			else
#endif
				clauseNode->addArg( item );
		}
#ifdef MOVE_NOT_AND_DISTINCT_TO_BACK
		for(size_t n = 0 ; n < toback.size() ; n++)
		{
			clauseNode->addArg( toback[n] );
		}
		toback.clear();
#endif
		matchToken( tokParClose );
		node = clauseNode;
	}
	else {
		node = new AtomNode( m_token.m_str );
		matchToken( tokName );
	}
	return node;
}


void Parser::parseProgram( vector<Node*>& statements )
{
	while ( m_token.m_tag != tokEOF ) {
		Node *node = parseClause();
		statements.push_back( node );
		//std::cerr << node->toStr() << std::endl;
	}
}


string nameOut( string fileNameIn )
{
	string fileNameOut;
	
	string::size_type pos = fileNameIn.find_last_of(".");
	if ( string::npos != pos ) {    
		fileNameOut = fileNameIn.substr(0, pos);
	}
	else {
		fileNameOut = fileNameIn;
	}
	fileNameOut += ".pl";
	return fileNameOut;
}


void wordSubs()
{
	map_wordsub["init"] = "state";
	map_wordsub["true"] = "state";
	
	map_wordsub["succ"] = "succs";
	map_wordsub["number"] = "x_number_x";
	map_wordsub["state"] = "x_state_x";
	map_wordsub["member"] = "x_member_x";

	map_wordsub["plus"] = "x_plus_x";
	map_wordsub["subtract"] = "x_subtract_x";
	map_wordsub["minus"] = "x_minus_x";
}

void classifyMoves(vector<Node*>& statements)
{
	int counter = 0;
	for(std::size_t n = 0 ; n < statements.size() ; n++)
	{
		if(statements[n]->getName() != "<=")
			continue;
		ClauseNode* c = static_cast<ClauseNode*>(statements[n]);
		if(c->getArg(0)->getName() != "legal")
			continue;
		c = static_cast<ClauseNode*>(c->getArg(0));
		//std::cerr << " -> " << c->getArg(1)->getName() << std::endl;
		std::stringstream ss;
		ss << c->getArg(1)->getName() << "_" << (++counter);
		c->getArg(1)->setName(ss.str());
		//std::cerr << "New statement :\n" << statements[n]->toStr() << std::endl;
	}
}

int main( int argc, char *argv[] )
{
	
	wordSubs();
	
	for ( int i=1 ; i<argc ; ++i ) {
		ifstream fin;
		fin.open( argv[i] );
		
		Lexical lex( fin );
		Parser parser( lex );
		vector<Node*> statements;
		
		parser.parseProgram( statements );
		fin.close();
		
		//classifyMoves(statements);
		
		string last_name;
		ofstream fout;
		fout.open( nameOut(argv[i]).c_str() );
		for ( unsigned int c=0 ; c<statements.size(); ++c ) {
			if ( statements[c]->getName() != last_name )  fout << "\n";
			last_name = statements[c]->getName();
			fout << statements[c]->toStr() << ".\n";
		}
		fout.close();
	}
	
	return 0;
}
