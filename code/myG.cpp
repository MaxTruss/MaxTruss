/***************
my class G
****************/

#include "common.h"
#include "myG.h"

/*****************
input:
        myG &oInitG
description:
        copy map
        calculate stage
******************/
bool myG::copyMap(myG &oInitG)
{
    m_mpBasicG = oInitG.m_mpBasicG;
    m_iMaxPId = oInitG.m_iMaxPId;
    m_iMaxEId = oInitG.m_iMaxEId;
    m_iMaxD = oInitG.m_iMaxD;
    m_iMaxK = oInitG.m_iMaxK;

    /* first time */
    DEBUG_ASSERT(NULL != m_pvPNeCnt);
    *m_pvPNeCnt = *(oInitG.m_pvPNeCnt);
    m_pvPNeCnt->reserve(oInitG.m_pvPNeCnt->capacity());

    /* first time */
    DEBUG_ASSERT(NULL != m_pvG);
    m_pvG->reserve(oInitG.m_pvG->capacity());
    *m_pvG = *(oInitG.m_pvG);
    return true;
}
/*****************
input:
        myG &oInitG
description:
        copy map
        calculate stage
******************/
int myG::induce(vector<int> &vDesP, myG &oResG, vector<pair<int, int> > &vNewCanE)
{
    vector<int>::iterator itP;
    /* pid, none */
    map<int, int> mpP;
    map<int, int>::iterator itmpP;
    map<int, int>::iterator itmpNext;

    MAP_BASIC_G::iterator itLowB;
    MAP_BASIC_G::iterator itHighB;
    int iCnt = 0;
    /* nei(u) < nei(v) */
    int u = 0;
    int w = 0;

    for (itP = vDesP.begin(); itP != vDesP.end(); ++itP)
    {
        mpP[*itP] = 0;
        //DEBUG_PRINTF("MY_G get node %d\n", *itP);
    }

    for (itmpP = mpP.begin(); itmpP != mpP.end(); ++itmpP)
    {
        u = itmpP->first;

        /* find u's neighbor , note u > 0 */
        itLowB = m_mpBasicG.upper_bound(pair<int, int>(u, u));
        itHighB = m_mpBasicG.upper_bound(pair<int, int>(u + 1, 0));
        while (itLowB != itHighB)
        {
            w = itLowB->first.second;
            if (mpP.find(w) == mpP.end())
            {
                /* not in the subgraph */
                ++itLowB;
                continue;
            }

            oResG.add(u, w);
            //DEBUG_PRINTF("MY_G add (%d, %d)\n", u, w);
            ++iCnt;

            ++itLowB;
        }
        itmpNext = itmpP;
        ++itmpNext;
        for (; itmpNext != mpP.end(); ++itmpNext)
        {
            w = itmpNext->first;
            if (oResG.m_mpBasicG.find(pair<int, int>(u, w)) == oResG.m_mpBasicG.end())
            {
                vNewCanE.push_back(pair<int, int>(u, w));
                //DEBUG_PRINTF("MY_G add new (%d, %d)\n", u, w);
            }
        }
    }

    return 0;
}

/*****************
input:
        myG &oInitG
description:
        copy map
        calculate stage
******************/
int myG::induceOneHop(vector<int> &vDesP, myG &oResG)
{
    vector<int>::iterator itP;
    vector<int>::iterator itNeibP;
    vector <int> vNodes;

    /* pid, none */
    map <int, int> mpNodes;
    map <int, int>::iterator itmpP;
    int iCurP = 0;

    /* find nodes */
    for (itP = vDesP.begin(); itP != vDesP.end(); ++itP)
    {
        mpNodes[*itP] = 1;
        vNodes.clear();
        findPNebP(*itP, vNodes);
        for (itNeibP = vNodes.begin(); itNeibP != vNodes.end(); ++itNeibP)
        {
            mpNodes[*itNeibP] = 1;
        }
    }
    /* find edges */
    for (itmpP = mpNodes.begin(); itmpP != mpNodes.end(); ++itmpP)
    {
        iCurP = itmpP->first;
        vNodes.clear();
        findPNebP(iCurP, vNodes);
        //DEBUG_PRINTF("SUB_G neighbor size: %d\n", vNodes.size());

        for (itP = vNodes.begin(); itP != vNodes.end(); ++itP)
        {
            if (iCurP > *itP)
            {
                /* ignore half */
                continue;
            }
            if (mpNodes.find(*itP) != mpNodes.end())
            {
                //DEBUG_PRINTF("SUB_G add (%d, %d)\n", iCurP, *itP);
                oResG.add(iCurP, *itP);
            }
        }
    }
    return 0;
}
/*****************
input:
        int x
        int y
        long double ldP
description:
        add new edge
        init stage
******************/
int myG::add(int x, int y)
{
    MAP_BASIC_G::iterator itGNode;
    TPST_MAP_BY_EID stTp = {0};
    int iPrevMaxPid = 0;

    itGNode = m_mpBasicG.find(pair<int, int>(x, y));
    if (itGNode != m_mpBasicG.end())
    {
        /* have joined, ignore */
        printf("DEBUG has joined\n");
        return 0;
    }

    iPrevMaxPid = m_iMaxPId;
    m_iMaxEId++;
    if (m_iMaxPId < x)
    {
        m_iMaxPId = x;
    }
    if (m_iMaxPId < y)
    {
        m_iMaxPId = y;
    }
    if (iPrevMaxPid < m_iMaxPId)
    {
        /* max pid change */
        m_pvPNeCnt->resize(m_iMaxPId + 1);
    }

    m_mpBasicG[pair<int, int>(x, y)] = m_iMaxEId;
    m_mpBasicG[pair<int, int>(y, x)] = m_iMaxEId;

    stTp.eid = m_iMaxEId;
    //stTp.iSup = 0;
    //stTp.iJuSup = 0;
    stTp.iSeSup = 0;
    //stTp.iGroupId = 0;
    //stTp.iOutSup = 0;
    stTp.paXY = myG_getPair(x, y);
    stTp.iTrussness = 2;
    //stTp.iAlleyType = 0;
    //stTp.bNewFlag = false;
    stTp.bUgdFlag = false;
    //stTp.bPseNewFlag = false;
    //stTp.bBdBallFlag = false;
    //stTp.bNewJuTriFlag = false;
    //stTp.bSeedFlag = false;
    //stTp.bVstFlag = false;
    stTp.bVisQFlag = false;
    //stTp.bHandledFlag = false;
    //stTp.bRmFlag = false;
    //stTp.bLock = false;
    //stTp.bInsLFlag = false;
    stTp.bCanQFlag = false;
    stTp.bDoneQFlag = false;
    stTp.bUsedCanFlag = false;
    stTp.bUsedDoneFlag = false;
    stTp.bUsedVisFlag = false;
    //stTp.bValueFlag = false;

    m_pvG->push_back(stTp);

    /* check right position */
    DEBUG_ASSERT(m_iMaxEId == (*m_pvG)[m_iMaxEId].eid);

    (*m_pvPNeCnt)[x]++;
    (*m_pvPNeCnt)[y]++;

    if ((*m_pvPNeCnt)[x] > m_iMaxD)
    {
        m_iMaxD = (*m_pvPNeCnt)[x];
        m_iDensePId = x;
    }
    if ((*m_pvPNeCnt)[y] > m_iMaxD)
    {
        m_iMaxD = (*m_pvPNeCnt)[y];
        m_iDensePId = y;
    }

    return m_iMaxEId;
}
/*****************
input:
        int x
        int y
        long double ldP
description:
        add new edge
        init stage
******************/
int myG::show()
{
    MAP_BASIC_G::iterator itGNode;
    TPST_MAP_BY_EID* pstNode = NULL;

    itGNode = m_mpBasicG.begin();
    for (; itGNode != m_mpBasicG.end(); ++itGNode)
    {
        if (itGNode->first.first < itGNode->first.second)
        {
            pstNode = findNode(itGNode->second);
            DEBUG_ASSERT(NULL != pstNode);
            DEBUG_PRINTF("SHOW_G,%d,%d,%d,%d\n", itGNode->first.first, itGNode->first.second,
                         pstNode->iTrussness, pstNode->iLayer);
        }
    }

    return m_iMaxEId;
}
/*****************
input:
        myG &mpG
        int iEid
description:
        simply add
******************/
int myG::addNeibTri(int iEid)
{
    TPST_MAP_BY_EID* pstENode = NULL;
    vector<int>::iterator itLfE;
    vector<int>::iterator itRtE;
    int x = 0;
    int y = 0;

    /* add and init */
    DEBUG_ASSERT(0 != iEid);
    pstENode = findNode(iEid);
    DEBUG_ASSERT(NULL != pstENode);
    x = pstENode->paXY.first;
    y = pstENode->paXY.second;

    pstENode->vLfE.clear();
    pstENode->vRtE.clear();
    //pstENode->iSup = findNeb(x, y, pstENode->vLfE, pstENode->vRtE);
    findNeb(x, y, pstENode->vLfE, pstENode->vRtE);

    itLfE = pstENode->vLfE.begin();
    itRtE = pstENode->vRtE.begin();
    for (; itLfE != pstENode->vLfE.end(); ++itLfE, ++itRtE)
    {
        TPST_MAP_BY_EID* pstLfNode = NULL;
        TPST_MAP_BY_EID* pstRtNode = NULL;

        pstLfNode = findNode(*itLfE);
        DEBUG_ASSERT(NULL != pstLfNode);
        pstRtNode = findNode(*itRtE);
        DEBUG_ASSERT(NULL != pstRtNode);

        if (pstLfNode->paXY.first == x)
        {
            /* z > x */
            pstLfNode->vLfE.push_back(iEid);
            pstLfNode->vRtE.push_back(*itRtE);
        }
        else
        {
            /* z < x */
            pstLfNode->vLfE.push_back(*itRtE);
            pstLfNode->vRtE.push_back(iEid);
        }
        if (pstRtNode->paXY.first == y)
        {
            /* z > y */
            pstRtNode->vLfE.push_back(iEid);
            pstRtNode->vRtE.push_back(*itLfE);
        }
        else
        {
            /* z < x */
            pstRtNode->vLfE.push_back(*itLfE);
            pstRtNode->vRtE.push_back(iEid);
        }
    }
    DEBUG_ASSERT(itRtE == pstENode->vRtE.end());

    return iEid;
}
#if 0
/*****************
input:
        int iEid
        int iLfEid
        int iRtEid
description:
        add new edge
        init stage
******************/
int myG::addLsNeib(int iEid, int iLfEid, int iRtEid)
{
    TPST_MAP_BY_EID *pstNode = NULL;
    //TPST_MAP_BY_EID *pstLfNode = NULL;
    //TPST_MAP_BY_EID *pstRtNode = NULL;
    TPST_LS_BY_EID stlsNode = {0};
    //TPST_LS_BY_EID stlsLfNode = {0};
    //TPST_LS_BY_EID stlsRtNode = {0};
    list<TPST_LS_BY_EID>::iterator pstls;
    //list<TPST_LS_BY_EID>::iterator pstlsLf;
    //list<TPST_LS_BY_EID>::iterator pstlsRt;

    pstNode = findNode(iEid);
    DEBUG_ASSERT(NULL != pstNode);
    /*pstLfNode = findNode(iLfEid);
    DEBUG_ASSERT(NULL != pstLfNode);
    pstRtNode = findNode(iRtEid);
    DEBUG_ASSERT(NULL != pstRtNode);*/

    pstNode->lsLcNebE.push_front(stlsNode);
    pstls = pstNode->lsLcNebE.begin();
    /*pstNode->lsLcNebE.erase(pstls);
    pstNode->lsLcNebE.emplace_front();
    pstls = pstNode->lsLcNebE.begin();*/
    /*pstLfNode->lsLcNebE.push_front(stlsLfNode);
    pstlsLf = pstLfNode->lsLcNebE.begin();
    pstRtNode->lsLcNebE.push_front(stlsRtNode);
    pstlsRt = pstRtNode->lsLcNebE.begin();*/

    pstls->iLfEid = iLfEid;
    pstls->iRtEid = iRtEid;
    //pstls->pstLfN = pstlsLf;
    //pstls->pstRtN = pstlsRt;

    /*pstlsLf->iLfEid = iRtEid;
    pstlsLf->iRtEid = iEid;
    pstlsLf->pstLfN = pstlsRt;
    pstlsLf->pstRtN = pstls;

    pstlsRt->iLfEid = iEid;
    pstlsRt->iRtEid = iLfEid;
    pstlsRt->pstLfN = pstls;
    pstlsRt->pstRtN = pstlsLf;*/

    return 0;
}
/*****************
input:
        int iEid
        TPST_LS_BY_EID *pstls
description:
        add new edge
        init stage
******************/
int myG::rmLsNeib(int iEid, list<TPST_LS_BY_EID>::iterator pstls)
{
    TPST_MAP_BY_EID *pstNode = NULL;
    TPST_MAP_BY_EID *pstLfNode = NULL;
    TPST_MAP_BY_EID *pstRtNode = NULL;

    DEBUG_ASSERT(0);

    pstNode = findNode(iEid);
    DEBUG_ASSERT(NULL != pstNode);
    /*pstLfNode = findNode(pstls->iLfEid);
    DEBUG_ASSERT(NULL != pstLfNode);
    pstRtNode = findNode(pstls->iRtEid);
    DEBUG_ASSERT(NULL != pstRtNode);

    pstLfNode->lsLcNebE.erase(pstls->pstLfN);
    pstRtNode->lsLcNebE.erase(pstls->pstRtN);*/

    pstNode->lsLcNebE.erase(pstls);

    return 0;
}
/*****************
input:
        int iEid
description:
        add new edge
        init stage
******************/
int myG::rmLsNeib(int iEid)
{
    TPST_MAP_BY_EID *pstNode = NULL;
    list <TPST_LS_BY_EID>::iterator itLcE;

    pstNode = findNode(iEid);
    DEBUG_ASSERT(NULL != pstNode);

    itLcE = pstNode->lsLcNebE.begin();
    while (itLcE != pstNode->lsLcNebE.end())
    {
        rmLsNeib(iEid, itLcE++);
    }

    return 0;
}
#endif
/*****************
input:
        none
description:
        init object
        init stage
******************/
myG::myG()
{
    TPST_MAP_BY_EID stTp = {0};

    m_mpBasicG.clear();
    /*m_vG.clear();
    m_vPNeCnt.clear();*/

    m_pvPNeCnt = new vector<int>();
    m_pvG = new vector<TPST_MAP_BY_EID>();

    m_iMaxPId = 0;
    m_iMaxEId = 0;

    m_pvG->push_back(stTp);
}
/*****************
input:
        myG &oInitG
description:
        copy map
        calculate stage
******************/
myG::~myG()
{
    m_mpBasicG.clear();

    m_iMaxPId = 0;
    m_iMaxEId = 0;

    m_pvPNeCnt->clear();

    m_pvG->clear();

    delete m_pvPNeCnt;
    delete m_pvG;
}

