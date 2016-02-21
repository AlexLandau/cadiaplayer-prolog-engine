#include "variable.h"
using namespace cadiaplayer::play::parsing;

Variable::Variable(Compound& c)
{
	setOperator(c.getOperator());
	setType(ct_VARIABLE);
	setParent(c.getParent());
}
Variable::Variable(unsigned int n)
{
	setName(n);
	setType(ct_VARIABLE);
	setParent(NULL);
}
Variable::Variable(unsigned int n, CompoundOperator o)
{
	setName(n);
	setOperator(o);
	setType(ct_VARIABLE);
	setParent(NULL);
}
