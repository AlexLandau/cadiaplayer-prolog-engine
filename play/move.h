#ifndef MOVE_H
#define MOVE_H

#include <stdint.h>
#include <math.h>
#include "parsing/compound.h"
#include "parsing/symtab.h"
#include "../utils/strutils.h"

#include "Yap/YapInterface.h"

//#define MAST_LEVELS
#ifdef MAST_LEVELS
#define MAST_LEVELS_SIZE 200
#endif

//#define CALCULATE_SD

namespace cadiaplayer {
	namespace play {
		
		const double INIT_AVG = 100;//static_cast<double>(LONG_MAX);
		const unsigned int Q_ESTIMATE_THRESHOLD = 10;
		const double Q_DEFAULT_ESTIMATION = 50;

		class Move
			{
			private:
				// Visits
				unsigned int n_avg;
				// Q(s,a) value
				double q_avg;
#ifdef CALCULATE_SD
				// Sum of Squares value
				double ss;
				// variance
				double var;
				// Standard deviation
				double sd;
#endif

#ifdef MAST_LEVELS
				std::vector<double>* q_levels;
				std::vector<unsigned int>* n_levels;
#endif
			public:
				// The compound behind the move
				parsing::Compound compound;
				
				// Constructors
				Move();
				Move(double initQ);
				Move(parsing::Compound& c);
				// Copy constructors
				Move(const Move& m);
				Move(const Move& m, double initQ);
				// Destructor
				~Move(){};
				// = operator
				Move& operator=(const Move&);
#ifndef MAST_LEVELS
				// Add a Q(s,a) value to this move
				void addQ(double q);
#else
				// Add a Q(s,a) value to this move
				void addQ(double q, unsigned int level);
				void initLevels(){q_levels.resize(MAST_LEVELS_SIZE, 0.0);n_levels.resize(MAST_LEVELS_SIZE, 0);};
#endif
				// Get the current Q(s,a) value
				double getQ(void);
#ifdef CALCULATE_SD
				// Get the current variance
				double getVar(void);
				// Get the current standard deviation
				double getSD(void);
#endif
				// Estimate the current Q(s,a) value
				double estimateQ(void);
				// Get current number of visits
				unsigned int getN(void);
				
				// To string
				std::string toString(parsing::SymbolTable* s){return compound.toString(s);};
				std::string toPlayString(parsing::SymbolTable* s){return compound.toPlayString(s);};
				
				static std::string roleMovesToString(std::vector<Move*>* moves, parsing::SymbolTable* s)
				{
					std::stringstream ss;
					for(std::size_t m = 0 ; m < moves->size() ; m++)
					{
						if(m)
							ss << " (" << (m+1) << ") " << (*moves)[m]->toString(s);
						else
							ss << "(1) " << (*moves)[m]->toString(s);
					}
					return ss.str();
				};
			};		
		typedef std::vector<Move> Moves;
		typedef Moves::iterator MovesItr;
		// Wrapper to traverse possible combination of moves by all players (because all games are 
		// inherently simultanious) in an iterator-style fashion
		class MoveCombinations
			{
				std::vector<std::vector<Move*>*>* moves;
				bool done;
				std::vector<size_t> index;
				std::vector<size_t> top;
			public:
				MoveCombinations(){done = true;};
				// Initialize with all move vectors(inner vector) of all players (outer vector) indexed on the role index 
				void begin(std::vector<std::vector<Move*>*>* combine);
				// Check if all combinations have been exhausted
				bool end(){return done;};
				// Move to next combination
				void operator ++();
				// Get the current combination. One move per player loaded into the combination parameter
				void get(std::vector<Move*>& combination);
				// Get the current combination. One move per player loaded into the combination parameter
				size_t getIndex(std::size_t role){
					return index[role];
				};
				void getIndex(std::vector<int>& i){
					for(int n = 0 ; n < index.size() ; ++n)
					{
						i.push_back(index[n]);
					}
				};
			};
		
		class MoveRating;
		typedef std::vector<MoveRating> MoveRatings;
		const char MOVERATINGS_DELIMITER = '&';
		
		class MoveRating
		{
		private:
			std::string m_move;
			uint32_t	m_samples;
			double		m_score;
			char		m_buffer[4096];
		public:
			MoveRating():m_move(""), m_samples(0), m_score(0.0){};
			MoveRating(Move* move, parsing::SymbolTable* symbols, int samples, double score):m_samples(samples), m_score(score){
				m_move = move->toPlayString(symbols);
			};
			MoveRating(std::string move, int samples, double score):m_move(move), m_samples(samples), m_score(score){};
			MoveRating(std::string encoded){
				fromString(encoded);
			};
			MoveRating(const MoveRating& rating){
				m_move = rating.m_move;
				m_samples = rating.m_samples;
				m_score = rating.m_score;
			};
			~MoveRating(){};
			
			void setMove(std::string move){m_move=move;};
			void setSamples(int samples){m_samples=samples;};
			void setScore(double score){m_score=score;};
			
			std::string getMove(){return m_move;};
			int getSamples(){return m_samples;};
			double getScore(){return m_score;};
			
			std::string toString()
			{
				std::stringstream ss;
				ss << m_move << std::endl << m_samples << std::endl << m_score;
				return ss.str();
			};
			void toString(std::stringstream& ss)
			{
				ss << m_move << std::endl << m_samples << std::endl << m_score;
			};
			void info(std::stringstream& ss)
			{
				ss << m_move << " n:" << m_samples << " val:" << m_score;
			};
			std::string info()
			{
				std::stringstream ss;
				ss << m_move << " n:" << m_samples << " val:" << m_score;
				return ss.str();
			};
			void fromString(std::string str)
			{
				std::stringstream ss;
				ss << str;
				ss.getline(m_buffer, 4096);
				m_move = m_buffer;
				ss >> m_samples;
				ss >> m_score;
			}
			void merge(MoveRating& rating)
			{
				uint32_t total = this->getSamples() + rating.getSamples();
				double a = this->getSamples();
				double b = (double)rating.getSamples();
				a = (a/total)*this->getScore();
				b = (b/total)*rating.getScore();
				this->setSamples(total);
				this->setScore(a+b);
			}
			void max(MoveRating& rating)
			{
				this->setSamples(this->getSamples() + rating.getSamples());
				this->setScore((this->getScore()>rating.getScore()?this->getScore():rating.getScore()));
			}
			
			static std::string toString(MoveRatings& ratings)
			{
				std::stringstream ss;
				if(ratings.empty())
					return "";
				ratings[0].toString(ss);
				for (std::size_t n = 1 ; n < ratings.size() ; n++) 
				{
					ss << MOVERATINGS_DELIMITER;
					ratings[n].toString(ss);
				}
				return ss.str();
			}
			static void fromString(std::string str, MoveRatings& ratings)
			{
				std::vector<std::string> strings;
				cadiaplayer::utils::StringUtils::tokenize(str, strings, MOVERATINGS_DELIMITER);
				for(std::size_t n = 0; n < strings.size(); n++)
				{
					ratings.push_back(MoveRating(strings[n]));
				}
			}
		};
		
		typedef __gnu_cxx::hash_map<std::string, MoveRating, cadiaplayer::utils::StringHash> MoveRatingMap;
		typedef MoveRatingMap::iterator MoveRatingMapItr;
		
		
	}} // namespaces
#endif //MOVE_H
