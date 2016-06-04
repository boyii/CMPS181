
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


RC Project::getNextTuple(void *data){}

// For attribute in vector<Attribute>, name it as rel.attr
void Project::getAttributes(vector<Attribute> &attrs) const{}


INLJoin::INLJoin(Iterator *leftIn,
		 IndexScan *rightIn,
		 const Condition &condition){}


RC INLJoin::getNextTuple(void *data){}

// For attribute in vector<Attribute>, name it as rel.attr
void INLJoin::getAttributes(vector<Attribute> &attrs) const{

}