/*****************
input:
        int x
        int y
description:
        delete edge
        calculate stage
******************/
bool myG::simpleRm(int x, int y)
{
    MAP_BASIC_G::iterator itGNode;
    TPST_MAP_BY_EID stTp = {0};
    int iPrevMaxPid = 0;
    int iEid = 0;

    itGNode = m_mpBasicG.find(pair<int, int>(x, y));
    if (itGNode == m_mpBasicG.end())
    {
        /* have removed, ignore */
        printf("DEBUG (%d, %d) has been removed\n", x, y);
        return false;
    }
    iEid = itGNode->second;
    m_mpBasicG.erase(pair<int, int>(x, y));
    m_mpBasicG.erase(pair<int, int>(y, x));
    (*m_pvPNeCnt)[x]--;
    (*m_pvPNeCnt)[y]--;

    return true;
}
#if 0
/*****************
input:
        int iEid
description:
        update jusup
******************/
int myG::updateJuSup(int iEid)
{
    TPST_MAP_BY_EID* pstENode = NULL;
    TPST_MAP_BY_EID* pstLfNode = NULL;
    TPST_MAP_BY_EID* pstRtNode = NULL;
    vector <int>::iterator itLfE;
    vector <int>::iterator itRtE;
    int iJuSup = 0;
    int iMinT = 0;

    pstENode = findNode(iEid);
    debug_assert(NULL != pstENode);

    itLfE = pstENode->vLfE.begin();
    itRtE = pstENode->vRtE.begin();
    for (; itLfE != pstENode->vLfE.end(); ++itLfE, ++itRtE)
    {
        pstLfNode = findNode(*itLfE);
        debug_assert(NULL != pstLfNode);
        pstRtNode = findNode(*itRtE);
        debug_assert(NULL != pstRtNode);

        iMinT = COMMON_MIN(pstLfNode->iTrussness, pstRtNode->iTrussness);
        if (pstENode->iTrussness <= iMinT)
        {
            ++iJuSup;
        }
    }
    DEBUG_ASSERT(itRtE == pstENode->vRtE.end());

    pstENode->iJuSup = iJuSup;

    return 0;
}

/*****************
input:
        int iEid
description:
        update SeSup
******************/
int myG::updateSeSup(int iEid)
{
    TPST_MAP_BY_EID* pstENode = NULL;
    TPST_MAP_BY_EID* pstLfNode = NULL;
    TPST_MAP_BY_EID* pstRtNode = NULL;
    vector <int>::iterator itLfE;
    vector <int>::iterator itRtE;
    int iSeSup = 0;
    int iMinT = 0;
    int iMinLayer = 0;

    pstENode = findNode(iEid);
    debug_assert(NULL != pstENode);

    itLfE = pstENode->vLfE.begin();
    itRtE = pstENode->vRtE.begin();
    for (; itLfE != pstENode->vLfE.end(); ++itLfE, ++itRtE)
    {
        pstLfNode = findNode(*itLfE);
        debug_assert(NULL != pstLfNode);
        pstRtNode = findNode(*itRtE);
        debug_assert(NULL != pstRtNode);

        iMinT = COMMON_MIN(pstLfNode->iTrussness, pstRtNode->iTrussness);
        if (pstENode->iTrussness < iMinT)
        {
            ++iSeSup;
        }
        else if (pstENode->iTrussness == iMinT)
        {
            iMinLayer = pstENode->iLayer;
            if (pstENode->iTrussness == pstLfNode->iTrussness)
            {
                iMinLayer = COMMON_MIN(iMinLayer, pstLfNode->iLayer);
            }
            if (pstENode->iTrussness == pstRtNode->iTrussness)
            {
                iMinLayer = COMMON_MIN(iMinLayer, pstRtNode->iLayer);
            }
            if (pstENode->iLayer <= iMinLayer)
            {
                ++iSeSup;
            }
        }
    }
    DEBUG_ASSERT(itRtE == pstENode->vRtE.end());

    pstENode->iSeSup = iSeSup;

    return 0;
}

