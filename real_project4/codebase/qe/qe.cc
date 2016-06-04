#include <vector>
#include <string>
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

Project::Project(Iterator *input, const vector<string> &attrNames){
    this->pIt = input;
    pAttrs = attrNames;
}


RC Project::getNextTuple(void *data){
   // pIt->getNextTuple(data);
   return 0; 
}

// For attribute in vector<Attribute>, name it as rel.attr
void Project::getAttributes(vector<Attribute> &attrs) const{
    attrs.clear(); // clear the passed in vector

    // get attributes from iterator field and store them in a vector
    vector<Attribute> iterAttrs;
    pIt->getAttributes(iterAttrs);

    // nested for loops to compare each item 
    // to project with all elements in a column
    for(auto i = 0; i < pAttrs.size(); i++) {
        for(auto j = 0; j < iterAttrs.size(); j++) {
            if(pAttrs[j].compare(iterAttrs[j].name) == 0){
                attrs.push_back(iterAttrs[j]);
            }
            break;
        }
    }

}


INLJoin::INLJoin(Iterator *leftIn,
		 IndexScan *rightIn,
		 const Condition &condition){}


RC INLJoin::getNextTuple(void *data){}

// For attribute in vector<Attribute>, name it as rel.attr
void INLJoin::getAttributes(vector<Attribute> &attrs) const{

}
