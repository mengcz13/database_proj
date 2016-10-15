#include "rm.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

using namespace std;

const int RSIZE = 13;
const int RNUM = 10;

int main() {
    MyBitMap::initConst();
    FileManager* fm = new FileManager();
    BufPageManager bpm(fm);
    RM_Manager rmm(bpm);
    rmm.createFile("test.db", RSIZE);
    RM_FileHandle fileHandle(bpm);
    rmm.openFile("test.db", fileHandle);

    char* str = new char[RSIZE];
    memset(str, 0, RSIZE);
    memcpy(str, "Hello World!", 13);
    RID rid;
    RM_Record record(RSIZE);
    for (int i = 0; i < RNUM; ++i) {
        fileHandle.insertRec(str, rid);
        cout << rid.pageNum << ' ' << rid.slotNum << endl;

        fileHandle.getRec(rid, record);
        char* d = NULL;
        record.getData(d);
        cout << d << endl;
        if (strncmp(d, str, 13)) {
            cout << "Error!" << endl;
            exit(-1);
        }

        memcpy(d, "Hello China!", 13);
        fileHandle.updateRec(record);

        fileHandle.getRec(rid, record);
        record.getData(d);
        cout << d << endl;
        if (strncmp(d, "Hello China!", 13)) {
            cout << "Error!" << endl;
            exit(-1);
        }

        // if (i & 1)
            // fileHandle.deleteRec(rid);
    }

    RM_FileScan rfs(fileHandle);
    memcpy(str, "Hello", 6);
    rfs.openScan(STRING, 5, 0, EQ_OP, str);
    while (rfs.getNextRec(record) == OK) {
        RID rid;
        record.getRid(rid);
        cout << rid.pageNum << ' ' << rid.slotNum << endl;
        char* p;
        record.getData(p);
        cout << p << endl;
    }

    rmm.closeFile(fileHandle);
    delete []str;
    delete fm;
    return 0;
}