/*****************
input:
        int iEid
description:
        update SeSup
******************/
int myG::checkSeSup(int iEid)
{
    TPST_MAP_BY_EID* pstNode = NULL;
    TPST_MAP_BY_EID* pstLfNode = NULL;
    TPST_MAP_BY_EID* pstRtNode = NULL;
    vector <int>::iterator itLfE;
    vector <int>::iterator itRtE;
    int iSeSup = 0;
    int iMinT = 0;
    int iMinLayer = 0;
    bool bFlag = false;

    pstNode = findNode(iEid);
    debug_assert(NULL != pstNode);

    itLfE = pstNode->vLfE.begin();
    itRtE = pstNode->vRtE.begin();
    for (; itLfE != pstNode->vLfE.end(); ++itLfE, ++itRtE)
    {
        pstLfNode = findNode(*itLfE);
        debug_assert(NULL != pstLfNode);
        pstRtNode = findNode(*itRtE);
        debug_assert(NULL != pstRtNode);

        iMinT = COMMON_MIN(pstLfNode->iTrussness, pstRtNode->iTrussness);
        if (pstNode->iTrussness < iMinT)
        {
            ++iSeSup;
        }
        else if (pstNode->iTrussness == iMinT)
        {
            if (pstNode->bLock && (!pstNode->bRmFlag))
            {
                if ((pstLfNode->bLock || pstLfNode->iTrussness > iMinT) &&
                    (pstRtNode->bLock || pstRtNode->iTrussness > iMinT))
                {
                    ++iSeSup;
                }
            }
            else
            {
                iMinLayer = COMMON_TRG_MIN(pstNode->iLayer,
                                pstLfNode->iLayer,
                                (pstLfNode->bLock || pstLfNode->iTrussness > iMinT),
                                pstRtNode->iLayer,
                                (pstRtNode->bLock || pstRtNode->iTrussness > iMinT));
                if (pstNode->iLayer <= iMinLayer)
                {
                    ++iSeSup;
                }
            }
        }
    }
    DEBUG_ASSERT(itRtE == pstNode->vRtE.end());

    if (pstNode->iSeSup != iSeSup)
    {
        printf("CHECK (%d, %d) k: %d layer: %d seSup: %d %d flag: %d han: %d rm: %d ins: %d\n",
                     pstNode->paXY.first, pstNode->paXY.second,
                     pstNode->iTrussness, pstNode->iLayer,
                     pstNode->iSeSup, iSeSup, pstNode->bLock,
                     pstNode->bHandledFlag, pstNode->bRmFlag, pstNode->bInsLFlag);
        itLfE = pstNode->vLfE.begin();
        itRtE = pstNode->vRtE.begin();
        for (; itLfE != pstNode->vLfE.end(); ++itLfE, ++itRtE)
        {
            pstLfNode = findNode(*itLfE);
            debug_assert(NULL != pstLfNode);
            pstRtNode = findNode(*itRtE);
            debug_assert(NULL != pstRtNode);

            bFlag = false;

            iMinT = COMMON_MIN(pstLfNode->iTrussness, pstRtNode->iTrussness);
            if (pstNode->iTrussness == iMinT)
            {
                if (pstNode->bLock && (!pstNode->bRmFlag))
                {
                    if ((pstLfNode->bLock || pstLfNode->iTrussness > iMinT) &&
                        (pstRtNode->bLock || pstRtNode->iTrussness > iMinT))
                    {
                        DEBUG_PRINTF("ERROR 1: (%d, %d) (%d, %d) k: %d %d layer: %d %d seSup: %d %d lock: %d %d self: (%d, %d) layer: %d flag: %d\n",
                                     pstLfNode->paXY.first, pstLfNode->paXY.second,
                                     pstRtNode->paXY.first, pstRtNode->paXY.second,
                                     pstLfNode->iTrussness, pstRtNode->iTrussness,
                                     pstLfNode->iLayer, pstRtNode->iLayer,
                                     pstLfNode->iSeSup, pstRtNode->iSeSup,
                                     pstLfNode->bLock, pstRtNode->bLock,
                                     pstNode->paXY.first, pstNode->paXY.second,
                                     pstNode->iLayer, pstNode->bLock);
                        bFlag = true;
                    }
                }
                else
                {
                    iMinLayer = COMMON_TRG_MIN(pstNode->iLayer,
                                    pstLfNode->iLayer,
                                    (pstLfNode->bLock || pstLfNode->iTrussness > iMinT),
                                    pstRtNode->iLayer,
                                    (pstRtNode->bLock || pstRtNode->iTrussness > iMinT));
                    if (pstNode->iLayer <= iMinLayer)
                    {
                        DEBUG_PRINTF("ERROR 2: (%d, %d) (%d, %d) k: %d %d layer: %d %d seSup: %d %d lock: %d %d self: (%d, %d) layer: %d flag: %d\n",
                                     pstLfNode->paXY.first, pstLfNode->paXY.second,
                                     pstRtNode->paXY.first, pstRtNode->paXY.second,
                                     pstLfNode->iTrussness, pstRtNode->iTrussness,
                                     pstLfNode->iLayer, pstRtNode->iLayer,
                                     pstLfNode->iSeSup, pstRtNode->iSeSup,
                                     pstLfNode->bLock, pstRtNode->bLock,
                                     pstNode->paXY.first, pstNode->paXY.second,
                                     pstNode->iLayer, pstNode->bLock);
                        bFlag = true;
                    }
                }
            }
            if (!bFlag)
            {
                DEBUG_PRINTF("ERROR 3: (%d, %d) (%d, %d) k: %d %d layer: %d %d seSup: %d %d lock: %d %d self: (%d, %d) layer: %d flag: %d\n",
                             pstLfNode->paXY.first, pstLfNode->paXY.second,
                             pstRtNode->paXY.first, pstRtNode->paXY.second,
                             pstLfNode->iTrussness, pstRtNode->iTrussness,
                             pstLfNode->iLayer, pstRtNode->iLayer,
                             pstLfNode->iSeSup, pstRtNode->iSeSup,
                             pstLfNode->bLock, pstRtNode->bLock,
                             pstNode->paXY.first, pstNode->paXY.second,
                             pstNode->iLayer, pstNode->bLock);
            }
        }
        DEBUG_ASSERT(0);
    }

    return 0;
}
/*****************
input:
        int iEid
description:
        show SeSup
******************/
int myG::showSeSup(int iEid)
{
    TPST_MAP_BY_EID* pstENode = NULL;
    TPST_MAP_BY_EID* pstLfNode = NULL;
    TPST_MAP_BY_EID* pstRtNode = NULL;
    vector <int>::iterator itLfE;
    vector <int>::iterator itRtE;
    int iSeSup = 0;
    int iMinT = 0;
    int iMinLayer = 0;

    pstENode = findNode(iEid);
    debug_assert(NULL != pstENode);

    itLfE = pstENode->vLfE.begin();
    itRtE = pstENode->vRtE.begin();
    for (; itLfE != pstENode->vLfE.end(); ++itLfE, ++itRtE)
    {
        pstLfNode = findNode(*itLfE);
        debug_assert(NULL != pstLfNode);
        pstRtNode = findNode(*itRtE);
        debug_assert(NULL != pstRtNode);

        iMinT = COMMON_MIN(pstLfNode->iTrussness, pstRtNode->iTrussness);
        if (pstENode->iTrussness < iMinT)
        {
            ++iSeSup;
            printf("SHOW k: %d (%d, %d) (%d, %d)\n", iMinT,
                   pstLfNode->paXY.first, pstLfNode->paXY.second,
                   pstRtNode->paXY.first, pstRtNode->paXY.second);
        }
        else if (pstENode->iTrussness == iMinT)
        {
            iMinLayer = pstENode->iLayer;
            if (pstENode->iTrussness == pstLfNode->iTrussness)
            {
                iMinLayer = COMMON_MIN(iMinLayer, pstLfNode->iLayer);
            }
            if (pstENode->iTrussness == pstRtNode->iTrussness)
            {
                iMinLayer = COMMON_MIN(iMinLayer, pstRtNode->iLayer);
            }
            if (pstENode->iLayer <= iMinLayer)
            {
                ++iSeSup;
                printf("SHOW k: %d (%d, %d) (%d, %d)\n", iMinT,
                       pstLfNode->paXY.first, pstLfNode->paXY.second,
                       pstRtNode->paXY.first, pstRtNode->paXY.second);
            }
        }
    }
    DEBUG_ASSERT(itRtE == pstENode->vRtE.end());

    pstENode->iSeSup = iSeSup;

    return 0;
}
#endif
/*****************
input:
        int x
        int y
        list<int> &lstNeib
description:
        delete edge
        calculate stage
******************/
int myG::findNeb(int x, int y, vector<int> &vNeib)
{
    MAP_BASIC_G::iterator itLowB;
    MAP_BASIC_G::iterator itHighB;
    int iNebCnt = 0;
    /* nei(u) < nei(v) */
    int u = 0;
    int v = 0;
    int w = 0;

    if ((*m_pvPNeCnt)[x] < (*m_pvPNeCnt)[y])
    {
        u = x;
        v = y;
    }
    else
    {
        u = y;
        v = x;
    }

    debug_assert(vNeib.empty());
    //printf("MYG get (%d, %d) u: %d\n", x, y, u);
    /* find u's neighbor , note u > 0 */
    itLowB = m_mpBasicG.upper_bound(pair<int, int>(u, 0));
    itHighB = m_mpBasicG.upper_bound(pair<int, int>(u + 1, 0));
    while (itLowB != itHighB)
    {

        w = itLowB->first.second;
        //printf("MYG w: %d\n", w);
        if (m_mpBasicG.find(pair<int, int>(v, w)) != m_mpBasicG.end())
        {
            vNeib.push_back(w);
            iNebCnt++;
        }
        itLowB++;
    }
    return iNebCnt;
}

/*****************
input:
        int x
        int y
        list<int> &lstNeib
description:
        delete edge
        calculate stage
******************/
int myG::findNeb(vector<int> &vLfE, vector<int> &vRtE, int iCurK, vector<pair<int, int> > &vNeib)
{
    vector<int>::iterator itLfE;
    vector<int>::iterator itRtE;
    DEBUG_ASSERT(vNeib.empty());
    itLfE = vLfE.begin();
    itRtE = vRtE.begin();
    for (; itLfE != vLfE.end(); ++itLfE, ++itRtE)
    {
        TPST_MAP_BY_EID* pstLf = findNode(*itLfE);
        DEBUG_ASSERT(NULL != pstLf);
        TPST_MAP_BY_EID* pstRt = findNode(*itRtE);
        DEBUG_ASSERT(NULL != pstRt);
        if ((pstLf->iTrussness >= iCurK) &&
            (pstRt->iTrussness >= iCurK))
        {
            vNeib.push_back(pair<int, int>(pstLf->eid, pstRt->eid));
        }
    }
    return vNeib.size();
}
/*****************
input:
        int x
        int y
        list<int> &lstNeib
description:
        delete edge
        calculate stage
******************/
int myG::cntHighNeib(vector<int> &vLfE, vector<int> &vRtE, int iGeqK)
{
    vector<int>::iterator itLfE;
    vector<int>::iterator itRtE;
    int iHighCnt = 0;
    itLfE = vLfE.begin();
    itRtE = vRtE.begin();
    for (; itLfE != vLfE.end(); ++itLfE, ++itRtE)
    {
        TPST_MAP_BY_EID* pstLf = findNode(*itLfE);
        DEBUG_ASSERT(NULL != pstLf);
        TPST_MAP_BY_EID* pstRt = findNode(*itRtE);
        DEBUG_ASSERT(NULL != pstRt);
        if ((iGeqK > pstLf->iTrussness) ||
            (iGeqK > pstRt->iTrussness))
        {
            continue;
        }
        ++iHighCnt;
    }
    return iHighCnt;
}
/*****************
input:
        int x
        int y
        vector<int> &vLfE
        vector<int> &vRtE
description:
        find neighbor eid
******************/
int myG::findNeb(int x, int y, vector<int> &vLfE, vector<int> &vRtE)
{
    MAP_BASIC_G::iterator itLowB;
    MAP_BASIC_G::iterator itHighB;
    int iNebCnt = 0;
    /* nei(u) < nei(v) */
    int u = 0;
    int v = 0;
    int w = 0;

    vector<int> * pvLfE = &vLfE;
    vector<int> * pvRtE = &vRtE;

    DEBUG_ASSERT(x < y);
    DEBUG_ASSERT(vLfE.empty());
    DEBUG_ASSERT(vRtE.empty());

    if ((*m_pvPNeCnt)[x] < (*m_pvPNeCnt)[y])
    {
        u = x;
        v = y;
    }
    else
    {
        u = y;
        v = x;
        pvLfE = &vRtE;
        pvRtE = &vLfE;
    }

    /* find u's neighbor , note u > 0 */
    itLowB = m_mpBasicG.upper_bound(pair<int, int>(u, 0));
    itHighB = m_mpBasicG.upper_bound(pair<int, int>(u + 1, 0));
    while (itLowB != itHighB)
    {
        w = itLowB->first.second;
        MAP_BASIC_G::iterator itDesE = m_mpBasicG.find(pair<int, int>(v, w));
        if (itDesE != m_mpBasicG.end())
        {
            pvLfE->push_back(itLowB->second);
            pvRtE->push_back(itDesE->second);
            iNebCnt++;
        }
        itLowB++;
    }
    return iNebCnt;
}
/*****************
input:
        int x
        int y
description:
        find neighbor eid
******************/
int myG::getLowB(int x, int y)
{
    vector<int> vLfE;
    vector<int> vRtE;
    vector <int>::iterator itLfE;
    vector <int>::iterator itRtE;
    TPST_MAP_BY_EID *pstLfNode = NULL;
    TPST_MAP_BY_EID *pstRtNode = NULL;
    int iLowB = 2;
    int iMinT = 0;
    int iVSize = 0;
    int iSup = 0;
    vector <int> vTCnt;

    findNeb(x, y, vLfE, vRtE);
    itLfE = vLfE.begin();
    itRtE = vRtE.begin();
    for (; itLfE != vLfE.end(); ++itLfE, ++itRtE)
    {
        pstLfNode = findNode(*itLfE);
        DEBUG_ASSERT(NULL != pstLfNode);
        pstRtNode = findNode(*itRtE);
        DEBUG_ASSERT(NULL != pstRtNode);

        iMinT = COMMON_MIN(pstLfNode->iTrussness, pstRtNode->iTrussness);
        while (iMinT >= iVSize)
        {
            ++iVSize;
            vTCnt.push_back(0);
        }
        vTCnt[iMinT]++;
        ++iSup;
    }

    for (iLowB = 2; iLowB < iVSize; ++iLowB)
    {
        if (iSup + 2 < iLowB)
        {
            break;
        }
        iSup -= vTCnt[iLowB];
    }
    return iLowB - 1;
}
/*****************
input:
        int x
        int y
description:
        find neighbor eid
******************/
int myG::getUpB(int x, int y)
{
    vector<int> vLfE;
    vector<int> vRtE;
    vector <int>::iterator itLfE;
    vector <int>::iterator itRtE;
    TPST_MAP_BY_EID *pstLfNode = NULL;
    TPST_MAP_BY_EID *pstRtNode = NULL;
    int iUpB = 2;
    int iMinT = 0;
    int iVSize = 0;
    int iSup = 0;
    vector <int> vTCnt;

    findNeb(x, y, vLfE, vRtE);
    itLfE = vLfE.begin();
    itRtE = vRtE.begin();
    for (; itLfE != vLfE.end(); ++itLfE, ++itRtE)
    {
        pstLfNode = findNode(*itLfE);
        DEBUG_ASSERT(NULL != pstLfNode);
        pstRtNode = findNode(*itRtE);
        DEBUG_ASSERT(NULL != pstRtNode);

        iMinT = COMMON_MIN(pstLfNode->iTrussness, pstRtNode->iTrussness);
        while (iMinT >= iVSize)
        {
            ++iVSize;
            vTCnt.push_back(0);
        }
        vTCnt[iMinT]++;
        ++iSup;
    }

    for (iUpB = 2; iUpB < iVSize; ++iUpB)
    {
        if (iSup + 2 <= iUpB + 1)
        {
            break;
        }
        iSup -= vTCnt[iUpB];
    }
    return iUpB + 1;
}

