#include "bufmanager/BufPageManager.h"
#include "fileio/FileManager.h"
#include "utils/pagedef.h"

class RM_Manager {
  public:
       RM_Manager  (PF_Manager &pfm);            // Constructor
       ~RM_Manager ();                           // Destructor
    RC CreateFile  (const char *fileName, int recordSize);  
                                                 // Create a new file
    RC DestroyFile (const char *fileName);       // Destroy a file
    RC OpenFile    (const char *fileName, RM_FileHandle &fileHandle);
                                                 // Open a file
    RC CloseFile   (RM_FileHandle &fileHandle);  // Close a file
};