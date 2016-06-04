#include <vector>
#include <string>
#include <iostream>
#include "qe.h"
#include <cstring>


bool Filter::checkScanCondition(char *recordString, CompOp compOp, const char *value)
{
    if (compOp == NO_OP) return true;
    int cmp = strcmp(recordString, value);
    cout << "cmp inside the function : " << recordString << endl;
    cout << "were comparing to : " << value  << endl;
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
    int * real;
    real = (int *) value;
    memcpy(&intValue, value, INT_SIZE);
//    cout << intValue << " : " << recordInt <<" : "<< *real << endl;
    
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
    int rf = -1;
    bool matched = false;
    void * l_data;
    void * r_data;

    for(int i = 0; i < R.size();i++){
        int len;


        if(R.at(i).type == TypeVarChar){
            int * str_len;
            str_len = (int *) ((char*) data + buff + 1);
//            memcpy(&str_len,((char *)data + buff), VARCHAR_LENGTH_SIZE);
            len = VARCHAR_LENGTH_SIZE + *str_len;
          //  cout << len << endl;
        } else len = 4;
        

        if(cond.lhsAttr.compare(R.at(i).name) == 0){
            lf = i;
        //    cout << "init1" << endl;
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
       // cout << "len is : " << len << " buff is: " << buff << endl;
        buff += len;
    }
    bool test = false;

//    cout << "this is test : " << test << endl;
//    cout << we_can_use_vars << endl;
//    if(lType != rType) return -1;
//            cout << "init1" << endl;
//    cout << "heres the problem" << endl;

    int strlen = 0;
    bool reset = !cond.bRhsIsAttr;
    if(reset == true){
        r_data = cond.rhsValue.data;
        rType = cond.rhsValue.type;
    }

    if(lType != rType) return -1;
    int valInt;
    float valFl;
    char * valStr1;
    char * valStr2;
    bool cmp = false;
    if(lType == TypeInt){
        memcpy(&valInt,(char *) l_data + 1, 4);
        cout << "cmp before : " << cmp << endl;
        cmp = checkScanCondition(valInt,our_cond.op,r_data);
        cout << "after: " << cmp << endl;
    } else
    if(lType == TypeReal){
        memcpy(&valFl,(char *) l_data + 1 ,4);
        cmp = checkScanCondition(valFl, our_cond.op, r_data);
    } else
    if(lType == TypeVarChar){
        int * newlen;
        int * newlen2;
        newlen = (int *) ((char *) l_data + 1);
        valStr1 = (char *) malloc(1 + *newlen);
        cout << "this should not always be 4: " << *newlen << endl;
//        valStr1 = (char *) l_data + 1 + *newlen;
        memcpy(valStr1,(char *) l_data +1 + *newlen , *newlen);
        valStr1[*newlen] = '\0';
        newlen2 = (int *) ((char *) r_data );
        valStr2 = (char *) malloc(*newlen2 + 1);
//        valStr2 = (char *) r_data  + *newlen2;
        memcpy(valStr2, (char *) r_data + 4, *newlen2);
        valStr2[*newlen2] = '\0';
        cout << "cmp before " << cmp << endl;
        cmp = checkScanCondition(valStr1, our_cond.op, valStr2);
        cout << cmp << endl;
    }
    bool t = true;
    //cout << "t is : " <<t <<endl;
    int ret = -1;

    if(cmp) ret = 0;
//    char * a = "the";
//    cout << a << endl;

    return ret;
}

RC Filter::getNextTuple(void *data){
    int i = ItF->getNextTuple(data);
    if(i == QE_EOF) return QE_EOF;

    int k = verify(our_cond,data, res);
    if(k == -1) i = getNextTuple(data);

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
		 const Condition &condition){}


RC INLJoin::getNextTuple(void *data){}

// For attribute in vector<Attribute>, name it as rel.attr
void INLJoin::getAttributes(vector<Attribute> &attrs) const{

}
