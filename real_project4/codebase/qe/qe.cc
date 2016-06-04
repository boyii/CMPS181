
#include "qe.h"

Filter::Filter(Iterator* input, const Condition &condition) {
    ItF = input;
    our_cond = condition;
}


RC Filter::getNextTuple(void *data){}

void Filter::getAttributes(vector<Attribute> &attrs)
const{}


Project::Project(Iterator *input, const vector<string> &attrNames){}


int Project::getNextTuple(void *data){}

// For attribute in vector<Attribute>, name it as rel.attr
void Project::getAttributes(vector<Attribute> &attrs) const{}


INLJoin::INLJoin(Iterator *leftIn,
		 IndexScan *rightIn,
		 const Condition &condition){}


RC INLJoin::getNextTuple(void *data){}

// For attribute in vector<Attribute>, name it as rel.attr
void INLJoin::getAttributes(vector<Attribute> &attrs) const{

}
