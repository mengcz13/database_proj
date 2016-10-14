#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "utils/utils.h"

typedef char Unit;
#define LBIT(x, u) (1 << ((sizeof(u) << 3) - 1 - x))
struct MyBitSet {
    Unit* _data;
    SlotNum size;

    MyBitSet(SlotNum s) {
        int sc = s - 1 + sizeof(Unit) / sizeof(Unit);
        _data = new Unit[sc];
        memset(_data, 0, sizeof(Unit) * sc);
        size = s;
    }

    ~MyBitSet() { delete []_data; }

    Unit* getPos(SlotNum n) { return _data + n / sizeof(Unit); }

    void set(SlotNum n) {   // set to 1
        (*(getPos(n))) |= LBIT(n % sizeof(Unit), Unit);
    }

    void clear(SlotNum n) { // set to 0
        (*(getPos(n))) &= (~LBIT(n % sizeof(Unit), Unit));
    }

    bool check(SlotNum n) { // check if is 1
        Unit t = *(getPos(n));
        return (t&=LBIT(n % sizeof(Unit), Unit));
    }

    SlotNum getSize() { return size; }

};

int main(int argc, char** argv) {
    int n = atoi(argv[1]);
    MyBitSet myBitSet(n);
    for (int i = 0; i < n; ++i) {
        bool pass = true;
        myBitSet.set(i);
        // for (int j = 0; j < n; ++j) {
        //     if (j == i && !myBitSet.check(j) || j != i && myBitSet.check(j)) {
        //         pass = false;
        //     }
        // }
        myBitSet.clear(i);
        // for (int j = 0; j < n; ++j) {
        //     if (myBitSet.check(j))
        //         pass = false;
        // }
        // if (!pass) {
        //     printf("error!\n");
        //     exit(-1);
        // }
        // else {
        //     // printf("pass %d\n", i);
        // }
    }
    return 0;
}