/*****************
input:
        int x
        int y
description:
        find neighbor eid
******************/
bool myG::checkIns(int x, int y, int iDesK, int iLayer)
{
    vector<int> vLfE;
    vector<int> vRtE;
    vector <int>::iterator itLfE;
    vector <int>::iterator itRtE;
    TPST_MAP_BY_EID *pstLfNode = NULL;
    TPST_MAP_BY_EID *pstRtNode = NULL;
    int iUpB = 2;
    int iMinT = 0;
    int iVSize = 0;
    int iSup = 0;
    int iMinLayer = 0;

    findNeb(x, y, vLfE, vRtE);
    itLfE = vLfE.begin();
    itRtE = vRtE.begin();
    for (; itLfE != vLfE.end(); ++itLfE, ++itRtE)
    {
        pstLfNode = findNode(*itLfE);
        DEBUG_ASSERT(NULL != pstLfNode);
        pstRtNode = findNode(*itRtE);
        DEBUG_ASSERT(NULL != pstRtNode);

        iMinT = COMMON_MIN(pstLfNode->iTrussness, pstRtNode->iTrussness);

        if (iMinT > iDesK)
        {
            ++iSup;
        }
        else if (iMinT == iDesK)
        {
            if (iDesK < pstLfNode->iTrussness)
            {
                iMinLayer = pstRtNode->iLayer;
            }
            else if (iDesK < pstRtNode->iTrussness)
            {
                iMinLayer = pstLfNode->iLayer;
            }
            else
            {
                iMinLayer = COMMON_MIN(pstLfNode->iLayer, pstRtNode->iLayer);
            }
            if (iLayer <= iMinLayer)
            {
                ++iSup;
            }
        }
    }

    return (iSup + 2 > iDesK);
}
/*****************
input:
        int x
        int y
description:
        find neighbor eid
******************/
bool myG::checkInsFourTruss(int x, int y, int iUndE)
{
    vector<int> vNeib;
    vector<int> vUndNeib;
    map <int, int> mpN;
    TPST_MAP_BY_EID *pstUndNode = NULL;
    bool res = false;

    findNeb(x, y, vNeib);

    pstUndNode = findNode(iUndE);
    DEBUG_ASSERT(NULL != pstUndNode);

    findNeb(pstUndNode->paXY.first, pstUndNode->paXY.second, vUndNeib);

    for (int iNode : vUndNeib)
    {
        mpN[iNode] = 1;
    }
    for (int iNode : vNeib)
    {
        if (mpN.find(iNode) != mpN.end())
        {
            res = true;
            break;
        }
    }

    return res;
}
/*****************
input:
        int x
        int y
description:
        find neighbor eid
******************/
bool myG::checkIns(int x, int y, int iUndE, int iDesK, int iLayer, int iGroup)
{
    vector<int> vLfE;
    vector<int> vRtE;
    vector <int>::iterator itLfE;
    vector <int>::iterator itRtE;
    TPST_MAP_BY_EID *pstLfNode = NULL;
    TPST_MAP_BY_EID *pstRtNode = NULL;
    TPST_MAP_BY_EID *pstUndNode = NULL;
    int iUpB = 2;
    int iMinT = 0;
    int iVSize = 0;
    int iSup = 0;
    int iMinLayer = 0;

    if (3 == iDesK)
    {
        return checkInsFourTruss(x, y, iUndE);
    }
    else if (3 > iDesK)
    {
        return true;
    }

    /* check undE */
    pstUndNode = findNode(iUndE);
    DEBUG_ASSERT(NULL != pstUndNode);

    if (pstUndNode->iTrussness <= iDesK)
    {
        if (pstUndNode->iTrussness < iDesK)
        {
            return false;
        }

        if (pstUndNode->iLayer < iLayer)
        {
            return false;
        }
    }

    findNeb(x, y, vLfE, vRtE);

    /*DEBUG_PRINTF("CHECK candidate (%d, %d) k: %d group: %d\n",
                 x, y, iDesK, iGroup);*/

    itLfE = vLfE.begin();
    itRtE = vRtE.begin();
    for (; itLfE != vLfE.end(); ++itLfE, ++itRtE)
    {
        pstLfNode = findNode(*itLfE);
        DEBUG_ASSERT(NULL != pstLfNode);
        pstRtNode = findNode(*itRtE);
        DEBUG_ASSERT(NULL != pstRtNode);

        /*DEBUG_PRINTF("CHECK (%d, %d) left (%d, %d) eid: %d k: %d layer: %d group: %d\n",
                     x, y,
                     pstLfNode->paXY.first, pstLfNode->paXY.second,
                     pstLfNode->eid, pstLfNode->iTrussness, pstLfNode->iLayer, pstLfNode->iGroupId);
        DEBUG_PRINTF("CHECK (%d, %d) right (%d, %d) eid: %d k: %d layer: %d group: %d\n",
                     x, y,
                     pstRtNode->paXY.first, pstRtNode->paXY.second,
                     pstRtNode->eid, pstRtNode->iTrussness, pstRtNode->iLayer, pstRtNode->iGroupId);*/

        iMinT = COMMON_MIN(pstLfNode->iTrussness, pstRtNode->iTrussness);

        if (iMinT > iDesK)
        {
            ++iSup;
        }
        else if (iMinT == iDesK)
        {
            if (iDesK < pstLfNode->iTrussness)
            {
                iMinLayer = pstRtNode->iLayer;
            }
            else if (iDesK < pstRtNode->iTrussness)
            {
                iMinLayer = pstLfNode->iLayer;
            }
            else
            {
                iMinLayer = COMMON_MIN(pstLfNode->iLayer, pstRtNode->iLayer);
            }
            if (iMinLayer < iLayer)
            {
                continue;
            }

            ++iSup;
            /*DEBUG_PRINTF("CHECK (%d, %d) success left (%d, %d) eid: %d k: %d layer: %d group: %d\n",
                         x, y,
                         pstLfNode->paXY.first, pstLfNode->paXY.second,
                         pstLfNode->eid, pstLfNode->iTrussness, pstLfNode->iLayer,
                         pstLfNode->iGroupId);
            DEBUG_PRINTF("CHECK (%d, %d) success right (%d, %d) eid: %d k: %d layer: %d group: %d\n",
                         x, y,
                         pstRtNode->paXY.first, pstRtNode->paXY.second,
                         pstRtNode->eid, pstRtNode->iTrussness, pstRtNode->iLayer,
                         pstRtNode->iGroupId);*/
        }
    }

    if (iSup + 2 <= iDesK)
    {
        return false;
    }

    //DEBUG_PRINTF("CHECK candidate (%d, %d) sup: %d\n", x, y, iSup);
    return (iSup + 2 > iDesK);
}
/*****************
input:
        int x
        int y
        vector<int> &vLfE
        vector<int> &vRtE
        MAP_BASIC_G &mapCalG
description:
        find neighbor eid
******************/
int myG::findNeb(int x, int y, vector<int> &vLfE, vector<int> &vRtE, MAP_BASIC_G &mapCalG)
{
    MAP_BASIC_G::iterator itLowB;
    MAP_BASIC_G::iterator itHighB;
    int iNebCnt = 0;
    /* nei(u) < nei(v) */
    int u = 0;
    int v = 0;
    int w = 0;

    vector<int> * pvLfE = &vLfE;
    vector<int> * pvRtE = &vRtE;

    DEBUG_ASSERT(x < y);
    DEBUG_ASSERT(vLfE.empty());
    DEBUG_ASSERT(vRtE.empty());

    if ((*m_pvPNeCnt)[x] < (*m_pvPNeCnt)[y])
    {
        u = x;
        v = y;
    }
    else
    {
        u = y;
        v = x;
        pvLfE = &vRtE;
        pvRtE = &vLfE;
    }

    /* find u's neighbor , note u > 0 */
    itLowB = mapCalG.upper_bound(pair<int, int>(u, 0));
    itHighB = mapCalG.upper_bound(pair<int, int>(u + 1, 0));
    while (itLowB != itHighB)
    {
        w = itLowB->first.second;
        MAP_BASIC_G::iterator itDesE = mapCalG.find(pair<int, int>(v, w));
        if (itDesE != mapCalG.end())
        {
            pvLfE->push_back(itLowB->second);
            pvRtE->push_back(itDesE->second);
            iNebCnt++;
        }
        itLowB++;
    }
    return iNebCnt;
}
/*****************
input:
        int x
        int y
        list<pair<int, int> > &lspaNeibE
description:
        delete edge
        calculate stage
******************/
int myG::findNebE(int x, int y, list<pair<int, int> > &lspaNeibE)
{
    MAP_BASIC_G::iterator itLowB;
    MAP_BASIC_G::iterator itHighB;
    MAP_BASIC_G::iterator itTrdN;
    int iNebCnt = 0;
    /* nei(u) < nei(v) */
    int u = 0;
    int v = 0;
    int w = 0;

    if ((*m_pvPNeCnt)[x] < (*m_pvPNeCnt)[y])
    {
        u = x;
        v = y;
    }
    else
    {
        u = y;
        v = x;
    }

    debug_assert(lspaNeibE.empty());
    /* find u's neighbor , note u > 0 */
    itLowB = m_mpBasicG.upper_bound(pair<int, int>(u, 0));
    itHighB = m_mpBasicG.upper_bound(pair<int, int>(u + 1, 0));
    while (itLowB != itHighB)
    {

        w = itLowB->first.second;
        //printf("MYG w: %d\n", w);
        itTrdN = m_mpBasicG.find(pair<int, int>(v, w));
        if (itTrdN != m_mpBasicG.end())
        {
            lspaNeibE.push_back(pair<int, int>(itLowB->second, itTrdN->second));
            iNebCnt++;
        }
        itLowB++;
    }
    return iNebCnt;
}

/*****************
input:
        int x
        int y
        list<int> &lstNeibE
        MAP_BASIC_G &mapCalG
        MAP_BASIC_G &mapBlackG
        int iTrussness
description:
        delete edge
        calculate stage
******************/
int myG::findNebE(int x, int y, list<int> &lstNeibE, MAP_BASIC_G &mapCalG, MAP_BASIC_G &mapBlackG, int iTrussness)
{
    MAP_BASIC_G::iterator itLowB;
    MAP_BASIC_G::iterator itHighB;
    int iNebCnt = 0;
    /* nei(u) < nei(v) */
    int u = 0;
    int v = 0;
    int w = 0;

    if ((*m_pvPNeCnt)[x] < (*m_pvPNeCnt)[y])
    {
        u = x;
        v = y;
    }
    else
    {
        u = y;
        v = x;
    }

    debug_assert(lstNeibE.empty());
    /* find u's neighbor , note u > 0 */
    itLowB = m_mpBasicG.upper_bound(pair<int, int>(u, 0));
    itHighB = m_mpBasicG.upper_bound(pair<int, int>(u + 1, 0));
    while (itLowB != itHighB)
    {
        w = itLowB->first.second;
        if (mapBlackG.find(itLowB->first) != mapBlackG.end())
        {
            /* in black, ignore */
            itLowB++;
            continue;
        }
        if (m_mpBasicG.find(pair<int, int>(v, w)) != m_mpBasicG.end())
        {
            TPST_MAP_BY_EID* pstENode = NULL;
            TPST_MAP_BY_EID* pstENodeO = NULL;
            int iMinTruss = 0;

            pstENode = this->findNode(u, w);
            debug_assert(NULL != pstENode);
            pstENodeO = this->findNode(v, w);
            debug_assert(NULL != pstENodeO);

            iMinTruss = pstENode->iTrussness<pstENodeO->iTrussness?pstENode->iTrussness:pstENodeO->iTrussness;

            if (iMinTruss != iTrussness)
            {
                /* not in same truss, ignore */
                itLowB++;
                continue;
            }
            if (mapBlackG.find(pair<int, int>(v, w)) != mapBlackG.end())
            {
                /* in black, ignore */
                itLowB++;
                continue;
            }
            /* exists in G, check in calG */
            if (mapCalG.find(pair<int, int>(v, w)) != mapCalG.end())
            {
                lstNeibE.push_back(m_mpBasicG[pair<int, int>(v, w)]);
                iNebCnt++;
            }
            if (mapCalG.find(pair<int, int>(u, w)) != mapCalG.end())
            {
                lstNeibE.push_back(m_mpBasicG[pair<int, int>(u, w)]);
                iNebCnt++;
            }
        }
        itLowB++;
    }

    return iNebCnt;
}

