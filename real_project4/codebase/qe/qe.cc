
#include "qe.h"

Filter::Filter(Iterator* input, const Condition &condition) {
    ItF = input;
    our_cond = condition;
}


RC Filter::getNextTuple(void *data){
    ItF->getNextTuple(data);

    return -1;
}

void Filter::getAttributes(vector<Attribute> &attrs) const{
    ItF->getAttributes(attrs);
}


Project::Project(Iterator *input, const vector<string> &attrNames){}


int Project::getNextTuple(void *data){}

// For attribute in vector<Attribute>, name it as rel.attr
void Project::getAttributes(vector<Attribute> &attrs) const{}


INLJoin::INLJoin(Iterator *leftIn,
		 IndexScan *rightIn,
		 const Condition &condition){
	_leftIn = leftIn;
	_rightIn = rightIn;
	_condition = condition;
}
//  For each tuple r in R do
//     For each tuple s in S do
//        If r and s satisfy the join condition
//           Then output the tuple <r,s>
INLJoin::~INLJoin(){}

RC INLJoin::getNextTuple(void *data){
	return -1; 
}

// For attribute in vector<Attribute>, name it as rel.attr
void INLJoin::getAttributes(vector<Attribute> &attrs) const{
	_leftIn->getAttributes(attrs);

	vector<Attribute> temp;
	_rightIn->getAttributes(temp);

	for(unsigned rightIndex = 0; rightIndex < temp.size(); rightIndex++){
		attrs.push_back(temp.at(rightIndex));
	}
}
