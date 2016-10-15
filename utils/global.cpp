#include "utils.h"

int current = 0;
int tt = 0;
unsigned char h[61];

// define RC
const RC OK = 0;
const RC FILEERROR = OK + 1;
const RC NOTFOUND = FILEERROR + 1;

const double EPS = 1e-6;