/*****************
input:
        int iNode
        list<int> &lsNeibP
        list<int> &liThirdE
        int iTrussness
description:
        delete edge
        calculate stage
******************/
int myG::findThirdE(int iNode, list<int> &lsThirdE)
{
    MAP_BASIC_G::iterator itLowB;
    MAP_BASIC_G::iterator itHighB;
    int iCnt = 0;
    /* nei(u) < nei(v) */
    int u = 0;
    int v = 0;
    int w = 0;

    u = iNode;

    /* find u's neighbor , note u > 0 */
    itLowB = m_mpBasicG.upper_bound(pair<int, int>(u, 0));
    itHighB = m_mpBasicG.upper_bound(pair<int, int>(u + 1, 0));
    while (itLowB != itHighB)
    {
        /* x: min degree */
        int x = 0;
        int y = 0;
        bool bChgFlag = false;
        MAP_BASIC_G::iterator itLcLowB;
        MAP_BASIC_G::iterator itLcHighB;
        MAP_BASIC_G::iterator itDesE;
        v = itLowB->first.second;

        x = u;
        y = v;

        if ((*m_pvPNeCnt)[x] > (*m_pvPNeCnt)[y])
        {
            x = v;
            y = u;
            bChgFlag = true;
        }
        itLcLowB = m_mpBasicG.upper_bound(pair<int, int>(x, 0));
        itLcHighB = m_mpBasicG.upper_bound(pair<int, int>(x + 1, 0));
        while (itLcLowB != itLcHighB)
        {

            w = itLcLowB->first.second;
            if (v <= w)
            {
                /* ignore half */
                itLcLowB++;
                continue;
            }
            itDesE = m_mpBasicG.find(pair<int, int>(y, w));
            if (itDesE != m_mpBasicG.end())
            {
                int iEid = 0;
                if (bChgFlag)
                {
                    itDesE = itLcLowB;
                }
                /*printf("MYG (v, w): (%d, %d), (x, y): (%d, %d) bool %d\n",
                       v, w, x, y, bChgFlag);*/
                iEid = itDesE->second;
                lsThirdE.push_back(iEid);
                ++iCnt;
            }
            itLcLowB++;
        }
        ++itLowB;
    }

    return iCnt;
}

/*****************
input:
        int iNode
        list<int> &lstP
        list<int> &liThirdE
description:
        delete edge
        calculate stage
******************/
int myG::findPNebPTrdE(int iNodeId, list<int> &lstP, list<int> &lsThirdE)
{
    MAP_BASIC_G::iterator itLowB;
    MAP_BASIC_G::iterator itHighB;
    int iCnt = 0;
    /* nei(u) < nei(v) */
    int u = 0;
    int v = 0;
    /* eid, none */
    vector<int> * pvNebE = NULL;
    vector<int>::iterator itE;

    u = iNodeId;

    DEBUG_ASSERT(lstP.empty());
    DEBUG_ASSERT(lsThirdE.empty());
    /* find u's neighbor , note u > 0 */
    itLowB = m_mpBasicG.upper_bound(pair<int, int>(u, 0));
    itHighB = m_mpBasicG.upper_bound(pair<int, int>(u + 1, 0));
    while (itLowB != itHighB)
    {
        /* x: min degree */
        int iEid = 0;
        TPST_MAP_BY_EID* pstENode = NULL;
        v = itLowB->first.second;
        iEid = itLowB->second;
        lstP.push_back(v);
        pstENode = findNode(iEid);
        DEBUG_ASSERT(NULL != pstENode);

        if (u < v)
        {
            pvNebE = &(pstENode->vRtE);
        }
        else
        {
            pvNebE = &(pstENode->vLfE);
        }
        for (itE = pvNebE->begin(); itE != pvNebE->end(); ++itE)
        {
            pstENode = findNode(*itE);
            DEBUG_ASSERT(NULL != pstENode);
            if (v != pstENode->paXY.first)
            {
                /* ignore half */
                continue;
            }
            lsThirdE.push_back(*itE);
            ++iCnt;
        }
        ++itLowB;
    }

    return iCnt;
}
/*****************
input:
        int x
        int y
        list<int> &lstNeibE
        MAP_BASIC_G &mapCalG
description:
        find eid in calG, at least trussness
******************/
int myG::findNebE(int x, int y, list<int> &lstNeibE, MAP_BASIC_G &mapCalG, int iTrussness)
{
    MAP_BASIC_G::iterator itLowB;
    MAP_BASIC_G::iterator itHighB;
    MAP_BASIC_G::iterator itO;
    int iNebCnt = 0;
    /* nei(u) < nei(v) */
    int u = 0;
    int v = 0;
    int w = 0;

    if ((*m_pvPNeCnt)[x] < (*m_pvPNeCnt)[y])
    {
        u = x;
        v = y;
    }
    else
    {
        u = y;
        v = x;
    }

    debug_assert(lstNeibE.empty());
    /* find u's neighbor , note u > 0 */
    itLowB = m_mpBasicG.upper_bound(pair<int, int>(u, 0));
    itHighB = m_mpBasicG.upper_bound(pair<int, int>(u + 1, 0));
    while (itLowB != itHighB)
    {
        w = itLowB->first.second;
        itO = m_mpBasicG.find(pair<int, int>(v, w));
        if (itO != m_mpBasicG.end())
        {
            TPST_MAP_BY_EID* pstENode = NULL;
            TPST_MAP_BY_EID* pstENodeO = NULL;
            int iMinTruss = 0;
            pstENode = this->findNode(itLowB->second);
            debug_assert(NULL != pstENode);
            pstENodeO = this->findNode(itO->second);
            debug_assert(NULL != pstENodeO);

            iMinTruss = pstENode->iTrussness<pstENodeO->iTrussness?pstENode->iTrussness:pstENodeO->iTrussness;

            if (iMinTruss < iTrussness)
            {
                /* ignore */
                itLowB++;
                continue;
            }
            /* exists in G, check in calG */
            if (mapCalG.find(pair<int, int>(v, w)) != mapCalG.end())
            {
                lstNeibE.push_back(m_mpBasicG[pair<int, int>(v, w)]);
                iNebCnt++;
            }
            if (mapCalG.find(pair<int, int>(u, w)) != mapCalG.end())
            {
                lstNeibE.push_back(m_mpBasicG[pair<int, int>(u, w)]);
                iNebCnt++;
            }
        }
        itLowB++;
    }

    return iNebCnt;
}

/*****************
input:
        int iNodeId
        list<int> &lstP
        MAP_BASIC_G &mapCalG
description:
        find node's neighbor node
******************/
int myG::findPNebP(int iNodeId, list<int> &lstP, MAP_BASIC_G &mapCalG)
{
    MAP_BASIC_G::iterator itLowB;
    MAP_BASIC_G::iterator itHighB;
    int iCnt = 0;
    /* nei(u) < nei(v) */
    int u = 0;
    int w = 0;
    TPST_MAP_BY_EID* pstENode = NULL;

    u = iNodeId;

    debug_assert(lstP.empty());
    /* find u's neighbor , note u > 0 */
    itLowB = mapCalG.upper_bound(pair<int, int>(u, 0));
    itHighB = mapCalG.upper_bound(pair<int, int>(u + 1, 0));
    while (itLowB != itHighB)
    {
        w = itLowB->first.second;

        lstP.push_back(w);
        ++iCnt;

        ++itLowB;
    }

    return iCnt;
}
/*****************
input:
        int iNodeId
        vector <int> &vNodes
description:
        find eid
******************/
int myG::findPNebP(int iNodeId, vector <int> &vNodes)
{
    MAP_BASIC_G::iterator itLowB;
    MAP_BASIC_G::iterator itHighB;
    int iCnt = 0;
    /* nei(u) < nei(v) */
    int u = 0;
    int w = 0;
    TPST_MAP_BY_EID* pstENode = NULL;

    u = iNodeId;

    DEBUG_ASSERT(vNodes.empty());
    /* find u's neighbor , note u > 0 */
    itLowB = m_mpBasicG.upper_bound(pair<int, int>(u, 0));
    itHighB = m_mpBasicG.upper_bound(pair<int, int>(u + 1, 0));
    while (itLowB != itHighB)
    {
        w = itLowB->first.second;

        vNodes.push_back(w);
        ++iCnt;

        ++itLowB;
    }

    return iCnt;
}

/*****************
input:
        int iNodeId
        vector <int> &vNodes
description:
        find eid
******************/
int myG::findPNebP(int iNodeId, vector <int> &vNodes, vector <int> &vEdges)
{
    MAP_BASIC_G::iterator itLowB;
    MAP_BASIC_G::iterator itHighB;
    int iCnt = 0;
    /* nei(u) < nei(v) */
    int u = 0;
    int w = 0;
    TPST_MAP_BY_EID* pstENode = NULL;

    u = iNodeId;

    //DEBUG_ASSERT(vNodes.empty());
    //DEBUG_ASSERT(vEdges.empty());
    /* find u's neighbor , note u > 0 */
    itLowB = m_mpBasicG.upper_bound(pair<int, int>(u, 0));
    itHighB = m_mpBasicG.upper_bound(pair<int, int>(u + 1, 0));
    while (itLowB != itHighB)
    {
        w = itLowB->first.second;

        vNodes.push_back(w);
        vEdges.push_back(itLowB->second);
        ++iCnt;

        ++itLowB;
    }

    return iCnt;
}
/*****************
input:
        int iNodeId
        vector<int> &vCanP
description:
        find node's non-neighbor node, can form triangle
******************/
int myG::findPCanTriP(int iNodeId, vector<int> &vCanP)
{
    MAP_BASIC_G::iterator itLowB;
    MAP_BASIC_G::iterator itHighB;
    int iCnt = 0;
    /* nei(u) < nei(v) */
    int u = 0;
    int v = 0;
    int w = 0;
    map<int, int> mpCanP;
    map<int, int>::iterator itmpP;

    u = iNodeId;

    /* find u's neighbor , note u > 0 */
    itLowB = m_mpBasicG.upper_bound(pair<int, int>(u, 0));
    itHighB = m_mpBasicG.upper_bound(pair<int, int>(u + 1, 0));
    while (itLowB != itHighB)
    {
        MAP_BASIC_G::iterator itLcLowB;
        MAP_BASIC_G::iterator itLcHighB;
        MAP_BASIC_G::iterator itDesE;
        v = itLowB->first.second;

        itLcLowB = m_mpBasicG.upper_bound(pair<int, int>(v, 0));
        itLcHighB = m_mpBasicG.upper_bound(pair<int, int>(v + 1, 0));
        while (itLcLowB != itLcHighB)
        {
            w = itLcLowB->first.second;
            if (u == w)
            {
                /* ignore self */
                continue;
            }
            itDesE = m_mpBasicG.find(pair<int, int>(u, w));
            if (itDesE == m_mpBasicG.end())
            {
                mpCanP[w] = 1;
            }
            itLcLowB++;
        }
        ++itLowB;
    }
    for (itmpP = mpCanP.begin(); itmpP != mpCanP.end(); ++itmpP)
    {
        vCanP.push_back(itmpP->first);
        ++iCnt;
    }

    return iCnt;
}

