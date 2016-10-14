#include "rm.h"

RM_Manager::RM_Manager(BufPageManager& bpm): 
    bufPageManager(bpm), fileManager(*(bpm.fileManager)) {

}

RM_Manager::~RM_Manager() {

}

RC RM_Manager::createFile(const char* fileName, int recordSize) {
    if (!fileManager.createFile(fileName))
        return FILEERROR;
    //init file with header
    FileHeader fileHeader(recordSize);
    int id = 0;
    fileManager.openFile(fileName, id);
    int index = 0;
    char* p = (char*)bufPageManager.getPage(id, 0, index);
    fileHeader.toCharArray(p);
    bufPageManager.markDirty(index);
    bufPageManager.writeBack(index);
    bufPageManager.release(index);
    fileManager.closeFile(id);
    return OK;
}

RC RM_Manager::destroyFile(const char* fileName) {
    //TODO: delete file
    return OK;
}

RC RM_Manager::openFile(const char* fileName, RM_FileHandle& fileHandle) {
    int id = 0;
    if (!fileManager.openFile(fileName, id))
        return FILEERROR;
    fileHandle.setFileID(id);
    fileHandle.setBufPageManager(bufPageManager);
    int index = 0;
    FileHeader fileHeader((char*)bufPageManager.getPage(id, 0, index));
    fileHandle.setFileHeader(fileHeader);
    bufPageManager.access(index);
    return OK;
}

RC RM_Manager::closeFile(RM_FileHandle& fileHandle) {
    fileManager.closeFile(fileHandle.getFileID());
    return OK;
}