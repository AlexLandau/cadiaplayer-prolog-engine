#include "move.h"
using namespace std;
using namespace cadiaplayer::play;

Move::Move() : 
n_avg(0),q_avg(INIT_AVG)
#ifdef CALCULATE_SD
,ss(-1),var(-1),sd(-1)
#endif
{}

Move::Move(double initQ) : 
n_avg(0),q_avg(initQ)
#ifdef CALCULATE_SD
,ss(-1),var(-1),sd(-1)
#endif
{}

Move::Move(parsing::Compound& c):
n_avg(0),q_avg(INIT_AVG)
#ifdef CALCULATE_SD
,ss(-1),var(-1),sd(-1)
#endif
{
	compound = c;
}
Move::Move(const Move& m):
n_avg(m.n_avg),q_avg(m.q_avg)
#ifdef CALCULATE_SD
,ss(m.ss),var(m.var),sd(m.sd)
#endif
{
	compound = m.compound;
}
Move::Move(const Move& m, double initQ):
n_avg(m.n_avg),q_avg(initQ)
#ifdef CALCULATE_SD
,ss(m.ss),var(m.var),sd(m.sd)
#endif
{
	compound = m.compound;
}
Move& Move::operator=(const Move& m)
{
	n_avg=m.n_avg;
	q_avg=m.q_avg;
#ifdef CALCULATE_SD
	ss=m.ss;
	var=m.var;
	sd=m.sd;
#endif
	compound = m.compound;
	
	return *this;
}

#ifndef MAST_LEVELS
void Move::addQ(double q)
{
	n_avg++;
	if(n_avg==1)
	{
		q_avg = q;
#ifdef CALCULATE_SD
		var=(q-50)*(q-50);
		sd=0;
#endif
		return;
	}
	double delta = q-q_avg;
	q_avg = q_avg + (1.0/n_avg)*(delta);
	
#ifdef CALCULATE_SD
	delta = (q-50)*(q-50) - var;
	var = var + (1.0/n_avg)*delta;
	sd=sqrt(var);
#endif
}
#else
void Move::addQ(double q, unsigned int level)
{
	n_avg++;
	if(n_avg==1)
	{
		q_avg = q;
#ifdef CALCULATE_SD
		var=(q-50)*(q-50);
		sd=0;
#endif
	}
	else
	{
		double delta = q-q_avg;
		q_avg = q_avg + (1.0/n_avg)*(delta);
	
#ifdef CALCULATE_SD
		delta = (q-50)*(q-50) - var;
		var = var + (1.0/n_avg)*delta;
		sd=sqrt(var);
#endif
	}
	
	// Levels
	n_level[level]++;
	if(n_level[level]==1)
	{
		q_level[level] = q;
		return;
	}
	else
	{
		double delta = q-q_level[level];
		q_level[level] = q_level[level] + (1.0/n_level[level])*(delta);
	}
}
#endif


double Move::getQ(void)
{
	return q_avg;
}
#ifdef CALCULATE_SD
double Move::getVar(void)
{
	return n_avg >= Q_ESTIMATE_THRESHOLD ? var : 100;
}
double Move::getSD(void)
{
	return n_avg >= Q_ESTIMATE_THRESHOLD ? sd : 100;
}
#endif
double Move::estimateQ(void)
{
	if(n_avg < Q_ESTIMATE_THRESHOLD) 
		return Q_DEFAULT_ESTIMATION;
	return q_avg;
}

unsigned int Move::getN(void)
{
	return n_avg;
}

// MoveCombinations class

void MoveCombinations::begin(std::vector<std::vector<Move*>*>* combine)
{
	// init index
	index.clear();
	if(combine->size() == 0)
	{
		done = true;
		return;
	}
	for(size_t n = 0 ; n < combine->size() ; ++n)
	{
		index.push_back(0);
		top.push_back((*(combine))[n]->size()-1);
	}
	moves = combine;
	done = false;
}

void MoveCombinations::operator ++()
{
	for(size_t n = 0 ; n < index.size() ; ++n)
	{
		if(index[n] < top[n])
		{
			++index[n];
			break;
		}
		if(n == index.size()-1)
		{
			done = true;
			break;
		}
		index[n] = 0;
	}
}

void MoveCombinations::get(std::vector<Move*>& combination)
{
	if(done)
		return;
	for(size_t n = 0 ; n < index.size() ; ++n)
	{
		combination.push_back((*(*(moves))[n])[index[n]]);
	}
}