/*****************
input:
        int iNodeId
        vector<int> &vCanP
description:
        find node's non-neighbor node, can form triangle
******************/
int myG::findECanTriP(int iEid, vector<pair<int, int> > &vCanE)
{
    MAP_BASIC_G::iterator itLowB;
    MAP_BASIC_G::iterator itHighB;
    int iCnt = 0;
    /* nei(u) < nei(v) */
    int u = 0;
    int v = 0;
    int w = 0;
    /* node, node */
    map<int, int> mpCanP;
    /* node, eid */
    map<int, int> mpSlave;
    map<int, int>::iterator itmpP;
    vector<int> vNeib;
    TPST_MAP_BY_EID *pstNode = NULL;
    int iCurK = 0;
    int iLayer = 0;
    int iUpB = 0;
    pair<int, int> paXY;

    DEBUG_ASSERT(vCanE.empty());

    pstNode = findNode(iEid);
    DEBUG_ASSERT(NULL != pstNode);
    u = pstNode->paXY.first;
    v = pstNode->paXY.second;
    iCurK = pstNode->iTrussness;
    iLayer = pstNode->iLayer;

    /* find u's neighbor , note u > 0 */
    itLowB = m_mpBasicG.upper_bound(pair<int, int>(u, 0));
    itHighB = m_mpBasicG.upper_bound(pair<int, int>(u + 1, 0));
    while (itLowB != itHighB)
    {
        w = itLowB->first.second;
        if (w == v)
        {
            ++itLowB;
            continue;
        }

        mpCanP[w] = v;
        mpSlave[w] = itLowB->second;

        ++itLowB;
    }
    itLowB = m_mpBasicG.upper_bound(pair<int, int>(v, 0));
    itHighB = m_mpBasicG.upper_bound(pair<int, int>(v + 1, 0));
    while (itLowB != itHighB)
    {
        w = itLowB->first.second;
        if (w == u)
        {
            ++itLowB;
            continue;
        }

        itmpP = mpCanP.find(w);

        if (itmpP != mpCanP.end())
        {
            /* form triangle, ignore */
            itmpP->second = -1;
            ++itLowB;
            continue;
        }

        mpCanP[w] = u;
        mpSlave[w] = itLowB->second;

        ++itLowB;
    }
    for (itmpP = mpCanP.begin(); itmpP != mpCanP.end(); ++itmpP)
    {
        if (itmpP->second < 0)
        {
            /* not meet */
            continue;
        }
        if (itmpP->first < itmpP->second)
        {
            paXY = *itmpP;
        }
        else
        {
            paXY = pair<int, int>(itmpP->second, itmpP->first);
        }
        //iUpB = getUpB(paXY.first, paXY.second);
        //bool bRes = checkIns(paXY.first, paXY.second, mpSlave[itmpP->first], iCurK, iLayer, iGroup);
        //DEBUG_PRINTF("MY_G upB get (%d, %d) bool %d\n", paXY.first, paXY.second, bRes);
        /* cannot upgrade */
        /*if (!bRes)
        {
            continue;
        }*/
        //DEBUG_PRINTF("MY_G useful (%d, %d)\n", paXY.first, paXY.second);
        vCanE.push_back(paXY);

        ++iCnt;
    }

    return iCnt;
}

/*****************
input:
        int iNodeId
        vector<int> &vCanP
description:
        find node's non-neighbor node, can form triangle
******************/
int myG::findECanTriPSort(int iEid, int iDesK, vector<TPSTV_CAN_E> &vCanE)
{
    MAP_BASIC_G::iterator itLowB;
    MAP_BASIC_G::iterator itHighB;
    int iCnt = 0;
    /* nei(u) < nei(v) */
    int u = 0;
    int v = 0;
    int w = 0;
    /* node, node */
    map<int, int> mpCanP;
    /* node, eid */
    map<int, int> mpSlave;
    map<int, int>::iterator itmpP;
    vector<int> vNeib;
    TPST_MAP_BY_EID *pstNode = NULL;
    int iCurK = 0;
    int iLayer = 0;
    int iUpB = 0;
    pair<int, int> paXY;

    DEBUG_ASSERT(vCanE.empty());

    pstNode = findNode(iEid);
    DEBUG_ASSERT(NULL != pstNode);
    u = pstNode->paXY.first;
    v = pstNode->paXY.second;
    iCurK = pstNode->iTrussness;
    iLayer = pstNode->iLayer;

    /* find u's neighbor , note u > 0 */
    itLowB = m_mpBasicG.upper_bound(pair<int, int>(u, 0));
    itHighB = m_mpBasicG.upper_bound(pair<int, int>(u + 1, 0));
    while (itLowB != itHighB)
    {
        w = itLowB->first.second;
        if (w == v)
        {
            ++itLowB;
            continue;
        }

        mpCanP[w] = v;
        mpSlave[w] = itLowB->second;

        ++itLowB;
    }
    itLowB = m_mpBasicG.upper_bound(pair<int, int>(v, 0));
    itHighB = m_mpBasicG.upper_bound(pair<int, int>(v + 1, 0));
    while (itLowB != itHighB)
    {
        w = itLowB->first.second;
        if (w == u)
        {
            ++itLowB;
            continue;
        }

        itmpP = mpCanP.find(w);

        if (itmpP != mpCanP.end())
        {
            /* form triangle, ignore */
            itmpP->second = -1;
            ++itLowB;
            continue;
        }

        mpCanP[w] = u;
        mpSlave[w] = itLowB->second;

        ++itLowB;
    }
    for (itmpP = mpCanP.begin(); itmpP != mpCanP.end(); ++itmpP)
    {
        if (itmpP->second < 0)
        {
            /* not meet */
            continue;
        }
        if (itmpP->first < itmpP->second)
        {
            paXY = *itmpP;
        }
        else
        {
            paXY = pair<int, int>(itmpP->second, itmpP->first);
        }

        vector<int> vLfE;
        vector<int> vRtE;

        findNeb(paXY.first, paXY.second, vLfE, vRtE);
        int iSup = findKSup(iDesK, vLfE, vRtE);

        TPSTV_CAN_E stE;
        stE.paXY = paXY;
        stE.iOEid = mpSlave[itmpP->first];
        stE.iSup = iSup;
        vCanE.push_back(stE);

        ++iCnt;
    }

    sort(vCanE.begin(), vCanE.end(), [](TPSTV_CAN_E &stE1, TPSTV_CAN_E &stE2){
                                return stE1.iSup > stE2.iSup;});

    return iCnt;
}
/*****************
input:
        int iNodeId
        vector<int> &vCanP
description:
        find node's non-neighbor node, can form triangle
******************/
int myG::findECanTriP(int iEid, int iMinK, int iDesK, vector<int> &vNeibE, vector<pair<int, int> > &vCanE)
{
    MAP_BASIC_G::iterator itLowB;
    MAP_BASIC_G::iterator itHighB;
    int iCnt = 0;
    /* nei(u) < nei(v) */
    int u = 0;
    int v = 0;
    int w = 0;
    /* node, node */
    map<int, int> mpCanP;
    /* node, eid */
    map<int, int> mpSlave;
    map<int, int>::iterator itmpP;
    vector<int> vNeib;
    TPST_MAP_BY_EID *pstNode = NULL;
    int iCurK = 0;
    int iLayer = 0;
    int iGroup = 0;
    int iUpB = 0;
    pair<int, int> paXY;

    DEBUG_ASSERT(vCanE.empty());

    pstNode = findNode(iEid);
    DEBUG_ASSERT(NULL != pstNode);
    u = pstNode->paXY.first;
    v = pstNode->paXY.second;

    /* find u's neighbor , note u > 0 */
    itLowB = m_mpBasicG.upper_bound(pair<int, int>(u, 0));
    itHighB = m_mpBasicG.upper_bound(pair<int, int>(u + 1, 0));
    while (itLowB != itHighB)
    {
        w = itLowB->first.second;
        if (w == v)
        {
            ++itLowB;
            continue;
        }

        mpCanP[w] = v;
        mpSlave[w] = itLowB->second;

        ++itLowB;
    }
    itLowB = m_mpBasicG.upper_bound(pair<int, int>(v, 0));
    itHighB = m_mpBasicG.upper_bound(pair<int, int>(v + 1, 0));
    while (itLowB != itHighB)
    {
        w = itLowB->first.second;
        if (w == u)
        {
            ++itLowB;
            continue;
        }

        itmpP = mpCanP.find(w);

        if (itmpP != mpCanP.end())
        {
            /* form triangle, ignore */
            itmpP->second = -1;
            ++itLowB;
            continue;
        }

        mpCanP[w] = u;
        mpSlave[w] = itLowB->second;

        ++itLowB;
    }
    for (itmpP = mpCanP.begin(); itmpP != mpCanP.end(); ++itmpP)
    {
        if (itmpP->second < 0)
        {
            /* not meet */
            continue;
        }
        if (itmpP->first < itmpP->second)
        {
            paXY = *itmpP;
        }
        else
        {
            paXY = pair<int, int>(itmpP->second, itmpP->first);
        }
        vector<int> vLfE;
        vector<int> vRtE;
        findNeb(paXY.first, paXY.second, vLfE, vRtE);
        int iGNeibCnt = findKSup(iMinK, vLfE, vRtE);
        if (iGNeibCnt + 2 < iDesK)
        {
            /* cannot upgrade */
            continue;
        }
        int iSlaveEid = mpSlave[itmpP->first];
        TPST_MAP_BY_EID *pstSlave = findNode(iSlaveEid);
        DEBUG_ASSERT(NULL != pstSlave);
        if ((pstSlave->iTrussness < iDesK) && (pstSlave->iTrussness >= iMinK))
        {
            iGNeibCnt = findKSup(iMinK, pstSlave->vLfE, pstSlave->vRtE);
            if (iGNeibCnt + 3 < iDesK)
            {
                /* cannot upgrade */
                continue;
            }
            vNeibE.push_back(pstSlave->eid);
        }
        else
        {
            /* keep the same size as vCanE */
            vNeibE.push_back(0);
        }
        /*DEBUG_PRINTF("MY_G (%d, %d) get (%d, %d) sup: %d\n",
                     pstNode->paXY.first, pstNode->paXY.second,
                     paXY.first, paXY.second, iGNeibCnt);*/
        vCanE.push_back(paXY);

        ++iCnt;
    }

    return iCnt;
}

