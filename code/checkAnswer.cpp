/***************
baseline
input repetition times and budget number
main cpp
****************/
#include "common.h"
#include "myG.h"
#include "backtrack.h"
#include "insert.h"
#include "file.h"

myG g_baseG;

long g_lCanETime;
long g_lInsertTime;
long g_lDAGBuildTime;
long g_lCutTime;
long g_lKMinusTime;
long g_lRandomTime;

/*****************
input:
        char *pcSrcPath: source data path
description:
        global init
******************/
int init(char *pcSrcFile)
{
    int iSize = 0;

    //printf("init read G\n");
    iSize = file_readBitG(g_baseG, pcSrcFile);

    return iSize;
}

/*****************
input:
        source database Graph file name
        output result path name
        dataset label
        insert file
        output flag
description:
        main function
******************/
int main(int argc, char *argv[])
{
	int iDesK = 0;

	vector<pair<int, int> > vCanE;

    if (4 > argc)
    {
        printf("argc: %d\n", argc);
        DEBUG_ASSERT(0);
    }

    char *pcSrcFile = argv[1];
    char *pcInsFile = argv[2];
    sscanf(argv[3], "%d", &iDesK);

    printf("start init\n");
    init(pcSrcFile);
    file_readInsE(pcInsFile, vCanE);

	myG myBackG;
    /* copy */
    myBackG.copyMap(g_baseG);

    int iScore = backtrack::tryIns(g_baseG, myBackG, iDesK, vCanE);
    printf("CHECK Score: %d used budget: %d\n",
                 iScore, vCanE.size());

    return 0;
}
