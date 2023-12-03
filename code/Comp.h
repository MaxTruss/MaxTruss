#pragma once
/***************
DAG
****************/
#define COMP_LARGE 1000
#define COMP_BIG 100
#define COMP_RANDOM_MAX 10
#define COMP_DAG_BLOCK_NUM_MAX 1000
typedef struct tpstCompNode
{
    int iEid;
    int iGEid;
    int iLocalId;
    int iGroupId;
    int iSup;
    int iBlockId;
    int iLayer;
    int iNeedSup;
    //int iKSup;
    bool bVstFlag;
    bool bAban;
    float fCost;
}TPST_COMP_NODE;

typedef struct tpstBlockEdge
{
    int iBlockId;
    int iCost;
    int iAffCnt;
    //vector<int> vAffectE;
}TPST_BLOCK_EDGE;

typedef struct tpstBlockNode
{
    int iBlockId;
    int iLayer;
    int iSize;
    //int iTopEid;
    //int iTopBlock;
    /* abandon all lower blocks */
    int iCost;
    bool bAban;
    vector<int> vEdges;
    /* block id */
    //vector<int> vFirst;
    vector<TPST_BLOCK_EDGE> vChildren;
}TPST_BLOCK_NODE;

typedef struct tpstGroupInfo
{
    int iGroupId;
    int iMaxBgt;
    //int iDesK;
    int iMaxBlock;
    //int iTopBlock;
    /* convert float to int */
    int iIntRatio;
    /* can be divided into small parts */
    bool bBroken;
    /* used k-clique */
    bool bBare;
    /* eid */
    vector<int> vFirst;
    vector<int> vEdges;
    vector<pair<int, int> > vCanE;
    /* start from 1 */
    vector<TPST_BLOCK_NODE> vBlocks;
}TPST_GROUP_INFO;

typedef struct tpstScoreInfo
{
    int iBgt;
    int iScore;
    vector<pair<int, int> > vCanE;
}TPST_SCORE_INFO;

typedef struct tpstTpScore
{
    int iScore;
    int iRealScore;
    vector<pair<int, int> > vCanE;
}TPST_TP_SCORE;

typedef struct tpstDPInfo
{
    int iScore;
    int iGroupNum;
    int iRealBdt;
    /* group id, budget */
    map<int, int> mpGroups;
}TPST_DP_INFO;

typedef struct tpstQueueInfo
{
    int iGroupId;
    int iBgt;
    pair<int, int> paPreKey;
}TPST_QUEUE_INFO;

class Comp
{
private:
    int m_iMaxEId;
    int m_iMaxGroupId;
    int m_iAvgRatio;
    myG *m_poG;
    myG *m_poBackG;

    int findNeb(int iEid, vector<int> &vNeibE);

    int add(int iEid);
    int construct();
    int BFSSameK(int iEid, vector <int> &vGroupE);
    int initGroup(int iGroupId, vector <int> &vGroupE);
    int divide();
    int buildLayer();
    int buildLayer(int iGroupId);
    int divideBlocks(int iGroupId);
    int divideBlocksStrong(int iGroupId);
    int blockPathDAG(int iGroupId);
    int blockPathDAG(int iGroupId, int iMaxBudget, map<int, TPST_TP_SCORE> &mpLcTable);
    //int shrinkBlocks(int iGroupId, map <pair<int, int>, vector <int> > &mpBlockRank, int iMaxCap);
    int getMaxBgt(int iGroupId, vector<pair<int, int> > &vNsrCanE, bool *pbBare);
    static int findKPlusNodes(myG &oInitG, int iDesK, int iEid, vector<int> &vNodes);
    //static int findKPlusNodesDrt(myG &oInitG, int iDesK, int iEid, vector<int> &vNodes);
    int findCanE(int iGroupId, vector<pair<int, int> > &vCanE);

    int refineTable(int iGroupId, map<int, TPST_TP_SCORE> &mpLcTable);
    int tableChsRandom(int iGroupId, int iMaxBudget, map<int, TPST_TP_SCORE> &mpLcTable);

    int tableChsBack(int iGroupId, int iMaxBudget, map<int, TPST_TP_SCORE> &mpLcTable);

    int tableChsInc(int iGroupId, int iMaxBudget, vector<pair<int, int> > &vCanE, map<int, TPST_TP_SCORE> &mpLcTable);

    //static int findStableSup(void *poComp, vector <int> &vLfE, vector <int> &vRtE);
    //int findUnstable(int iGroupId, vector<int> &vUnstableE);
    //int findStableCanE(int iGroupId, vector<int> &vDes, vector<pair<int, int> > &vResCanE);

    //int findDownCanE(int iGroupId, vector<int> &vBlocks, vector<pair<int, int> > &vCanE);
    //int findInsCanE(vector<pair<int, int> > &vIns, vector<int> &vDes, vector<pair<int, int> > &vCanE);
    //int findInsCanEClique(vector<pair<int, int> > &vIns, vector<int> &vDes, vector<pair<int, int> > &vCanE);

    int tableChsDAG(int iGroupId, int iMaxBudget, map<int, TPST_TP_SCORE> &mpLcTable);
public:
    int m_iDesK;
    int m_iCurK;
    int m_iTotalBudget;
    int m_iRandomMax;
    vector <int> m_vG2L;
    vector<TPST_COMP_NODE> *m_pvComp;
    /* group id, <eid> */
    map<int, TPST_GROUP_INFO > m_mpGroups;
    /* group, budget */
    vector<map<int, TPST_SCORE_INFO> > m_vScoreTable;
    vector<vector<TPST_DP_INFO> > m_vDPTable;

    static bool findKTrussCanEClique(myG &oInitG, int iDesK, vector<int> &vDesE, vector<int> &vEPool, vector<int> &vRmEPool, vector<pair<int, int> > &vResCanE, int iMaxBgt);
    static bool findKTrussCanEInc(myG &oInitG, myG &oBackG, int iBgt, int iMinK, int iDesK, vector<int> &vDesE, vector<int> &vEPool, vector<int> &vRmEPool, vector<pair<int, int> > &vResCanE);
    static bool findKTrussCanETry(myG &oInitG, myG &oBackG, int iMinK, int iDesK, vector<int> &vDes, vector<pair<int, int> > &vResCanE, Comp *poComp, vector<int> &vBareE, int iMaxBgt);
    static bool findKTrussCanE(myG &oInitG, myG &oBackG, int iMinK, int iDesK, vector<int> &vDes, vector<pair<int, int> > &vCanE, Comp *poComp, bool bBareStop, vector<int> &vBareE, int iMaxBgt);

    static bool getAbaState(void *poComp, int iGroupId, int iGEid);
    static bool getAbaStateNoG(void *poComp, int iGroupId, int iGEid);

    Comp(myG &oInitG, myG &oBackG, int iCurK, int iDesK);
    ~Comp();

    int getUpBgt(int iBgt);

    int tableRandom();
    int tableDAG();
    int normalDPTable(vector<pair<int, int> > &vCanE);
    int largeDPTable(vector<pair<int, int> > &vCanE, bool bAll);
    int insertAll(vector<pair<int, int> > &vCanE);

    int showGroup(int iGroupId);

    TPST_COMP_NODE * findNode(int iEid);

};