/*****************
input:
        int iNodeId
        vector<int> &vCanP
description:
        find node's non-neighbor node, can form triangle
******************/
int myG::findECanTriP(int iEid, int iMinK, int iDesK, vector<pair<int, int> > &vCanE, void * pPara1, int iPara2, bool (*fun_state)(void *, int, int))
{
    MAP_BASIC_G::iterator itLowB;
    MAP_BASIC_G::iterator itHighB;
    int iCnt = 0;
    /* nei(u) < nei(v) */
    int u = 0;
    int v = 0;
    int w = 0;
    map<int, int> mpCanP;
    map<int, int>::iterator itmpP;
    vector<int> vNeib;
    TPST_MAP_BY_EID *pstNode = NULL;
    int iUpB = 0;
    pair<int, int> paXY;

    DEBUG_ASSERT(vCanE.empty());

    pstNode = findNode(iEid);
    DEBUG_ASSERT(NULL != pstNode);
    u = pstNode->paXY.first;
    v = pstNode->paXY.second;

    /* find u's neighbor , note u > 0 */
    itLowB = m_mpBasicG.upper_bound(pair<int, int>(u, 0));
    itHighB = m_mpBasicG.upper_bound(pair<int, int>(u + 1, 0));
    while (itLowB != itHighB)
    {
        w = itLowB->first.second;
        if (w == v)
        {
            ++itLowB;
            continue;
        }

        pstNode = findNode(itLowB->second);
        DEBUG_ASSERT(NULL != pstNode);

        if (pstNode->iTrussness >= iDesK)
        {
            mpCanP[w] = v;
        }
        else if (pstNode->iTrussness >= iMinK)
        {
            if (!fun_state(pPara1, iPara2, pstNode->eid))
            {
                mpCanP[w] = v;
            }
            else
            {
                /* not meet */
                mpCanP[w] = -1;
            }
        }
        else
        {
            /* not meet */
            mpCanP[w] = -1;
        }

        ++itLowB;
    }
    itLowB = m_mpBasicG.upper_bound(pair<int, int>(v, 0));
    itHighB = m_mpBasicG.upper_bound(pair<int, int>(v + 1, 0));
    while (itLowB != itHighB)
    {
        w = itLowB->first.second;
        if (w == u)
        {
            ++itLowB;
            continue;
        }

        itmpP = mpCanP.find(w);

        if (itmpP != mpCanP.end())
        {
            /* form triangle, ignore */
            itmpP->second = -1;
            ++itLowB;
            continue;
        }

        pstNode = findNode(itLowB->second);
        DEBUG_ASSERT(NULL != pstNode);
        if (pstNode->iTrussness >= iDesK)
        {
            mpCanP[w] = u;
        }
        else if (pstNode->iTrussness >= iMinK)
        {
            if (!fun_state(pPara1, iPara2, pstNode->eid))
            {
                mpCanP[w] = u;
            }
            else
            {
                /* not meet */
                mpCanP[w] = -1;
            }
        }
        else
        {
            /* not meet */
            mpCanP[w] = -1;
        }

        ++itLowB;
    }
    for (itmpP = mpCanP.begin(); itmpP != mpCanP.end(); ++itmpP)
    {
        if (itmpP->second < 0)
        {
            /* not meet */
            continue;
        }
        if (itmpP->first < itmpP->second)
        {
            paXY = *itmpP;
        }
        else
        {
            paXY = pair<int, int>(itmpP->second, itmpP->first);
        }

        vector<int> vLfE;
        vector<int> vRtE;

        findNeb(paXY.first, paXY.second, vLfE, vRtE);
        int iSup = findKSup(iMinK, iDesK, vLfE, vRtE, pPara1, iPara2, fun_state);

        if (iSup + 2 < iDesK)
        {
            /* cannot upgrade */
            continue;
        }
        vCanE.push_back(paXY);

        ++iCnt;
    }

    return iCnt;
}
/*****************
input:
        int iNodeId
        vector<int> &vCanP
description:
        find node's non-neighbor node, can form triangle
******************/
int myG::findECanTriP(int iEid, int iMinK, int iDesK, vector<int> &vNeibE, vector<pair<int, int> > &vCanE, void * pPara1, int iPara2, bool (*fun_state)(void *, int, int))
{
    MAP_BASIC_G::iterator itLowB;
    MAP_BASIC_G::iterator itHighB;
    int iCnt = 0;
    /* nei(u) < nei(v) */
    int u = 0;
    int v = 0;
    int w = 0;
    map<int, int> mpCanP;
    map<int, int> mpSlave;
    map<int, int>::iterator itmpP;
    vector<int> vNeib;
    TPST_MAP_BY_EID *pstNode = NULL;
    int iUpB = 0;
    pair<int, int> paXY;

    DEBUG_ASSERT(vCanE.empty());

    pstNode = findNode(iEid);
    DEBUG_ASSERT(NULL != pstNode);
    u = pstNode->paXY.first;
    v = pstNode->paXY.second;

    /* find u's neighbor , note u > 0 */
    itLowB = m_mpBasicG.upper_bound(pair<int, int>(u, 0));
    itHighB = m_mpBasicG.upper_bound(pair<int, int>(u + 1, 0));
    while (itLowB != itHighB)
    {
        w = itLowB->first.second;
        if (w == v)
        {
            ++itLowB;
            continue;
        }

        pstNode = findNode(itLowB->second);
        DEBUG_ASSERT(NULL != pstNode);

        if (pstNode->iTrussness >= iDesK)
        {
            mpCanP[w] = v;
            mpSlave[w] = 0;
        }
        else if (pstNode->iTrussness >= iMinK)
        {
            if (!fun_state(pPara1, iPara2, pstNode->eid))
            {
                mpCanP[w] = v;
                mpSlave[w] = pstNode->eid;
            }
            else
            {
                /* not meet */
                mpCanP[w] = -1;
                mpSlave[w] = 0;
            }
        }
        else
        {
            /* not meet */
            mpCanP[w] = -1;
            mpSlave[w] = 0;
        }

        ++itLowB;
    }
    itLowB = m_mpBasicG.upper_bound(pair<int, int>(v, 0));
    itHighB = m_mpBasicG.upper_bound(pair<int, int>(v + 1, 0));
    while (itLowB != itHighB)
    {
        w = itLowB->first.second;
        if (w == u)
        {
            ++itLowB;
            continue;
        }

        itmpP = mpCanP.find(w);

        if (itmpP != mpCanP.end())
        {
            /* form triangle, ignore */
            itmpP->second = -1;
            ++itLowB;
            mpSlave[w] = 0;
            continue;
        }

        pstNode = findNode(itLowB->second);
        DEBUG_ASSERT(NULL != pstNode);
        if (pstNode->iTrussness >= iDesK)
        {
            mpCanP[w] = u;
            mpSlave[w] = 0;
        }
        else if (pstNode->iTrussness >= iMinK)
        {
            if (!fun_state(pPara1, iPara2, pstNode->eid))
            {
                mpCanP[w] = u;
                mpSlave[w] = pstNode->eid;
            }
            else
            {
                /* not meet */
                mpCanP[w] = -1;
                mpSlave[w] = 0;
            }
        }
        else
        {
            /* not meet */
            mpCanP[w] = -1;
            mpSlave[w] = 0;
        }

        ++itLowB;
    }
    for (itmpP = mpCanP.begin(); itmpP != mpCanP.end(); ++itmpP)
    {
        if (itmpP->second < 0)
        {
            /* not meet */
            continue;
        }
        if (itmpP->first < itmpP->second)
        {
            paXY = *itmpP;
        }
        else
        {
            paXY = pair<int, int>(itmpP->second, itmpP->first);
        }

        vector<int> vLfE;
        vector<int> vRtE;

        findNeb(paXY.first, paXY.second, vLfE, vRtE);
        int iSup = findKSup(iMinK, iDesK, vLfE, vRtE, pPara1, iPara2, fun_state);

        if (iSup + 2 < iDesK)
        {
            /* cannot upgrade */
            continue;
        }
        vCanE.push_back(paXY);
        vNeibE.push_back(mpSlave[itmpP->first]);

        ++iCnt;
    }

    return iCnt;
}

/*****************
input:
        int iNodeId
        vector<int> &vCanP
description:
        find node's non-neighbor node, can form triangle
******************/
int myG::findECanTriPSort(int iEid, int iMinK, int iDesK, vector<pair<pair<int, int>, int> > &vCanE, void * pPara1, int iPara2, bool (*fun_state)(void *, int, int))
{
    MAP_BASIC_G::iterator itLowB;
    MAP_BASIC_G::iterator itHighB;
    int iCnt = 0;
    /* nei(u) < nei(v) */
    int u = 0;
    int v = 0;
    int w = 0;
    map<int, int> mpCanP;
    map<int, int>::iterator itmpP;
    vector<int> vNeib;
    TPST_MAP_BY_EID *pstNode = NULL;
    int iUpB = 0;
    pair<int, int> paXY;

    DEBUG_ASSERT(vCanE.empty());

    pstNode = findNode(iEid);
    DEBUG_ASSERT(NULL != pstNode);
    u = pstNode->paXY.first;
    v = pstNode->paXY.second;

    /* find u's neighbor , note u > 0 */
    itLowB = m_mpBasicG.upper_bound(pair<int, int>(u, 0));
    itHighB = m_mpBasicG.upper_bound(pair<int, int>(u + 1, 0));
    while (itLowB != itHighB)
    {
        w = itLowB->first.second;
        if (w == v)
        {
            ++itLowB;
            continue;
        }

        pstNode = findNode(itLowB->second);
        DEBUG_ASSERT(NULL != pstNode);

        if (pstNode->iTrussness >= iDesK)
        {
            mpCanP[w] = v;
        }
        else if (pstNode->iTrussness >= iMinK)
        {
            if (!fun_state(pPara1, iPara2, pstNode->eid))
            {
                mpCanP[w] = v;
            }
            else
            {
                /* not meet */
                mpCanP[w] = -1;
            }
        }
        else
        {
            /* not meet */
            mpCanP[w] = -1;
        }

        ++itLowB;
    }
    itLowB = m_mpBasicG.upper_bound(pair<int, int>(v, 0));
    itHighB = m_mpBasicG.upper_bound(pair<int, int>(v + 1, 0));
    while (itLowB != itHighB)
    {
        w = itLowB->first.second;
        if (w == u)
        {
            ++itLowB;
            continue;
        }

        itmpP = mpCanP.find(w);

        if (itmpP != mpCanP.end())
        {
            /* form triangle, ignore */
            itmpP->second = -1;
            ++itLowB;
            continue;
        }

        pstNode = findNode(itLowB->second);
        DEBUG_ASSERT(NULL != pstNode);

        if (pstNode->iTrussness >= iDesK)
        {
            mpCanP[w] = u;
        }
        else if (pstNode->iTrussness >= iMinK)
        {
            if (!fun_state(pPara1, iPara2, pstNode->eid))
            {
                mpCanP[w] = u;
            }
            else
            {
                /* not meet */
                mpCanP[w] = -1;
            }
        }
        else
        {
            /* not meet */
            mpCanP[w] = -1;
        }

        ++itLowB;
    }
    for (itmpP = mpCanP.begin(); itmpP != mpCanP.end(); ++itmpP)
    {
        if (itmpP->second < 0)
        {
            /* not meet */
            continue;
        }
        if (itmpP->first < itmpP->second)
        {
            paXY = *itmpP;
        }
        else
        {
            paXY = pair<int, int>(itmpP->second, itmpP->first);
        }

        vector<int> vLfE;
        vector<int> vRtE;

        findNeb(paXY.first, paXY.second, vLfE, vRtE);
        int iSup = findKSup(iMinK, iDesK, vLfE, vRtE, pPara1, iPara2, fun_state);

        vCanE.push_back(pair<pair<int, int>, int>(paXY, iSup));

        ++iCnt;
    }

    sort(vCanE.begin(), vCanE.end(), [](pair<pair<int, int>, int> &atE1, pair<pair<int, int>, int> &atE2){
                                return atE1.second > atE2.second;});

    return iCnt;
}
/*****************
input:
        int iNodeId
        vector<int> &vCanP
description:
        find node's non-neighbor node, can form triangle
******************/
int myG::findKSup(int iDesK, vector <int> &vLfE, vector <int> &vRtE)
{
    TPST_MAP_BY_EID *pstLfNode = NULL;
    TPST_MAP_BY_EID *pstRtNode = NULL;

    int iSup = 0;
    int iMinT = 0;
    vector <int>::iterator itLfE;
    vector <int>::iterator itRtE;

    itLfE = vLfE.begin();
    itRtE = vRtE.begin();
    for (; itLfE != vLfE.end(); ++itLfE, ++itRtE)
    {
        pstLfNode = findNode(*itLfE);
        DEBUG_ASSERT(NULL != pstLfNode);
        pstRtNode = findNode(*itRtE);
        DEBUG_ASSERT(NULL != pstRtNode);

        iMinT = COMMON_MIN(pstLfNode->iTrussness, pstRtNode->iTrussness);

        if (iMinT >= iDesK)
        {
            ++iSup;
        }
    }

    return iSup;
}

