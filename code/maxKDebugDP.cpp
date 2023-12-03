/***************
baseline
input repetition times and budget number
main cpp
****************/
#include "common.h"
#include "myG.h"
#include "insert.h"
#include "Comp.h"
#include "file.h"
#include "backtrack.h"

myG g_baseG;

long g_lCanETime;
long g_lInsertTime;
long g_lDAGBuildTime;
long g_lCutTime;
long g_lKMinusTime;
long g_lRandomTime;

extern long g_lDAGTime;
extern long g_lDAGPathTime;
extern long g_lDAGDivideTime;
extern bool g_bFirstDAG;

/*****************
input:
        char *pcSrcPath: source data path
description:
        global init
******************/
int init(char *pcSrcFile)
{
    int iSize = 0;

    printf("init read G\n");
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
	struct timeval tv;
	long lStartTime = 0;
	long lCurTime = 0;
	long lTotalStartTime = 0;

	long lInitCompTime = 0;
	long lTableTime = 0;
	long lDPATime = 0;
	long lDPBTime = 0;
	long lTestDPTime = 0;

	int iAScore = 0;
	int iBScore = 0;

	int iBgtNum = 0;

	int iDesK = 0;

	float fTopRatio = 0;

	g_bFirstDAG = true;

	vector<pair<int, int> > vCanE;
    int iTotalScore = 0;

    if (5 > argc)
    {
        printf("argc: %d\n", argc);
        DEBUG_ASSERT(0);
    }

    char *pcSrcFile = argv[1];
    sscanf(argv[2], "%d", &iBgtNum);
    sscanf(argv[3], "%d", &iDesK);
    sscanf(argv[4], "%f", &fTopRatio);

    printf("start init\n");
    init(pcSrcFile);
    //g_baseG.show();

    gettimeofday(&tv, NULL);
    lTotalStartTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;

	myG myBackG;
    /* copy */
    myBackG.copyMap(g_baseG);

    int iRemainBgt = iBgtNum;
    int iCurK = iDesK;
    bool bFirst = true;
    while (0 < iRemainBgt)
    {
        --iCurK;
        if (iCurK < 3)
        {
            /* cannot find anymore */
            break;
        }
        DEBUG_PRINTF("============current k: %d\n", iCurK);
        vector<pair<int, int> > vCurCanE;
        gettimeofday(&tv, NULL);
        lStartTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        Comp oDesComp(g_baseG, myBackG, iCurK, iDesK);
        oDesComp.m_iRandomMax = 100;
        int iFullBgt = oDesComp.getUpBgt(iRemainBgt);
        gettimeofday(&tv, NULL);
        lCurTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        lInitCompTime += lCurTime - lStartTime;
        DEBUG_PRINTF("Upper bound: %d\n", iFullBgt);

        if (iFullBgt <= iRemainBgt)
        {
            /* choose all */
            iRemainBgt -= iFullBgt;
            iTotalScore += oDesComp.insertAll(vCurCanE);
        }
        else
        {
            if (bFirst)
            {
                bFirst = false;
            }
            /* choose partial */
            gettimeofday(&tv, NULL);
            lStartTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
            int iGroupNum = oDesComp.tableDAG();
            gettimeofday(&tv, NULL);
            lCurTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
            lTableTime += lCurTime - lStartTime;

            DEBUG_PRINTF("Start DP 1\n");
            gettimeofday(&tv, NULL);
            lStartTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
            vector<pair<int, int> > vTpCanE;
            int iTpAScore = oDesComp.normalDPTable(vTpCanE);
            gettimeofday(&tv, NULL);
            lCurTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
            lDPATime += lCurTime - lStartTime;
            iAScore += iTpAScore;

            DEBUG_PRINTF("Start DP 2\n");
            gettimeofday(&tv, NULL);
            lStartTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
            int iTpBScore = oDesComp.largeDPTable(vCurCanE, true);
            gettimeofday(&tv, NULL);
            lCurTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
            lDPBTime += lCurTime - lStartTime;
            iTotalScore += iTpBScore;
            iBScore += iTpBScore;

            backtrack::batIns(g_baseG, myBackG, vCurCanE);

            iRemainBgt -= vCurCanE.size();
        }
        vCanE.insert(vCanE.end(), vCurCanE.begin(), vCurCanE.end());
        if ((!bFirst) && (vCurCanE.empty()))
        {
            /* cannot find anymore */
            break;
        }
    }

    /* save result */
    if (5 < argc)
    {
        file_insE(g_baseG, argv[5], vCanE, iBgtNum, iDesK);
    }
    if (6 < argc)
    {
        file_saveTrussness(g_baseG, argv[6]);
    }

	printf("DP Algorithm 1 time: %.2f s\n", (lDPATime) / 1000.0);
	printf("DP Algorithm 2 time: %.2f s\n", (lDPBTime) / 1000.0);
	printf("DP Algorithm 1 score: %d\n", iAScore);
	printf("DP Algorithm 2 score: %d\n", iBScore);

    return 0;
}
