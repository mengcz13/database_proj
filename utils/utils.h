#ifndef UTILS_H
#define UTILS_H

typedef int PageNum;
typedef int SlotNum;
typedef int RC; // Error Code
typedef int FP; // pointer to free page

// define RC
extern const RC OK;
extern const RC FILEERROR;
extern const RC NOTFOUND;

// define AttrType
// extern const AttrType INTEGER;
// extern const AttrType FLOAT;
// extern const AttrType STRING;
enum AttrType {
    INTEGER, 
    FLOAT,
    STRING
};

// define CompOp
// extern const CompOp EQ_OP;
// extern const CompOp LT_OP;
// extern const CompOp GT_OP;
// extern const CompOp LE_OP;
// extern const CompOp GE_OP;
// extern const CompOp NE_OP;
// extern const CompOp NO_OP;
enum CompOp {
    EQ_OP,
    LT_OP,
    GT_OP,
    LE_OP,
    GE_OP,
    NE_OP,
    NO_OP
};

extern const double EPS;

#endif