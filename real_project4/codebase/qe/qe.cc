#include <vector>
#include <string>
#include <iostream>
#include "qe.h"
#include <cstring>


bool Filter::checkScanCondition(char *recordString, CompOp compOp, const void *value)
{
        if (compOp == NO_OP) return true;

    int32_t valueSize;
    memcpy(&valueSize, value, VARCHAR_LENGTH_SIZE);
    char valueStr[valueSize + 1];
    valueStr[valueSize] = '\0';
    memcpy(valueStr, (char*) value + VARCHAR_LENGTH_SIZE, valueSize);
    int cmp = strcmp(recordString, valueStr);
    switch (compOp)
        {
            case EQ_OP: return cmp == 0;
            case LT_OP: return cmp <  0;
            case GT_OP: return cmp >  0;
            case LE_OP: return cmp <= 0;
            case GE_OP:return cmp >= 0;
            case NE_OP: return cmp != 0;
            default: return false;
        }
}


bool Filter::checkScanCondition(float r, CompOp compOp, const void *value){

    float realValue;
    memcpy (&realValue, value, REAL_SIZE);
    switch (compOp)
     {   
        case EQ_OP: return r == realValue;
        case LT_OP: return r <  realValue;  
        case GT_OP: return r > realValue;                                                                                
        case LE_OP: return r <= realValue;
        case GE_OP: return r >= realValue;
        case NE_OP: return r != realValue;
                                                                                                                                                                default: return false;
     } 

}


bool Filter::checkScanCondition(int recordInt, CompOp compOp, const void * value){
    int32_t intValue;
    memcpy(&intValue, value, INT_SIZE);
    switch(compOp){
        case EQ_OP: return recordInt == intValue;
        case LT_OP: return recordInt < intValue;
        case GT_OP: return recordInt > intValue;
        case LE_OP: return recordInt <= intValue;
        case GE_OP: return recordInt >= intValue;
        case NE_OP: return recordInt != intValue;
        defualt: return false;
    }
}



Filter::Filter(Iterator* input, const Condition &condition) {
    ItF = input;
    our_cond = condition;
    ItF->getAttributes(res);
}


RC Filter::verify(Condition &cond, void * data, vector<Attribute> R){
    // turn void* into usable variable
    
    unsigned buff = 0;    
    bool we_can_use_vars = false;
    AttrType rType;
    AttrType lType;
    int lf = -1; // so the comparison isnt accidently true
    int rf = -2;
    bool matched = false;
    void * l_data;
    void * r_data;

    for(int i = 0; i < R.size();i++){
        int len;


        if(R.at(i).type == TypeVarChar){
            len = *(int *) ((char *) data + buff + 1);
        } else len = 4;
        
        if(cond.lhsAttr.compare(R.at(i).name) == 0){
            lf = i;
            l_data = (char *) data + buff;
            lType = R.at(i).type;
            we_can_use_vars = true;
        } else if(cond.bRhsIsAttr){
            cout << "init1" << endl;
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
    char * valStr1;
    char * valStr2;
    bool cmp;
    cout << lType << endl;
    if(lType == TypeInt){
        memcpy(&valInt, l_data, 4);
        cmp = checkScanCondition(valInt,our_cond.op,r_data);
    } else
    if(lType == TypeReal){
        memcpy(&valFl, l_data,4);
        cmp = checkScanCondition(valFl, our_cond.op, r_data);
    } else
    if(lType == TypeVarChar){
        int newlen;
        int newlen2;
        memcpy(&newlen, l_data, 4);
        valStr1 = (char *) malloc(1 + newlen);
        memcpy(valStr1,(char *) l_data + 4, newlen);
        valStr1[newlen] = '\0';
        memcpy(&newlen2, r_data, 4);
        valStr2 = (char *) malloc(newlen2 + 1);
        memcpy(valStr2, (char *) l_data + 4, newlen2);
        valStr2[newlen2] = '\0';
        cout << valStr2 << endl;
        cout << valStr1 << endl;
        cmp = checkScanCondition(valStr1, our_cond.op, valStr2);
    }

    int ret;
    if(cmp) ret = 0;

    return ret;
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
//	_leftIn->getNextTuple(data);
//	_rightIn->getNextTuple();
	return -1; 
}

// For attribute in vector<Attribute>, name it as rel.attr
void INLJoin::getAttributes(vector<Attribute> &attrs) const{
	_leftIn->getAttributes(attrs);

	vector<Attribute> SAttrs;
	_rightIn->getAttributes(SAttrs);

	for(unsigned rightIndex = 0; rightIndex < SAttrs.size(); rightIndex++){
		attrs.push_back(SAttrs.at(rightIndex));
	}
}
