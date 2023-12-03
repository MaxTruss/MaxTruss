/***************
input file output file function
****************/

#include "common.h"
#include "myG.h"
#include "insert.h"
#include "backtrack.h"

/*****************
input:
        none
description:
        init object
        init stage
******************/
backtrack::backtrack(myG &mpInitG, myG &mpBackG, int iBat, vector<pair<int, int> > &vCanE, int iDesK, vector <pair<int, int> > &vInsE)
{
    int i = 0;
    vector <pair<int, int> >::iterator itvE;
    m_iSize = iBat;
    m_iDesK = iDesK;
    m_pvStack = new vector<TPST_V_BY_DEPTH>(m_iSize);
    for (i = 0; i < m_iSize; ++i)
    {
        m_pvStack->at(i).iDepth = i;
        m_pvStack->at(i).pstInsG = new TPST_INS_G();
        m_pvStack->at(i).itCurPos = m_pvStack->at(i).vCurCanE.end();
        //DEBUG_PRINTF("DEBUG init %p\n", m_pvStack->at(i).itCurPos);
    }
    m_pvCanE = &vCanE;
    m_pmyG = &mpInitG;
    m_pmyBackG = &mpBackG;
    m_iBestScore = -1;
    m_pstBestInsG = new TPST_INS_G();
    m_vBestE.clear();

    for (itvE = vInsE.begin(); itvE != vInsE.end(); ++itvE)
    {
        m_mpNewE[*itvE] = 1;
    }
}
/*****************
input:
        none
description:
        free memory
******************/
backtrack::~backtrack()
{
    int i = 0;
    for (i = 0; i < m_iSize; ++i)
    {
        m_pvStack->at(i).iDepth = 0;
        delete m_pvStack->at(i).pstInsG;
        m_pvStack->at(i).pstInsG = NULL;
    }
    delete m_pvStack;
    delete m_pstBestInsG;
    m_pvStack = NULL;
    m_iSize = 0;
    m_iDesK = 0;
    m_iBestScore = 0;
    m_vBestE.clear();
}

