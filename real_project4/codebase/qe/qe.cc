#include <vector>
#include <string>
#include <iostream>
#include "qe.h"
#include <cstring>
Filter::Filter(Iterator* input, const Condition &condition) {
    ItF = input;
    our_cond = condition;
}


RC Filter::verify(Condition &cond, void * data, vector<Attribute> R){
    // turn void* into usable variable
    
    unsigned buff = 0;    
    bool we_can_use_vars = false;
    AttrType rType;
    AttrType lType;
    int lf = -2; // so the comparison isnt accidently true
    int rf = -1;
    bool matched = false;
    void * l_data;
    void * r_data;

    for(int i = 0; i < R.size();i++){
        int len;


        if(R.at(i).type == TypeVarChar){
            int str_len;
            memcpy(&str_len,data,4);
            len = 4 + str_len;
        } else len = 4;
        

        if(cond.lhsAttr.compare(R.at(i).name) == 0){
            lf = i;
            l_data = (char *) data + buff;
            lType = R.at(i).type;
            we_can_use_vars = true;
        } else

        if(cond.bRhsIsAttr){
            if(cond.rhsAttr.compare(R.at(i).name) == 0){
                rf = i;
                r_data  = (char * ) data + buff;
                rType = R.at(i).type;
                we_can_use_vars = true;
            
            }
        }

        buff += len;
    }


    if(lf != rf) return -1;
   
    int strlen = 0;
    bool reset = cond.bRhsIsAttr;
    if(reset == true){
        r_data = cond.rhsValue.data;
        rType = cond.rhsValue.type;
    }
    
    int valInt;
    float valFl;
    char * valStr;

    if(lType == TypeInt){
        memcpy(&valInt, l_data, 4);

    } else
    if(lType == TypeReal){
        memcpy(&valFl, l_data,4);
    } else
    if(lType == TypeVarChar){

    }

    return 0;
}

RC Filter::getNextTuple(void *data){
    int i = ItF->getNextTuple(data);
    if(i == QE_EOF) return QE_EOF;

    int k = verify(our_cond,data, res);
    if(k != -1) i = getNextTuple(data);

    return i;
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
RC Project::getNextTuple(void *data){}

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
