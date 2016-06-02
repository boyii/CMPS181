
#include "qe.h"

Filter::Filter(Iterator* input, const Condition &condition) {
}

~Filter::Filter(){}

RC Filter::getNextTuple(void *data){}

void Filter::getAttributes(vector<Attribute> &attrs)
const{}


Project::Project(Iterator *input, const vector<string> &attrNames){}

~Project::Project(){}

int Project::getNextTuple(void *data){}

// For attribute in vector<Attribute>, name it as rel.attr
void Project::getAttributes(vector<Attribute> &attrs) 
const{}


INLJoin::INLJoin(Iterator *leftIn,
		 IndexScan *rightIn,
		 const Condition &condition){}

~INLJoin::INLJoin(){}

RC INLJoin::getNextTuple(void *data){}

// For attribute in vector<Attribute>, name it as rel.attr
void getAttributes(vector<Attribute> &attrs)
const{

}
