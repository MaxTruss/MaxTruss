#pragma once
/***************
my class G
****************/

/* base = init - sub */
template <typename mpTA, typename mpTB>
bool myG_mapSub(mpTA &mpBase, mpTB &mpInitG, mpTA &mpSub)
{
    typename mpTB::iterator itmapT;

    for (itmapT = mpInitG.begin(); itmapT != mpInitG.end(); itmapT++)
    {
        if (mpSub.find(itmapT->first) == mpSub.end())
        {
            /* doesn't have */
            mpBase[itmapT->first];
        }
    }
    return 0;
}

#define myG_getPair(iX, iY) ((iX < iY) ? (pair<int, int>(iX, iY)) : (pair<int, int>(iY, iX)))

typedef map<pair<int, int>, int > MAP_BASIC_G;
typedef struct tpstlsByEid
{
    int iLfEid;
    int iRtEid;
}TPST_LS_BY_EID;

typedef struct tpstvCanE
{
    pair<int, int> paXY;
    int iOEid;
    int iSup;
}TPSTV_CAN_E;

typedef struct tpstMapByEid
{
    int eid;
    //int iSup;
    //int iJuSup;
    int iSeSup;
    //int iOutSup;
    //int iTrigger;
    int iTrussness;
    //int iLowB;
    int iLayer;
    //int iGroupId;
    //int iAlleyType;
    //bool bNewFlag;
    //bool bPseNewFlag;
    bool bUgdFlag;
    //bool bBdBallFlag;
    //bool bNewJuTriFlag;
    //bool bSeedFlag;
    bool bVstFlag;
    bool bVisQFlag;
    //bool bHandledFlag;
    //bool bRmFlag;
    //bool bLock;
    bool bInsLFlag;
    bool bCanQFlag;
    bool bDoneQFlag;
    bool bUsedCanFlag;
    bool bUsedDoneFlag;
    bool bUsedVisFlag;
    pair<int, int> paXY;
    vector<int> vLfE;
    vector<int> vRtE;
    //list<TPST_LS_BY_EID> lsLcNebE;
}TPST_MAP_BY_EID;

class myG
{
public:
    /* double */
    MAP_BASIC_G m_mpBasicG;
    vector<int> *m_pvPNeCnt;

    int m_iMaxPId;
    int m_iMaxEId;
    int m_iMaxD;
    int m_iMaxK;
    int m_iDensePId;
    /* single, x < y */
    vector<TPST_MAP_BY_EID> *m_pvG;

    myG();
    ~myG();
    int add(int x, int y);
    int addNeibTri(int iEid);
    //int addLsNeib(int iEid, int iLfEid, int iRtEid);
    //int rmLsNeib(int iEid, list<TPST_LS_BY_EID>::iterator pstls);
    //int rmLsNeib(int iEid);
    bool copyMap(myG &oInitG);
    int induce(vector<int> &vDesP, myG &oResG, vector<pair<int, int> > &vNewCanE);
    int induceOneHop(vector<int> &vDesP, myG &oResG);
    //int updateJuSup(int iEid);
    //int updateSeSup(int iEid);
    //int checkSeSup(int iEid);
    //int showSeSup(int iEid);

    int getLowB(int x, int y);
    int getUpB(int x, int y);
    bool checkInsFourTruss(int x, int y, int iUndE);
    bool checkIns(int x, int y, int iDesK, int iLayer);
    bool checkIns(int x, int y, int iUndE, int iDesK, int iLayer, int iGroup);

    /* no need to be exist */
    int findKSup(int iDesK, vector <int> &vLfE, vector <int> &vRtE);
    int findKSup(int iMinK, int iDesK, vector <int> &vLfE, vector <int> &vRtE, void * pPara1, int iPara2, bool (*fun_state)(void *, int, int));

    int findNeb(int x, int y, vector<int> &vNeib);
    int findNeb(int x, int y, vector<int> &vLfE, vector<int> &vRtE);
    int findNeb(int x, int y, vector<int> &vLfE, vector<int> &vRtE, MAP_BASIC_G &mapCalG);
    int findNeb(vector<int> &vLfE, vector<int> &vRtE, int iDesK, vector<pair<int, int> > &vNeib);
    int cntHighNeib(vector<int> &vLfE, vector<int> &vRtE, int iGeqK);

    int findNebE(int x, int y, list<pair<int, int> > &lspaNeibE);

    /* find eid in calG, at least trussness */
    int findNebE(int x, int y, list<int> &lstNeibE, MAP_BASIC_G &mapCalG, int iTrussness);

    /* find neighbor edge in calG, not in BlackG, mintruss == truss */
    int findNebE(int x, int y, list<int> &lstNeibE, MAP_BASIC_G &mapCalG, MAP_BASIC_G &mapBlackG, int iTrussness);

    /* find neighbor edge in calG, not in BlackG, mintruss == truss */
    int findThirdE(int iNode, list<int> &lsThirdE);

    int findPNebP(int iNodeId, vector <int> &vNodes);
    int findPNebP(int iNodeId, vector <int> &vNodes, vector <int> &vEdges);
    int findPNebP(int iNodeId, list<int> &lstP, MAP_BASIC_G &mapCalG);

    int findPCanTriP(int iNodeId, vector<int> &vCanP);
    /* t(e) >= k*/
    int findECanTriP(int iEid, vector<pair<int, int> > &vCanE);
    int findECanTriPSort(int iEid, int iDesK, vector<TPSTV_CAN_E> &vCanE);
    int findECanTriP(int iEid, int iMinK, int iDesK, vector<int> &vNeibE, vector<pair<int, int> > &vCanE);
    int findECanTriP(int iEid, int iMinK, int iDesK, vector<pair<int, int> > &vCanE, void * pPara1, int iPara2, bool (*fun_state)(void *, int, int));
    int findECanTriP(int iEid, int iMinK, int iDesK, vector<int> &vNeibE, vector<pair<int, int> > &vCanE, void * pPara1, int iPara2, bool (*fun_state)(void *, int, int));
    int findECanTriPSort(int iEid, int iMinK, int iDesK, vector<pair<pair<int, int>, int> > &vCanE, void * pPara1, int iPara2, bool (*fun_state)(void *, int, int));
    //int findECanTriP(int iEid, vector<pair<int, int> > &vCanE, int iLayer);
    int findENewCanTriP(int x, int y, vector<pair<int, int> > &vNewCanE);
    int findAllCanE(vector<pair<int, int> > &vNewCanE);
    int findPCanE(vector<int> &vCanP, vector<pair<int, int> > &vNewCanE);
    int findPNebPTrdE(int iNodeId, list<int> &lstP, list<int> &lsThirdE);

    bool simpleRm(int x, int y);
    /* will not copy map, will copy array */
    TPST_MAP_BY_EID * findNode(int eid);
    TPST_MAP_BY_EID * findNode(int x, int y);

    int show();
    pair <int, int> findByEid(int eid)
    {
        return (*m_pvG)[eid].paXY;
    }

};