/*****************
input:
        myG &myResG
        myG &myInitG
        TPST_INS_G* pstInsG
        vector<int> &vChgE
description:
        restore G
        myResG has changed, edges are recorded in vChgE
        myInitG is the original graph
        pstInsG saves information that should be restored
******************/
int backtrack::addG(myG &myResG, TPST_INS_G* pstInsG)
{
    map<int, TPST_MAP_BY_EID>::iterator itmpE;
    map<int, int>::iterator itmpP;
    vector<int>::iterator itvChgE;
    TPST_MAP_BY_EID* pstNode = NULL;
    map<pair<int, int>, int>::iterator itmpNewE;

    myResG.m_iMaxEId = pstInsG->iMaxEId;
    myResG.m_iMaxD = pstInsG->iMaxD;
    myResG.m_iMaxK = pstInsG->iMaxK;
    myResG.m_pvG->resize(myResG.m_iMaxEId + 1);
    /* restore to current G */
    for (itmpE = pstInsG->mpChgdE.begin(); itmpE != pstInsG->mpChgdE.end(); ++itmpE)
    {
        DEBUG_ASSERT(itmpE->first <= myResG.m_iMaxEId);
        (*(myResG.m_pvG))[itmpE->first] = itmpE->second;
    }
    for (itmpP = pstInsG->mpChgdD.begin(); itmpP != pstInsG->mpChgdD.end(); ++itmpP)
    {
        (*(myResG.m_pvPNeCnt))[itmpP->first] = itmpP->second;
    }
    for (itmpNewE = pstInsG->mpNewE.begin(); itmpNewE != pstInsG->mpNewE.end(); ++itmpNewE)
    {
        myResG.m_mpBasicG[itmpNewE->first] = itmpNewE->second;
        myResG.m_mpBasicG[pair<int, int>(itmpNewE->first.second, itmpNewE->first.first)] = itmpNewE->second;
    }
    DEBUG_PRINTF("RESTORE done\n");
    return 0;
}
/*****************
input:
        myG &myResG
        myG &myInitG
        TPST_INS_G* pstInsG
        vector<int> &vChgE
description:
        restore G
        myResG has changed, edges are recorded in vChgE
        myInitG is the original graph
        pstInsG saves information that should be restored
******************/
int backtrack::restoreG(myG &myResG, myG &myInitG, TPST_INS_G* pstInsG, vector<int> &vChgE)
{
    map<int, TPST_MAP_BY_EID>::iterator itmpE;
    map<int, int>::iterator itmpP;
    vector<int>::iterator itvChgE;
    TPST_MAP_BY_EID* pstNode = NULL;

    //DEBUG_PRINTF("RESTORE start %d, %d\n", myResG.m_iMaxEId, pstInsG->iMaxEId);
    DEBUG_ASSERT(myResG.m_iMaxEId >= pstInsG->iMaxEId);
    /* restore to initial G */
    for (itvChgE = vChgE.begin(); itvChgE != vChgE.end(); ++itvChgE)
    {
        pstNode = myResG.findNode(*itvChgE);
        DEBUG_ASSERT(NULL != pstNode);

        if (pstNode->eid > myInitG.m_iMaxEId)
        {
            if (pstNode->eid > pstInsG->iMaxEId)
            {
                /* should be removed */
                //DEBUG_PRINTF("RESTORE start rm eid: %d\n", pstNode->eid);
                myResG.simpleRm(pstNode->paXY.first, pstNode->paXY.second);
                //DEBUG_PRINTF("RESTORE rm done\n");
            }
        }
        else
        {
            /*DEBUG_PRINTF("RESTORE start copy eid: %d size: %d %d\n",
                         pstNode->eid, myResG.m_pvG->size(), myInitG.m_pvG->size());*/
            (*(myResG.m_pvG))[pstNode->eid] = (*(myInitG.m_pvG))[pstNode->eid];
            //DEBUG_PRINTF("RESTORE copy done\n");
        }
    }
    myResG.m_iMaxEId = pstInsG->iMaxEId;
    myResG.m_iMaxD = pstInsG->iMaxD;
    myResG.m_iMaxK = pstInsG->iMaxK;
    myResG.m_pvG->resize(myResG.m_iMaxEId + 1);
    //DEBUG_PRINTF("RESTORE done\n");
    //DEBUG_PRINTF("RESTORE copy\n");
    /* restore to current G */
    for (itmpE = pstInsG->mpChgdE.begin(); itmpE != pstInsG->mpChgdE.end(); ++itmpE)
    {
        DEBUG_ASSERT(itmpE->first <= myResG.m_iMaxEId);
        (*(myResG.m_pvG))[itmpE->first] = itmpE->second;

        pstNode = myResG.findNode(itmpE->first);

        //DEBUG_ASSERT(!pstNode->bNewFlag);
        //DEBUG_ASSERT(!pstNode->bPseNewFlag);
        DEBUG_ASSERT(!pstNode->bUgdFlag);
        //DEBUG_ASSERT(!pstNode->bBdBallFlag);
        //DEBUG_ASSERT(!pstNode->bNewJuTriFlag);
        //DEBUG_ASSERT(!pstNode->bSeedFlag);
        DEBUG_ASSERT(!pstNode->bVstFlag);

        DEBUG_ASSERT(!pstNode->bVisQFlag);
        //DEBUG_ASSERT(!pstNode->bHandledFlag);
        //DEBUG_ASSERT(!pstNode->bRmFlag);
        //DEBUG_ASSERT(!pstNode->bLock);
        DEBUG_ASSERT(!pstNode->bInsLFlag);
        DEBUG_ASSERT(!pstNode->bCanQFlag);
        DEBUG_ASSERT(!pstNode->bDoneQFlag);
        DEBUG_ASSERT(!pstNode->bUsedCanFlag);
        DEBUG_ASSERT(!pstNode->bUsedDoneFlag);
        DEBUG_ASSERT(!pstNode->bUsedVisFlag);
    }
    for (itmpP = pstInsG->mpChgdD.begin(); itmpP != pstInsG->mpChgdD.end(); ++itmpP)
    {
        (*(myResG.m_pvPNeCnt))[itmpP->first] = itmpP->second;
    }
    //DEBUG_PRINTF("RESTORE copy done\n");
    return 0;
}
/*****************
input:
        myG &myResG
        myG &myInitG
        TPST_INS_G* pstInsG
        vector<int> &vChgE
description:
        increase G
******************/
int backtrack::keepChg(myG &myResG, TPST_INS_G* pstInsG, vector<int> &vChgE)
{
    vector<int>::iterator itvChgE;
    TPST_MAP_BY_EID* pstNode = NULL;

    pstInsG->iMaxEId = myResG.m_iMaxEId;
    pstInsG->iMaxD = myResG.m_iMaxD;
    pstInsG->iMaxK = myResG.m_iMaxK;

    for (itvChgE = vChgE.begin(); itvChgE != vChgE.end(); ++itvChgE)
    {
        pstNode = myResG.findNode(*itvChgE);
        DEBUG_ASSERT(NULL != pstNode);
        pstInsG->mpChgdE[pstNode->eid] = *pstNode;
        if (pstNode->eid == pstInsG->iMaxEId)
        {
            /* insertion edge */
            pstInsG->mpChgdD[pstNode->paXY.first] = myResG.m_pvPNeCnt->at(pstNode->paXY.first);
            pstInsG->mpChgdD[pstNode->paXY.second] = myResG.m_pvPNeCnt->at(pstNode->paXY.second);
            pstInsG->mpNewE[pstNode->paXY] = pstNode->eid;
        }
    }
    return 0;
}
/*****************
input:
        TPST_INS_G* pstDesG
        TPST_INS_G* pstSrcG
description:
        copy delta G
******************/
int backtrack::copyInsG(TPST_INS_G* pstDesG, TPST_INS_G* pstSrcG)
{
    pstDesG->iMaxEId = pstSrcG->iMaxEId;
    pstDesG->iMaxD = pstSrcG->iMaxD;
    pstDesG->iMaxK = pstSrcG->iMaxK;
    pstDesG->mpChgdD = pstSrcG->mpChgdD;
    pstDesG->mpChgdE = pstSrcG->mpChgdE;
    pstDesG->mpNewE = pstSrcG->mpNewE;

    return 0;
}
/*****************
input:
        TPST_INS_G* pstDesG
        TPST_INS_G* pstSrcG
description:
        clear delta G
******************/
int backtrack::clearInsG(myG &mySrcG, TPST_INS_G* pstDesG)
{
    pstDesG->iMaxEId = mySrcG.m_iMaxEId;
    pstDesG->iMaxD = mySrcG.m_iMaxD;
    pstDesG->iMaxK = mySrcG.m_iMaxK;
    pstDesG->mpChgdD.clear();
    pstDesG->mpChgdE.clear();
    pstDesG->mpNewE.clear();

    return 0;
}
/*****************
input:
        myG &mpInitG
        vector<pair<int, int> > &vCanE
description:
        return size
******************/
long long backtrack::findCanE(myG &mpInitG, vector<pair<int, int> > &vCanE)
{
    int i = 0;
    vector<int>::iterator itP;
    long long llCnt = 0;
    int iLowB = 0;

    for (i = 0; i < mpInitG.m_iMaxPId; ++i)
    {
        if (mpInitG.m_pvPNeCnt->at(i) > 0)
        {
            vector<int> vCanP;
            llCnt += mpInitG.findPCanTriP(i, vCanP);
            for (itP = vCanP.begin(); itP != vCanP.end(); ++itP)
            {
                iLowB = mpInitG.getLowB(i, *itP);
                if (iLowB < m_iDesK - 1)
                {
                    continue;
                }
                vCanE.push_back(pair<int, int> (i, *itP));
            }
        }
    }
    return llCnt;
}
/*****************
input:
        myG &mpInitG
        vector<pair<int, int> > &vCanE
description:
        return score
******************/
int backtrack::showCanE(vector<pair<int, int> > &vCanE)
{
    vector<pair<int, int> >::iterator itE;
    for (itE = vCanE.begin(); itE != vCanE.end(); ++itE)
    {
        DEBUG_PRINTF("SHOW candidate (%d, %d)\n", itE->first, itE->second);
    }
    return 0;
}
/*****************
input:
        myG &mpInitG
description:
        init
******************/
int backtrack::init(myG &mpInitG)
{
    int i = 0;
    /* first */
    m_pvStack->at(0).vCurCanE.insert(m_pvStack->at(0).vCurCanE.end(), m_pvCanE->begin(), m_pvCanE->end());
    /*m_pvStack->at(0).vCurCanE.insert(m_pvStack->at(0).vCurCanE.end(),
                                     m_pvCanE->begin(),
                                     m_pvCanE->begin() + 4);*/
    m_pvStack->at(0).itCurPos = m_pvStack->at(0).vCurCanE.end();

    return 0;
}
/*****************
input:
        myG &mpInitG
description:
        init
******************/
bool backtrack::process(int iDepth)
{
    int i = 0;
    TPST_V_BY_DEPTH* pstCurStack = NULL;
    TPST_V_BY_DEPTH* pstPreStack = NULL;
    map<pair<int, int>, int>::iterator itmpE;
    vector<pair<int, int> >::iterator itPrePos;
    pair<int, int> paPrevE;
    //vector<pair<int, int> > vNewCanE;

    //DEBUG_PRINTF("PROCESS now at %d level\n", iDepth);
    pstCurStack = &(m_pvStack->at(iDepth));
    DEBUG_ASSERT(NULL != pstCurStack);

    if (pstCurStack->itCurPos == pstCurStack->vCurCanE.end())
    {
        /* first time */
        pstCurStack->itCurPos = pstCurStack->vCurCanE.begin();
        /* move, avoid repetition */
        if (0 < iDepth)
        {
            pstPreStack = &(m_pvStack->at(iDepth - 1));
            itPrePos = pstPreStack->vCurCanE.begin();
            while (itPrePos != pstPreStack->itCurPos)
            {
                if (*(pstCurStack->itCurPos) != *(itPrePos))
                {
                    break;
                }
                ++itPrePos;
                pstCurStack->itCurPos++;
            }
            if (itPrePos == pstPreStack->itCurPos)
            {
                /* find the position, move one */
                pstCurStack->itCurPos++;
            }
        }
    }
    else
    {
        /* remove previous insertion */
        paPrevE = *(pstCurStack->itCurPos);
        itmpE = m_mpNewE.find(paPrevE);
        DEBUG_ASSERT(itmpE != m_mpNewE.end());
        m_mpNewE.erase(itmpE);
        /* move to next */
        pstCurStack->itCurPos++;
        /* init */
        /* clear trace */
        clearInsG(*m_pmyBackG, pstCurStack->pstInsG);
        if (0 < iDepth)
        {
            /* restore to last layer */
            restoreG(*m_pmyG, *m_pmyBackG, m_pvStack->at(iDepth - 1).pstInsG, pstCurStack->vChgE);
        }
        else
        {
            /* first layer, directly clear */
            restoreG(*m_pmyG, *m_pmyBackG, pstCurStack->pstInsG, pstCurStack->vChgE);
        }
        pstCurStack->vChgE.clear();
    }

    //DEBUG_PRINTF("PROCESS start find new edge\n");
    /* find a new edge */
    while (pstCurStack->itCurPos != pstCurStack->vCurCanE.end())
    {
        itmpE = m_mpNewE.find(*(pstCurStack->itCurPos));
        if (itmpE == m_mpNewE.end())
        {
            /* new */
            break;
        }
        /* move to next */
        pstCurStack->itCurPos++;
    }

    //DEBUG_PRINTF("PROCESS start clear\n");
    if (pstCurStack->itCurPos == pstCurStack->vCurCanE.end())
    {
        //DEBUG_PRINTF("PROCESS start clear DELTA G\n");
        /* done, clear trace */
        clearInsG(*m_pmyBackG, pstCurStack->pstInsG);
        //DEBUG_PRINTF("PROCESS start restore\n");
        /* restore myG */
        if (0 < iDepth)
        {
            /* restore to last layer */
            restoreG(*m_pmyG, *m_pmyBackG, m_pvStack->at(iDepth - 1).pstInsG, pstCurStack->vChgE);
        }
        else
        {
            /* first layer, directly clear */
            restoreG(*m_pmyG, *m_pmyBackG, pstCurStack->pstInsG, pstCurStack->vChgE);
        }
        pstCurStack->vCurCanE.clear();
        pstCurStack->vChgE.clear();
        return true;
    }
    //DEBUG_PRINTF("PROCESS start copy\n");
    /* copy last layer delta G */
    if (0 < iDepth)
    {
        copyInsG(pstCurStack->pstInsG, m_pvStack->at(iDepth - 1).pstInsG);
    }
    else
    {
        /* first layer, directly clear */
        clearInsG(*m_pmyBackG, pstCurStack->pstInsG);
    }
    //DEBUG_PRINTF("DEBUG process %p %p\n", pstCurStack->itCurPos, pstCurStack->vCurCanE.begin());
    //DEBUG_PRINTF("PROCESS start set\n");

    /*for (i = 0; i < iDepth; ++i)
    {
        DEBUG_PRINTF("-");
    }
    DEBUG_PRINTF("-(%d, %d)\n", pstCurStack->itCurPos->first,
                 pstCurStack->itCurPos->second);*/

    m_mpNewE[*(pstCurStack->itCurPos)] = 1;
    /* insert */
    //DEBUG_PRINTF("DEBUG start insertion\n");
    pstCurStack->vCurNewCanE.clear();
    /*m_pmyG->findECanTriP(pstCurStack->itCurPos->first,
                 pstCurStack->itCurPos->second, pstCurStack->vCurNewCanE);*/
    insertPart::insertOne(*m_pmyG, pstCurStack->itCurPos->first,
                 pstCurStack->itCurPos->second, pstCurStack->vChgE);
    //DEBUG_PRINTF("DEBUG insertion done\n");
    /* save changed parts */
    //DEBUG_PRINTF("DEBUG start saving\n");
    keepChg(*m_pmyG, pstCurStack->pstInsG, pstCurStack->vChgE);
    //DEBUG_PRINTF("DEBUG saving done\n");

    /* set next */
    if (iDepth + 1 < m_iSize)
    {
        m_pvStack->at(iDepth + 1).vCurCanE.insert(m_pvStack->at(iDepth + 1).vCurCanE.end(),
                                                  pstCurStack->vCurCanE.begin(),
                                                  pstCurStack->vCurCanE.end());
        /*m_pvStack->at(iDepth + 1).vCurCanE.insert(m_pvStack->at(iDepth + 1).vCurCanE.end(),
                                                  pstCurStack->vCurNewCanE.begin(),
                                                  pstCurStack->vCurNewCanE.end());*/
        m_pvStack->at(iDepth + 1).itCurPos = m_pvStack->at(iDepth + 1).vCurCanE.end();
    }

    //showCanE(m_pvStack->at(iDepth).vCurCanE);
    return false;
}
/*****************
input:
        none
description:
        get score
******************/
int backtrack::getScore()
{
    /* eid, none */
    map<int, int> mpChgE;
    map<int, int>::iterator itmpE;
    vector<int>::iterator itvE;
    TPST_MAP_BY_EID* pstNode = NULL;
    TPST_MAP_BY_EID* pstOldNode = NULL;
    int i = 0;
    int iScore = 0;
    TPST_V_BY_DEPTH* pstCurStack = NULL;

    for (i = 0; i < m_iSize; ++i)
    {
        pstCurStack = &(m_pvStack->at(i));
        for (itvE = pstCurStack->vChgE.begin(); itvE != pstCurStack->vChgE.end(); ++itvE)
        {
            mpChgE[*itvE] = 1;
        }
    }
    for (itmpE = mpChgE.begin(); itmpE != mpChgE.end(); ++itmpE)
    {
        pstNode = m_pmyG->findNode(itmpE->first);
        DEBUG_ASSERT(NULL != pstNode);
        if (pstNode->eid > m_pmyBackG->m_iMaxEId)
        {
            /* new edge */
            if (pstNode->iTrussness >= m_iDesK)
            {
                ++iScore;
            }
        }
        else
        {
            /* old edge */
            pstOldNode = m_pmyBackG->findNode(itmpE->first);
            DEBUG_ASSERT(NULL != pstOldNode);
            if ((pstNode->iTrussness >= m_iDesK) &&
                (pstOldNode->iTrussness < m_iDesK))
            {
                /* become k-truss */
                ++iScore;
            }
        }
    }
    return iScore;
}
/*****************
input:
        none
description:
        save best
******************/
int backtrack::saveBest()
{
    int i = 0;

    //DEBUG_PRINTF("========= current best score: %d\n", m_iBestScore);

    copyInsG(m_pstBestInsG, m_pvStack->at(m_iSize - 1).pstInsG);
    m_vBestE.clear();
    m_vBestNewCanE.clear();
    for (i = 0; i < m_iSize; ++i)
    {
        m_vBestE.push_back(*(m_pvStack->at(i).itCurPos));
        //DEBUG_PRINTF("SHOW (%d, %d)\n", m_vBestE[i].first, m_vBestE[i].second);
        m_vBestNewCanE.insert(m_vBestNewCanE.end(),
                              m_pvStack->at(i).vCurNewCanE.begin(),
                              m_pvStack->at(i).vCurNewCanE.end());
    }
    return 0;
}
/*****************
input:
        vector<pair<int, int> > &vInsE
description:
        return score
******************/
int backtrack::run(vector<pair<int, int> > &vCanE, vector<pair<int, int> > &vInsE)
{
    int i = 0;
    bool bEnd = false;
    int iScore = 0;
    vector<int> vEmpty;
    DEBUG_ASSERT(m_pmyG->m_iMaxEId == m_pmyBackG->m_iMaxEId);
    DEBUG_ASSERT(m_iSize > 0);
    init(*m_pmyG);

    //showCanE(*m_pvCanE);
    /* end when the first level is ended */
    for (i = 0; i > -1; ++i)
    {
        /* stay at bottom */
        if (i >= m_iSize)
        {
            iScore = getScore();
            if (iScore > m_iBestScore)
            {
                m_iBestScore = iScore;
                saveBest();
            }
            --i;
        }
        //DEBUG_PRINTF("DEBUG start process\n");
        bEnd = process(i);
        //DEBUG_PRINTF("DEBUG process done\n");
        /* retur to last level */
        if (bEnd)
        {
            i -= 2;
        }
    }
    DEBUG_PRINTF("DEBUG current best score: %d\n", m_iBestScore);
    /* copy best graph */
    addG(*m_pmyG, m_pstBestInsG);
    addG(*m_pmyBackG, m_pstBestInsG);
    vInsE.insert(vInsE.end(), m_vBestE.begin(), m_vBestE.end());
    vCanE.insert(vCanE.end(), m_vBestNewCanE.begin(), m_vBestNewCanE.end());

    return m_iBestScore;
}
/*****************
input:
        vector<pair<int, int> > &vInsE
description:
        return score
******************/
int backtrack::tryRun(vector<pair<int, int> > &vCanE, vector<pair<int, int> > &vInsE)
{
    int i = 0;
    bool bEnd = false;
    int iScore = 0;
    vector<int> vEmpty;
    DEBUG_ASSERT(m_pmyG->m_iMaxEId == m_pmyBackG->m_iMaxEId);
    DEBUG_ASSERT(m_iSize > 0);
    init(*m_pmyG);

    //showCanE(*m_pvCanE);
    /* end when the first level is ended */
    for (i = 0; i > -1; ++i)
    {
        /* stay at bottom */
        if (i >= m_iSize)
        {
            iScore = getScore();
            if (iScore > m_iBestScore)
            {
                m_iBestScore = iScore;
                saveBest();
            }
            --i;
        }
        //DEBUG_PRINTF("DEBUG start process\n");
        bEnd = process(i);
        //DEBUG_PRINTF("DEBUG process done\n");
        /* retur to last level */
        if (bEnd)
        {
            i -= 2;
        }
    }
    /* save */
    vInsE.insert(vInsE.end(), m_vBestE.begin(), m_vBestE.end());

    return m_iBestScore;
}
/*****************
input:
        vector<pair<int, int> > &vInsE
description:
        return score
******************/
int backtrack::restoreG(myG &mpRunG, myG &mpBackG, vector<int> &vChgE)
{
    vector<int>::iterator itChgE;
    TPST_MAP_BY_EID* pstNode = NULL;

    /* restore */
    DEBUG_ASSERT(mpRunG.m_iMaxEId >= mpBackG.m_iMaxEId);

    /* restore to initial G */
    for (itChgE = vChgE.begin(); itChgE != vChgE.end(); ++itChgE)
    {
        pstNode = mpRunG.findNode(*itChgE);
        DEBUG_ASSERT(NULL != pstNode);

        if (pstNode->eid > mpBackG.m_iMaxEId)
        {
            mpRunG.simpleRm(pstNode->paXY.first, pstNode->paXY.second);
        }
        else
        {
            (*(mpRunG.m_pvG))[pstNode->eid] = (*(mpBackG.m_pvG))[pstNode->eid];
        }
    }
    mpRunG.m_iMaxEId = mpBackG.m_iMaxEId;
    mpRunG.m_iMaxD = mpBackG.m_iMaxD;
    mpRunG.m_iMaxK = mpBackG.m_iMaxK;
    mpRunG.m_pvG->resize(mpRunG.m_iMaxEId + 1);

    return 0;
}
/*****************
input:
        vector<pair<int, int> > &vInsE
description:
        return score
******************/
int backtrack::tryIns(myG &mpRunG, myG &mpBackG, int iDesK, vector<pair<int, int> > &vInsE)
{
    vector<pair<int, int> >::iterator itE;
    vector<int> vChgE;
    TPST_MAP_BY_EID* pstNode = NULL;
    TPST_MAP_BY_EID* pstTpNode = NULL;
    int iScore = 0;

    DEBUG_ASSERT(mpRunG.m_iMaxEId == mpBackG.m_iMaxEId);

    for (itE = vInsE.begin(); itE != vInsE.end(); ++itE)
    {
        vector<int> vTpChgE;
        insertPart::insertOne(mpRunG, itE->first, itE->second, vTpChgE);

        vChgE.insert(vChgE.begin(), vTpChgE.begin(), vTpChgE.end());
    }

    COMMON_UNIQUE(vChgE);
    /* calculate score */
    for (int iEid : vChgE)
    {
        pstNode = mpRunG.findNode(iEid);
        DEBUG_ASSERT(NULL != pstNode);

        if (pstNode->eid > mpBackG.m_iMaxEId)
        {
            if (pstNode->iTrussness >= iDesK)
            {
                ++iScore;
            }
        }
        else
        {
            pstTpNode = mpBackG.findNode(iEid);
            DEBUG_ASSERT(NULL != pstTpNode);
            if ((pstNode->iTrussness >= iDesK) && (pstTpNode->iTrussness < iDesK))
            {
                ++iScore;
            }
        }
    }
    /* restore */
    restoreG(mpRunG, mpBackG, vChgE);

    return iScore;
}

