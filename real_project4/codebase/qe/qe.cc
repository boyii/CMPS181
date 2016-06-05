#include <vector>
#include <string>
#include <iostream>
#include "qe.h"
#include <cstring>


bool Filter::checkScanCondition(char *recordString, CompOp compOp, const char *value)
{
    if (compOp == NO_OP) return true;
    int cmp = strcmp(recordString, value);
//    cout << "our first string (first arg) : " << recordString << endl;
//    cout << "were comparing to (third arg): " << value  << endl;
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
            len = VARCHAR_LENGTH_SIZE + *str_len;
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
    bool test = false;

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
        cmp = checkScanCondition(valInt,our_cond.op,r_data);
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
        memcpy(valStr1,(char *) l_data + 5, *newlen); // were testing this
        valStr1[*newlen] = '\0';
        newlen2 = (int *) ((char *) r_data );
        valStr2 = (char *) malloc(*newlen2 + 1);
        memcpy(valStr2, (char *) r_data + 4, *newlen2);
        valStr2[*newlen2] = '\0';
        cmp = checkScanCondition(valStr1, our_cond.op, valStr2);
    }
    bool t = true;
    int ret = -1;

    if(cmp) ret = 0;

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

Project::~Project(){}

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
    for(unsigned i = 0; i < pAttrs.size(); i++) {
        for(unsigned j = 0; j < iterAttrs.size(); j++) {
            if(pAttrs[j].compare(iterAttrs[j].name) == 0){
                attrs.push_back(iterAttrs[j]);
                break;
            }
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

INLJoin::~INLJoin(){}

RC INLJoin::getNextTuple(void *data){
        void * innerData = malloc(PAGE_SIZE);
        void * outerData = malloc(PAGE_SIZE);
        vector<Attribute> attrs;
        getAttributes(attrs);

        //runs an outer loop that scan through outer tuples and an inner loop that scans through
        //inner tuples index. When the data is the same in both, which satisfies the <EQ> condition
        //it returns the tuple filled with attributes. 
        bool outer = true;
        bool inner = true;
        while(outer){
            _leftIn->getNextTuple(outerData);
            while(inner){
                if(_rightIn->getNextTuple(innerData) == EOF){
                        inner = false;
                }

                //checks if the innerData and outerData are equal
                if(memcmp(innerData, outerData, PAGE_SIZE) == 0){
                        for(unsigned i = 0; i < attrs.size(); i++){
                                memcpy(data, attrs.at(i).name.c_str(), (size_t)attrs.at(i).length);
                        }
                        outer = false;
                        inner = false;
                }
            }
        inner = true;
        }
	free(innerData);
	free(outerData);
        return 0;
}

//gets both attribute sets and combines them and returns
void INLJoin::getAttributes(vector<Attribute> &attrs) const{
        _leftIn->getAttributes(attrs);
        vector<Attribute> SAttrs;

        _rightIn->getAttributes(SAttrs);
        for(unsigned i = 0; i < SAttrs.size(); i++){
                attrs.push_back(SAttrs.at(i));
        }
}