/*****************
input:
        int iNodeId
        vector<int> &vCanP
description:
        >= iDesK, or false for state
******************/
int myG::findKSup(int iMinK, int iDesK, vector <int> &vLfE, vector <int> &vRtE, void * pPara1, int iPara2, bool (*fun_state)(void *, int, int))
{
    TPST_MAP_BY_EID *pstLfNode = NULL;
    TPST_MAP_BY_EID *pstRtNode = NULL;

    int iSup = 0;
    int iMinT = 0;
    vector <int>::iterator itLfE;
    vector <int>::iterator itRtE;

    itLfE = vLfE.begin();
    itRtE = vRtE.begin();
    for (; itLfE != vLfE.end(); ++itLfE, ++itRtE)
    {
        pstLfNode = findNode(*itLfE);
        DEBUG_ASSERT(NULL != pstLfNode);
        pstRtNode = findNode(*itRtE);
        DEBUG_ASSERT(NULL != pstRtNode);

        iMinT = COMMON_MIN(pstLfNode->iTrussness, pstRtNode->iTrussness);

        if (iMinT >= iDesK)
        {
            ++iSup;
        }
        else if (iMinT >= iMinK)
        {
            if ((pstLfNode->iTrussness < iDesK) && (pstRtNode->iTrussness < iDesK))
            {
                if (!fun_state(pPara1, iPara2, pstLfNode->eid) && !fun_state(pPara1, iPara2, pstRtNode->eid))
                {
                    ++iSup;
                }
            }
            else if (pstLfNode->iTrussness < iDesK)
            {
                if (!fun_state(pPara1, iPara2, pstLfNode->eid))
                {
                    ++iSup;
                }
            }
            else if (pstRtNode->iTrussness < iDesK)
            {
                if (!fun_state(pPara1, iPara2, pstRtNode->eid))
                {
                    ++iSup;
                }
            }
        }
        else
        {
            /* cannot */
        }
    }

    return iSup;
}

#if 0
/*****************
input:
        int iNodeId
        vector<int> &vCanP
description:
        find node's non-neighbor node, can form triangle
******************/
int myG::findECanTriP(int iEid, vector<pair<int, int> > &vCanE, int iLayer)
{
    MAP_BASIC_G::iterator itLowB;
    MAP_BASIC_G::iterator itHighB;
    int iCnt = 0;
    /* nei(u) < nei(v) */
    int u = 0;
    int v = 0;
    int w = 0;
    map<int, int> mpCanP;
    map<int, int>::iterator itmpP;
    vector<int> vNeib;
    TPST_MAP_BY_EID *pstNode = NULL;
    int iCurK = 0;
    int iGroup = 0;
    int iUpB = 0;
    bool res = false;
    pair<int, int> paXY;

    DEBUG_ASSERT(vCanE.empty());

    pstNode = findNode(iEid);
    DEBUG_ASSERT(NULL != pstNode);
    u = pstNode->paXY.first;
    v = pstNode->paXY.second;
    iCurK = pstNode->iTrussness;
    iGroup = pstNode->iGroupId;

    /* find u's neighbor , note u > 0 */
    itLowB = m_mpBasicG.upper_bound(pair<int, int>(u, 0));
    itHighB = m_mpBasicG.upper_bound(pair<int, int>(u + 1, 0));
    while (itLowB != itHighB)
    {
        w = itLowB->first.second;
        if (w == v)
        {
            ++itLowB;
            continue;
        }

        pstNode = findNode(itLowB->second);
        DEBUG_ASSERT(NULL != pstNode);

        if (pstNode->iTrussness >= iCurK)
        {
            mpCanP[w] = v;
        }
        else
        {
            /* not meet */
            mpCanP[w] = -1;
        }

        ++itLowB;
    }
    itLowB = m_mpBasicG.upper_bound(pair<int, int>(v, 0));
    itHighB = m_mpBasicG.upper_bound(pair<int, int>(v + 1, 0));
    while (itLowB != itHighB)
    {
        w = itLowB->first.second;
        if (w == u)
        {
            ++itLowB;
            continue;
        }

        pstNode = findNode(itLowB->second);
        DEBUG_ASSERT(NULL != pstNode);

        if (pstNode->iTrussness >= iCurK)
        {
            itmpP = mpCanP.find(w);

            if (itmpP == mpCanP.end())
            {
                mpCanP[w] = u;
            }
            else
            {
                /* form triangle, ignore */
                itmpP->second = -1;
            }
        }
        else
        {
            /* not meet */
            mpCanP[w] = -1;
        }
        ++itLowB;
    }
    for (itmpP = mpCanP.begin(); itmpP != mpCanP.end(); ++itmpP)
    {
        if (itmpP->second < 0)
        {
            /* not meet */
            continue;
        }
        if (itmpP->first < itmpP->second)
        {
            paXY = *itmpP;
        }
        else
        {
            paXY = pair<int, int>(itmpP->second, itmpP->first);
        }
        //DEBUG_PRINTF("MY_G upB get (%d, %d)\n", paXY.first, paXY.second);
        res = checkIns(paXY.first, paXY.second, iCurK, iLayer, iGroup);
        if (!res)
        {
            /* cannot upgrade */
            continue;
        }
        vCanE.push_back(paXY);

        ++iCnt;
    }

    return iCnt;
}
#endif
/*****************
input:
        int iNodeId
        vector<int> &vCanP
description:
        find node's non-neighbor node, can form triangle
******************/
int myG::findENewCanTriP(int x, int y, vector<pair<int, int> > &vNewCanE)
{
    MAP_BASIC_G::iterator itLowB;
    MAP_BASIC_G::iterator itHighB;
    int iCnt = 0;
    /* nei(u) < nei(v) */
    int u = x;
    int v = y;
    int w = 0;
    map<int, int> mpCanP;
    map<int, int>::iterator itmpP;
    vector<int> vNeib;

    DEBUG_ASSERT(vNewCanE.empty());
    //DEBUG_PRINTF("MYG enter finding (%d, %d) new edge\n", x, y);

    /* find u's neighbor , note u > 0 */
    itLowB = m_mpBasicG.upper_bound(pair<int, int>(u, 0));
    itHighB = m_mpBasicG.upper_bound(pair<int, int>(u + 1, 0));
    while (itLowB != itHighB)
    {
        w = itLowB->first.second;

        mpCanP[w] = v;
        ++itLowB;
    }
    itLowB = m_mpBasicG.upper_bound(pair<int, int>(v, 0));
    itHighB = m_mpBasicG.upper_bound(pair<int, int>(v + 1, 0));
    while (itLowB != itHighB)
    {
        w = itLowB->first.second;

        itmpP = mpCanP.find(w);

        if (itmpP == mpCanP.end())
        {
            mpCanP[w] = u;
        }
        else
        {
            itmpP->second = -1;
        }
        ++itLowB;
    }
    for (itmpP = mpCanP.begin(); itmpP != mpCanP.end(); ++itmpP)
    {
        //DEBUG_PRINTF("MYG map (%d, %d)\n", itmpP->first, itmpP->second);
        if (itmpP->second < 0)
        {
            /* not meet */
            continue;
        }
        vNeib.clear();
        findNeb(itmpP->first, itmpP->second, vNeib);
        if (0 < vNeib.size())
        {
            /* not new candidate edge */
            continue;
        }
        if (itmpP->first < itmpP->second)
        {
            vNewCanE.push_back(*itmpP);
            //DEBUG_PRINTF("MYG new edge (%d, %d)\n", itmpP->first, itmpP->second);
        }
        else
        {
            vNewCanE.push_back(pair<int, int>(itmpP->second, itmpP->first));
            //DEBUG_PRINTF("MYG new edge (%d, %d)\n", itmpP->second, itmpP->first);
        }

        ++iCnt;
    }

    return iCnt;
}

/*****************
input:
        int iNodeId
        vector<int> &vCanP
description:
        find node's non-neighbor node, can form triangle
******************/
int myG::findPCanE(vector<int> &vCanP, vector<pair<int, int> > &vNewCanE)
{
    int iCnt = 0;

    int i = 0;
    int j = 0;
    int x = 0;
    int y = 0;

    for (i = 0; i < vCanP.size(); ++i)
    {
        for (j = i + 1; j < vCanP.size(); ++j)
        {
            if (vCanP[i] < vCanP[j])
            {
                x = vCanP[i];
                y = vCanP[j];
            }
            else
            {
                x = vCanP[j];
                y = vCanP[i];
            }
            if (m_mpBasicG.find(pair<int, int>(x, y)) == m_mpBasicG.end())
            {
                ++iCnt;
                vNewCanE.push_back(pair<int, int>(x, y));
            }
        }
    }

    return iCnt;
}
/*****************
input:
        int iNodeId
        vector<int> &vCanP
description:
        find node's non-neighbor node, can form triangle
******************/
int myG::findAllCanE(vector<pair<int, int> > &vNewCanE)
{
    MAP_BASIC_G::iterator itLowB;
    int iCnt = 0;
    /* nei(u) < nei(v) */
    int u = 0;
    int v = 0;
    int w = 0;
    map<int, int> mpCanP;
    map<int, int>::iterator itmpP;
    vector<int> vNeib;

    DEBUG_ASSERT(vNewCanE.empty());
    itLowB = m_mpBasicG.begin();
    while ( itLowB != m_mpBasicG.end() )
    {
        if (u != itLowB->first.first)
        {
            if (0 != u)
            {
                /* not first time */
                while (w <= m_iMaxPId)
                {
                    if (0 < m_pvPNeCnt->at(w))
                    {
                        /* exists */
                        vNewCanE.push_back(pair<int, int>(u, w));
                        //printf("FIND get 1 (%d, %d)\n", u, w);
                    }
                    ++w;
                }
            }
            u = itLowB->first.first;
            w = u + 1;
            //DEBUG_PRINTF("MY_G_FIND get center %d\n", u);
        }
        v = itLowB->first.second;
        if (u > v)
        {
            ++itLowB;
            continue;
        }
        //printf("FIND get neighbor %d\n", v);
        while (w < v)
        {
            if (0 < m_pvPNeCnt->at(w))
            {
                /* exists */
                vNewCanE.push_back(pair<int, int>(u, w));
                //printf("FIND get 2 (%d, %d)\n", u, w);
            }
            ++w;
        }
        /* ignore v*/
        ++w;
        ++itLowB;
    }
    /* last */
    DEBUG_ASSERT(0 != u);
    while (w <= m_iMaxPId)
    {
        if (0 < m_pvPNeCnt->at(w))
        {
            /* exists */
            vNewCanE.push_back(pair<int, int>(u, w));
            //printf("FIND get 3 (%d, %d)\n", u, w);
        }
        ++w;
    }

    return vNewCanE.size();
}

/*****************
input:
        int x
        int y
description:
        find node
******************/
TPST_MAP_BY_EID * myG::findNode(int x, int y)
{
    int eid = 0;
    if (m_mpBasicG.find(pair<int, int>(x, y)) == m_mpBasicG.end())
    {
        //printf("ERROR MYG no edge (%d, %d)\n", x, y);
        //debug_assert(0);
        return NULL;
    }
    eid = m_mpBasicG[pair<int, int>(x, y)];
    //printf("MYG (%d, %d), eid: %d\n", x, y, eid);
    return findNode(eid);
}

/*****************
input:
        int eid
description:
        find node
******************/
TPST_MAP_BY_EID * myG::findNode(int eid)
{
    if (!((0 < eid) && (eid <= m_iMaxEId)))
    {
        printf("MYG ERROR eid: %d maxEid: %d\n",
               eid, m_iMaxEId);
        DEBUG_ASSERT(0);
    }
    return &((*m_pvG)[eid]);
}