/*****************
input:
        vector<pair<int, int> > &vInsE
description:
        return score
******************/
int backtrack::tryIns(myG &mpRunG, myG &mpBackG, int iDesK, vector<pair<int, int> > &vInsE, vector<int> &vDesE)
{
    vector<pair<int, int> >::iterator itE;
    vector<int> vChgE;
    vector<int>::iterator itChgE;
    /* eid, none */
    map<int, int> mpChgE;
    map<int, int>::iterator itmpE;
    map<int, int>::iterator itmpEnd;
    TPST_MAP_BY_EID* pstNode = NULL;
    TPST_MAP_BY_EID* pstTpNode = NULL;
    int iScore = 0;

    DEBUG_ASSERT(mpRunG.m_iMaxEId == mpBackG.m_iMaxEId);

    for (itE = vInsE.begin(); itE != vInsE.end(); ++itE)
    {
        vector<int> vTpChgE;
        insertPart::insertOne(mpRunG, itE->first, itE->second, vTpChgE);
        for (itChgE = vTpChgE.begin(); itChgE != vTpChgE.end(); ++itChgE)
        {
            mpChgE[*itChgE] = 1;
        }
        vChgE.insert(vChgE.begin(), vTpChgE.begin(), vTpChgE.end());
    }
    COMMON_UNIQUE(vChgE);
    /* limit score */
    //DEBUG_PRINTF("DEBUG limit begin\n");
    for (itChgE = vDesE.begin(); itChgE != vDesE.end(); ++itChgE)
    {
        if (mpChgE.find(*itChgE) == mpChgE.end())
        {
            /* not changed, ignore */
            continue;
        }
        pstNode = mpRunG.findNode(*itChgE);
        DEBUG_ASSERT(NULL != pstNode);

        pstTpNode = mpBackG.findNode(pstNode->eid);
        DEBUG_ASSERT(NULL != pstTpNode);
        if ((pstNode->iTrussness >= iDesK) && (pstTpNode->iTrussness < iDesK))
        {
            ++iScore;
        }
        /*else
        {
            DEBUG_PRINTF("DEBUG_BT not meet (%d, %d) eid: %d k: %d layer: %d\n",
                         pstNode->paXY.first, pstNode->paXY.second, pstNode->eid,
                         pstTpNode->iTrussness, pstTpNode->iLayer);
        }*/
    }
    /* new edge */
    itmpE = mpChgE.upper_bound(mpBackG.m_iMaxEId);
    itmpEnd = mpChgE.upper_bound(mpRunG.m_iMaxEId);
    for (; itmpE !=itmpEnd; ++itmpE)
    {
        pstNode = mpRunG.findNode(itmpE->first);
        DEBUG_ASSERT(NULL != pstNode);
        if (pstNode->iTrussness >= iDesK)
        {
            ++iScore;
        }
        /*else
        {
            DEBUG_PRINTF("DEBUG_BT new edge not meet (%d, %d) eid: %d k: %d layer: %d\n",
                         pstNode->paXY.first, pstNode->paXY.second, pstNode->eid,
                         pstNode->iTrussness, pstNode->iLayer);
        }*/
    }
    //DEBUG_PRINTF("DEBUG limit end\n");
    /* restore */
    restoreG(mpRunG, mpBackG, vChgE);

    return iScore;
}
/*****************
input:
        vector<pair<int, int> > &vInsE
description:
        return score
******************/
int backtrack::tryIns(myG &mpRunG, myG &mpBackG, int iDesK, vector<pair<int, int> > &vInsE, vector<int> &vDesE, vector<pair<int, int> > &vSuccE, int *piRealScore)
{
    vector<pair<int, int> >::iterator itE;
    vector<int> vChgE;
    vector<int>::iterator itChgE;
    /* eid, none */
    map<int, int> mpChgE;
    map<int, int>::iterator itmpE;
    map<int, int>::iterator itmpEnd;
    TPST_MAP_BY_EID* pstNode = NULL;
    TPST_MAP_BY_EID* pstTpNode = NULL;
    int iScore = 0;
    int iRealScore = 0;

    DEBUG_ASSERT(mpRunG.m_iMaxEId == mpBackG.m_iMaxEId);

    for (itE = vInsE.begin(); itE != vInsE.end(); ++itE)
    {
        vector<int> vTpChgE;
        insertPart::insertOne(mpRunG, itE->first, itE->second, vTpChgE);
        for (itChgE = vTpChgE.begin(); itChgE != vTpChgE.end(); ++itChgE)
        {
            mpChgE[*itChgE] = 1;
        }
        vChgE.insert(vChgE.begin(), vTpChgE.begin(), vTpChgE.end());
    }
    COMMON_UNIQUE(vChgE);
    /* calculate real score */
    for (int iEid : vChgE)
    {
        pstNode = mpRunG.findNode(iEid);
        DEBUG_ASSERT(NULL != pstNode);

        if (pstNode->eid > mpBackG.m_iMaxEId)
        {
            if (pstNode->iTrussness >= iDesK)
            {
                ++iRealScore;
            }
        }
        else
        {
            pstTpNode = mpBackG.findNode(iEid);
            DEBUG_ASSERT(NULL != pstTpNode);
            if ((pstNode->iTrussness >= iDesK) && (pstTpNode->iTrussness < iDesK))
            {
                ++iRealScore;
            }
        }
    }
    /* limit score */
    //DEBUG_PRINTF("DEBUG limit begin\n");
    for (itChgE = vDesE.begin(); itChgE != vDesE.end(); ++itChgE)
    {
        if (mpChgE.find(*itChgE) == mpChgE.end())
        {
            /* not changed, ignore */
            //DEBUG_PRINTF("TRY_INSERT not change: %d\n", *itChgE);
            continue;
        }
        pstNode = mpRunG.findNode(*itChgE);
        DEBUG_ASSERT(NULL != pstNode);

        pstTpNode = mpBackG.findNode(pstNode->eid);
        DEBUG_ASSERT(NULL != pstTpNode);
        if ((pstNode->iTrussness >= iDesK) && (pstTpNode->iTrussness < iDesK))
        {
            ++iScore;
        }
#if 0
        else
        {
            vector <int>::iterator itLfE;
            vector <int>::iterator itRtE;
            DEBUG_PRINTF("TRY_INSERT not change: (%d, %d) k: %d L: %d\n",
                         pstNode->paXY.first, pstNode->paXY.second, pstNode->iTrussness, pstNode->iLayer);
            /*itLfE = pstNode->vLfE.begin();
            itRtE = pstNode->vRtE.begin();
            for (; itLfE != pstNode->vLfE.end(); ++itLfE, ++itRtE)
            {
                TPST_MAP_BY_EID* pstGLfNode = mpRunG.findNode(*itLfE);
                DEBUG_ASSERT(NULL != pstGLfNode);
                TPST_MAP_BY_EID* pstGRtNode = mpRunG.findNode(*itRtE);
                DEBUG_ASSERT(NULL != pstGRtNode);
                DEBUG_PRINTF("TRY_INSERT neighbors: (%d, %d) (%d, %d) group: %d %d k: %d %d L: %d %d\n",
                             pstGLfNode->paXY.first, pstGLfNode->paXY.second,
                             pstGRtNode->paXY.first, pstGRtNode->paXY.second,
                             pstGLfNode->iGroupId, pstGRtNode->iGroupId,
                             pstGLfNode->iTrussness, pstGRtNode->iTrussness,
                             pstGLfNode->iLayer, pstGRtNode->iLayer);
            }*/
        }
#endif
    }
    /* new edge */
    itmpE = mpChgE.upper_bound(mpBackG.m_iMaxEId);
    itmpEnd = mpChgE.upper_bound(mpRunG.m_iMaxEId);
    for (; itmpE !=itmpEnd; ++itmpE)
    {
        pstNode = mpRunG.findNode(itmpE->first);
        DEBUG_ASSERT(NULL != pstNode);
        if (pstNode->iTrussness >= iDesK)
        {
            ++iScore;
            vSuccE.push_back(pstNode->paXY);
        }
    }
    //DEBUG_PRINTF("DEBUG limit end\n");
    /* restore */
    restoreG(mpRunG, mpBackG, vChgE);

    *piRealScore = iRealScore;

    return iScore;
}
/*****************
input:
        vector<pair<int, int> > &vInsE
description:
        return score
******************/
int backtrack::batIns(myG &mpInitG, myG &mpBackG, vector<pair<int, int> > &vInsE)
{
    vector<int> vEmpty;
    vector<pair<int, int> >::iterator itE;
    int iRes = 0;

    DEBUG_ASSERT(mpInitG.m_iMaxEId == mpInitG.m_iMaxEId);

    for (itE = vInsE.begin(); itE != vInsE.end(); ++itE)
    {
        //DEBUG_PRINTF("BACKTRACK batch insert (%d, %d)\n", itE->first, itE->second);
        vEmpty.clear();
        iRes += insertPart::insertOne(mpInitG, itE->first, itE->second, vEmpty);
        vEmpty.clear();
        iRes += insertPart::insertOne(mpBackG, itE->first, itE->second, vEmpty);
    }

    return iRes;
}
