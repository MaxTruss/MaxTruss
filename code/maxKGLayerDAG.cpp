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
	long lDPTime = 0;
	long lTestDPTime = 0;

	int iBgtNum = 0;

	int iDesK = 0;

	float fTopRatio = 0;

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

    /*gettimeofday(&tv, NULL);
    lStartTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    Comp oDesComp(g_baseG, myBackG, iDesK - 1, iBgtNum);
    gettimeofday(&tv, NULL);
    lCurTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    lInitCompTime = lCurTime - lStartTime;

    gettimeofday(&tv, NULL);
    lStartTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    oDesComp.tableDAG(fTopRatio);
    gettimeofday(&tv, NULL);
    lCurTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    lTableTime = lCurTime - lStartTime;

    gettimeofday(&tv, NULL);
    lStartTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    iTotalScore = oDesComp.largeDPTable(vCanE, iBgtNum);
    gettimeofday(&tv, NULL);
    lCurTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    lDPTime = lCurTime - lStartTime;*/

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
        int iFullBgt = oDesComp.getUpBgt(iRemainBgt);
        gettimeofday(&tv, NULL);
        lCurTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        lInitCompTime += lCurTime - lStartTime;

        DEBUG_PRINTF("DEBUG current full budget: %d\n", iFullBgt);

        if (iFullBgt <= iRemainBgt)
        {
            /* choose all */
            iRemainBgt -= iFullBgt;
            iTotalScore += oDesComp.insertAll(vCurCanE);
            if (0 == iRemainBgt)
            {
                DEBUG_PRINTF("DEBUG upper bound budget: %d\n", iBgtNum);
            }
        }
        else
        {
            if (bFirst)
            {
                bFirst = false;
                DEBUG_PRINTF("DEBUG upper bound budget: %d\n", iBgtNum - iRemainBgt + iFullBgt);
            }
            /* choose partial */
            gettimeofday(&tv, NULL);
            lStartTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
            int iGroupNum = oDesComp.tableDAG();
            gettimeofday(&tv, NULL);
            lCurTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
            lTableTime += lCurTime - lStartTime;

            gettimeofday(&tv, NULL);
            lStartTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
            if (iRemainBgt > iGroupNum)
            {
                iTotalScore += oDesComp.normalDPTable(vCurCanE);
            }
            else
            {
                iTotalScore += oDesComp.largeDPTable(vCurCanE, ((iCurK <= 3) || (iRemainBgt < 0.5 * iFullBgt)));
            }

            gettimeofday(&tv, NULL);
            lCurTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
            lDPTime += lCurTime - lStartTime;

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

    DEBUG_PRINTF("DEBUG total score: %d used budget: %d\n", iTotalScore, vCanE.size());

    /* save result */
    if (5 < argc)
    {
        file_insE(g_baseG, argv[5], vCanE, iBgtNum, iDesK);
    }
    if (6 < argc)
    {
        file_saveTrussness(g_baseG, argv[6]);
    }

	printf(" init component time: %ld ms\n", lInitCompTime);
	printf(" build table time: %ld ms\n", lTableTime);
	printf(" DAG time: %ld ms\n", g_lDAGTime);
	printf(" CanE time: %ld ms\n", g_lCanETime);
	printf(" insert time: %ld ms\n", g_lInsertTime);
	printf(" divide DAG time: %ld ms\n", g_lDAGDivideTime);
	printf(" path DAG time: %ld ms\n", g_lDAGPathTime);
	printf(" build DAG time: %ld ms\n", g_lDAGBuildTime);
	printf(" cut time: %ld ms\n", g_lCutTime);
	printf(" (k - 2)-truss time: %ld ms\n", g_lKMinusTime);
	printf(" DP time: %ld ms test DP time: %ld ms\n", lDPTime, lTestDPTime);

	printf("Total running time: %.2f s\n", (lCurTime - lTotalStartTime) / 1000.0);

    return 0;
}
