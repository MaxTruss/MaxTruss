/***************
my class G
****************/

#include "common.h"
#include <math.h>
#include "DAG.h"
#include "myG.h"
#include "Comp.h"
#include "insert.h"
#include "backtrack.h"

extern long g_lCanETime;
extern long g_lInsertTime;
extern long g_lDAGBuildTime;
extern long g_lCutTime;
extern long g_lKMinusTime;
extern long g_lRandomTime;

long g_lDAGTime;
long g_lDAGPathTime;
long g_lDAGDivideTime;

bool g_bFirstDAG;
/*****************
input:
        none
description:
        init object
        init stage
******************/
Comp::Comp(myG &oInitG, myG &oBackG, int iCurK, int iDesK)
{
    m_poG = &oInitG;
    m_poBackG = &oBackG;
    m_iDesK = iDesK;
    m_iCurK = iCurK;
    m_iRandomMax = COMP_RANDOM_MAX;
    //m_iMaxGroupId = 0;
    m_iMaxEId = 0;
    m_vG2L.resize(oInitG.m_iMaxEId + 1, 0);
    m_pvComp = new vector<TPST_COMP_NODE>();
    m_pvComp->reserve(oInitG.m_iMaxEId);
    add(0);
    construct();
    DEBUG_PRINTF("INIT construct done\n");

    m_iMaxGroupId = 0;
    divide();
    DEBUG_PRINTF("INIT divide done groups: %d\n", m_iMaxGroupId);
    buildLayer();
    DEBUG_PRINTF("INIT build layer done\n");
}
/*****************
input:
        myG &oInitG
description:
        copy map
        calculate stage
******************/
Comp::~Comp()
{
    m_iMaxEId = 0;

    m_pvComp->clear();

    delete m_pvComp;
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
int Comp::add(int iEid)
{
    TPST_COMP_NODE *pstNode = NULL;

    if(0 < iEid)
    {
        ++m_iMaxEId;
        m_pvComp->resize(m_iMaxEId + 1);
        pstNode = &(m_pvComp->at(m_iMaxEId));
        pstNode->iEid = m_iMaxEId;
        pstNode->iGEid = iEid;
        m_vG2L[iEid] = m_iMaxEId;
        pstNode->iGroupId = 0;
    }
    else
    {
        /* init */
        m_pvComp->resize(1);
        pstNode = &(m_pvComp->at(0));
        pstNode->iEid = 0;
        pstNode->iGEid = 0;
        m_vG2L[0] = 0;
        pstNode->iGroupId = 0;
        pstNode->bAban = true;
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
bool Comp::getAbaState(void *pComp, int iGroupId, int iGEid)
{
    Comp *poComp = (Comp *)pComp;
    TPST_COMP_NODE *pstNode = poComp->findNode(poComp->m_vG2L[iGEid]);
    return (pstNode->bAban || (iGroupId != pstNode->iGroupId ));
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
bool Comp::getAbaStateNoG(void *pComp, int iGroupId, int iGEid)
{
    Comp *poComp = (Comp *)pComp;
    TPST_COMP_NODE *pstNode = poComp->findNode(poComp->m_vG2L[iGEid]);
    return (pstNode->bAban);
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
int Comp::showGroup(int iGroupId)
{
    TPST_GROUP_INFO *pstCurGroup = NULL;
    TPST_COMP_NODE *pstNode = NULL;
    TPST_MAP_BY_EID *pstGNode = NULL;

    pstCurGroup = &(m_mpGroups[iGroupId]);
    DEBUG_ASSERT(iGroupId == pstCurGroup->iGroupId);

    /* pid, none */
    map<int, int> mpNodes;
    for (auto atE : pstCurGroup->vEdges)
    {
        pstNode = findNode(atE);
        pstGNode = m_poG->findNode(pstNode->iGEid);
        DEBUG_ASSERT(NULL != pstGNode);
        DEBUG_PRINTF("SHOW_G,%d,%d,%d,%d\n",
                     pstGNode->paXY.first, pstGNode->paXY.second, pstGNode->iTrussness, pstNode->iLayer);

        mpNodes[pstGNode->paXY.first];
        mpNodes[pstGNode->paXY.second];
    }
    for (auto atN : mpNodes)
    {
        vector<int> vNeibP;
        vector<int> vNeibE;
        int iNeibCnt = m_poG->findPNebP(atN.first, vNeibP, vNeibE);
        DEBUG_PRINTF("SHOW_G,p,%d",
                     atN.first);
        for (int i = 0; i < iNeibCnt; ++i)
        {
            TPST_MAP_BY_EID *pstNeib = m_poG->findNode(vNeibE[i]);
            DEBUG_PRINTF(";%d,%d,%d",
                         vNeibP[i], pstNeib->iTrussness, pstNeib->iLayer);
        }
        DEBUG_PRINTF("\n");
    }

    return 0;
}

/*****************
input:
        int x
        int y
        long double ldP
description:
        return max group id
******************/
int Comp::divideBlocks(int iGroupId)
{
    //map<int, TPST_GROUP_INFO >::iterator itGroup;
    TPST_GROUP_INFO* pstGroup = NULL;
    int iMinT = 0;
    int iMinL = 0;
    vector <int>::iterator itLfE;
    vector <int>::iterator itRtE;
    vector <int>::iterator itE;
    vector <int> vVisited;
    TPST_COMP_NODE *pstNode = NULL;
    TPST_COMP_NODE *pstLfNode = NULL;
    TPST_COMP_NODE *pstRtNode = NULL;
    TPST_MAP_BY_EID *pstGNode = NULL;
    TPST_MAP_BY_EID *pstGLfNode = NULL;
    TPST_MAP_BY_EID *pstGRtNode = NULL;

    TPST_BLOCK_NODE stEmpty = {0};
    TPST_BLOCK_NODE *pstBlock = NULL;

    pstGroup = &(m_mpGroups[iGroupId]);

    //DEBUG_PRINTF("DIVIDE_BLOCKS group: %d size: %d\n", iGroupId, pstGroup->vEdges.size());

    pstGroup->iMaxBlock = 0;
    pstGroup->vBlocks.clear();
    pstGroup->vBlocks.push_back(stEmpty);
    for (itE = pstGroup->vEdges.begin(); itE != pstGroup->vEdges.end(); ++itE)
    {
        //DEBUG_PRINTF("DIVIDE_BLOCKS max: %d current edge: %d\n", m_pvComp->size(), *itE);
        pstNode = &(m_pvComp->at(*itE));
        /*DEBUG_PRINTF("DIVIDE_BLOCKS current edge: %d group: %d layer: %d group: %d block: %d\n",
                     *itE, pstNode->iGroupId, pstNode->iLayer, pstNode->iGroupId, pstNode->iBlockId);*/

        if (0 < pstNode->iBlockId)
        {
            /* has given */
            continue;
        }
        /* generate new block */
        pstGroup->iMaxBlock++;
        pstGroup->vBlocks.push_back(stEmpty);
        pstBlock = &(pstGroup->vBlocks[pstGroup->iMaxBlock]);
        pstBlock->iBlockId = pstGroup->iMaxBlock;
        //pstBlock->iTopEid = *itE;
        pstBlock->iSize = 0;
        pstBlock->bAban = false;

        pstBlock->iLayer = pstNode->iLayer;
        pstNode->bVstFlag = true;
        vVisited.push_back(pstNode->iEid);

        //DEBUG_PRINTF("DIVIDE_BLOCKS block: %d layer: %d\n", pstBlock->iBlockId, pstBlock->iLayer);

        list<int> lsQ;
        lsQ.push_back(pstNode->iEid);
        //DEBUG_PRINTF("DIVIDE_BLOCKS push 0 eid: %d\n", pstNode->iEid);
        while (!lsQ.empty())
        {
            int iCurEid = lsQ.front();
            lsQ.pop_front();
            //DEBUG_PRINTF("DIVIDE_BLOCKS pop eid: %d\n", iCurEid);
            pstBlock->vEdges.push_back(iCurEid);
            pstBlock->iSize++;

            pstNode = &(m_pvComp->at(iCurEid));
            pstGNode = m_poG->findNode(pstNode->iGEid);
            DEBUG_ASSERT(NULL != pstGNode);

            if (0 != pstNode->iBlockId)
            {
                DEBUG_PRINTF("ERROR eid: %d group: %d block: %d\n",
                             pstNode->iEid, pstNode->iGroupId, pstNode->iBlockId);
                DEBUG_ASSERT(0);
            }

            pstNode->iBlockId = pstBlock->iBlockId;

            itLfE = pstGNode->vLfE.begin();
            itRtE = pstGNode->vRtE.begin();
            for (; itLfE != pstGNode->vLfE.end(); ++itLfE, ++itRtE)
            {
                pstGLfNode = m_poG->findNode(*itLfE);
                DEBUG_ASSERT(NULL != pstGLfNode);
                pstGRtNode = m_poG->findNode(*itRtE);
                DEBUG_ASSERT(NULL != pstGRtNode);
                pstLfNode = findNode(m_vG2L[pstGLfNode->eid]);
                DEBUG_ASSERT(NULL != pstLfNode);
                pstRtNode = findNode(m_vG2L[pstGRtNode->eid]);
                DEBUG_ASSERT(NULL != pstRtNode);

                iMinT = COMMON_MIN(pstGLfNode->iTrussness, pstGRtNode->iTrussness);
                if (iMinT >= m_iDesK)
                {
                    continue;
                }
                if (iMinT < m_iCurK)
                {
                    continue;
                }
                if ((pstGLfNode->iTrussness < m_iDesK) && (pstLfNode->iGroupId != iGroupId))
                {
                    continue;
                }
                if ((pstGRtNode->iTrussness < m_iDesK) && (pstRtNode->iGroupId != iGroupId))
                {
                    continue;
                }

                iMinL = 0;
                if (m_iDesK <= pstGLfNode->iTrussness)
                {
                    iMinL = pstRtNode->iLayer;
                    DEBUG_ASSERT(pstRtNode->iGroupId == iGroupId);
                }
                else if (m_iDesK <= pstGRtNode->iTrussness)
                {
                    iMinL = pstLfNode->iLayer;
                    DEBUG_ASSERT(pstLfNode->iGroupId == iGroupId);
                }
                else
                {
                    iMinL = COMMON_MIN(pstLfNode->iLayer, pstRtNode->iLayer);
                    DEBUG_ASSERT(pstLfNode->iGroupId == iGroupId);
                    DEBUG_ASSERT(pstRtNode->iGroupId == iGroupId);
                }
                if (iMinL != pstNode->iLayer)
                {
                    continue;
                }

                if ((0 < pstLfNode->iEid) &&
                    (pstNode->iLayer == pstLfNode->iLayer) &&
                    (!pstLfNode->bVstFlag))
                {
                    pstLfNode->bVstFlag = true;
                    vVisited.push_back(pstLfNode->iEid);
                    lsQ.push_back(pstLfNode->iEid);
                    //DEBUG_PRINTF("DIVIDE_BLOCKS push 1 eid: %d\n", pstLfNode->iEid);
                }
                if ((0 < pstRtNode->iEid) &&
                    (pstNode->iLayer == pstRtNode->iLayer) &&
                    (!pstRtNode->bVstFlag))
                {
                    pstRtNode->bVstFlag = true;
                    vVisited.push_back(pstRtNode->iEid);
                    lsQ.push_back(pstRtNode->iEid);
                    //DEBUG_PRINTF("DIVIDE_BLOCKS push 2 eid: %d\n", pstRtNode->iEid);
                }
            }
        }

        //DEBUG_PRINTF("DIVIDE_BLOCKS block size: %d\n", pstBlock->iSize);
    }

    /* restore */
    for (itE = vVisited.begin(); itE != vVisited.end(); ++itE)
    {
        pstNode = &(m_pvComp->at(*itE));
        pstNode->bVstFlag = false;
    }
    return pstGroup->iMaxBlock;
}

/*****************
input:
        int x
        int y
        long double ldP
description:
        return max group id
******************/
int Comp::divideBlocksStrong(int iGroupId)
{
    TPST_GROUP_INFO* pstGroup = NULL;
    int iMinT = 0;
    int iMinL = 0;
    vector <int>::iterator itLfE;
    vector <int>::iterator itRtE;
    vector <int>::iterator itE;
    vector <int> vVisited;
    TPST_COMP_NODE *pstNode = NULL;
    TPST_COMP_NODE *pstLfNode = NULL;
    TPST_COMP_NODE *pstRtNode = NULL;
    TPST_MAP_BY_EID *pstGNode = NULL;
    TPST_MAP_BY_EID *pstGLfNode = NULL;
    TPST_MAP_BY_EID *pstGRtNode = NULL;

    TPST_BLOCK_NODE stEmpty = {0};
    TPST_BLOCK_NODE *pstBlock = NULL;

    pstGroup = &(m_mpGroups[iGroupId]);

    //DEBUG_PRINTF("DIVIDE_BLOCKS group: %d size: %d\n", iGroupId, pstGroup->vEdges.size());

    pstGroup->iMaxBlock = 0;
    pstGroup->vBlocks.clear();
    pstGroup->vBlocks.push_back(stEmpty);
    for (itE = pstGroup->vEdges.begin(); itE != pstGroup->vEdges.end(); ++itE)
    {
        pstNode = &(m_pvComp->at(*itE));
        pstNode->iBlockId = 0;
    }
    for (itE = pstGroup->vEdges.begin(); itE != pstGroup->vEdges.end(); ++itE)
    {
        pstNode = &(m_pvComp->at(*itE));
        /*DEBUG_PRINTF("DIVIDE_BLOCKS current edge: %d group: %d layer: %d group: %d block: %d\n",
                     *itE, pstNode->iGroupId, pstNode->iLayer, pstNode->iGroupId, pstNode->iBlockId);*/

        if (0 < pstNode->iBlockId)
        {
            /* has given */
            continue;
        }
        /* generate new block */
        pstGroup->iMaxBlock++;
        pstGroup->vBlocks.push_back(stEmpty);
        pstBlock = &(pstGroup->vBlocks[pstGroup->iMaxBlock]);
        pstBlock->iBlockId = pstGroup->iMaxBlock;
        //pstBlock->iTopEid = *itE;
        pstBlock->iSize = 0;
        pstBlock->bAban = false;

        pstBlock->iLayer = pstNode->iLayer;
        pstNode->bVstFlag = true;
        vVisited.push_back(pstNode->iEid);

        //DEBUG_PRINTF("DIVIDE_BLOCKS block: %d layer: %d\n", pstBlock->iBlockId, pstBlock->iLayer);

        list<int> lsQ;
        lsQ.push_back(pstNode->iEid);
        //DEBUG_PRINTF("DIVIDE_BLOCKS push 0 eid: %d\n", pstNode->iEid);
        while (!lsQ.empty())
        {
            int iCurEid = lsQ.front();
            lsQ.pop_front();
            //DEBUG_PRINTF("DIVIDE_BLOCKS pop eid: %d\n", iCurEid);
            pstBlock->vEdges.push_back(iCurEid);
            pstBlock->iSize++;

            pstNode = &(m_pvComp->at(iCurEid));
            pstGNode = m_poG->findNode(pstNode->iGEid);
            DEBUG_ASSERT(NULL != pstGNode);

            if (0 != pstNode->iBlockId)
            {
                DEBUG_PRINTF("ERROR eid: %d group: %d block: %d\n",
                             pstNode->iEid, pstNode->iGroupId, pstNode->iBlockId);
                DEBUG_ASSERT(0);
            }

            pstNode->iBlockId = pstBlock->iBlockId;

            vector <int> vNodes;
            vector <int> vEdges;
            m_poG->findPNebP(pstGNode->paXY.first, vNodes, vEdges);
            m_poG->findPNebP(pstGNode->paXY.second, vNodes, vEdges);
            for (int iGEid : vEdges)
            {
                pstGNode = m_poG->findNode(iGEid);
                DEBUG_ASSERT(NULL != pstGNode);
                if (0 >= m_vG2L[pstGNode->eid])
                {
                    continue;
                }
                TPST_COMP_NODE *pstTpNode = findNode(m_vG2L[pstGNode->eid]);
                DEBUG_ASSERT(NULL != pstTpNode);

                if (iGroupId != pstTpNode->iGroupId)
                {
                    continue;
                }
                if (pstNode->iLayer != pstTpNode->iLayer)
                {
                    continue;
                }
                if (!pstTpNode->bVstFlag)
                {
                    lsQ.push_back(pstTpNode->iEid);
                    pstTpNode->bVstFlag = true;
                    vVisited.push_back(pstTpNode->iEid);
                }
            }
        }

        //DEBUG_PRINTF("DIVIDE_BLOCKS block size: %d\n", pstBlock->iSize);
    }

    /* restore */
    for (itE = vVisited.begin(); itE != vVisited.end(); ++itE)
    {
        pstNode = &(m_pvComp->at(*itE));
        pstNode->bVstFlag = false;
    }
    return pstGroup->iMaxBlock;
}
/*****************
input:
        int x
        int y
        long double ldP
description:
        return max group id
******************/
int Comp::tableRandom()
{
    map<int, TPST_GROUP_INFO >::iterator itGroup;
    int iBdt = 0;
    int iGroupId = 0;
    int iTopNum = 0;
    int iCurNum = 0;
    int iKSpCnt = 0;
    int iKMinusSpCnt = 0;

    /* control size */
    int iVaTotalBgt = 0;
    int iVaTotalScore = 0;
    int iIgnKGNum = 0;
    int iIgnKMGNum = 0;
    int iVaKGNum = 0;
    m_iAvgRatio = 0;
    /* budget, group id */
    myPriQueueBig<int, int>priQ;
    /* size, group id */
    myPriQueueBig<int, int>priKMinusQ;

	struct timeval tv;
	long lStartTime = 0;
	long lCurTime = 0;

    /* init */
    m_vScoreTable.resize(m_iMaxGroupId + 1);

    for (itGroup = m_mpGroups.begin(); itGroup != m_mpGroups.end(); ++itGroup)
    {
        iGroupId = itGroup->first;

        if (itGroup->second.iMaxBgt <= m_iTotalBudget)
        {
            int iScore = itGroup->second.vCanE.size() + itGroup->second.vEdges.size();
            m_vScoreTable[iGroupId][itGroup->second.iMaxBgt].iBgt = itGroup->second.iMaxBgt;
            m_vScoreTable[iGroupId][itGroup->second.iMaxBgt].iScore = iScore;
            m_vScoreTable[iGroupId][itGroup->second.iMaxBgt].vCanE = itGroup->second.vCanE;
        }
        //if ((1 < itGroup->second.iMaxBgt) && (2 < itGroup->second.vEdges.size()))
        if (2 < itGroup->second.vEdges.size())
        {
            /* may be divided */
            priQ.insertByOrder(itGroup->second.vEdges.size(),
                               iGroupId);
        }
    }
    while (!priQ.empty())
    {
        map<int, TPST_TP_SCORE> mpLcTable;
        map<int, TPST_TP_SCORE> mpTpTable;

        iGroupId = priQ.getTop();
        priQ.pop();

        itGroup = m_mpGroups.find(iGroupId);
        DEBUG_ASSERT(itGroup != m_mpGroups.end());

//        DEBUG_PRINTF("TABLE group: %d, size: %d total budget: %d bare: %d first: %d\n",
//                     iGroupId, itGroup->second.vEdges.size(), itGroup->second.iMaxBgt,
//                     itGroup->second.bBare, itGroup->second.vFirst.size());

        itGroup->second.bBroken = true;

        gettimeofday(&tv, NULL);
        lStartTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        tableChsRandom(iGroupId, COMMON_MIN(itGroup->second.iMaxBgt, m_iTotalBudget + 1), mpLcTable);
        gettimeofday(&tv, NULL);
        lCurTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        g_lRandomTime += lCurTime - lStartTime;

        /*if (itGroup->second.bBare)
        {
            int iUseBgt = tableChsBack(iGroupId, COMMON_MIN(itGroup->second.iMaxBgt, m_iTotalBudget + 1), mpLcTable);
        }*/
        int iUseBgt = tableChsBack(iGroupId, COMMON_MIN(itGroup->second.iMaxBgt, m_iTotalBudget + 1), mpLcTable);
        refineTable(iGroupId, mpLcTable);
    }


//    DEBUG_PRINTF("TABLE iValidKGNum: %d iVaTotalBgt: %d avg: %d\n",
//                 iVaKGNum, iVaTotalBgt, m_iAvgRatio);

    return m_iMaxGroupId;
}
/*****************
input:
        int x
        int y
        long double ldP
description:
        return max group id
******************/
int Comp::tableDAG()
{
    map<int, TPST_GROUP_INFO >::iterator itGroup;
    int iBdt = 0;
    int iGroupId = 0;
    int iTopNum = 0;
    int iCurNum = 0;
    int iKSpCnt = 0;
    int iKMinusSpCnt = 0;

    /* control size */
    int iVaTotalBgt = 0;
    int iVaTotalScore = 0;
    int iIgnKGNum = 0;
    int iIgnKMGNum = 0;
    int iVaKGNum = 0;
    m_iAvgRatio = 0;
    /* budget, group id */
    myPriQueueBig<int, int>priQ;
    /* size, group id */
    myPriQueueBig<int, int>priKMinusQ;

	struct timeval tv;
	long lStartTime = 0;
	long lCurTime = 0;

    /* init */
    m_vScoreTable.resize(m_iMaxGroupId + 1);

    for (itGroup = m_mpGroups.begin(); itGroup != m_mpGroups.end(); ++itGroup)
    {
        iGroupId = itGroup->first;

        //DEBUG_PRINTF("TABLE group: %d first: %d\n", iGroupId, itGroup->second.vFirst.size());

        /*DEBUG_PRINTF("SHOW group: %d, size: %d total budget: %d bare: %d\n",
                     iGroupId, itGroup->second.vEdges.size(), itGroup->second.iMaxBgt,
                     itGroup->second.bBare);*/

        if (itGroup->second.iMaxBgt <= m_iTotalBudget)
        {
            int iScore = itGroup->second.vCanE.size() + itGroup->second.vEdges.size();
            m_vScoreTable[iGroupId][itGroup->second.iMaxBgt].iBgt = itGroup->second.iMaxBgt;
            m_vScoreTable[iGroupId][itGroup->second.iMaxBgt].iScore = iScore;
            m_vScoreTable[iGroupId][itGroup->second.iMaxBgt].vCanE = itGroup->second.vCanE;
        }
        //if ((1 < itGroup->second.iMaxBgt) && (2 < itGroup->second.vEdges.size()))
        if (2 < itGroup->second.vEdges.size())
        {
            /* may be divided */
            priQ.insertByOrder(itGroup->second.vEdges.size(),
                               iGroupId);
        }
    }
    while (!priQ.empty())
    {
        map<int, TPST_TP_SCORE> mpLcTable;
        map<int, TPST_TP_SCORE> mpTpTable;

        iGroupId = priQ.getTop();
        priQ.pop();

        itGroup = m_mpGroups.find(iGroupId);
        DEBUG_ASSERT(itGroup != m_mpGroups.end());

        /*DEBUG_PRINTF("TABLE group: %d, size: %d total budget: %d bare: %d first: %d\n",
                     iGroupId, itGroup->second.vEdges.size(), itGroup->second.iMaxBgt,
                     itGroup->second.bBare, itGroup->second.vFirst.size());*/

        itGroup->second.bBroken = true;

        /*if (m_iCurK == m_iDesK - 1)
        {
            tableChsRandom(iGroupId, COMMON_MIN(itGroup->second.iMaxBgt, m_iTotalBudget + 1), mpLcTable);
        }*/

        gettimeofday(&tv, NULL);
        lStartTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        if (1 < itGroup->second.iMaxBgt)
        {
            tableChsDAG(iGroupId, COMMON_MIN(itGroup->second.iMaxBgt, m_iTotalBudget + 1), mpLcTable);
        }
        gettimeofday(&tv, NULL);
        lCurTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        g_lDAGTime += lCurTime - lStartTime;
        int iUseBgt = tableChsBack(iGroupId, COMMON_MIN(itGroup->second.iMaxBgt, m_iTotalBudget + 1), mpLcTable);
        /*if (itGroup->second.bBare)
        {
            int iUseBgt = tableChsBack(iGroupId, COMMON_MIN(itGroup->second.iMaxBgt, m_iTotalBudget + 1), mpLcTable);
        }*/
        //DEBUG_PRINTF("TABLE back done\n");
        refineTable(iGroupId, mpLcTable);
    }


    /*DEBUG_PRINTF("TABLE iValidKGNum: %d iVaTotalBgt: %d avg: %d\n",
                 iVaKGNum, iVaTotalBgt, m_iAvgRatio);*/
    return m_iMaxGroupId;
}
/*****************
input:
        vector<pair<int, int> > &vCanE
        int iBudget
description:
        return score
******************/
int Comp::insertAll(vector<pair<int, int> > &vCanE)
{
    int iCurBdt = 0;
    int iGroupNum = 0;
    int iGroupId = 0;
    int iScore = 0;
    int iBestScore = 0;
    int iBestGroup = 0;
    int iBestUpBgt = 0;

    TPST_GROUP_INFO *pstGroup = NULL;
    map<int, TPST_SCORE_INFO>::iterator itScoreT;

    /* local data */
    /* budget, <score, <group id> */
    map<int, map<int, vector<int> > > mpLcData;
    map<int, map<int, vector<int> > >::iterator itLc;
    map<int, vector<int> >::reverse_iterator ritLcScore;

    /* fill local data */
    vCanE.clear();
    for (iGroupId = 1; iGroupId <= m_iMaxGroupId; ++iGroupId)
    {
        pstGroup = &(m_mpGroups[iGroupId]);
        DEBUG_ASSERT(pstGroup->iGroupId == iGroupId);
        vCanE.insert(vCanE.end(), pstGroup->vCanE.begin(),
                     pstGroup->vCanE.end());
    }
    COMMON_UNIQUE(vCanE);
    /* recalculate score */
    iScore = backtrack::tryIns(*m_poG, *m_poBackG, m_iDesK, vCanE);
    DEBUG_PRINTF("DEBUG insert all real score: %d used budget: %d\n",
                 iScore, vCanE.size());
    backtrack::batIns(*m_poG, *m_poBackG, vCanE);

    return iScore;
}
/*****************
input:
        vector<pair<int, int> > &vCanE
        int iBudget
description:
        return score
******************/
int Comp::largeDPTable(vector<pair<int, int> > &vCanE, bool bAll)
{
    int iCurBdt = 0;
    int iGroupNum = 0;
    int iGroupId = 0;
    int iScore = 0;
    int iBestScore = 0;
    int iBestGroup = 0;
    int iBestUpBgt = 0;

    TPST_GROUP_INFO *pstGroup = NULL;
    map<int, TPST_SCORE_INFO>::iterator itScoreT;

    /* local data */
    /* budget, <score, <group id> */
    map<int, map<int, vector<int> > > mpLcData;
    map<int, map<int, vector<int> > >::iterator itLc;
    map<int, vector<int> >::reverse_iterator ritLcScore;

    int iAdqBudget = m_iTotalBudget;
    int iDepth = COMMON_MIN(m_iMaxGroupId, iAdqBudget);
    /* save space */
    m_vDPTable.resize(2);
    for (iGroupNum = 0; iGroupNum < m_vDPTable.size(); ++iGroupNum)
    {
        m_vDPTable[iGroupNum].resize(iAdqBudget + 1);
        for (iCurBdt = 0; iCurBdt <= iAdqBudget; ++iCurBdt)
        {
            m_vDPTable[iGroupNum][iCurBdt].iScore = 0;
            m_vDPTable[iGroupNum][iCurBdt].iGroupNum = 0;
            m_vDPTable[iGroupNum][iCurBdt].iRealBdt = 0;
            m_vDPTable[iGroupNum][iCurBdt].mpGroups.clear();
        }
    }

    vector<TPST_DP_INFO> *pPrevDP = &(m_vDPTable[0]);
    vector<TPST_DP_INFO> *pCurDP = &(m_vDPTable[1]);
    vector<TPST_DP_INFO> *pTempDP = NULL;
    /* fill local data */
    for (iGroupId = 1; iGroupId <= m_iMaxGroupId; ++iGroupId)
    {
        //DEBUG_PRINTF("LARGE_DP group: %d size: %d score: \n", iGroupId, m_mpGroups[iGroupId].vEdges.size());
        for (itScoreT = m_vScoreTable[iGroupId].begin(); itScoreT != m_vScoreTable[iGroupId].end(); ++itScoreT)
        {
            int iCurBgt = itScoreT->first;
            int iCurScore = itScoreT->second.iScore;
            mpLcData[iCurBgt][iCurScore].push_back(iGroupId);
            //DEBUG_PRINTF("(%d, %d) ", iCurBgt, iCurScore);
        }
        //DEBUG_PRINTF("\n");
    }

    //DEBUG_PRINTF("SHOW_DP\n");
    for (iGroupNum = 1; iGroupNum <= iDepth; ++iGroupNum)
    {
        for (iCurBdt = 1; iCurBdt <= iAdqBudget; ++iCurBdt)
        {
            //DEBUG_PRINTF("DEBUG now at [%d][%d]\n", iGroupNum, iCurBdt);
            /* direct upper cell */
            if ((*pCurDP)[iCurBdt].iScore < (*pPrevDP)[iCurBdt].iScore)
            {
                (*pCurDP)[iCurBdt] = (*pPrevDP)[iCurBdt];
            }

            if (iGroupNum > iCurBdt)
            {
                /* using upper cell directly */
                //DEBUG_PRINTF("%3d,", (*pCurDP)[iCurBdt].iScore);
                continue;
            }

            /* direct left cell */
            if ((*pCurDP)[iCurBdt].iScore < (*pCurDP)[iCurBdt - 1].iScore)
            {
                (*pCurDP)[iCurBdt] = (*pCurDP)[iCurBdt - 1];
            }

            /* left upper cell */
            iBestScore = (*pCurDP)[iCurBdt].iScore;
            iBestGroup = 0;
            iBestUpBgt = 0;
            for (itLc = mpLcData.begin(); itLc != mpLcData.end(); ++itLc)
            {
                int iUpBgt = iCurBdt - itLc->first;
                //DEBUG_PRINTF("DEBUG up at [%d][%d]\n", iGroupNum - 1, iUpBgt);
                if (0 > iUpBgt)
                {
                    break;
                }

                if (iGroupNum - 1 > iUpBgt)
                {
                    /* fake cell, ignore */
                    continue;
                }
                if ((*pPrevDP)[iUpBgt].iGroupNum != iGroupNum - 1)
                {
                    /* fake cell, ignore */
                    continue;
                }
                if ((*pPrevDP)[iUpBgt].iRealBdt != iUpBgt)
                {
                    /* fake cell, ignore */
                    continue;
                }
                //DEBUG_PRINTF("DEBUG enter\n");
                for (ritLcScore = itLc->second.rbegin(); ritLcScore != itLc->second.rend(); ++ritLcScore)
                {
                    int iLcScore = ritLcScore->first;
                    int iCurScore = 0;
                    int iChosGroup = 0;
                    bool bFound = false;

                    iCurScore = (*pPrevDP)[iUpBgt].iScore + iLcScore;
                    //DEBUG_PRINTF("DEBUG get score: %d\n", iCurScore);
                    if (iCurScore <= iBestScore)
                    {
                        /* no need try */
                        break;
                    }
                    for (auto atGroupId : ritLcScore->second)
                    {
                        //DEBUG_PRINTF("DEBUG try group: %d\n", atGroupId);
                        if ((*pPrevDP)[iUpBgt].mpGroups.find(atGroupId) ==
                            (*pPrevDP)[iUpBgt].mpGroups.end())
                        {
                            /* new, found */
                            iChosGroup = atGroupId;
                            bFound = true;
                            break;
                        }
                    }
                    //DEBUG_PRINTF("DEBUG found: %d\n", bFound);
                    if (bFound)
                    {
                        iBestScore = iCurScore;
                        iBestGroup = iChosGroup;
                        iBestUpBgt = iUpBgt;
                        /* found, no need to try next */
                        break;
                    }
                }
            }

            if ((*pCurDP)[iCurBdt].iScore < iBestScore)
            {
                (*pCurDP)[iCurBdt] = (*pPrevDP)[iBestUpBgt];
                (*pCurDP)[iCurBdt].iScore = iBestScore;
                (*pCurDP)[iCurBdt].iGroupNum = iGroupNum;
                (*pCurDP)[iCurBdt].iRealBdt = iCurBdt;
                (*pCurDP)[iCurBdt].mpGroups[iBestGroup] = iCurBdt - iBestUpBgt;
            }
            //DEBUG_PRINTF("%3d,", (*pCurDP)[iCurBdt].iScore);
            /* right cell, fill in advance */
            if ((*pCurDP)[iCurBdt].iGroupNum != iGroupNum)
            {
                /* fake cell, ignore */
                continue;
            }
            if ((*pCurDP)[iCurBdt].iRealBdt != iCurBdt)
            {
                /* fake cell, ignore */
                continue;
            }
            for (auto atGroup : (*pCurDP)[iCurBdt].mpGroups)
            {
                int iUsedBgt = atGroup.second;
                iGroupId = atGroup.first;

                pstGroup = &(m_mpGroups[iGroupId]);
                DEBUG_ASSERT(pstGroup->iGroupId == iGroupId);
                if (!pstGroup->bBroken)
                {
                    continue;
                }
                itScoreT = m_vScoreTable[iGroupId].find(iUsedBgt);
                DEBUG_ASSERT(itScoreT != m_vScoreTable[iGroupId].end());
                int iRemScore = (*pCurDP)[iCurBdt].iScore - itScoreT->second.iScore;
                ++itScoreT;
                for (; itScoreT != m_vScoreTable[iGroupId].end(); ++itScoreT)
                {
                    int iTpBgt = itScoreT->first;
                    int iTpScore = iRemScore + itScoreT->second.iScore;
                    int iUpdatedBgt = iCurBdt - iUsedBgt + iTpBgt;

                    DEBUG_ASSERT(iUpdatedBgt > iCurBdt);

                    if (iUpdatedBgt > m_iTotalBudget)
                    {
                        /* exceed */
                        break;
                    }
                    if (iTpScore > (*pCurDP)[iUpdatedBgt].iScore)
                    {
                        /* update */
                        (*pCurDP)[iUpdatedBgt] = (*pCurDP)[iCurBdt];
                        (*pCurDP)[iUpdatedBgt].iScore = iTpScore;
                        (*pCurDP)[iUpdatedBgt].iRealBdt = iUpdatedBgt;
                        (*pCurDP)[iUpdatedBgt].mpGroups[iGroupId] = iTpBgt;
                    }
                }
            }
        }
        //DEBUG_PRINTF("L: %d\n", iGroupNum);
        pTempDP = pPrevDP;
        pPrevDP = pCurDP;
        pCurDP = pTempDP;
    }
    //DEBUG_PRINTF("SHOW_DP\n");

    pCurDP = pPrevDP;
    int iUsedB = m_iTotalBudget;
    if (!bAll)
    {
        /* partial */
        int i = 0;
        float fMaxRatio = 0;
        for (i = 1; i <= m_iTotalBudget; ++i)
        {
            float fTpRatio = (*pCurDP)[i].iScore / i;
            fMaxRatio = COMMON_MAX(fMaxRatio, fTpRatio);
        }
        float fMinRatio = (*pCurDP)[m_iTotalBudget].iScore / m_iTotalBudget;
        float fCutRatio = fMinRatio + 0.1 * (fMaxRatio - fMinRatio);
        printf("DP ratio max: %f min: %f cut: %f\n", fMaxRatio, fMinRatio, fCutRatio);
        for (i = 0; i < m_iTotalBudget; ++i)
        {
            float fTpRatio = (*pCurDP)[m_iTotalBudget - i].iScore / (m_iTotalBudget - i);
            if (fTpRatio >= fCutRatio)
            {
                /* found */
                break;
            }
            float fMaj = (m_iTotalBudget - i + 0.0) / m_iTotalBudget;
            if (fMaj < 0.9)
            {
                /* found */
                break;
            }
        }
        if (i < m_iTotalBudget)
        {
            iUsedB = m_iTotalBudget - i;
        }
    }
    int iTpBgt = iUsedB;
    vector<pair<int, int> > vTpCanE;
    while (iTpBgt <= iAdqBudget)
    {
        //printf("DP iTpBgt: %d\n", iTpBgt);
        iScore = (*pCurDP)[iTpBgt].iScore;
        /* find candidature edges */
        vTpCanE.clear();
        for (auto atGroup : (*pCurDP)[iTpBgt].mpGroups)
        {
            vTpCanE.insert(vTpCanE.end(), m_vScoreTable[atGroup.first][atGroup.second].vCanE.begin(),
                         m_vScoreTable[atGroup.first][atGroup.second].vCanE.end());
        }
        COMMON_UNIQUE(vTpCanE);
        printf("DP iTpBgt: %d vTpCanE size: %d\n", iTpBgt, vTpCanE.size());
        if (vTpCanE.size() < iUsedB)
        {
            /* can use more budget */
            vCanE.swap(vTpCanE);
            ++iTpBgt;
        }
        else if (vTpCanE.size() > iUsedB)
        {
            /* exceed */
            break;
        }
        else
        {
            vCanE.swap(vTpCanE);
            break;
        }
    }
    /* recalculate score */
    iScore = backtrack::tryIns(*m_poG, *m_poBackG, m_iDesK, vCanE);
    DEBUG_PRINTF("DEBUG estimate score: %d real score: %d used budget: %d\n",
                 (*pCurDP)[iUsedB].iScore, iScore, vCanE.size());

    /* show choose groups */
    /*for (auto atGroup : (*pCurDP)[iUsedB].mpGroups)
    {
        pstGroup = &(m_mpGroups[atGroup.first]);
        DEBUG_ASSERT(pstGroup->iGroupId == atGroup.first);
        DEBUG_PRINTF("SHOW_DP choose group: %d budget: %d score: %d break: %d bare: %d\n",
                     atGroup.first, atGroup.second,
                     m_vScoreTable[atGroup.first][atGroup.second].iScore,
                     pstGroup->bBroken, pstGroup->bBare);
    }*/
    return iScore;
}

/*****************
input:
        vector<pair<int, int> > &vCanE
        int iBudget
description:
        return score
******************/
int Comp::normalDPTable(vector<pair<int, int> > &vCanE)
{
    int iCurBdt = 0;
    int iGroupNum = 0;
    int iScore = 0;

    TPST_GROUP_INFO *pstGroup = NULL;
    int iBudget = m_iTotalBudget;

    /* init, save space */
    m_vDPTable.resize(2);
    for (int i = 0; i < m_vDPTable.size(); ++i)
    {
        m_vDPTable[i].resize(iBudget + 1);
        for (iCurBdt = 0; iCurBdt <= iBudget; ++iCurBdt)
        {
            m_vDPTable[i][iCurBdt].iScore = 0;
            m_vDPTable[i][iCurBdt].iGroupNum = 0;
            m_vDPTable[i][iCurBdt].iRealBdt = 0;
            m_vDPTable[i][iCurBdt].mpGroups.clear();
        }
    }
    vector<TPST_DP_INFO> *pPrevDP = &(m_vDPTable[0]);
    vector<TPST_DP_INFO> *pCurDP = &(m_vDPTable[1]);
    vector<TPST_DP_INFO> *pTempDP = NULL;

    //DEBUG_PRINTF("SHOW_DP\n");
    for (iGroupNum = 1; iGroupNum <= m_iMaxGroupId; ++iGroupNum)
    {
        /* debug */
        /*DEBUG_PRINTF("LARGE_DP group: %d size: %d score: \n", iGroupNum, m_mpGroups[iGroupNum].vEdges.size());
        for (auto atScore : m_vScoreTable[iGroupNum])
        {
            int iCurBgt = atScore.first;
            int iCurScore = atScore.second.iScore;
            DEBUG_PRINTF("(%d, %d) ", iCurBgt, iCurScore);
        }
        DEBUG_PRINTF("\n");*/

        /* direct upper cell */
        //m_vDPTable[iGroupNum] = m_vDPTable[iGroupNum - 1];

        for (iCurBdt = 1; iCurBdt <= iBudget; ++iCurBdt)
        {
            int iBestScore = 0;
            int iBestBgt = 0;

            map<int, TPST_SCORE_INFO> *pmpScoreTable = &(m_vScoreTable[iGroupNum]);

            for (auto atScore : *pmpScoreTable)
            {
                int iTryBgt = atScore.first;
                int iCurScore = atScore.second.iScore;

                DEBUG_ASSERT(atScore.second.vCanE.size() == iTryBgt);

                if (iTryBgt > iCurBdt)
                {
                    /* exceed */
                    break;
                }

                /* fake cell */
                /*if (m_vDPTable[iGroupNum - 1][iCurBdt - iTryBgt].iRealBdt != iCurBdt - iTryBgt)
                {
                    continue;
                }*/

                int iTpScore = iCurScore + (*pPrevDP)[iCurBdt - iTryBgt].iScore;

                if (iTpScore > iBestScore)
                {
                    /* update */
                    iBestScore = iTpScore;
                    iBestBgt = iTryBgt;
                }
            }
            if (iBestScore > (*pPrevDP)[iCurBdt].iScore)
            {
                /* update */
                (*pCurDP)[iCurBdt] = (*pPrevDP)[iCurBdt - iBestBgt];
                (*pCurDP)[iCurBdt].iScore = iBestScore;

                (*pCurDP)[iCurBdt].mpGroups[iGroupNum] = iBestBgt;
                (*pCurDP)[iCurBdt].iRealBdt += iBestBgt;

                /* debug */
                /*if (m_vDPTable[iGroupNum][iCurBdt].mpGroups.size() !=
                    m_vDPTable[iGroupNum - 1][iCurBdt - iBestBgt].mpGroups.size() + 1)
                {
                    DEBUG_ASSERT(0);
                }
                int iSum = 0;
                for (auto atGroup : m_vDPTable[iGroupNum][iCurBdt].mpGroups)
                {
                    iSum += atGroup.second;
                }
                if (iCurBdt != iSum)
                {
                    DEBUG_PRINTF("ERROR DP[%d][%d]: \n",
                                 iGroupNum, iCurBdt);
                    for (auto atGroup : m_vDPTable[iGroupNum][iCurBdt].mpGroups)
                    {
                        DEBUG_PRINTF("ERROR group: %d budget: %d\n",
                                     atGroup.first, atGroup.second);
                    }
                    DEBUG_ASSERT(0);
                }*/
            }
            else
            {
                (*pCurDP)[iCurBdt] = (*pPrevDP)[iCurBdt];
            }
            //DEBUG_PRINTF("%3d,", m_vDPTable[iGroupNum][iCurBdt].iScore);
        }
        //DEBUG_PRINTF("L: %d\n", iGroupNum);
        // move on
        pTempDP = pPrevDP;
        pPrevDP = pCurDP;
        pCurDP = pTempDP;
    }
    //DEBUG_PRINTF("SHOW_DP\n");

    pCurDP = pPrevDP;
    vCanE.clear();
    /* find candidature edges */
    for (auto atGroup : (*pCurDP)[iBudget].mpGroups)
    {
        DEBUG_ASSERT(atGroup.second == m_vScoreTable[atGroup.first][atGroup.second].vCanE.size());
        vCanE.insert(vCanE.end(), m_vScoreTable[atGroup.first][atGroup.second].vCanE.begin(),
                     m_vScoreTable[atGroup.first][atGroup.second].vCanE.end());
    }
    DEBUG_PRINTF("NORMAL_DP total budget: %d estimate: %d get: %d\n",
                 iBudget, (*pCurDP)[iBudget].iRealBdt, vCanE.size());
    COMMON_UNIQUE(vCanE);
    /* recalculate score */
    iScore = backtrack::tryIns(*m_poG, *m_poBackG, m_iDesK, vCanE);
    DEBUG_PRINTF("NORMAL_DP estimate score: %d budget: %d real score: %d used budget: %d\n",
                 (*pCurDP)[iBudget].iScore, iBudget, iScore, vCanE.size());
    /* show choose groups */
    /*for (auto atGroup : m_vDPTable[iBestGroupId][iBudget].mpGroups)
    {
        pstGroup = &(m_mpGroups[atGroup.first]);
        DEBUG_ASSERT(pstGroup->iGroupId == atGroup.first);
        DEBUG_PRINTF("SHOW_DP choose group: %d budget: %d score: %d break: %d bare: %d rank: %d\n",
                     atGroup.first, atGroup.second,
                     m_vScoreTable[atGroup.first][atGroup.second].iScore,
                     pstGroup->bBroken, pstGroup->bBare,
                     pstGroup->vEdges.size());
    }*/
    return iScore;
}
/*****************
input:
        myG &oInitG
        int iDesK
        int iEid
        vector<int> &vNodes
description:
        find k+1 nodes
******************/
int Comp::findKPlusNodes(myG &oInitG, int iDesK, int iEid, vector<int> &vNodes)
{
    TPST_MAP_BY_EID *pstNode = NULL;
    TPST_MAP_BY_EID *pstLfNode = NULL;
    TPST_MAP_BY_EID *pstRtNode = NULL;
    TPST_MAP_BY_EID *pstLcNode = NULL;
    TPST_MAP_BY_EID *pstLcLfNode = NULL;
    TPST_MAP_BY_EID *pstLcRtNode = NULL;

    /* pid, none */
    map <int, int> mpChosNodesPool;
    /* pid, cnt */
    map <int, int> mpNodePool;
    vector <int>::iterator itLfE;
    vector <int>::iterator itRtE;

    vector <int>::iterator itLcLfE;
    vector <int>::iterator itLcRtE;
    list <int> lsTpE;
    list <int>::iterator itTpE;

    int iMinT = 0;

    /* first level 2-hop triangle */

    pstNode = oInitG.findNode(iEid);
    DEBUG_ASSERT(NULL != pstNode);
    /*DEBUG_PRINTF("FIND_NODES self: (%d, %d) k: %d layer: %d target k: %d\n",
                 pstNode->paXY.first, pstNode->paXY.second,
                 pstNode->iTrussness, pstNode->iLayer, iDesK);*/
    mpChosNodesPool[pstNode->paXY.first] = 0;
    mpChosNodesPool[pstNode->paXY.second] = 0;
    mpNodePool[pstNode->paXY.first] = 100;
    mpNodePool[pstNode->paXY.second] = 100;

    itLfE = pstNode->vLfE.begin();
    itRtE = pstNode->vRtE.begin();
    for (; itLfE != pstNode->vLfE.end(); ++itLfE, ++itRtE)
    {
        pstLfNode = oInitG.findNode(*itLfE);
        DEBUG_ASSERT(NULL != pstLfNode);
        pstRtNode = oInitG.findNode(*itRtE);
        DEBUG_ASSERT(NULL != pstRtNode);

        int iTrdN = pstLfNode->paXY.first;
        if (iTrdN == pstNode->paXY.first)
        {
            iTrdN = pstLfNode->paXY.second;
        }
        mpNodePool[iTrdN] = 2;
    }

    /* find extra nodes */
    itLfE = pstNode->vLfE.begin();
    itRtE = pstNode->vRtE.begin();
    for (; itLfE != pstNode->vLfE.end(); ++itLfE, ++itRtE)
    {
        pstLfNode = oInitG.findNode(*itLfE);
        DEBUG_ASSERT(NULL != pstLfNode);
        pstRtNode = oInitG.findNode(*itRtE);
        DEBUG_ASSERT(NULL != pstRtNode);

        /* count neighbor */
        lsTpE.push_back(pstLfNode->eid);
        lsTpE.push_back(pstRtNode->eid);

    }
    for (itTpE = lsTpE.begin(); itTpE != lsTpE.end(); ++itTpE)
    {
        //DEBUG_PRINTF("FIND_NODES get: %d\n", *itTpE);
        pstLcNode = oInitG.findNode(*itTpE);
        DEBUG_ASSERT(NULL != pstLcNode);

        itLcLfE = pstLcNode->vLfE.begin();
        itLcRtE = pstLcNode->vRtE.begin();
        for (; itLcLfE != pstLcNode->vLfE.end(); ++itLcLfE, ++itLcRtE)
        {
            pstLcLfNode = oInitG.findNode(*itLcLfE);
            DEBUG_ASSERT(NULL != pstLcLfNode);
            pstLcRtNode = oInitG.findNode(*itLcRtE);
            DEBUG_ASSERT(NULL != pstLcRtNode);

            /*DEBUG_PRINTF("FIND_NODES neib left: (%d, %d) k: %d layer: %d\n",
                         pstLcLfNode->paXY.first, pstLcLfNode->paXY.second,
                    pstLcLfNode->iTrussness, pstLcLfNode->iLayer);
            DEBUG_PRINTF("FIND_NODES neib right: (%d, %d) k: %d layer: %d\n",
                         pstLcRtNode->paXY.first, pstLcRtNode->paXY.second,
                    pstLcRtNode->iTrussness, pstLcRtNode->iLayer);*/

            int iTrdN = pstLcLfNode->paXY.first;
            if (iTrdN == pstLcNode->paXY.first)
            {
                iTrdN = pstLcLfNode->paXY.second;
            }
            if (mpNodePool.find(iTrdN) == mpNodePool.end())
            {
                mpNodePool[iTrdN] = 2;
            }
            else
            {
                mpNodePool[iTrdN]++;
            }
        }
    }
    /*DEBUG_PRINTF("FIND_NODES level 1 size: %d\n",
                 mpNodePool.size());*/
    /* second level, 1-hop edge */
    if (mpNodePool.size() < iDesK)
    {
        vector<int> vTpN;
        for (auto atP : mpNodePool)
        {
            vTpN.push_back(atP.first);
        }
        for (auto atP : vTpN)
        {
            vector<int> vTpNeib;
            vector<int> vTpNeibE;
            oInitG.findPNebP(atP, vTpNeib, vTpNeibE);
            for (int i = 0; i < vTpNeib.size(); ++i)
            {
                if (mpNodePool.find(vTpNeib[i]) == mpNodePool.end())
                {
                    mpNodePool[vTpNeib[i]] = 1;
                }
            }
        }
    }
    /*DEBUG_PRINTF("FIND_NODES level 2 size: %d\n",
                 mpNodePool.size());*/
    /* third level, nodes in the graph */
    int iTpEid = 1;
    while (mpNodePool.size() < iDesK)
    {
        for (; iTpEid <= oInitG.m_iMaxEId; ++iTpEid )
        {
            pstLcNode = oInitG.findNode(iTpEid);
            DEBUG_ASSERT(NULL != pstLcNode);
            if (pstLcNode->eid != iTpEid)
            {
                continue;
            }
            if (pstLcNode->iTrussness > iDesK)
            {
                continue;
            }
            if (mpNodePool.find(pstLcNode->paXY.first) == mpNodePool.end())
            {
                mpNodePool[pstLcNode->paXY.first] = 0;
                break;
            }
            if (mpNodePool.find(pstLcNode->paXY.second) == mpNodePool.end())
            {
                mpNodePool[pstLcNode->paXY.second] = 0;
                break;
            }
        }
    }
    /*DEBUG_PRINTF("FIND_NODES level 3 size: %d\n",
                 mpNodePool.size());*/
    /* order */
    int iChsNum = 2;
    DEBUG_ASSERT(vNodes.empty());
    vNodes.push_back(pstNode->paXY.first);
    vNodes.push_back(pstNode->paXY.second);

    map<pair<int, int>, int> mpOrder;
    map<pair<int, int>, int>::reverse_iterator ritP;
    for (auto atN : mpNodePool)
    {
        if (mpChosNodesPool.find(atN.first) != mpChosNodesPool.end())
        {
            continue;
        }
        mpOrder[pair<int, int>(atN.second, atN.first)] = 0;
        /*DEBUG_PRINTF("FIND_NODES order: %d rank: %d\n",
                 atN.first, atN.second);*/
    }
    for (ritP = mpOrder.rbegin(); ritP != mpOrder.rend(); ++ritP)
    {
        vNodes.push_back(ritP->first.second);
        ++iChsNum;
        if (iChsNum >= iDesK)
        {
            break;
        }
    }
    DEBUG_ASSERT(iChsNum >= iDesK);
    /*DEBUG_PRINTF("FIND_NODES final chosen size: %d\n",
                 vNodes.size());*/
    return vNodes.size();
}
#if 0
/*****************
input:
        int x
        int y
        long double ldP
description:
        return max group id
******************/
int Comp::findKPlusNodesDrt(myG &oInitG, int iDesK, int iEid, vector<int> &vNodes)
{
    TPST_MAP_BY_EID *pstNode = NULL;
    TPST_MAP_BY_EID *pstLfNode = NULL;
    TPST_MAP_BY_EID *pstRtNode = NULL;
    TPST_MAP_BY_EID *pstLcNode = NULL;
    TPST_MAP_BY_EID *pstLcLfNode = NULL;
    TPST_MAP_BY_EID *pstLcRtNode = NULL;

    /* eid, state */
    map <int, int> mpEdgesPool;
    /* pid, none */
    map <int, int> mpChosNodesPool;
    /* pid, cnt */
    map <int, int> mpCanNodesPool;
    /* cnt, <pid> */
    map <int, vector<int> > mpNodesRank;
    map <int, vector<int> >::reverse_iterator ritRank;
    map <int, int>::iterator itmpP;
    vector <int>::iterator itLfE;
    vector <int>::iterator itRtE;

    vector <int>::iterator itLcLfE;
    vector <int>::iterator itLcRtE;
    list <int> lsTpE;
    list <int>::iterator itTpE;


    int iMinT = 0;
    int iMaxCnt = 0;
    int iMaxP = 0;

    /* fill mpChosNodesPool, k nodes */
    pstNode = oInitG.findNode(iEid);
    DEBUG_ASSERT(NULL != pstNode);
    //DEBUG_PRINTF("FIND_NODES start (%d, %d)\n", pstNode->paXY.first, pstNode->paXY.second);

    itLfE = pstNode->vLfE.begin();
    itRtE = pstNode->vRtE.begin();
    for (; itLfE != pstNode->vLfE.end(); ++itLfE, ++itRtE)
    {
        pstLfNode = oInitG.findNode(*itLfE);
        DEBUG_ASSERT(NULL != pstLfNode);
        pstRtNode = oInitG.findNode(*itRtE);
        DEBUG_ASSERT(NULL != pstRtNode);

        iMinT = COMMON_MIN(pstLfNode->iTrussness, pstRtNode->iTrussness);
        if (iMinT < pstNode->iTrussness)
        {
            continue;
        }
        /* unused */
        mpEdgesPool[pstLfNode->eid] = 1;
        mpEdgesPool[pstRtNode->eid] = 1;
        lsTpE.push_back(pstLfNode->eid);
        lsTpE.push_back(pstRtNode->eid);
        /* any one is ok */
        mpChosNodesPool[pstLfNode->paXY.first] = 0;
        mpChosNodesPool[pstLfNode->paXY.second] = 0;
        //DEBUG_PRINTF("FIND_NODES init choose (%d, %d)\n", pstLfNode->paXY.first, pstLfNode->paXY.second);
    }
    /* used */
    mpEdgesPool[iEid] = 2;
    lsTpE.push_back(iEid);
    mpChosNodesPool[pstNode->paXY.first] = 1;
    mpChosNodesPool[pstNode->paXY.second] = 1;
    DEBUG_ASSERT(vNodes.empty());
    vNodes.push_back(pstNode->paXY.first);
    vNodes.push_back(pstNode->paXY.second);
    if (mpChosNodesPool.size() <= iDesK)
    {
        /* find extra node */
        for (itTpE = lsTpE.begin(); itTpE != lsTpE.end(); ++itTpE)
        {
            //DEBUG_PRINTF("FIND_NODES get: %d\n", *itTpE);
            pstLcNode = oInitG.findNode(*itTpE);
            DEBUG_ASSERT(NULL != pstLcNode);
            //DEBUG_PRINTF("FIND_NODES current (%d, %d)\n", pstLcNode->paXY.first, pstLcNode->paXY.second);

            mpEdgesPool[pstLcNode->eid] = 2;

            itLcLfE = pstLcNode->vLfE.begin();
            itLcRtE = pstLcNode->vRtE.begin();
            for (; itLcLfE != pstLcNode->vLfE.end(); ++itLcLfE, ++itLcRtE)
            {
                pstLcLfNode = oInitG.findNode(*itLcLfE);
                DEBUG_ASSERT(NULL != pstLcLfNode);
                pstLcRtNode = oInitG.findNode(*itLcRtE);
                DEBUG_ASSERT(NULL != pstLcRtNode);

                /*DEBUG_PRINTF("FIND_NODES neib left: (%d, %d) k: %d layer: %d\n",
                             pstLcLfNode->paXY.first, pstLcLfNode->paXY.second,
                        pstLcLfNode->iTrussness, pstLcLfNode->iLayer);
                DEBUG_PRINTF("FIND_NODES neib right: (%d, %d) k: %d layer: %d\n",
                             pstLcRtNode->paXY.first, pstLcRtNode->paXY.second,
                        pstLcRtNode->iTrussness, pstLcRtNode->iLayer);*/

                if (mpChosNodesPool.find(pstLcLfNode->paXY.first) == mpChosNodesPool.end())
                {
                    if (mpCanNodesPool.find(pstLcLfNode->paXY.first) == mpCanNodesPool.end())
                    {
                        mpCanNodesPool[pstLcLfNode->paXY.first] = pstLcLfNode->vLfE.size();
                        //DEBUG_PRINTF("FIND_NODES new %d\n", pstLcLfNode->paXY.first);
                    }
                }
                if (mpChosNodesPool.find(pstLcLfNode->paXY.second) == mpChosNodesPool.end())
                {
                    /* again */
                    if (mpCanNodesPool.find(pstLcLfNode->paXY.second) == mpCanNodesPool.end())
                    {
                        mpCanNodesPool[pstLcLfNode->paXY.second] = pstLcLfNode->vLfE.size();
                        //DEBUG_PRINTF("FIND_NODES new %d\n", pstLcLfNode->paXY.second);
                    }
                }
                if (mpCanNodesPool.size() + mpChosNodesPool.size() <= iDesK)
                {
                    /* need more */
                    if (mpEdgesPool.find(pstLcLfNode->eid) == mpEdgesPool.end())
                    {
                        mpEdgesPool[pstLcLfNode->eid] = 1;
                        lsTpE.push_back(pstLcLfNode->eid);
                        /*DEBUG_PRINTF("FIND_NODES add 1 left: (%d, %d) k: %d layer: %d\n",
                                     pstLcLfNode->paXY.first, pstLcLfNode->paXY.second,
                                pstLcLfNode->iTrussness, pstLcLfNode->iLayer);*/
                    }
                    if (mpEdgesPool.find(pstLcRtNode->eid) == mpEdgesPool.end())
                    {
                        mpEdgesPool[pstLcRtNode->eid] = 1;
                        lsTpE.push_back(pstLcRtNode->eid);
                        /*DEBUG_PRINTF("FIND_NODES add 1 left: (%d, %d) k: %d layer: %d\n",
                                     pstLcRtNode->paXY.first, pstLcRtNode->paXY.second,
                                pstLcRtNode->iTrussness, pstLcRtNode->iLayer);*/
                    }
                }
            }
        }

    }
    for (itmpP = mpChosNodesPool.begin(); itmpP != mpChosNodesPool.end(); ++itmpP)
    {
        if (0 == itmpP->second)
        {
            /* not joined */
            vNodes.push_back(itmpP->first);
            //DEBUG_PRINTF("FIND_NODES 1 eid: %d push %d\n", iEid, itmpP->first);
        }
    }
    if (vNodes.size() <= iDesK)
    {
        /* find extra nodes */
        for (itmpP = mpCanNodesPool.begin(); itmpP != mpCanNodesPool.end(); ++itmpP)
        {
            //DEBUG_PRINTF("FIND_NODES p: %d cnt: %d\n", itmpP->first, itmpP->second);
            mpNodesRank[itmpP->second].push_back(itmpP->first);
        }
        for (ritRank = mpNodesRank.rbegin(); ritRank != mpNodesRank.rend(); ++ritRank)
        {
            for (auto atE : ritRank->second)
            {
                vNodes.push_back(atE);
                //DEBUG_PRINTF("FIND_NODES 2 eid: %d push %d\n", iEid, atE);
                if (vNodes.size() > iDesK)
                {
                    //DEBUG_PRINTF("FIND_NODES get1 size: %d\n", vNodes.size());
                    return vNodes.size();
                }
            }
        }

    }
    if (vNodes.size() <= iDesK)
    {
        return 0;
    }
    vNodes.resize(iDesK + 1);
    //DEBUG_PRINTF("FIND_NODES get size: %d\n", vNodes.size());

    return vNodes.size();
}
#endif
/*****************
input:
        int x
        int y
        long double ldP
description:
        return max group id
******************/
int Comp::tableChsRandom(int iGroupId, int iMaxBudget, map<int, TPST_TP_SCORE> &mpLcTable)
{
    TPST_GROUP_INFO *pstCurGroup = NULL;
    int iTotalBatch = COMP_RANDOM_MAX;
    int iRandomId = 0;
    vector<pair<int, int> > vCanEPool;
    vector<pair<int, int> > vCanE;
    vector<pair<int, int> > vSuccE;
    vector <int> vGEdges;
    map<int, TPST_TP_SCORE>::iterator itmpLc;

	struct timeval tv;
	long lStartTime = 0;
	long lCurTime = 0;

    findCanE(iGroupId, vCanEPool);

    if (vCanEPool.empty())
    {
        return 0;
    }

    pstCurGroup = &(m_mpGroups[iGroupId]);
    DEBUG_ASSERT(iGroupId == pstCurGroup->iGroupId);
    vGEdges.resize(pstCurGroup->vEdges.size());
    for (int i = 0; i < pstCurGroup->vEdges.size(); ++i)
    {
        TPST_COMP_NODE *pstE = findNode(pstCurGroup->vEdges[i]);
        vGEdges[i] = pstE->iGEid;
    }
    iTotalBatch = sqrt(m_iDesK * pstCurGroup->vEdges.size()* vCanEPool.size() + 1);
    int iMaxOpt = vCanEPool.size() * (vCanEPool.size() - 1) / 2;
    if (iTotalBatch > iMaxOpt)
    {
        iTotalBatch = iMaxOpt;
    }
    iTotalBatch = COMMON_MIN(iTotalBatch, m_iRandomMax);

    /*DEBUG_PRINTF("DEBUG group: %d edges: %d pool size: %d random count: %d\n",
                 iGroupId, pstCurGroup->vEdges.size(), vCanEPool.size(), iTotalBatch);*/
    for (int iBat = 0; iBat < iTotalBatch; ++iBat)
    {
        int iBdt = myRandom::random(2 * iMaxBudget) + 1;
        /* try a random set */
        vCanE.clear();
        for (int j = 0; j < iBdt; ++j)
        {
            iRandomId = myRandom::random((int)vCanEPool.size());
            vCanE.push_back(vCanEPool[iRandomId]);
        }
        vSuccE.clear();
        int iRealScore = 0;
        gettimeofday(&tv, NULL);
        lStartTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        int iScore = backtrack::tryIns(*m_poG, *m_poBackG, m_iDesK, vCanE, vGEdges, vSuccE, &iRealScore);
        gettimeofday(&tv, NULL);
        lCurTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        g_lInsertTime += lCurTime - lStartTime;
        //printf("RANDOM insert: %d get success: %d score: %d\n", vCanE.size(), vSuccE.size(), iScore);

        if (vSuccE.size() > iMaxBudget)
        {
            continue;
        }
        itmpLc = mpLcTable.find(vSuccE.size());
        if (itmpLc != mpLcTable.end())
        {
            if (iScore < itmpLc->second.iScore)
            {
                continue;
            }
            else if (iScore == itmpLc->second.iScore)
            {
                if (iRealScore <= itmpLc->second.iRealScore)
                {
                    continue;
                }
            }
        }
        mpLcTable[vSuccE.size()] = (TPST_TP_SCORE){iScore, iRealScore, vSuccE};
    }

//    int iPrevScore = 0;
//    for (itmpLc = mpLcTable.begin(); itmpLc != mpLcTable.end(); ++itmpLc)
//    {
//        int iCurScore = itmpLc->second.first;
//        if (iCurScore > iPrevScore)
//        {
//            iPrevScore = iCurScore;
//            DEBUG_PRINTF("(%d, %d) ", itmpLc->first, iCurScore);
//        }
//    }
//    DEBUG_PRINTF("\n");
    //DEBUG_ASSERT(0);
    return 0;
}

/*****************
input:
        int x
        int y
        long double ldP
description:
        return max group id
******************/
int Comp::tableChsBack(int iGroupId, int iMaxBudget, map<int, TPST_TP_SCORE> &mpLcTable)
{
    vector<int> vDes;
    vector <int> vGEdges;

    map<int, int> mpEdges;
    TPST_GROUP_INFO *pstCurGroup = NULL;
    pstCurGroup = &(m_mpGroups[iGroupId]);
    vDes.reserve(pstCurGroup->vEdges.size());
    vGEdges.resize(pstCurGroup->vEdges.size());
    for (int i = 0; i < pstCurGroup->vEdges.size(); ++i)
    {
        TPST_COMP_NODE *pstNode = findNode(pstCurGroup->vEdges[i]);

        mpEdges[pstNode->iGEid] = pstCurGroup->vEdges[i];
        vGEdges[i] = pstNode->iGEid;
    }
    int iUsedBgt = 0;
    int iAbanCnt = 0;
    while (iAbanCnt < pstCurGroup->vEdges.size())
    {
        vector<int> vBareE;
        vector<pair<int, int> > vCanE;

        vDes.clear();
        for (auto atN : mpEdges)
        {
            vDes.push_back(atN.first);
        }
        bool bBare = findKTrussCanETry(*m_poG, *m_poBackG, m_iCurK, m_iDesK, vDes, vCanE, this, vBareE, iMaxBudget);
        /*DEBUG_PRINTF("BACK group: %d bare: %d abandon size: %d\n",
                     iGroupId, bBare, vBareE.size());*/

        if (bBare)
        {
            if (vBareE.empty())
            {
                /* exceed budget */
                return 0;
            }
            for (int iGEid : vBareE)
            {
                DEBUG_ASSERT(mpEdges.find(iGEid) != mpEdges.end());

                TPST_COMP_NODE *pstNode = findNode(mpEdges[iGEid]);
                pstNode->bAban = true;
                iAbanCnt++;
                mpEdges.erase(iGEid);
                TPST_MAP_BY_EID *pstGNode = m_poG->findNode(iGEid);
                //DEBUG_PRINTF("BACK abandon (%d, %d)\n", pstGNode->paXY.first, pstGNode->paXY.second);
            }
        }
        else
        {
            if (!vCanE.empty() && (vCanE.size() <= iMaxBudget))
            {
                int iRealScore = 0;
                vector<pair<int, int> > vSuccE;
                int iScore = backtrack::tryIns(*m_poG, *m_poBackG, m_iDesK, vCanE, vGEdges, vSuccE, &iRealScore);
                //int iScore = vCanE.size() + pstCurGroup->vEdges.size() - iAbanCnt;
                /*DEBUG_PRINTF("BACK group: %d score: %d iRealScore: %d budget: %d\n",
                                     iGroupId, iScore, iRealScore, vSuccE.size());
                DEBUG_ASSERT(0);*/
                map<int, TPST_TP_SCORE>::iterator itmpLc = mpLcTable.find(vSuccE.size());
                if (itmpLc != mpLcTable.end())
                {
                    if (iScore < itmpLc->second.iScore)
                    {
                        break;
                    }
                    else if (iScore == itmpLc->second.iScore)
                    {
                        if (iRealScore <= itmpLc->second.iRealScore)
                        {
                            break;
                        }
                    }
                }
                mpLcTable[vSuccE.size()] = (TPST_TP_SCORE){iScore, iRealScore, vSuccE};
                iUsedBgt = vSuccE.size();
                /*DEBUG_PRINTF("BACK group: %d insert: %d (%d, %d)\n",
                             iGroupId, vCanE.size(), vSuccE.size(), iScore);*/
            }
            break;
        }

    }
    /* restore */
    for (int iEid : pstCurGroup->vEdges)
    {
        TPST_COMP_NODE *pstNode = findNode(iEid);
        pstNode->bAban = false;
    }
    return iUsedBgt;
}

/*****************
input:
        int x
        int y
        long double ldP
description:
        return max group id
******************/
int Comp::tableChsInc(int iGroupId, int iMaxBudget, vector<pair<int, int> > &vCanE, map<int, TPST_TP_SCORE> &mpLcTable)
{
    map<int, TPST_TP_SCORE>::iterator itmpLc;
    TPST_GROUP_INFO *pstCurGroup = &(m_mpGroups[iGroupId]);

    vector <int> vGEdges;
    vGEdges.resize(pstCurGroup->vEdges.size());
    for (int i = 0; i < pstCurGroup->vEdges.size(); ++i)
    {
        TPST_COMP_NODE *pstE = findNode(pstCurGroup->vEdges[i]);
        vGEdges[i] = pstE->iGEid;
    }

    vector<pair<int, int> > vSuccE;
    int iPrevScore = 0;
    for (int i = 0; i < vCanE.size(); ++i)
    {
        vector<pair<int, int> > vTpCanE(vCanE.begin(), vCanE.begin() + i + 1);

        vSuccE.clear();
        int iRealScore = 0;
        int iScore = backtrack::tryIns(*m_poG, *m_poBackG, m_iDesK, vCanE, vGEdges, vSuccE, &iRealScore);
        /*printf("RANDOM group: %d insert: %d get success: %d score: %d real: %d\n",
               iGroupId, vCanE.size(), vSuccE.size(), iScore, iRealScore);*/

        if (vSuccE.size() > iMaxBudget)
        {
            break;
        }

        if (iScore > iPrevScore)
        {
            iPrevScore = iScore;
        }
        else
        {
            /* no new increase */
            continue;
        }

        itmpLc = mpLcTable.find(vSuccE.size());
        if (itmpLc != mpLcTable.end())
        {
            if (iScore < itmpLc->second.iScore)
            {
                continue;
            }
            else if (iScore == itmpLc->second.iScore)
            {
                if (iRealScore <= itmpLc->second.iRealScore)
                {
                    continue;
                }
            }
        }
        mpLcTable[vSuccE.size()] = (TPST_TP_SCORE){iScore, iRealScore, vSuccE};
    }
    return 0;
}
/*****************
input:
        int x
        int y
        long double ldP
description:
        return max group id
******************/
int Comp::tableChsDAG(int iGroupId, int iMaxBudget, map<int, TPST_TP_SCORE> &mpLcTable)
{
	struct timeval tv;
	long lStartTime = 0;
	long lCurTime = 0;

    gettimeofday(&tv, NULL);
    lStartTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    TPST_GROUP_INFO *pstCurGroup = NULL;
    int iPrevBestScore = 0;
    vector<pair<int, int> > vCanE;
    vector<TPST_BLOCK_NODE>::iterator itBlock;
    vector<TPST_BLOCK_EDGE>::iterator itChild;
    vector<int> vAbaBlocks;
    map<int, TPST_TP_SCORE>::iterator itmpLc;

    pstCurGroup = &(m_mpGroups[iGroupId]);
    DEBUG_ASSERT(iGroupId == pstCurGroup->iGroupId);

    /*DEBUG_PRINTF("DAG scheme group: %d maximum budget: %d size: %d\n",
                 iGroupId, iMaxBudget, pstCurGroup->vEdges.size());
    showGroup(iGroupId);*/

    vector <int> vGEdges;
    vGEdges.resize(pstCurGroup->vEdges.size());
    for (int i = 0; i < pstCurGroup->vEdges.size(); ++i)
    {
        TPST_COMP_NODE *pstE = findNode(pstCurGroup->vEdges[i]);
        vGEdges[i] = pstE->iGEid;
    }

    //DEBUG_PRINTF("DAG start divide\n");
    int iBlocksNum = divideBlocks(iGroupId);
    /*DEBUG_PRINTF("DAG scheme group: %d block size: %d \n",
                 iGroupId, iBlocksNum);*/
    if (COMP_DAG_BLOCK_NUM_MAX < iBlocksNum)
    {
        iBlocksNum = divideBlocksStrong(iGroupId);
        /*DEBUG_PRINTF("DAG scheme group: %d strong block size: %d \n",
                     iGroupId, iBlocksNum);*/
    }
    gettimeofday(&tv, NULL);
    lCurTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    g_lDAGDivideTime += lCurTime - lStartTime;
    //DEBUG_PRINTF("DAG divide done\n");
    gettimeofday(&tv, NULL);
    lStartTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    int iRelationCnt = blockPathDAG(iGroupId);
    //blockPathDAG(iGroupId, iMaxBudget, mpLcTable);
    gettimeofday(&tv, NULL);
    lCurTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    g_lDAGPathTime += lCurTime - lStartTime;
    //DEBUG_PRINTF("DAG path done\n");

    if (g_bFirstDAG)
    {
        g_bFirstDAG = false;
        DEBUG_PRINTF("DAG largest group: %d edges: %d block size: %d relation: %d\n",
                     iGroupId, pstCurGroup->vEdges.size(), iBlocksNum, iRelationCnt);
    }

    gettimeofday(&tv, NULL);
    lStartTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    DAG oDAG;
    /* construct */
    for (itBlock = pstCurGroup->vBlocks.begin(); itBlock != pstCurGroup->vBlocks.end(); ++itBlock)
    {
        if (0 >= itBlock->iBlockId)
        {
            continue;
        }
        oDAG.addNode(itBlock->iBlockId, itBlock->vEdges.size());
        /*DEBUG_PRINTF("COMP_DAG add node: %d, size: %d children size: %d\n",
                     itBlock->iBlockId, itBlock->vEdges.size(), itBlock->vChildren.size());*/
        /*for (itChild = itBlock->vChildren.begin(); itChild != itBlock->vChildren.end(); ++itChild)
        {
            itChild->iCost = itChild->vAffectE.size() * m_iDesK * pstCurGroup->iIntRatio;
        }*/
    }
    for (itBlock = pstCurGroup->vBlocks.begin(); itBlock != pstCurGroup->vBlocks.end(); ++itBlock)
    {
        if (0 >= itBlock->iBlockId)
        {
            continue;
        }
        if (itBlock->vChildren.empty())
        {
            oDAG.addT(itBlock->iBlockId, itBlock->iCost);
            //oDAG.addT(itBlock->iBlockId, itBlock->vEdges.size());
        }
        else
        {
            for (itChild = itBlock->vChildren.begin(); itChild != itBlock->vChildren.end(); ++itChild)
            {
                oDAG.add(itBlock->iBlockId, itChild->iBlockId, itChild->iCost);
                /*DEBUG_PRINTF("COMP_DAG add edge: %d, %d, %d\n",
                             itBlock->iBlockId, itChild->iBlockId, itChild->iCost);*/
            }
        }
    }

    //DEBUG_PRINTF("DAG start construct\n");
    //oDAG.construct();
    oDAG.constructGoldberg();
    //DEBUG_PRINTF("DAG construct done\n");

    gettimeofday(&tv, NULL);
    lCurTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    g_lDAGBuildTime += lCurTime - lStartTime;
    //showGroup(iGroupId);
    //oDAG.show();
    //DEBUG_ASSERT(0);
    gettimeofday(&tv, NULL);
    lStartTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    //oDAG.prim(iMaxBudget * 2 * pstCurGroup->iIntRatio);
    //oDAG.sizeFirst(iMaxBudget * 2 * pstCurGroup->iIntRatio);
    /*DEBUG_PRINTF("DAG scheme group: %d maximum flow: %d size: %d\n",
                 iGroupId, iMaxBudget * 2 * pstCurGroup->iIntRatio, pstCurGroup->vEdges.size());*/
    oDAG.Goldberg(iMaxBudget * 2);
    //oDAG.runMinCut(pstCurGroup->iMaxBgt * pstCurGroup->iIntRatio);
    //DEBUG_PRINTF("COMP_DAG total instances: %d\n", oDAG.m_vRes.size());
    gettimeofday(&tv, NULL);
    lCurTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    g_lCutTime += lCurTime - lStartTime;
    /*DEBUG_PRINTF("DAG scheme group: %d maximum flow: %d size: %d time: %ld ms\n",
                 iGroupId, iMaxBudget * 2, pstCurGroup->vEdges.size(), lCurTime - lStartTime);*/

    //oDAG.show();

    for (int iIns = 0; iIns < oDAG.m_vRes.size(); ++iIns)
    {
        if (0 >= oDAG.m_vRes[iIns].iScore)
        {
            continue;
        }
        /*DEBUG_PRINTF("COMP_DAG instance: %d budget: %d score: %d\n",
                     iIns, iIns, oDAG.m_vRes[iIns].iScore);*/

        gettimeofday(&tv, NULL);
        lStartTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        vector <int> vUnsE;
        vector<pair<int, int> > vInsE;
        vector<pair<int, int> > vSuccE;
        vCanE.clear();

        //DEBUG_PRINTF("DEBUG abandon size: %d\n", oDAG.m_vRes[iIns].vAbaP.size());
        for (int iCurBlockId : oDAG.m_vRes[iIns].vAbaP)
        {
//            DEBUG_PRINTF("DEBUG abandon block: %d max: %d\n", iCurBlockId, pstCurGroup->vBlocks.size());
            DEBUG_ASSERT(iCurBlockId < pstCurGroup->vBlocks.size());
            TPST_BLOCK_NODE *pstBlock = &(pstCurGroup->vBlocks[iCurBlockId]);
            for (int iEid : pstBlock->vEdges)
            {
                //DEBUG_PRINTF("DEBUG current edge: %d\n", iEid);
                TPST_COMP_NODE *pstE = findNode(iEid);
                DEBUG_ASSERT(NULL != pstE);
                pstE->bAban = true;
            }
        }

        for (int iEid : pstCurGroup->vEdges)
        {
            //DEBUG_PRINTF("DEBUG restore edge: %d\n", iEid);
            TPST_COMP_NODE *pstE = findNode(iEid);
            if (!pstE->bAban)
            {
                vUnsE.push_back(pstE->iGEid);
            }
        }
        vector<int> vBareE;
        findKTrussCanE(*m_poG, *m_poBackG, m_iCurK, m_iDesK, vUnsE, vCanE, this, false, vBareE, iMaxBudget);

        gettimeofday(&tv, NULL);
        lCurTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        g_lCanETime += lCurTime - lStartTime;
        /*DEBUG_PRINTF("tableChsDAG group: %d ins: %d candidate edges: %d flow: %d estimate budget: %d max: %d unstable edges: %d time: %ld\n",
                     iGroupId, iIns, vCanE.size(), oDAG.m_vRes[iIns].iFlow, oDAG.m_vRes[iIns].iFlow / pstCurGroup->iIntRatio,
                     iMaxBudget, vUnsE.size(), lCurTime - lStartTime);*

        /* restore */
        for (int iCurBlockId : oDAG.m_vRes[iIns].vAbaP)
        {
            TPST_BLOCK_NODE *pstBlock = &(pstCurGroup->vBlocks[iCurBlockId]);
            for (int iEid : pstBlock->vEdges)
            {
                //DEBUG_PRINTF("DEBUG restore edge: %d\n", iEid);
                TPST_COMP_NODE *pstE = findNode(iEid);
                pstE->bAban = false;
            }
        }
        if (vCanE.size() > iMaxBudget)
        {
            continue;
        }
        if (vCanE.size() <= 0)
        {
            continue;
        }
        //DEBUG_PRINTF("TABLE_DAG restore done\n");
        gettimeofday(&tv, NULL);
        long lTpStartTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        int iRealScore = 0;
        int iScore = backtrack::tryIns(*m_poG, *m_poBackG, m_iDesK, vCanE, vGEdges, vSuccE, &iRealScore);
        gettimeofday(&tv, NULL);
        lCurTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        g_lInsertTime += lCurTime - lTpStartTime;
        /*vector<pair<int, int> > vDebugSuccE;
        int iDebugScore = backtrack::tryIns(*m_poG, *m_poBackG, m_iDesK + 1, vSuccE, pstCurGroup->vEdges, vDebugSuccE);
        DEBUG_ASSERT(vSuccE.size() == vDebugSuccE.size());
        DEBUG_ASSERT(iScore == iDebugScore);*/
        //DEBUG_ASSERT(vSuccE.size() == vCanE.size());
        /*if (vSuccE.size() != vCanE.size())
        {
            DEBUG_PRINTF("ERROR DAG instance: %d, size: %d, %d\n", iIns, vSuccE.size(), vCanE.size());
            COMMON_UNIQUE(vCanE);
            DEBUG_PRINTF("ERROR after %d, %d\n", vSuccE.size(), vCanE.size());
            DEBUG_ASSERT(0);
        }*/

        /*DEBUG_PRINTF("DEBUG ins: %d insert: %d estimated : %d real: (%d, %d)\n",
                     iIns, vCanE.size(), oDAG.m_vRes[iIns].iScore, vSuccE.size(), iScore);*/

        if (vSuccE.size() >= iMaxBudget)
        {
            continue;
        }
        itmpLc = mpLcTable.find(vSuccE.size());
        if (itmpLc != mpLcTable.end())
        {
            if (iScore < itmpLc->second.iScore)
            {
                continue;
            }
            else if (iScore == itmpLc->second.iScore)
            {
                if (iRealScore <= itmpLc->second.iRealScore)
                {
                    continue;
                }
            }
        }
        mpLcTable[vSuccE.size()] = (TPST_TP_SCORE){iScore, iRealScore, vSuccE};

        /*DEBUG_PRINTF("DAG group: %d ins: %d insert: %d (%d, %d)\n",
                     iGroupId, iIns, vCanE.size(), vSuccE.size(), iScore);*/
    }
    /*int iPrevScore = 0;
    for (itmpLc = mpLcTable.begin(); itmpLc != mpLcTable.end(); ++itmpLc)
    {
        int iCurScore = itmpLc->second.iScore;
        if (iCurScore > iPrevScore)
        {
            iPrevScore = iCurScore;
            DEBUG_PRINTF("(%d, %d) ", itmpLc->first, iCurScore);
        }
    }
    DEBUG_PRINTF("\n");*/
    return 0;
}
/*****************
input:
        int x
        int y
        long double ldP
description:
        return max group id
******************/
int Comp::refineTable(int iGroupId, map<int, TPST_TP_SCORE> &mpLcTable)
{
    int iBgt = 0;
    int iScore = 0;
    int iPrevBestBgt = 0;
    int iPrevBestScore = 0;
    map<int, TPST_TP_SCORE>::iterator itLcTable;
    map<int, TPST_SCORE_INFO>::iterator itGbTable;
    TPST_GROUP_INFO *pstCurGroup = &(m_mpGroups[iGroupId]);
    DEBUG_ASSERT(iGroupId == pstCurGroup->iGroupId);
    bool bRm = false;
    int iComBgt = pstCurGroup->iMaxBgt;

    //DEBUG_PRINTF("REFINE_TABLE group: %d size: %d final:\n", iGroupId, mpLcTable.size());
    for (itLcTable = mpLcTable.begin(); itLcTable != mpLcTable.end(); ++itLcTable)
    {
        iBgt = itLcTable->first;
        iScore = itLcTable->second.iScore - iBgt;
        /* compare with previous cells */
        if (iScore > iPrevBestScore)
        {
            iPrevBestScore = iScore;
            iPrevBestBgt = iBgt;
            /*m_mpScoreTable[pair<int, int>(iGroupId, iBgt)] =
                pair<int, vector<pair<int, int> > >(iScore, itLcTable->second.second);*/
            m_vScoreTable[iGroupId][iBgt].iBgt = iBgt;
            m_vScoreTable[iGroupId][iBgt].iScore = itLcTable->second.iScore;
            m_vScoreTable[iGroupId][iBgt].vCanE = itLcTable->second.vCanE;
            //DEBUG_PRINTF("(%d, %d),", iBgt, itLcTable->second.iScore);
        }
        if ((iBgt < iComBgt) && (iScore >= pstCurGroup->vEdges.size()))
        {
            /* complete */
            pstCurGroup->iMaxBgt = iBgt;
            pstCurGroup->vCanE = itLcTable->second.vCanE;
            bRm = true;
//            DEBUG_PRINTF("\nREFINE_TABLE break\n");
            break;
        }
    }
    DEBUG_ASSERT(iComBgt >= iPrevBestBgt);
    if (iComBgt <= m_iTotalBudget)
    {
        itGbTable = m_vScoreTable[iGroupId].find(iComBgt);
        if (itGbTable != m_vScoreTable[iGroupId].end())
        {
            if (bRm)
            {
//                DEBUG_PRINTF("REFINE_TABLE remove\n");
                m_vScoreTable[iGroupId].erase(itGbTable);
            }
        }
    }
//    DEBUG_PRINTF("\n");

    return iPrevBestScore;
}
#if 0
/*****************
input:
        int x
        int y
        long double ldP
description:
        return max group id
******************/
int Comp::findStableSup(void *pComp, vector <int> &vLfE, vector <int> &vRtE)
{
    TPST_MAP_BY_EID *pstGLfNode = NULL;
    TPST_MAP_BY_EID *pstGRtNode = NULL;
    Comp *poComp = (Comp*)pComp;

    int iSup = 0;
    int iMinT = 0;
    int iDesK = poComp->m_iDesK;
    vector <int>::iterator itLfE;
    vector <int>::iterator itRtE;

    itLfE = vLfE.begin();
    itRtE = vRtE.begin();
    for (; itLfE != vLfE.end(); ++itLfE, ++itRtE)
    {
        pstGLfNode = poComp->m_poG->findNode(*itLfE);
        DEBUG_ASSERT(NULL != pstGLfNode);
        pstGRtNode = poComp->m_poG->findNode(*itRtE);
        DEBUG_ASSERT(NULL != pstGRtNode);

        iMinT = COMMON_MIN(pstGLfNode->iTrussness, pstGRtNode->iTrussness);

        if (iMinT >= iDesK)
        {
            ++iSup;
            continue;
        }

        if (iDesK > pstGLfNode->iTrussness)
        {
            /* not this group and not higher support */
            continue;
        }
        if (iDesK > pstGRtNode->iTrussness)
        {
            /* not this group and not higher support */
            continue;
        }
        ++iSup;
    }

    return iSup;
}

/*****************
input:
        int x
        int y
        long double ldP
description:
        return max group id
******************/
int Comp::findUnstable(int iGroupId, vector<int> &vUnstableE)
{
    TPST_GROUP_INFO *pstCurGroup = NULL;
    TPST_COMP_NODE *pstNode = NULL;
    TPST_MAP_BY_EID *pstGNode = NULL;
    /* eid, sup */
    map <int, int> mpSup;

    vector <int>::iterator itE;

    pstCurGroup = &(m_mpGroups[iGroupId]);
    DEBUG_ASSERT(iGroupId == pstCurGroup->iGroupId);

    for (itE = pstCurGroup->vEdges.begin(); itE != pstCurGroup->vEdges.end(); ++itE)
    {
        pstNode = &(m_pvComp->at(*itE));
        if (pstNode->bAban)
        {
            /* give up */
            continue;
        }
        pstGNode = m_poG->findNode(*itE);
        DEBUG_ASSERT(NULL != pstGNode);

        mpSup[pstNode->iEid] = m_poG->findKSup(m_iDesK, pstGNode->vLfE, pstGNode->vRtE);
    }
    for (auto mpKey : mpSup)
    {
        if (mpKey.second + 2 <= m_iDesK)
        {
            /* unstable edge */
            vUnstableE.push_back(mpKey.first);
        }
    }

    return 0;
}
#endif
/*****************
input:
        int x
        int y
        long double ldP
description:
        return max group id
******************/
int Comp::blockPathDAG(int iGroupId, int iMaxBudget, map<int, TPST_TP_SCORE> &mpLcTable)
{
    TPST_GROUP_INFO* pstGroup = NULL;
    int iMinT = 0;
    int iMinL = 0;
    vector <int>::iterator itLfE;
    vector <int>::iterator itRtE;
    vector <int>::iterator itE;
    vector <int> vVisited;
    TPST_COMP_NODE *pstNode = NULL;
    TPST_COMP_NODE *pstLfNode = NULL;
    TPST_COMP_NODE *pstRtNode = NULL;
    TPST_MAP_BY_EID *pstGNode = NULL;
    TPST_MAP_BY_EID *pstGLfNode = NULL;
    TPST_MAP_BY_EID *pstGRtNode = NULL;

    TPST_BLOCK_NODE stEmpty = {0};
    TPST_BLOCK_EDGE stEmptyE = {0};
    TPST_BLOCK_NODE *pstBlock = NULL;

    /* <layer, size>, <block id> */
    map <pair<int, int>, vector <int> > mpBlockRank;
    map <pair<int, int>, vector <int> >::iterator itmpBlock;

    vector<TPST_BLOCK_NODE>::iterator itBlock;

    pstGroup = &(m_mpGroups[iGroupId]);
    //pstGroup->iTopBlock = 0;

    /* rank block */
    mpBlockRank.clear();
    //DEBUG_PRINTF("SHOW group: %d size: %d first: %d\n", iGroupId, pstGroup->vEdges.size(), pstGroup->vFirst.size());
    for (itBlock = pstGroup->vBlocks.begin(); itBlock != pstGroup->vBlocks.end(); ++itBlock)
    {
        if (0 == itBlock->iBlockId)
        {
            /* empty */
            continue;
        }
        mpBlockRank[pair<int, int>(itBlock->iLayer, itBlock->iSize)].push_back(itBlock->iBlockId);
        //itBlock->iTopBlock = 0;
    }

    /* find relationships */
    for (itmpBlock = mpBlockRank.begin(); itmpBlock != mpBlockRank.end(); ++itmpBlock)
    {
        vector <int>::iterator itBlockId;
        for (itBlockId = itmpBlock->second.begin(); itBlockId != itmpBlock->second.end(); ++itBlockId)
        {
            /* block id, eid */
            map <int, vector<int> > mpChildren;
            int iCurBlock = *itBlockId;
            pstBlock = &(pstGroup->vBlocks[iCurBlock]);
            DEBUG_ASSERT(pstBlock->iBlockId == iCurBlock);

            //DEBUG_PRINTF("SHOW block: %d layer: %d\n", iCurBlock, pstBlock->iLayer);

            /*if (0 == pstGroup->iTopBlock)
            {
                pstGroup->iTopBlock = iCurBlock;
            }
            if (0 == pstBlock->iTopBlock)
            {
                pstBlock->iTopBlock = iCurBlock;
            }*/
            for (itE = pstBlock->vEdges.begin(); itE != pstBlock->vEdges.end(); ++itE)
            {
                pstNode = &(m_pvComp->at(*itE));

                pstGNode = m_poG->findNode(pstNode->iGEid);
                DEBUG_ASSERT(NULL != pstGNode);

                itLfE = pstGNode->vLfE.begin();
                itRtE = pstGNode->vRtE.begin();
                for (; itLfE != pstGNode->vLfE.end(); ++itLfE, ++itRtE)
                {
                    pstGLfNode = m_poG->findNode(*itLfE);
                    DEBUG_ASSERT(NULL != pstGLfNode);
                    pstGRtNode = m_poG->findNode(*itRtE);
                    DEBUG_ASSERT(NULL != pstGRtNode);

                    pstLfNode = findNode(m_vG2L[pstGLfNode->eid]);
                    DEBUG_ASSERT(NULL != pstLfNode);
                    pstRtNode = findNode(m_vG2L[pstGRtNode->eid]);
                    DEBUG_ASSERT(NULL != pstRtNode);

                    iMinT = COMMON_MIN(pstGLfNode->iTrussness, pstGRtNode->iTrussness);
                    if (iMinT >= m_iDesK)
                    {
                        continue;
                    }
                    if (iMinT < m_iCurK)
                    {
                        continue;
                    }

                    if ((pstGLfNode->iTrussness < m_iDesK) && (pstLfNode->iGroupId != iGroupId))
                    {
                        continue;
                    }
                    if ((pstGRtNode->iTrussness < m_iDesK) && (pstRtNode->iGroupId != iGroupId))
                    {
                        continue;
                    }

                    iMinL = 0;
                    if (m_iDesK <= pstGLfNode->iTrussness)
                    {
                        iMinL = pstRtNode->iLayer;
                        DEBUG_ASSERT(pstRtNode->iGroupId == iGroupId);
                    }
                    else if (m_iDesK <= pstGRtNode->iTrussness)
                    {
                        iMinL = pstLfNode->iLayer;
                        DEBUG_ASSERT(pstLfNode->iGroupId == iGroupId);
                    }
                    else
                    {
                        iMinL = COMMON_MIN(pstLfNode->iLayer, pstRtNode->iLayer);
                        DEBUG_ASSERT(pstLfNode->iGroupId == iGroupId);
                        DEBUG_ASSERT(pstRtNode->iGroupId == iGroupId);
                    }
                    if (iMinL != pstNode->iLayer - 1)
                    {
                        /* find lower blocks */
                        continue;
                    }

                    if ((pstLfNode->iGroupId == iGroupId) &&
                        (pstNode->iLayer - 1 == pstLfNode->iLayer))
                    {
                        mpChildren[pstLfNode->iBlockId].push_back(pstNode->iEid);
                    }
                    if ((pstRtNode->iGroupId == iGroupId) &&
                        (pstNode->iLayer - 1 == pstRtNode->iLayer))
                    {
                        mpChildren[pstRtNode->iBlockId].push_back(pstNode->iEid);
                    }
                }

                //DEBUG_PRINTF("SHOW block size: %d\n", pstBlock->iSize);
            }
            for (auto atmpBlock : mpChildren)
            {
                TPST_BLOCK_NODE *pstTpBlock = &(pstGroup->vBlocks[atmpBlock.first]);
                stEmptyE.iBlockId = pstTpBlock->iBlockId;
                COMMON_UNIQUE(atmpBlock.second);
                stEmptyE.iAffCnt = atmpBlock.second.size();
                //stEmptyE.iCost = stEmptyE.iAffCnt;
                //stEmptyE.vAffectE = atmpBlock.second;
                pstBlock->vChildren.push_back(stEmptyE);
            }
        }
    }

    /* calculate score */
    for (itmpBlock = mpBlockRank.begin(); itmpBlock != mpBlockRank.end(); ++itmpBlock)
    {
        vector <int>::iterator itBlockId;
        for (itBlockId = itmpBlock->second.begin(); itBlockId != itmpBlock->second.end(); ++itBlockId)
        {
            int iCurBlock = *itBlockId;
            pstBlock = &(pstGroup->vBlocks[iCurBlock]);
            DEBUG_ASSERT(pstBlock->iBlockId == iCurBlock);
            vector<int> vGEdges;
            vector<pair<int, int> > vCanE;

            for (int iEid : pstBlock->vEdges)
            {
                pstNode = &(m_pvComp->at(iEid));
                vGEdges.push_back(pstNode->iGEid);
            }

            vector<int> vBareE;
            findKTrussCanE(*m_poG, *m_poBackG, m_iCurK, m_iDesK, vGEdges, vCanE, this, false, vBareE, iMaxBudget * pstBlock->vEdges.size());

            int iCost = vCanE.size();
            if (vCanE.empty())
            {
                /* not enough */
                iCost = iMaxBudget * pstBlock->vEdges.size() + 1;
            }
            else if (vCanE.size() <= iMaxBudget)
            {
                tableChsInc(iGroupId, iMaxBudget, vCanE, mpLcTable);
            }

            pstBlock->iCost = pstGroup->iIntRatio * iCost;

            int iTotalAff = 0;
            for (auto atN : pstBlock->vChildren)
            {
                iTotalAff += atN.iAffCnt;
            }

            for (auto &atN : pstBlock->vChildren)
            {
                atN.iCost = pstBlock->iCost * atN.iAffCnt / iTotalAff;
                atN.iCost = COMMON_MAX(1, atN.iCost);
            }

            /* abandon */
            for (int iEid : pstBlock->vEdges)
            {
                pstNode = &(m_pvComp->at(iEid));
                pstNode->bAban = true;
            }

        }
    }
    /* restore */
    for (int iEid : pstGroup->vEdges)
    {
        pstNode = &(m_pvComp->at(iEid));
        pstNode->bAban = false;
    }
    return 0;
}

/*****************
input:
        int x
        int y
        long double ldP
description:
        return max group id
******************/
int Comp::blockPathDAG(int iGroupId)
{
    TPST_GROUP_INFO* pstGroup = NULL;
    int iMinT = 0;
    int iMinL = 0;
    int iRelationCnt = 0;
    vector <int>::iterator itLfE;
    vector <int>::iterator itRtE;
    vector <int>::iterator itE;
    TPST_COMP_NODE *pstNode = NULL;
    TPST_COMP_NODE *pstLfNode = NULL;
    TPST_COMP_NODE *pstRtNode = NULL;
    TPST_MAP_BY_EID *pstGNode = NULL;
    TPST_MAP_BY_EID *pstGLfNode = NULL;
    TPST_MAP_BY_EID *pstGRtNode = NULL;

    TPST_BLOCK_NODE stEmpty = {0};
    TPST_BLOCK_EDGE stEmptyE = {0};
    TPST_BLOCK_NODE *pstBlock = NULL;

    /* <layer, size>, <block id> */
    map <pair<int, int>, vector <int> > mpBlockRank;
    map <pair<int, int>, vector <int> >::iterator itmpBlock;

    vector<TPST_BLOCK_NODE>::iterator itBlock;

    pstGroup = &(m_mpGroups[iGroupId]);

    /* rank block */
    mpBlockRank.clear();
    for (itBlock = pstGroup->vBlocks.begin(); itBlock != pstGroup->vBlocks.end(); ++itBlock)
    {
        if (0 == itBlock->iBlockId)
        {
            /* empty */
            continue;
        }
        mpBlockRank[pair<int, int>(itBlock->iLayer, itBlock->iSize)].push_back(itBlock->iBlockId);
    }

    /* find relationships */
    for (itmpBlock = mpBlockRank.begin(); itmpBlock != mpBlockRank.end(); ++itmpBlock)
    {
        vector <int>::iterator itBlockId;
        for (itBlockId = itmpBlock->second.begin(); itBlockId != itmpBlock->second.end(); ++itBlockId)
        {
            /* block id, eid */
            map <int, vector<int> > mpChildren;
            int iCurBlock = *itBlockId;
            pstBlock = &(pstGroup->vBlocks[iCurBlock]);
            DEBUG_ASSERT(pstBlock->iBlockId == iCurBlock);

            for (itE = pstBlock->vEdges.begin(); itE != pstBlock->vEdges.end(); ++itE)
            {
                pstNode = &(m_pvComp->at(*itE));

                pstGNode = m_poG->findNode(pstNode->iGEid);
                DEBUG_ASSERT(NULL != pstGNode);

                itLfE = pstGNode->vLfE.begin();
                itRtE = pstGNode->vRtE.begin();
                for (; itLfE != pstGNode->vLfE.end(); ++itLfE, ++itRtE)
                {
                    pstGLfNode = m_poG->findNode(*itLfE);
                    DEBUG_ASSERT(NULL != pstGLfNode);
                    pstGRtNode = m_poG->findNode(*itRtE);
                    DEBUG_ASSERT(NULL != pstGRtNode);

                    pstLfNode = findNode(m_vG2L[pstGLfNode->eid]);
                    DEBUG_ASSERT(NULL != pstLfNode);
                    pstRtNode = findNode(m_vG2L[pstGRtNode->eid]);
                    DEBUG_ASSERT(NULL != pstRtNode);

                    iMinT = COMMON_MIN(pstGLfNode->iTrussness, pstGRtNode->iTrussness);
                    if (iMinT >= m_iDesK)
                    {
                        continue;
                    }
                    if (iMinT < m_iCurK)
                    {
                        continue;
                    }

                    if ((pstGLfNode->iTrussness < m_iDesK) && (pstLfNode->iGroupId != iGroupId))
                    {
                        continue;
                    }
                    if ((pstGRtNode->iTrussness < m_iDesK) && (pstRtNode->iGroupId != iGroupId))
                    {
                        continue;
                    }

                    iMinL = 0;
                    if (m_iDesK <= pstGLfNode->iTrussness)
                    {
                        iMinL = pstRtNode->iLayer;
                        DEBUG_ASSERT(pstRtNode->iGroupId == iGroupId);
                    }
                    else if (m_iDesK <= pstGRtNode->iTrussness)
                    {
                        iMinL = pstLfNode->iLayer;
                        DEBUG_ASSERT(pstLfNode->iGroupId == iGroupId);
                    }
                    else
                    {
                        iMinL = COMMON_MIN(pstLfNode->iLayer, pstRtNode->iLayer);
                        DEBUG_ASSERT(pstLfNode->iGroupId == iGroupId);
                        DEBUG_ASSERT(pstRtNode->iGroupId == iGroupId);
                    }
                    if (iMinL != pstNode->iLayer - 1)
                    {
                        /* find lower blocks */
                        continue;
                    }

                    if ((pstLfNode->iGroupId == iGroupId) &&
                        (pstNode->iLayer - 1 == pstLfNode->iLayer))
                    {
                        mpChildren[pstLfNode->iBlockId].push_back(pstNode->iEid);
                    }
                    if ((pstRtNode->iGroupId == iGroupId) &&
                        (pstNode->iLayer - 1 == pstRtNode->iLayer))
                    {
                        mpChildren[pstRtNode->iBlockId].push_back(pstNode->iEid);
                    }
                }

            }
            for (auto atmpBlock : mpChildren)
            {
                TPST_BLOCK_NODE *pstTpBlock = &(pstGroup->vBlocks[atmpBlock.first]);
                stEmptyE.iBlockId = pstTpBlock->iBlockId;
                COMMON_UNIQUE(atmpBlock.second);
                stEmptyE.iAffCnt = atmpBlock.second.size();
                pstBlock->vChildren.push_back(stEmptyE);
            }
            iRelationCnt += pstBlock->vChildren.size();
        }
    }

    /* calculate score */
    for (itmpBlock = mpBlockRank.begin(); itmpBlock != mpBlockRank.end(); ++itmpBlock)
    {
        vector <int>::iterator itBlockId;
        for (itBlockId = itmpBlock->second.begin(); itBlockId != itmpBlock->second.end(); ++itBlockId)
        {
            int iCurBlock = *itBlockId;
            pstBlock = &(pstGroup->vBlocks[iCurBlock]);
            DEBUG_ASSERT(pstBlock->iBlockId == iCurBlock);

            if (pstBlock->vChildren.empty())
            {
                pstBlock->iCost = pstBlock->vEdges.size();
            }
            else
            {
                for (auto &atN : pstBlock->vChildren)
                {
                    atN.iCost = atN.iAffCnt;
                    atN.iCost = COMMON_MAX(1, atN.iCost);
                }
            }

        }
    }
    return iRelationCnt;
}

#if 0
/*****************
input:
        int x
        int y
        long double ldP
description:
        return max group id
******************/
int Comp::shrinkBlocks(int iGroupId, map <pair<int, int>, vector <int> > &mpBlockRank, int iMaxCap)
{
    TPST_GROUP_INFO* pstGroup = NULL;
    int iMinT = 0;
    int iMinL = 0;
    vector <int>::iterator itLfE;
    vector <int>::iterator itRtE;
    vector <int>::iterator itE;
    TPST_COMP_NODE *pstNode = NULL;
    TPST_COMP_NODE *pstLfNode = NULL;
    TPST_COMP_NODE *pstRtNode = NULL;
    TPST_MAP_BY_EID *pstGNode = NULL;
    TPST_MAP_BY_EID *pstGLfNode = NULL;
    TPST_MAP_BY_EID *pstGRtNode = NULL;

    TPST_BLOCK_NODE stEmpty = {0};
    TPST_BLOCK_EDGE stEmptyE = {0};
    TPST_BLOCK_NODE *pstBlock = NULL;

    map <pair<int, int>, vector <int> >::iterator itmpBlock;

    vector<TPST_BLOCK_NODE>::iterator itBlock;

    pstGroup = &(m_mpGroups[iGroupId]);

    /* find relationships */
    for (itmpBlock = mpBlockRank.begin(); itmpBlock != mpBlockRank.end(); ++itmpBlock)
    {
        vector <int>::iterator itBlockId;
        for (itBlockId = itmpBlock->second.begin(); itBlockId != itmpBlock->second.end(); ++itBlockId)
        {
            /* block id, eid */
            map <int, vector<int> > mpChildren;
            int iCurBlock = *itBlockId;
            pstBlock = &(pstGroup->vBlocks[iCurBlock]);
            DEBUG_ASSERT(pstBlock->iBlockId == iCurBlock);

            if (pstBlock->vChildren.empty())
            {
                continue;
            }

            bool bMerge = true;
            for (auto &atN : pstBlock->vChildren)
            {
                if (atN.iCost <= iMaxCap)
                {
                    bMerge = false;
                    break;
                }
            }
            if (bMerge)
            {
                /* merge blocks */
                /** collect children id */
                vector<int> vChildren;
                for (auto &atN : pstBlock->vChildren)
                {
                    vChildren.push_back(atN.iBlockId);
                }
                /** merge edges */
                for (int iChildId : vChildren)
                {
                    TPST_BLOCK_NODE *pstChild = &(pstGroup->vBlocks[iChildId]);
                    pstBlock->vEdges.insert(pstBlock->vEdges.end(),
                                            pstChild->vEdges.begin(), pstChild->vEdges.end());
                    pstChild->vEdges.clear();
                }
                /** adopt children's children */
                /* id, affected count */
                map <int, int> mpGrand;
                for (int iChildId : vChildren)
                {
                    TPST_BLOCK_NODE *pstChild = &(pstGroup->vBlocks[iChildId]);
                    for (auto &atN : pstChild->vChildren)
                    {
                        map <int, int>::iterator itGrand = mpGrand.find(atN.iBlockId);
                        if (itGrand == mpGrand.end())
                        {
                            /* new */
                            mpGrand[atN.iBlockId] = atN.iAffCnt;
                        }
                        else
                        {
                            itGrand->second += atN.iAffCnt;
                        }
                    }
                }
                pstBlock->vChildren.clear();
                for (auto &atG : mpGrand)
                {
                    stEmptyE.iBlockId = atG->first;
                    stEmptyE.iAffCnt = atG->second;
                    pstBlock->vChildren.push_back(stEmptyE);
                }
                /** merge children's parents */
                /** remove children blocks */
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
        return max group id
******************/
int Comp::findDownCanE(int iGroupId, vector<int> &vBlocks, vector<pair<int, int> > &vCanE)
{
    TPST_GROUP_INFO *pstCurGroup = NULL;
    TPST_BLOCK_NODE *pstBlock = NULL;

    vector<int> vEdges;

    pstCurGroup = &(m_mpGroups[iGroupId]);
    DEBUG_ASSERT(iGroupId == pstCurGroup->iGroupId);

    for (auto atBlock : vBlocks)
    {
        pstBlock = &(pstCurGroup->vBlocks[atBlock]);

        vEdges.insert(vEdges.end(), pstBlock->vEdges.begin(), pstBlock->vEdges.end());
    }
    vector<int> vBareE;
    findKTrussCanE(*m_poG, *m_poBackG, m_iCurK, m_iDesK, vEdges, vCanE, NULL, false, vBareE);
    return 0;
}
#endif
/*****************
input:
        int x
        int y
        long double ldP
description:
        return max group id
******************/
bool Comp::findKTrussCanEClique(myG &oInitG, int iDesK, vector<int> &vDesE, vector<int> &vEPool, vector<int> &vRmEPool, vector<pair<int, int> > &vResCanE, int iMaxBgt)
{
    vector<int>::iterator itE;
    vector<int>::iterator itTpE;
    TPST_COMP_NODE *pstNode = NULL;
    TPST_MAP_BY_EID *pstGNode = NULL;
    TPST_MAP_BY_EID *pstGTpNode = NULL;

    /* candidate edge, none */
    int iNeCnt = 0;
    map<pair<int, int>, int > mpNecessaryCanE;
    map<pair<int, int>, int >::iterator itmpNsrCanE;
    vector<pair<int, int> >::iterator itpaE;
    vector <int> vCanP;
    /* affected edge, candidate edges */
    map<int, int> mpEPool;
    map<int, int>::iterator itmpE;
    vector<int> vBareE;

    for (int iEid : vEPool)
    {
        mpEPool[iEid] = 1;
    }
    for (int iEid : vDesE)
    {
        pstGNode = oInitG.findNode(iEid);
        DEBUG_ASSERT(NULL != pstGNode);
        if (pstGNode->iTrussness < iDesK)
        {
            vBareE.push_back(iEid);
        }
    }
    /* increase k by finding (k+1)-clique */
    for (itE = vBareE.begin(); itE != vBareE.end(); ++itE)
    {
        myG mySubG;
        vector<pair<int, int> > vNewCanE;
        vector <int> vAffE;

        pstGNode = oInitG.findNode(*itE);
        DEBUG_ASSERT(NULL != pstGNode);

        vAffE.push_back(pstGNode->eid);
        /*DEBUG_PRINTF("TABLE cur bare edge: %d (%d, %d)\n",
                     pstGNode->eid, pstGNode->paXY.first, pstGNode->paXY.second);*/

        /* find k+1 nodes */
        vCanP.clear();
        findKPlusNodes(oInitG, iDesK, pstGNode->eid, vCanP);
        //DEBUG_PRINTF("DEBUG_FIND subgraph nodes: %d\n", vCanP.size());
        DEBUG_ASSERT(iDesK == vCanP.size());
        oInitG.induce(vCanP, mySubG, vNewCanE);
        //mySubG.findAllCanE(vNewCanE);
        //DEBUG_PRINTF("DEBUG_FIND all edges: %d need %d\n", mySubG.m_iMaxEId, vNewCanE.size());
        if (vNewCanE.empty())
        {
            mySubG.show();
            DEBUG_ASSERT(0);
        }
        /* rm current edge */
        for (itmpE = mpEPool.begin(); itmpE != mpEPool.end(); )
        {
            pstGTpNode = oInitG.findNode(itmpE->first);
            DEBUG_ASSERT(NULL != pstGTpNode);
            if (NULL != mySubG.findNode(pstGTpNode->paXY.first, pstGTpNode->paXY.second))
            {
                /* in the subgraph */
                vAffE.push_back(pstGTpNode->eid);
                vRmEPool.push_back(pstGTpNode->eid);
                //DEBUG_PRINTF("DEBUG bare rm eid: %d\n", itmpE->first);
                mpEPool.erase(itmpE++);
                continue;
            }
            ++itmpE;
        }
        //DEBUG_PRINTF("DEBUG_FIND rm normal edges done\n");
        itTpE = itE;
        ++itTpE;
        for (; itTpE != vBareE.end(); )
        {
            pstGTpNode = oInitG.findNode(*itTpE);
            DEBUG_ASSERT(NULL != pstGTpNode);
            //DEBUG_PRINTF("DEBUG_FIND check bare edge: %d\n", *itTpE);
            if (NULL != mySubG.findNode(pstGTpNode->paXY.first, pstGTpNode->paXY.second))
            {
                /* in the subgraph */
                //DEBUG_PRINTF("DEBUG_FIND rm bare edge: %d replaced by %d\n", *itTpE, pstGNode->eid);
                vAffE.push_back(pstGTpNode->eid);
                vBareE.erase(itTpE);
                continue;
            }
            ++itTpE;
        }
        //DEBUG_PRINTF("DEBUG_FIND rm bare edges done\n");

        /* save necessary edges */
        for (itpaE = vNewCanE.begin(); itpaE != vNewCanE.end(); ++itpaE)
        {
            /*DEBUG_PRINTF("SHOW bare eid: %d candidate: (%d, %d)\n",
                         pstGNode->eid, itpaE->first, itpaE->second);*/
            auto atIt = mpNecessaryCanE.find(*itpaE);
            if (atIt == mpNecessaryCanE.end())
            {
                mpNecessaryCanE[*itpaE] = 0;
                ++iNeCnt;
            }
        }
        if (iNeCnt > iMaxBgt)
        {
            /* exceed */
            return false;
        }
        //DEBUG_PRINTF("DEBUG_FIND save edges done\n");
    }
    //DEBUG_PRINTF("DEBUG_FIND handle bare edges done\n");
    /* save */
    for (itmpNsrCanE = mpNecessaryCanE.begin(); itmpNsrCanE != mpNecessaryCanE.end(); ++itmpNsrCanE)
    {
        vResCanE.push_back(itmpNsrCanE->first);
        /*DEBUG_PRINTF("COMP_BUDGET final necessary candidate edge: (%d, %d)\n",
                     itmpNsrCanE->first.first, itmpNsrCanE->first.second);*/
    }
    return true;
}

/*****************
input:
        int x
        int y
        long double ldP
description:
        return max group id
******************/
bool Comp::findKTrussCanEInc(myG &oInitG, myG &oBackG, int iBgt, int iMinK, int iDesK, vector<int> &vDesE, vector<int> &vEPool, vector<int> &vRmEPool, vector<pair<int, int> > &vResCanE)
{
    vector<int>::iterator itE;
    vector<int>::iterator itTpE;
    TPST_COMP_NODE *pstNode = NULL;
    TPST_MAP_BY_EID *pstGNode = NULL;
    TPST_MAP_BY_EID *pstGTpNode = NULL;

    /* candidate edge, none */
    map<pair<int, int>, int > mpNecessaryCanE;
    map<pair<int, int>, int >::iterator itmpNsrCanE;
    vector<pair<int, int> >::iterator itpaE;
    vector <int> vCanP;
    /* affected edge, candidate edges */
    map<int, int> mpEPool;
    map<int, int>::iterator itmpE;
    vector<int> vBareE;
    vector<int> vChgE;
    vector<int> iGlobalInsE;

    for (int iEid : vEPool)
    {
        mpEPool[iEid] = 1;
    }
    for (int iEid : vDesE)
    {
        pstGNode = oInitG.findNode(iEid);
        DEBUG_ASSERT(NULL != pstGNode);
        if (pstGNode->iTrussness < iDesK)
        {
            vBareE.push_back(iEid);
        }
    }
    bool bSuccess = true;
    int iInsCnt = 0;
    /* increase k by inserting edges one by one */
    vector<int> vSlaveE;
//    DEBUG_PRINTF("FIND_CAN_INS des: %d bare: %d\n",
//                     vDesE.size(), vBareE.size());
    while ((!vBareE.empty()) || (!vSlaveE.empty()))
    {
        if (vBareE.empty())
        {
            vBareE.swap(vSlaveE);

//            DEBUG_PRINTF("FIND_CAN_INS new batch: %d\n",
//                            vBareE.size());
        }
        int iCurEid = vBareE.back();
        vBareE.pop_back();
        pstGNode = oInitG.findNode(iCurEid);
        DEBUG_ASSERT(NULL != pstGNode);

        if (pstGNode->iTrussness >= iDesK)
        {
            continue;
        }

        vector<pair<int, int> > vNeib;
        int iNeedSup = iDesK - 2 - oInitG.findKSup(iMinK, pstGNode->vLfE, pstGNode->vRtE);
//        DEBUG_PRINTF("FIND_CAN_INS need: %d\n",
//                        iNeedSup);

        iNeedSup = COMMON_MAX(iNeedSup, 1);

        vector<TPSTV_CAN_E> vCanE;
        oInitG.findECanTriPSort(pstGNode->eid, iMinK, vCanE);
//        DEBUG_PRINTF("FIND_CAN_INS candidate: %d\n",
//                        vCanE.size());

        if ((vCanE.size() < iNeedSup) || (0 == vCanE.size()))
        {
            /* cannot upgraded */
            bSuccess = false;
            break;
        }

        vector<int> iBatInsE;
        vector<int> iBatOE;
        for (int i = 0; i < iNeedSup; ++i)
        {
            vector<int> vTpChgE;
            int iEid = insertPart::insertOne(oInitG, vCanE[i].paXY.first, vCanE[i].paXY.second, vTpChgE);
            if (0 < iEid)
            {
                //vResCanE.push_back(vCanE[i].paXY);
                vChgE.insert(vChgE.begin(), vTpChgE.begin(), vTpChgE.end());
                iBatInsE.push_back(iEid);
                iGlobalInsE.push_back(iEid);
                iBatOE.push_back(vCanE[i].iOEid);
            }

//            if ((vCanE[i].paXY.first == 2773) && (vCanE[i].paXY.second == 3118))
//            {
//                DEBUG_PRINTF("FIND_CAN_INS insert\n");
//            }
        }
        iInsCnt += iBatInsE.size();
//        DEBUG_PRINTF("FIND_CAN_INS increase: %d\n",
//                        iInsCnt);
        if (iInsCnt > iBgt)
        {
            /* cannot upgraded */
            bSuccess = false;
            break;
        }
        pstGNode = oInitG.findNode(iCurEid);
        DEBUG_ASSERT(NULL != pstGNode);
        if (pstGNode->iTrussness < iDesK)
        {
            vSlaveE.push_back(pstGNode->eid);
            for (int iEid : iBatInsE)
            {
                pstGNode = oInitG.findNode(iEid);
                DEBUG_ASSERT(NULL != pstGNode);
                if (pstGNode->iTrussness < iDesK)
                {
                    vSlaveE.push_back(iEid);
                }
            }
            for (int iEid : iBatOE)
            {
                pstGNode = oInitG.findNode(iEid);
                DEBUG_ASSERT(NULL != pstGNode);
                if (pstGNode->iTrussness < iDesK)
                {
                    vSlaveE.push_back(iEid);
                }
            }
        }
    }
    if (bSuccess)
    {
        for (int iEid : vEPool)
        {
            pstGNode = oInitG.findNode(iEid);
            DEBUG_ASSERT(NULL != pstGNode);
            if (pstGNode->iTrussness >= iDesK)
            {
                vRmEPool.push_back(iEid);
            }
        }
        for (int iEid : iGlobalInsE)
        {
            pstGNode = oInitG.findNode(iEid);
            DEBUG_ASSERT(NULL != pstGNode);
            if (pstGNode->iTrussness >= iDesK)
            {
                vResCanE.push_back(pstGNode->paXY);
            }
        }
    }
    /* restore */
    COMMON_UNIQUE(vChgE);
    backtrack::restoreG(oInitG, oBackG, vChgE);

    return bSuccess;
}
/*****************
input:
        int x
        int y
        long double ldP
description:
        return max group id
******************/
bool Comp::findKTrussCanE(myG &oInitG, myG &oBackG, int iMinK, int iDesK, vector<int> &vDes, vector<pair<int, int> > &vResCanE, Comp *poComp, bool bBareStop, vector<int> &vBareE, int iMaxBgt)
{
    map<int, TPST_COMP_NODE> mpLcEdges;
    vector<int>::iterator itE;
    TPST_COMP_NODE *pstNode = NULL;
    TPST_MAP_BY_EID *pstGNode = NULL;
    int iDesLayer = 0;
    bool bBare = false;

    /* candidate edge, affected edges */
    map<pair<int, int>, vector<int> > mpCanE;
    map<pair<int, int>, vector<int> >::iterator itmpCanE;
    /* candidate edge, none */
    int iNeCnt = 0;
    map<pair<int, int>, int > mpNecessaryCanE;
    map<pair<int, int>, int >::iterator itmpNsrCanE;
    vector<pair<int, int> >::iterator itpaE;
    /* cannot convert by just one edge */
    //vector <int> vBareE;
    /* affected edge, candidate edges */
    map<int, vector<pair<int, int> > > mpEPool;
    vector<pair<int, int> > vCanE;

    /* find first candidates */
    for (itE = vDes.begin(); itE != vDes.end(); ++itE)
    {
        pstGNode = oInitG.findNode(*itE);
        DEBUG_ASSERT(NULL != pstGNode);
        if (iDesK <= pstGNode->iTrussness)
        {
            /* upgraded, ignore */
            continue;
        }

        pstNode = &(mpLcEdges[pstGNode->eid]);
        pstNode->iEid = pstGNode->eid;
        pstNode->bVstFlag = false;
        pstNode->fCost = 0;
        vector<pair<int, int> > vNeib;
        pstNode->iNeedSup = iDesK - 2 - oInitG.findKSup(iMinK, iDesK, pstGNode->vLfE, pstGNode->vRtE, poComp, pstNode->iGroupId, getAbaState);

        vCanE.clear();
        oInitG.findECanTriP(pstGNode->eid, iMinK, iDesK, vCanE, poComp, pstNode->iGroupId, getAbaState);
        if ((vCanE.size() < pstNode->iNeedSup) || (0 == vCanE.size()))
        {
            /* cannot upgraded */
            vCanE.clear();
            vBareE.push_back(pstGNode->eid);
        }
        else
        {
            for (itpaE = vCanE.begin(); itpaE != vCanE.end(); ++itpaE)
            {
                //DEBUG_PRINTF("SHOW e: %d find candidate (%d, %d)\n", pstGNode->eid, itpaE->first, itpaE->second);
                mpCanE[*itpaE].push_back(pstGNode->eid);
            }
            mpEPool[pstGNode->eid].swap(vCanE);
        }
        /*DEBUG_PRINTF("K_TRUSS_CAN (%d, %d) eid: %d k: %d, layer: %d find sup: %d eid: %d need: %d\n",
                     pstGNode->paXY.first, pstGNode->paXY.second, pstGNode->eid,
                     pstGNode->iTrussness, pstGNode->iLayer,
                     vCanE.size(), pstNode->iEid, pstNode->iNeedSup);*/
    }
    //DEBUG_PRINTF("K_TRUSS_CAN bare size: %d\n", vBareE.size());
    if (!vBareE.empty())
    {
        bBare = true;
        if (bBareStop)
        {
            return bBare;
        }
        vector<int> vEPool;
        vector<int> vRmEPool;
        vector<pair<int, int> > vTpCanE;
        for (auto atE : mpEPool)
        {
            vEPool.push_back(atE.first);
        }
        bool bRes = findKTrussCanEClique(oInitG, iDesK, vBareE, vEPool, vRmEPool, vTpCanE, iMaxBgt - iNeCnt);
        if (!bRes)
        {
            /* exceed */
            return bBare;
        }

        int iMaxIns = COMMON_MIN(vTpCanE.size(), iMaxBgt - iNeCnt);
        vector<int> vInsRmEPool;
        vector<pair<int, int> > vTpInsCanE;
        //DEBUG_PRINTF("CAN_E before clique: %d\n", iMaxIns);
        bool bSucc = findKTrussCanEInc(oInitG, oBackG, iMaxIns, iMinK, iDesK, vBareE, vEPool, vInsRmEPool, vTpInsCanE);

//        if (iMaxIns <= vTpInsCanE.size())
//        {
//            printf("CAN_E success: %d clique: %d increase: %d\n", bSucc, iMaxIns, vTpInsCanE.size());
//            DEBUG_ASSERT(0);
//        }

        if (bSucc)
        {
            DEBUG_ASSERT(vTpInsCanE.size() <= iMaxIns);
            //DEBUG_PRINTF("CAN_E success clique: %d increase: %d\n", iMaxIns, vTpInsCanE.size());
            vRmEPool.swap(vInsRmEPool);
            vTpCanE.swap(vTpInsCanE);

            /* debug */
//            for (auto atN : vTpCanE)
//            {
//                if ((atN.first == 2773) && (atN.second == 3118))
//                {
//                    DEBUG_PRINTF("CAN_E in it\n");
//                    break;
//                }
//            }
//            DEBUG_ASSERT(0);
        }

        for (int iEid : vRmEPool)
        {
            mpEPool.erase(iEid);
            /*DEBUG_PRINTF("K_TRUSS_CAN rm pool eid: %d)\n",
                         iEid);*/
        }
        for (auto atE : vTpCanE)
        {
            auto atIt = mpNecessaryCanE.find(atE);
            if (atIt == mpNecessaryCanE.end())
            {
                mpNecessaryCanE[atE] = 0;
                ++iNeCnt;
            }
            /*DEBUG_PRINTF("TABLE find bare edge fixed insert: (%d, %d)\n",
                         atE.first, atE.second);*/
        }
        if (iNeCnt > iMaxBgt)
        {
            /* exceed */
            return bBare;
        }
        //DEBUG_PRINTF("DEBUG_FIND handle bare edges done\n");
        /* filter candidate edges */
        for (itmpCanE = mpCanE.begin(); itmpCanE != mpCanE.end(); )
        {
            /*DEBUG_PRINTF("COMP_BUDGET filter current: (%d, %d)\n",
                         itmpCanE->first.first, itmpCanE->first.second);*/
            if (mpNecessaryCanE.end() != mpNecessaryCanE.find(itmpCanE->first))
            {
                /* already recored */
                for (itE = itmpCanE->second.begin(); itE != itmpCanE->second.end(); ++itE)
                {
                    //DEBUG_PRINTF("COMP_BUDGET meet edge: %d\n", *itE);
                    if (mpEPool.find(*itE) != mpEPool.end())
                    {
                        pstNode = &(mpLcEdges[*itE]);
                        //DEBUG_PRINTF("COMP_BUDGET eid: %d need: %d\n", pstNode->iEid, pstNode->iNeedSup);
                        pstNode->iNeedSup--;
                        //DEBUG_PRINTF("COMP_BUDGET need: %d\n", pstNode->iNeedSup);
                        if (0 >= pstNode->iNeedSup)
                        {
                            /* upgrade */
                            /* rm edges */
                            mpEPool.erase(*itE);
                            //DEBUG_PRINTF("K_TRUSS_CAN rm eid: %d\n", *itE);
                        }
                    }
                }
                //DEBUG_PRINTF("COMP_BUDGET filter rm\n");
                mpCanE.erase(itmpCanE++);
                continue;
            }
            ++itmpCanE;
        }
    }

    //DEBUG_PRINTF("DEBUG_FIND filter done\n");
    /** find top occupied edges */
    /* score, candidate edge */
    myPriQueueBig<int, pair<int, int> > myPirQ;
    /* fill queue */
    for (itmpCanE = mpCanE.begin(); itmpCanE != mpCanE.end(); )
    {
        int iAffCnt = 0;
        /*DEBUG_PRINTF("COMP_BUDGET fill current: (%d, %d)\n",
                     itmpCanE->first.first, itmpCanE->first.second);*/
        for (itE = itmpCanE->second.begin(); itE != itmpCanE->second.end(); )
        {
            //DEBUG_PRINTF("COMP_BUDGET meet edge: %d\n", *itE);
            if (mpEPool.find(*itE) != mpEPool.end())
            {
                /* in it */
                ++iAffCnt;
                ++itE;
            }
            else
            {
                itmpCanE->second.erase(itE);
            }
        }
        if (0 < iAffCnt)
        {
            myPirQ.insertByOrder(iAffCnt, itmpCanE->first);
        }
        else
        {
            /* rm useless edge */
            mpCanE.erase(itmpCanE++);
            //DEBUG_PRINTF("COMP_BUDGET rm useless\n");
            continue;
        }
        ++itmpCanE;
    }
    //DEBUG_PRINTF("DEBUG_FIND fill queue done\n");
    /* find all useful from large score */
    while (!(myPirQ.empty() || mpEPool.empty()))
    {
        pair<int, int> paXY = myPirQ.getTop();
        int iKey = myPirQ.getKey();
        int iAffCnt = 0;
        /*DEBUG_PRINTF("COMP_BUDGET greed find top: (%d, %d) cnt: %d\n",
                     paXY.first, paXY.second, iKey);*/
        myPirQ.pop();

        itmpCanE = mpCanE.find(paXY);
        DEBUG_ASSERT(itmpCanE != mpCanE.end());
        for (itE = itmpCanE->second.begin(); itE != itmpCanE->second.end(); )
        {
            if (mpEPool.find(*itE) != mpEPool.end())
            {
                /* in it */
                ++iAffCnt;
                ++itE;
            }
            else
            {
                itmpCanE->second.erase(itE);
            }
        }
        /* next */
        if (!myPirQ.empty())
        {
            iKey = myPirQ.getKey();
            if (iKey > iAffCnt)
            {
                /* not yet, insert back */
                myPirQ.insertByOrder(iAffCnt, paXY);
                continue;
            }
        }

        /*DEBUG_PRINTF("COMP_BUDGET greed find one: (%d, %d) cnt: %d\n",
                     paXY.first, paXY.second, iKey);*/
        /* find one */
        auto atIt = mpNecessaryCanE.find(paXY);
        if (atIt == mpNecessaryCanE.end())
        {
            mpNecessaryCanE[paXY] = 0;
            ++iNeCnt;
            if (iNeCnt > iMaxBgt)
            {
                /* exceed */
                return bBare;
            }
        }
        /* calculate score */
        float fScore = 1.0 / iAffCnt;
        for (itE = itmpCanE->second.begin(); itE != itmpCanE->second.end(); ++itE)
        {
            pstNode = &(mpLcEdges[*itE]);
            pstNode->iNeedSup--;
            pstNode->fCost += fScore;
            //DEBUG_PRINTF("SHOW eid: %d single score: %f\n", pstNode->iEid, fScore);
            if (0 >= pstNode->iNeedSup)
            {
                /* upgrade */
                /* rm edges */
                mpEPool.erase(*itE);
                //DEBUG_PRINTF("K_TRUSS_CAN eid: %d meet by (%d, %d)\n", pstNode->iEid, paXY.first, paXY.second);
            }
        }
    }
    if (!mpEPool.empty())
    {
        for (auto atmpE : mpEPool)
        {
            DEBUG_PRINTF("ERROR eid: %d\n", atmpE.first);
        }
        DEBUG_ASSERT(0);
    }

    //DEBUG_PRINTF("DEBUG_FIND save begin\n");
    /* save */
    for (itmpNsrCanE = mpNecessaryCanE.begin(); itmpNsrCanE != mpNecessaryCanE.end(); ++itmpNsrCanE)
    {
        vResCanE.push_back(itmpNsrCanE->first);
        /*DEBUG_PRINTF("COMP_BUDGET necessary candidate edge: (%d, %d)\n",
                     itmpNsrCanE->first.first, itmpNsrCanE->first.second);*/
    }
    if (NULL != poComp)
    {
        for (auto mpKey : mpLcEdges)
        {
            poComp->m_pvComp->at(poComp->m_vG2L[mpKey.first]).fCost = mpKey.second.fCost;
        }
    }
    return bBare;
}
/*****************
input:
        int x
        int y
        long double ldP
description:
        return max group id
******************/
bool Comp::findKTrussCanETry(myG &oInitG, myG &oBackG, int iMinK, int iDesK, vector<int> &vDes, vector<pair<int, int> > &vResCanE, Comp *poComp, vector<int> &vBareE, int iMaxBgt)
{
    map<int, TPST_COMP_NODE> mpLcEdges;
    vector<int>::iterator itE;
    TPST_COMP_NODE *pstNode = NULL;
    TPST_MAP_BY_EID *pstGNode = NULL;
    int iDesLayer = 0;
    bool bBare = false;

    /* candidate edge, affected edges */
    map<pair<int, int>, vector<int> > mpCanE;
    map<pair<int, int>, vector<int> >::iterator itmpCanE;
    /* candidate edge, none */
    int iNeCnt = 0;
    map<pair<int, int>, int > mpNecessaryCanE;
    map<pair<int, int>, int >::iterator itmpNsrCanE;
    vector<pair<int, int> >::iterator itpaE;
    /* cannot convert by just one edge */
    //vector <int> vBareE;
    /* affected edge, candidate edges */
    map<int, vector<pair<int, int> > > mpEPool;
    vector<pair<int, int> > vCanE;
    vector<int> vNeibE;

    /* find first candidates */
    for (itE = vDes.begin(); itE != vDes.end(); ++itE)
    {
        pstGNode = oInitG.findNode(*itE);
        DEBUG_ASSERT(NULL != pstGNode);
        if (iDesK <= pstGNode->iTrussness)
        {
            /* upgraded, ignore */
            continue;
        }

        pstNode = &(mpLcEdges[pstGNode->eid]);
        pstNode->iEid = pstGNode->eid;
        pstNode->bVstFlag = false;
        pstNode->fCost = 0;
        vector<pair<int, int> > vNeib;
        pstNode->iNeedSup = iDesK - 2 - oInitG.findKSup(iMinK, iDesK, pstGNode->vLfE, pstGNode->vRtE, poComp, pstNode->iGroupId, getAbaStateNoG);
        if (0 >= pstNode->iNeedSup)
        {
            pstNode->iNeedSup = 0;
            continue;
        }

        vCanE.clear();
        vNeibE.clear();
        oInitG.findECanTriP(pstGNode->eid, iMinK, iDesK, vNeibE, vCanE, poComp, pstNode->iGroupId, getAbaStateNoG);
        //DEBUG_PRINTF("FIND CANDIDATE e: %d need: %d candidate: %d\n", pstGNode->eid, pstNode->iNeedSup, vCanE.size());
        if ((vCanE.size() < pstNode->iNeedSup) || (0 == vCanE.size()))
        {
            /* cannot upgraded */
            vCanE.clear();
            vBareE.push_back(pstGNode->eid);
        }
        else
        {
            for (int i = 0; i < vCanE.size(); ++i)
            {
                //DEBUG_PRINTF("SHOW e: %d find candidate (%d, %d)\n", pstGNode->eid, itpaE->first, itpaE->second);
                mpCanE[vCanE[i]].push_back(pstGNode->eid);
                if (0 < vNeibE[i])
                {
                    if (mpLcEdges.find(vNeibE[i]) == mpLcEdges.end())
                    {
                        /* new */
                        pstNode = &(mpLcEdges[vNeibE[i]]);
                        pstNode->iEid = vNeibE[i];
                        pstNode->bVstFlag = false;
                        pstNode->fCost = 0;
                        pstNode->iNeedSup = 1;
                    }

                    mpCanE[vCanE[i]].push_back(vNeibE[i]);

                }
            }
            mpEPool[pstGNode->eid].swap(vCanE);
        }
        /*DEBUG_PRINTF("K_TRUSS_CAN (%d, %d) eid: %d k: %d, layer: %d find sup: %d eid: %d need: %d\n",
                     pstGNode->paXY.first, pstGNode->paXY.second, pstGNode->eid,
                     pstGNode->iTrussness, pstGNode->iLayer,
                     vCanE.size(), pstNode->iEid, pstNode->iNeedSup);*/
    }
    //DEBUG_PRINTF("K_TRUSS_CAN bare size: %d\n", vBareE.size());
    if (!vBareE.empty())
    {
        bBare = true;
        return bBare;
    }

    //DEBUG_PRINTF("DEBUG_FIND unstable: %d\n", mpEPool.size());
    /** find top occupied edges */
    /* score, candidate edge */
    myPriQueueBig<int, pair<int, int> > myPirQ;
    /* fill queue */
    for (itmpCanE = mpCanE.begin(); itmpCanE != mpCanE.end(); )
    {
        int iAffCnt = 0;
        /*DEBUG_PRINTF("COMP_BUDGET fill current: (%d, %d)\n",
                     itmpCanE->first.first, itmpCanE->first.second);*/
        COMMON_UNIQUE(itmpCanE->second);
        for (itE = itmpCanE->second.begin(); itE != itmpCanE->second.end(); )
        {
            //DEBUG_PRINTF("COMP_BUDGET meet edge: %d\n", *itE);
            pstNode = &(mpLcEdges[*itE]);
            if (0 < pstNode->iNeedSup)
            {
                /* in it */
                ++iAffCnt;
                if (mpEPool.find(*itE) != mpEPool.end())
                {
                    iAffCnt += 100;
                }
                ++itE;
            }
            else
            {
                itmpCanE->second.erase(itE);
            }
        }
        if (0 < iAffCnt)
        {
            myPirQ.insertByOrder(iAffCnt, itmpCanE->first);
            /*DEBUG_PRINTF("FIND CANDIDATE: (%d, %d) affect: %d\n",
                         itmpCanE->first.first, itmpCanE->first.second, iAffCnt);*/
        }
        else
        {
            /* rm useless edge */
            mpCanE.erase(itmpCanE++);
            //DEBUG_PRINTF("COMP_BUDGET rm useless\n");
            continue;
        }
        ++itmpCanE;
    }
    //DEBUG_PRINTF("DEBUG_FIND fill queue done\n");
    /* find all useful from large score */
    while (!(myPirQ.empty() || mpEPool.empty()))
    {
        pair<int, int> paXY = myPirQ.getTop();
        int iKey = myPirQ.getKey();
        int iAffCnt = 0;
        myPirQ.pop();
        /*DEBUG_PRINTF("FIND CANDIDATE greed find top: (%d, %d) cnt: %d\n",
                     paXY.first, paXY.second, iKey);*/

        itmpCanE = mpCanE.find(paXY);
        DEBUG_ASSERT(itmpCanE != mpCanE.end());
        for (itE = itmpCanE->second.begin(); itE != itmpCanE->second.end(); )
        {
            pstNode = &(mpLcEdges[*itE]);
            if (0 < pstNode->iNeedSup)
            {
                /* in it */
                ++iAffCnt;
                if (mpEPool.find(*itE) != mpEPool.end())
                {
                    iAffCnt += 100;
                    //DEBUG_PRINTF("DEBUG_FIND hint: %d\n", *itE);
                }
                ++itE;
            }
            else
            {
                itmpCanE->second.erase(itE);
            }
        }
        /*DEBUG_PRINTF("FIND CANDIDATE greed find top: (%d, %d) real: %d\n",
                     paXY.first, paXY.second, iAffCnt);*/
        /* next */
        if (!myPirQ.empty())
        {
            iKey = myPirQ.getKey();
            if (iKey > iAffCnt)
            {
                /* not yet, insert back */
                if (0 < iAffCnt)
                {
                    myPirQ.insertByOrder(iAffCnt, paXY);
                }
                continue;
            }
        }

        /* find one */
        /*DEBUG_PRINTF("FIND CANDIDATE greed find top: (%d, %d) next key: %d real: %d remained: %d\n",
                     paXY.first, paXY.second, iKey, iAffCnt, mpEPool.size());*/
        auto atIt = mpNecessaryCanE.find(paXY);
        if (atIt == mpNecessaryCanE.end())
        {
            mpNecessaryCanE[paXY] = 0;
            ++iNeCnt;
            if (iNeCnt > iMaxBgt)
            {
                /* exceed */
                bBare = true;
                //DEBUG_PRINTF("FIND exceed: %d max: %d\n", iNeCnt, iMaxBgt);
                return bBare;
            }
        }
        /* calculate score */
        float fScore = 1.0 / iAffCnt;
        for (itE = itmpCanE->second.begin(); itE != itmpCanE->second.end(); ++itE)
        {
            pstNode = &(mpLcEdges[*itE]);
            pstNode->iNeedSup--;
            pstNode->fCost += fScore;
            //DEBUG_PRINTF("FIND CANDIDATE eid: %d need: %d\n", pstNode->iEid, pstNode->iNeedSup);
            if (0 >= pstNode->iNeedSup)
            {
                /* upgrade */
                /* rm edges */
                mpEPool.erase(*itE);
                /*DEBUG_PRINTF("FIND CANDIDATE eid: %d meet by (%d, %d) remained: %d\n",
                             pstNode->iEid, paXY.first, paXY.second, mpEPool.size());*/
            }
        }
        /*DEBUG_PRINTF("COMP_BUDGET greed find one: (%d, %d) found: %d remained: %d\n",
                     paXY.first, paXY.second, iNeCnt, mpEPool.size());*/
    }
    if (!mpEPool.empty())
    {
        for (auto atmpE : mpEPool)
        {
            DEBUG_PRINTF("ERROR eid: %d\n", atmpE.first);
        }
        DEBUG_ASSERT(0);
    }

    //DEBUG_PRINTF("DEBUG_FIND save begin\n");
    /* save */
    for (itmpNsrCanE = mpNecessaryCanE.begin(); itmpNsrCanE != mpNecessaryCanE.end(); ++itmpNsrCanE)
    {
        vResCanE.push_back(itmpNsrCanE->first);
        /*DEBUG_PRINTF("COMP_BUDGET necessary candidate edge: (%d, %d)\n",
                     itmpNsrCanE->first.first, itmpNsrCanE->first.second);*/
    }
    return bBare;
}
#if 0
/*****************
input:
        int x
        int y
        long double ldP
description:
        return max group id
******************/
int Comp::findStableCanE(int iGroupId, vector<int> &vDes, vector<pair<int, int> > &vResCanE)
{
    //map<int, TPST_COMP_NODE> mpLcEdges;
    vector<int>::iterator itE;
    int iCurEid = 0;
    TPST_COMP_NODE *pstNode = NULL;
    TPST_MAP_BY_EID *pstGNode = NULL;
    int iSup = 0;

    /* candidate edge, affected edges */
    map<pair<int, int>, vector<int> > mpCanE;
    map<pair<int, int>, vector<int> >::iterator itmpCanE;
    /* candidate edge, none */
    map<pair<int, int>, int > mpNecessaryCanE;
    map<pair<int, int>, int >::iterator itmpNsrCanE;
    vector<pair<int, int> >::iterator itpaE;
    /* cannot convert by just one edge */
    vector <int> vBareE;
    /* affected edge, candidate edges */
    map<int, vector<pair<int, int> > > mpEPool;
    map<int, vector<pair<int, int> > >::iterator itmpE;
    vector<pair<int, int> > vCanE;

    /* find first candidates */
    //DEBUG_PRINTF("FIND init begin\n");
    for (itE = vDes.begin(); itE != vDes.end(); ++itE)
    {
        int iSelfSup = 0;
        //DEBUG_PRINTF("FIND unstable edge: %d\n", *itE);
        pstNode = findNode(*itE);
        pstGNode = m_poG->findNode(pstNode->iGEid);
        DEBUG_ASSERT(NULL != pstGNode);
        if (m_iDesK <= pstGNode->iTrussness)
        {
            /* upgraded, ignore */
            continue;
        }
        //DEBUG_ASSERT(m_iDesK == pstGNode->iTrussness);

        //DEBUG_PRINTF("FIND support begin\n");
        iSelfSup = m_poG->findKSup(m_iCurK, m_iDesK, pstGNode->vLfE, pstGNode->vRtE, this, iGroupId, getAbaState);
        //DEBUG_PRINTF("FIND support end\n");

        //pstNode = &(mpLcEdges[pstGNode->eid]);
        //pstNode->iEid = pstGNode->eid;
        pstNode->iNeedSup = m_iDesK - iSelfSup - 2;

        vCanE.clear();
        //DEBUG_PRINTF("FIND candidate begin\n");
        iSup = m_poG->findECanTriP(pstGNode->eid, m_iCurK, m_iDesK, vCanE, this, iGroupId, getAbaState);
        //DEBUG_PRINTF("FIND candidate end\n");
        if (iSup < pstNode->iNeedSup)
        {
            /* cannot upgraded */
            vCanE.clear();
        }

        for (itpaE = vCanE.begin(); itpaE != vCanE.end(); ++itpaE)
        {
            mpCanE[*itpaE].push_back(pstGNode->eid);
        }
        /* save */
        if (vCanE.empty())
        {
            vBareE.push_back(pstGNode->eid);
        }
        else
        {
            mpEPool[pstGNode->eid].swap(vCanE);
        }
    }
    //DEBUG_PRINTF("FIND bare size: %d\n", vBareE.size());
    if (!vBareE.empty())
    {
        /* increase k by finding (k+1)-clique */
        for (itE = vBareE.begin(); itE != vBareE.end(); ++itE)
        {
            myG mySubG;
            vector<pair<int, int> > vNewCanE;
            vector <int> vAffE;
            vector <int> vCanP;
            TPST_MAP_BY_EID *pstGTpNode = NULL;

            pstGNode = m_poG->findNode(*itE);
            DEBUG_ASSERT(NULL != pstGNode);

            vAffE.push_back(pstGNode->eid);

            /* find k+1 nodes */
            findKPlusNodes(*m_poG, m_iDesK, pstGNode->eid, vCanP);
            DEBUG_ASSERT(m_iDesK == vCanP.size());
            m_poG->induce(vCanP, mySubG, vNewCanE);
            //mySubG.findAllCanE(vNewCanE);
            if (vNewCanE.empty())
            {
                mySubG.show();
                DEBUG_ASSERT(0);
            }
            /* rm current edge */
            for (itmpE = mpEPool.begin(); itmpE != mpEPool.end(); )
            {
                pstGTpNode = m_poG->findNode(itmpE->first);
                DEBUG_ASSERT(NULL != pstGNode);
                if (NULL != mySubG.findNode(pstGTpNode->paXY.first, pstGTpNode->paXY.second))
                {
                    /* in the subgraph */
                    vAffE.push_back(pstGTpNode->eid);
                    mpEPool.erase(itmpE++);
                    continue;
                }
                ++itmpE;
            }
            vector<int>::iterator itTpE = itE;
            ++itTpE;
            for (; itTpE != vBareE.end(); )
            {
                pstGTpNode = m_poG->findNode(*itTpE);
                DEBUG_ASSERT(NULL != pstGNode);
                if (NULL != mySubG.findNode(pstGTpNode->paXY.first, pstGTpNode->paXY.second))
                {
                    /* in the subgraph */
                    vAffE.push_back(pstGTpNode->eid);
                    vBareE.erase(itTpE);
                    continue;
                }
                ++itTpE;
            }
            /* save score */
            float fScore = (vNewCanE.size() + 0.0 ) / vAffE.size();
            for (auto atE : vAffE)
            {
                pstNode = findNode(m_vG2L[atE]);
                pstNode->fCost = fScore;
            }
            /* save necessary edges */
            for (itpaE = vNewCanE.begin(); itpaE != vNewCanE.end(); ++itpaE)
            {
                mpNecessaryCanE[*itpaE] = 0;
            }
        }
        /* filter candidate edges */
        for (itmpCanE = mpCanE.begin(); itmpCanE != mpCanE.end(); )
        {
            if (mpNecessaryCanE.end() != mpNecessaryCanE.find(itmpCanE->first))
            {
                /* already recored */
                for (itE = itmpCanE->second.begin(); itE != itmpCanE->second.end(); ++itE)
                {
                    if (mpEPool.find(*itE) != mpEPool.end())
                    {
                        //pstNode = &(mpLcEdges[*itE]);
                        pstNode = findNode(m_vG2L[*itE]);
                        pstNode->iNeedSup--;
                        if (0 >= pstNode->iNeedSup)
                        {
                            /* upgrade */
                            /* rm edges */
                            mpEPool.erase(*itE);
                        }
                    }
                }
                mpCanE.erase(itmpCanE++);
                continue;
            }
            ++itmpCanE;
        }
    }
    //DEBUG_PRINTF("FIND init done\n");
    /** find top occupied edges */
    /* score, candidate edge */
    myPriQueueBig<int, pair<int, int> > myPirQ;
    /* fill queue */
    for (itmpCanE = mpCanE.begin(); itmpCanE != mpCanE.end(); )
    {
        int iAffCnt = 0;
        for (itE = itmpCanE->second.begin(); itE != itmpCanE->second.end(); )
        {
            if (mpEPool.find(*itE) != mpEPool.end())
            {
                /* in it */
                ++iAffCnt;
                ++itE;
            }
            else
            {
                itmpCanE->second.erase(itE);
            }
        }
        if (0 < iAffCnt)
        {
            myPirQ.insertByOrder(iAffCnt, itmpCanE->first);
        }
        else
        {
            /* rm useless edge */
            mpCanE.erase(itmpCanE++);
            continue;
        }
        ++itmpCanE;
    }
    //DEBUG_PRINTF("FIND fill queue done\n");
    /* find all useful from large score */
    while (!mpEPool.empty())
    {
        pair<int, int> paXY = myPirQ.getTop();
        int iKey = myPirQ.getKey();
        int iAffCnt = 0;
        /*DEBUG_PRINTF("COMP_BUDGET greed find top: (%d, %d) cnt: %d\n",
                     paXY.first, paXY.second, iKey);*/
        myPirQ.pop();

        itmpCanE = mpCanE.find(paXY);
        DEBUG_ASSERT(itmpCanE != mpCanE.end());
        for (itE = itmpCanE->second.begin(); itE != itmpCanE->second.end(); )
        {
            if (mpEPool.find(*itE) != mpEPool.end())
            {
                /* in it */
                ++iAffCnt;
                ++itE;
            }
            else
            {
                itmpCanE->second.erase(itE);
            }
        }
        /* next */
        iKey = myPirQ.getKey();
        if (iKey > iAffCnt)
        {
            /* not yet, insert back */
            myPirQ.insertByOrder(iAffCnt, paXY);
            continue;
        }

        /* find one */
        mpNecessaryCanE[paXY] = 0;
        /* calculate score */
        float fScore = 1.0 / iAffCnt;
        for (itE = itmpCanE->second.begin(); itE != itmpCanE->second.end(); ++itE)
        {
            pstNode = findNode(m_vG2L[*itE]);
            //pstNode = &(mpLcEdges[*itE]);
            pstNode->iNeedSup--;
            if (0 >= pstNode->iNeedSup)
            {
                /* upgrade */
                /* rm edges */
                mpEPool.erase(*itE);
            }
        }
    }
    //DEBUG_PRINTF("FIND handle done\n");
    /* save */
    for (itmpNsrCanE = mpNecessaryCanE.begin(); itmpNsrCanE != mpNecessaryCanE.end(); ++itmpNsrCanE)
    {
        vResCanE.push_back(itmpNsrCanE->first);
        /*DEBUG_PRINTF("COMP_BUDGET necessary candidate edge: (%d, %d)\n",
                     itmpNsrCanE->first.first, itmpNsrCanE->first.second);*/
    }
    return 0;
}

/*****************
input:
        int x
        int y
        long double ldP
description:
        return max group id
******************/
int Comp::findInsCanE(vector<pair<int, int> > &vIns, vector<int> &vDes, vector<pair<int, int> > &vCanE)
{
    vector<int>::iterator itE;
    vector<pair<int, int> >::iterator itpaE;
    vector<int> vChgE;
    /* eid, none */
    map<int, int> mpChgE;
    map<int, int>::iterator itmpE;
    vector<pair<int, int> > vTpCanE;
    TPST_MAP_BY_EID* pstNode = NULL;
    TPST_MAP_BY_EID* pstTpNode = NULL;

    /* insertion */
    for (itpaE = vIns.begin(); itpaE != vIns.end(); ++itpaE)
    {
        insertPart::insertOne(*m_poG, itpaE->first, itpaE->second, vChgE);
        //DEBUG_PRINTF("DEBUG insert (%d, %d)\n", itpaE->first, itpaE->second);
        for (itE = vChgE.begin(); itE != vChgE.end(); ++itE)
        {
            mpChgE[*itE] = 1;
        }
        vChgE.clear();
    }

    /* find candidate edges */
    //DEBUG_PRINTF("DEBUG truss candidate edges begin\n");
    findKTrussCanE(*m_poG, m_iCurK, m_iDesK, vDes, vCanE, NULL);
    //DEBUG_PRINTF("DEBUG truss candidate edges end\n");

    /* restore to initial G */
    for (itmpE = mpChgE.begin(); itmpE != mpChgE.end(); ++itmpE)
    {
        pstNode = m_poG->findNode(itmpE->first);
        DEBUG_ASSERT(NULL != pstNode);

        if (pstNode->eid > m_poBackG->m_iMaxEId)
        {
            m_poG->simpleRm(pstNode->paXY.first, pstNode->paXY.second);
        }
        else
        {
            (*(m_poG->m_pvG))[pstNode->eid] = (*(m_poBackG->m_pvG))[pstNode->eid];
        }
    }
    m_poG->m_iMaxEId = m_poBackG->m_iMaxEId;
    m_poG->m_iMaxD = m_poBackG->m_iMaxEId;
    m_poG->m_iMaxK = m_poBackG->m_iMaxEId;
    m_poG->m_pvG->resize(m_poG->m_iMaxEId + 1);

    return 0;
}
/*****************
input:
        int x
        int y
        long double ldP
description:
        return max group id
******************/
int Comp::findInsCanEClique(vector<pair<int, int> > &vIns, vector<int> &vDes, vector<pair<int, int> > &vCanE)
{
    vector<int>::iterator itE;
    vector<pair<int, int> >::iterator itpaE;
    vector<int> vChgE;
    /* eid, none */
    map<int, int> mpChgE;
    map<int, int>::iterator itmpE;
    vector<pair<int, int> > vTpCanE;
    TPST_MAP_BY_EID* pstNode = NULL;
    TPST_MAP_BY_EID* pstTpNode = NULL;

    /* insertion */
    for (itpaE = vIns.begin(); itpaE != vIns.end(); ++itpaE)
    {
        insertPart::insertOne(*m_poG, itpaE->first, itpaE->second, vChgE);
        //DEBUG_PRINTF("DEBUG insert (%d, %d)\n", itpaE->first, itpaE->second);
        for (itE = vChgE.begin(); itE != vChgE.end(); ++itE)
        {
            mpChgE[*itE] = 1;
        }
        vChgE.clear();
    }

    /* find candidate edges */
    //DEBUG_PRINTF("DEBUG truss candidate edges begin\n");
    vector<int> vEPool;
    vector<int> vRmEPool;
    map<int, TPST_COMP_NODE> mpLcEdges;
    //findKTrussCanEClique(*m_poG, m_iDesK, vDes, vEPool, vRmEPool, mpLcEdges, vCanE);
    findKTrussCanE(*m_poG, m_iCurK, m_iDesK, vDes, vCanE, this);
    //DEBUG_PRINTF("DEBUG truss candidate edges end\n");

    /* restore to initial G */
    for (itmpE = mpChgE.begin(); itmpE != mpChgE.end(); ++itmpE)
    {
        pstNode = m_poG->findNode(itmpE->first);
        DEBUG_ASSERT(NULL != pstNode);

        if (pstNode->eid > m_poBackG->m_iMaxEId)
        {
            m_poG->simpleRm(pstNode->paXY.first, pstNode->paXY.second);
        }
        else
        {
            (*(m_poG->m_pvG))[pstNode->eid] = (*(m_poBackG->m_pvG))[pstNode->eid];
        }
    }
    m_poG->m_iMaxEId = m_poBackG->m_iMaxEId;
    m_poG->m_iMaxD = m_poBackG->m_iMaxEId;
    m_poG->m_iMaxK = m_poBackG->m_iMaxEId;
    m_poG->m_pvG->resize(m_poG->m_iMaxEId + 1);

    return 0;
}
#endif
/*****************
input:
        int x
        int y
        long double ldP
description:
        return max group id
******************/
int Comp::findCanE(int iGroupId, vector<pair<int, int> > &vCanE)
{
    TPST_GROUP_INFO *pstCurGroup = NULL;
    vector<int>::iterator itE;
    vector<pair<int, int> > vTpCanE;

    pstCurGroup = &(m_mpGroups[iGroupId]);
    DEBUG_ASSERT(iGroupId == pstCurGroup->iGroupId);

    //vCanE = pstCurGroup->vCanE;

    for (itE = pstCurGroup->vEdges.begin(); itE != pstCurGroup->vEdges.end(); ++itE)
    {
        vTpCanE.clear();
        TPST_COMP_NODE *pstNode = findNode(*itE);
        m_poG->findECanTriP(pstNode->iGEid, m_iCurK, m_iDesK, vTpCanE, this, iGroupId, getAbaState);
        vCanE.insert(vCanE.end(), vTpCanE.begin(), vTpCanE.end());
    }
#if 0
    if (vCanE.empty())
    {
        /* direct */
        for (itE = pstCurGroup->vEdges.begin(); itE != pstCurGroup->vEdges.end(); ++itE)
        {
            vTpCanE.clear();
            m_poG->findECanTriP(*itE, vTpCanE);
            vCanE.insert(vCanE.end(), vTpCanE.begin(), vTpCanE.end());
        }
        //showGroup(iGroupId);
    }
#endif
    vCanE.insert(vCanE.end(), pstCurGroup->vCanE.begin(), pstCurGroup->vCanE.end());
    COMMON_UNIQUE(vCanE);
    //DEBUG_ASSERT(!vCanE.empty());
    return vCanE.size();
}

/*****************
input:
        int x
        int y
        long double ldP
description:
        return max group id
******************/
int Comp::getMaxBgt(int iGroupId, vector<pair<int, int> > &vNsrCanE, bool *pbBare)
{
    vector<int> vDes;
    TPST_GROUP_INFO *pstCurGroup = NULL;
    pstCurGroup = &(m_mpGroups[iGroupId]);
    vDes.resize(pstCurGroup->vFirst.size());
    for (int i = 0; i < pstCurGroup->vFirst.size(); ++i)
    {
        TPST_COMP_NODE *pstNode = findNode(pstCurGroup->vFirst[i]);
        vDes[i] = pstNode->iGEid;
    }
    vector<int> vBareE;
    *pbBare = findKTrussCanE(*m_poG, *m_poBackG, m_iCurK, m_iDesK, vDes, vNsrCanE, this, false, vBareE, m_iTotalBudget);
    if (vNsrCanE.empty())
    {
        return m_iTotalBudget + 1;
    }

    vector <int> vGEdges;
    vGEdges.resize(pstCurGroup->vEdges.size());
    for (int i = 0; i < pstCurGroup->vEdges.size(); ++i)
    {
        TPST_COMP_NODE *pstE = findNode(pstCurGroup->vEdges[i]);
        vGEdges[i] = pstE->iGEid;
    }

    vector<pair<int, int> >vSuccE;
	struct timeval tv;
	long lStartTime = 0;
	long lCurTime = 0;

    gettimeofday(&tv, NULL);
    lStartTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    int iRealScore = 0;
    int iDebugScore = backtrack::tryIns(*m_poG, *m_poBackG, m_iDesK,
                                        vNsrCanE, vGEdges, vSuccE, &iRealScore);
    gettimeofday(&tv, NULL);
    lCurTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    g_lInsertTime += lCurTime - lStartTime;

    if ((vSuccE.size() != vNsrCanE.size()) || (iDebugScore < vNsrCanE.size() + vGEdges.size()))
    {
        DEBUG_PRINTF("ERROR group: %d, size: %d total budget: %d bare: %d score: %d\n",
                 iGroupId, pstCurGroup->vEdges.size(), vNsrCanE.size(),
                 *pbBare, iDebugScore);

        map <pair<int, int>, int> mpTp;
        //DEBUG_PRINTF("ERROR candidate: \n");
        for (auto atTpE : vNsrCanE)
        {
            //DEBUG_PRINTF("(%d, %d) ", atTpE.first, atTpE.second);
            mpTp[pair<int, int>(atTpE.first, atTpE.second)];
        }
        //DEBUG_PRINTF("\n");
        //DEBUG_PRINTF("ERROR success: \n");
        for (auto atTpE : vSuccE)
        {
            //DEBUG_PRINTF("(%d, %d) ", atTpE.first, atTpE.second);
            mpTp.erase(pair<int, int>(atTpE.first, atTpE.second));
        }
        //DEBUG_PRINTF("\n");
        DEBUG_PRINTF("ERROR not success: \n");
        for (auto atTpE : mpTp)
        {
            DEBUG_PRINTF("(%d, %d) ", atTpE.first.first, atTpE.first.second);
            vector<int> vLfE;
            vector<int> vRtE;
            m_poG->findNeb(atTpE.first.first, atTpE.first.second, vLfE, vRtE);
            int iKSup = m_poG->findKSup(m_iCurK, m_iDesK, vLfE, vRtE, this, iGroupId, getAbaState);
            DEBUG_PRINTF("%d ", iKSup);
        }
        DEBUG_PRINTF("\n");
        //showGroup(iGroupId);
        DEBUG_ASSERT(0);
    }

    return vNsrCanE.size();
}

/*****************
input:
        int x
        int y
        long double ldP
description:
        return max group id
******************/
int Comp::BFSSameK(int iEid, vector <int> &vGroupE)
{
    list<int> lsQueue;
    int iCurEid = 0;
    TPST_COMP_NODE *pstNode = NULL;
    TPST_COMP_NODE *pstLfNode = NULL;
    TPST_COMP_NODE *pstRtNode = NULL;
    TPST_MAP_BY_EID *pstGNode = NULL;
    TPST_MAP_BY_EID *pstGLfNode = NULL;
    TPST_MAP_BY_EID *pstGRtNode = NULL;
    int iMinT = 0;
    vector <int> vVst;

    vector <int>::iterator itLfE;
    vector <int>::iterator itRtE;

    DEBUG_ASSERT(iEid);
    pstNode = findNode(iEid);
    DEBUG_ASSERT(NULL != pstNode);
    DEBUG_ASSERT(iEid == pstNode->iEid);
    pstGNode = m_poG->findNode(pstNode->iGEid);
    DEBUG_ASSERT(NULL != pstGNode);

    DEBUG_ASSERT(0 == pstNode->iGroupId);
    DEBUG_ASSERT(vGroupE.empty());

    int iDesK = pstGNode->iTrussness;

    lsQueue.push_back(pstNode->iEid);
    pstNode->bVstFlag = true;
    vVst.push_back(pstNode->iEid);

    while (!lsQueue.empty())
    {
        iCurEid = lsQueue.front();
        lsQueue.pop_front();

        DEBUG_ASSERT(iCurEid);
        pstNode = findNode(iCurEid);
        DEBUG_ASSERT(NULL != pstNode);
        pstGNode = m_poG->findNode(pstNode->iGEid);
        DEBUG_ASSERT(NULL != pstGNode);

        vGroupE.push_back(iCurEid);

        /*DEBUG_PRINTF("DIVIDE current edge: %d (%d, %d) k: %d bool: %d\n", pstGNode->eid,
                     pstGNode->paXY.first, pstGNode->paXY.second,
                     pstGNode->iTrussness, pstGNode->bVstFlag);*/

        /*vector <int> vNodes;
        vector <int> vEdges;
        m_poG->findPNebP(pstGNode->paXY.first, vNodes, vEdges);
        m_poG->findPNebP(pstGNode->paXY.second, vNodes, vEdges);
        for (int iGEid : vEdges)
        {
            pstGNode = m_poG->findNode(iGEid);
            DEBUG_ASSERT(NULL != pstGNode);
            if (0 >= m_vG2L[pstGNode->eid])
            {
                continue;
            }
            if ((pstGNode->iTrussness >= m_iDesK) || (pstGNode->iTrussness < m_iCurK))
            {
                continue;
            }
            pstNode = findNode(m_vG2L[pstGNode->eid]);
            DEBUG_ASSERT(NULL != pstNode);
            if (!pstNode->bVstFlag)
            {
                lsQueue.push_back(pstNode->iEid);
                pstNode->bVstFlag = true;
                vVst.push_back(pstNode->iEid);
            }
        }*/
        itLfE = pstGNode->vLfE.begin();
        itRtE = pstGNode->vRtE.begin();
        for (; itLfE != pstGNode->vLfE.end(); ++itLfE, ++itRtE)
        {
            pstGLfNode = m_poG->findNode(*itLfE);
            DEBUG_ASSERT(NULL != pstGLfNode);
            pstGRtNode = m_poG->findNode(*itRtE);
            DEBUG_ASSERT(NULL != pstGRtNode);

            pstLfNode = findNode(m_vG2L[pstGLfNode->eid]);
            DEBUG_ASSERT(NULL != pstLfNode);
            pstRtNode = findNode(m_vG2L[pstGRtNode->eid]);
            DEBUG_ASSERT(NULL != pstRtNode);

            iMinT = COMMON_MIN(pstGLfNode->iTrussness, pstGRtNode->iTrussness);
            if ((iMinT >= m_iDesK) || (iMinT < m_iCurK))
            {
                continue;
            }

            if ((pstGLfNode->iTrussness < m_iDesK) && (pstGLfNode->iTrussness >= m_iCurK)
                && (0 < pstLfNode->iEid) && (!pstLfNode->bVstFlag))
            {
                lsQueue.push_back(pstLfNode->iEid);
                pstLfNode->bVstFlag = true;
                vVst.push_back(pstLfNode->iEid);
            }
            if ((pstGRtNode->iTrussness < m_iDesK) && (pstGRtNode->iTrussness >= m_iCurK)
                && (0 < pstRtNode->iEid)  && (!pstRtNode->bVstFlag))
            {
                lsQueue.push_back(pstRtNode->iEid);
                pstRtNode->bVstFlag = true;
                vVst.push_back(pstRtNode->iEid);
            }
        }
    }

    /* restore */
    for (int iTpEid : vVst)
    {
        pstNode = findNode(iTpEid);
        DEBUG_ASSERT(NULL != pstNode);
        pstNode->bVstFlag = false;
    }
    return 0;
}

/*****************
input:
        int x
        int y
        long double ldP
description:
        return max group id
******************/
int Comp::initGroup(int iGroupId, vector <int> &vGroupE)
{
    /* new group */
    TPST_GROUP_INFO *pstCurGroup = &(m_mpGroups[iGroupId]);
    pstCurGroup->iGroupId = iGroupId;
    pstCurGroup->iMaxBgt = 0;
    pstCurGroup->iIntRatio = 100;
    pstCurGroup->bBroken = false;
    //printf("INIT_GROUP before size: %d\n", vGroupE.size());
    pstCurGroup->vEdges.swap(vGroupE);
    //printf("INIT_GROUP after size: %d\n", pstCurGroup->vEdges.size());
    int iGroupSize = pstCurGroup->vEdges.size();
    //pstCurGroup->iDesK = 0;

    for (int iCurEid : pstCurGroup->vEdges)
    {
        TPST_COMP_NODE *pstNode = findNode(iCurEid);
        DEBUG_ASSERT(NULL != pstNode);
        if (0 != pstNode->iGroupId)
        {
            printf("ERROR group: %d get another group: %d k: %d size: %d %d cur eid: %d head eid: %d\n",
                   iGroupId, pstNode->iGroupId,
                   m_iCurK,
                   iGroupSize, m_mpGroups[pstNode->iGroupId].vEdges.size(),
                   iCurEid, pstCurGroup->vEdges.front());
            DEBUG_ASSERT(0);
        }

        pstNode->iGroupId = iGroupId;
    }
    /* avoid overflow */
    if (COMP_LARGE < iGroupSize)
    {
        pstCurGroup->iIntRatio = 1;
    }
    else if (COMP_BIG < iGroupSize)
    {
        pstCurGroup->iIntRatio = 10;
    }

    return 0;
}

/*****************
input:
        int x
        int y
        long double ldP
description:
        return max group id
******************/
int Comp::divide()
{
    TPST_GROUP_INFO *pstCurGroup = NULL;
    TPST_COMP_NODE *pstNode = NULL;

    //int iMinKTrussSize = m_iDesK * m_iTotalBudget;
    //int iMaxNotKGroupSize = 10 * m_iDesK * m_iTotalBudget;
    //int iSameKCnt = 0;

    for (int iEid = 1; iEid <= m_iMaxEId; ++iEid)
    {
        pstNode = findNode(iEid);
        DEBUG_ASSERT(NULL != pstNode);

        if (iEid != pstNode->iEid)
        {
            continue;
        }

        if (0 < pstNode->iGroupId)
        {
            /* assigned */
            continue;
        }
        if (0 > pstNode->iGroupId)
        {
            /* cannot */
            continue;
        }

        vector <int> vGroupE;
        BFSSameK(iEid, vGroupE);
        ++m_iMaxGroupId;
        int iGroupSize = vGroupE.size();
        //DEBUG_PRINTF("DIVIDE find new mix group: %d size: %d\n", m_iMaxGroupId, iGroupSize);
        initGroup(m_iMaxGroupId, vGroupE);
    }
    DEBUG_PRINTF("DIVIDE group number: %d\n", m_iMaxGroupId);
    return 0;
}
/*****************
input:
        int x
        int y
        long double ldP
description:
        return max group id
******************/
int Comp::buildLayer()
{
    TPST_GROUP_INFO *pstCurGroup = NULL;
    for (int iGroupId = 1; iGroupId <= m_iMaxGroupId; ++iGroupId)
    {
        //DEBUG_PRINTF("BUILD_LAYER current group: %d\n", iGroupId);
        pstCurGroup = &(m_mpGroups[iGroupId]);
        if (m_iDesK - 1 > m_iCurK)
        {
            /* mix k */
            buildLayer(iGroupId);
        }
        else
        {
            /* using layer in k-truss */
            for (int iEid : pstCurGroup->vEdges)
            {
                TPST_COMP_NODE *pstNode = findNode(iEid);
                TPST_MAP_BY_EID *pstGNode = m_poG->findNode(pstNode->iGEid);
                DEBUG_ASSERT(NULL != pstGNode);
                pstNode->iLayer = pstGNode->iLayer;
            }
        }

        /* first layer */
        for (int iEid : pstCurGroup->vEdges)
        {
            TPST_COMP_NODE *pstNode = findNode(iEid);
            if (1 >= pstNode->iLayer)
            {
                pstCurGroup->vFirst.push_back(iEid);
            }
        }
        //showGroup(iGroupId);
    }
    return 0;
}
/*****************
input:
        int x
        int y
        long double ldP
description:
        return max group id
******************/
int Comp::getUpBgt(int iBgt)
{
    TPST_GROUP_INFO *pstCurGroup = NULL;
    int iTotalUpBgt = 0;
    int iDebugCnt = 0;
    m_iTotalBudget = iBgt;
    for (int iGroupId = 1; iGroupId <= m_iMaxGroupId; ++iGroupId)
    {
        //DEBUG_PRINTF("UP_BGT current group: %d\n", iGroupId);
        pstCurGroup = &(m_mpGroups[iGroupId]);
        /* get upper bound budget */
        pstCurGroup->vCanE.clear();
        int iBdt = getMaxBgt(iGroupId, pstCurGroup->vCanE, &(pstCurGroup->bBare));
        pstCurGroup->iMaxBgt = iBdt;
        //DEBUG_PRINTF("UP_BGT current group: %d max budget: %d canE: %d\n", iGroupId, iBdt, pstCurGroup->vCanE.size());

        /*++iDebugCnt;
        if (0 < iDebugCnt)
        {
            showGroup(iGroupId);
            DEBUG_ASSERT(0);
        }*/

        iTotalUpBgt += iBdt;
    }
    return iTotalUpBgt;
}

/*****************
input:
        int x
        int y
        long double ldP
description:
        return max group id
******************/
int Comp::findNeb(int iEid, vector<int> &vNeibE)
{
    TPST_COMP_NODE *pstNode = findNode(iEid);
    TPST_MAP_BY_EID *pstGNode = m_poG->findNode(pstNode->iGEid);
    int iSup = 0;

    DEBUG_ASSERT(vNeibE.empty());

    vector <int>::iterator itLfE = pstGNode->vLfE.begin();
    vector <int>::iterator itRtE = pstGNode->vRtE.begin();
    for (; itLfE != pstGNode->vLfE.end(); ++itLfE, ++itRtE)
    {
        TPST_MAP_BY_EID *pstGLfNode = m_poG->findNode(*itLfE);
        DEBUG_ASSERT(NULL != pstGLfNode);
        TPST_MAP_BY_EID *pstGRtNode = m_poG->findNode(*itRtE);
        DEBUG_ASSERT(NULL != pstGRtNode);

        int iMinT = COMMON_MIN(pstGLfNode->iTrussness, pstGRtNode->iTrussness);
        if ((iMinT >= m_iDesK) || (iMinT < m_iCurK))
        {
            continue;
        }
        ++iSup;

        if (0 < m_vG2L[pstGLfNode->eid])
        {
            vNeibE.push_back(m_vG2L[pstGLfNode->eid]);
        }
        if (0 < m_vG2L[pstGRtNode->eid])
        {
            vNeibE.push_back(m_vG2L[pstGRtNode->eid]);
        }
    }
    return iSup;
}
/*****************
input:
        int x
        int y
        long double ldP
description:
        return max group id
******************/
int Comp::buildLayer(int iGroupId)
{
    TPST_GROUP_INFO *pstCurGroup = NULL;
    vector <int>::iterator itLfE;
    vector <int>::iterator itRtE;
    pstCurGroup = &(m_mpGroups[iGroupId]);
    int iTpIdMax = pstCurGroup->vEdges.size();
    int iMaxSup = 0;

    //DEBUG_PRINTF("BUILD_LAYER group: %d size: %d start rename\n", iGroupId, iTpIdMax);
    /* rename */
    for (int iTpId = 0; iTpId < iTpIdMax; ++iTpId)
    {
        TPST_COMP_NODE *pstNode = findNode(pstCurGroup->vEdges[iTpId]);
        pstNode->iLocalId = iTpId;
        TPST_MAP_BY_EID *pstGNode = m_poG->findNode(pstNode->iGEid);
        vector<int> vNeibE;
        pstNode->iSup = findNeb(pstNode->iEid, vNeibE);
        iMaxSup = COMMON_MAX(iMaxSup, pstNode->iSup);
    }
    //DEBUG_PRINTF("BUILD_LAYER end rename max sup: %d\n", iMaxSup);

    //DEBUG_ASSERT(iMaxSup + 2 > m_iDesK);
    if (iMaxSup + 2 < m_iDesK)
    {
        /* all layer 1 */
        /* save result */
        for (int iTpId = 0; iTpId < iTpIdMax; ++iTpId)
        {
            int iEid = pstCurGroup->vEdges[iTpId];

            TPST_COMP_NODE *pstNode = findNode(iEid);
            DEBUG_ASSERT(NULL != pstNode);
            pstNode->iLayer = 1;
        }
        return 0;
    }
    vector<vector<int> > bin(iMaxSup + 1);
    vector<int> vSup(iTpIdMax, 0);

    for (int iTpId = 0; iTpId < iTpIdMax; ++iTpId)
    {
        TPST_COMP_NODE *pstNode = findNode(pstCurGroup->vEdges[iTpId]);
        vSup[iTpId] = pstNode->iSup;
        bin[pstNode->iSup].push_back(iTpId);
        //DEBUG_PRINTF("BUILD_LAYER tpId: %d sup: %d\n", iTpId, vSup[iTpId]);
    }
    //DEBUG_PRINTF("BUILD_LAYER end fill bins\n");

    vector<int> v_L(iTpIdMax, 0);
    vector<bool> vRmd(iTpIdMax, false);
    vector<bool> vWaitRm(iTpIdMax, false);
    int iCurL = 1;
    int iRmCnt = 0;
    vector<int> vWait;
    vWait.reserve(iTpIdMax);
    // 2.2.1. process the edges layer by layer
    while (iRmCnt < iTpIdMax)
    {
        vWait.clear();
        int iUnsPos = COMMON_MIN(m_iDesK - 1, bin.size());
        for (int i = 0; i < iUnsPos; ++i)
        {
            for (auto iTpId : bin[i])
            {
                /* avoid repetition */
                if (!vWaitRm[iTpId])
                {
                    vWait.push_back(iTpId);
                    v_L[iTpId] = iCurL;
                    vWaitRm[iTpId] = true;
                }
            }
            bin[i].clear();
        }
        ++iCurL;

        if (vWait.empty())
        {
            /* end */
            break;
        }

        for (auto iTpId : vWait)
        {
            if (vRmd[iTpId])
            {
                /* removed, continue */
                continue;
            }
            vRmd[iTpId] = true;
            ++iRmCnt;
            //DEBUG_PRINTF("BUILD_LAYER rm tpId: %d sup: %d\n", iTpId, vSup[iTpId]);

            int iEid = pstCurGroup->vEdges[iTpId];
            TPST_COMP_NODE *pstNode = findNode(iEid);
            DEBUG_ASSERT(NULL != pstNode);
            TPST_MAP_BY_EID *pstGNode = m_poG->findNode(pstNode->iGEid);
            DEBUG_ASSERT(NULL != pstGNode);

            itLfE = pstGNode->vLfE.begin();
            itRtE = pstGNode->vRtE.begin();
            for (; itLfE != pstGNode->vLfE.end(); ++itLfE, ++itRtE)
            {
                TPST_MAP_BY_EID *pstGLfNode = m_poG->findNode(*itLfE);
                DEBUG_ASSERT(NULL != pstGLfNode);
                TPST_MAP_BY_EID *pstGRtNode = m_poG->findNode(*itRtE);
                DEBUG_ASSERT(NULL != pstGRtNode);

                TPST_COMP_NODE *pstLfNode = findNode(m_vG2L[pstGLfNode->eid]);
                DEBUG_ASSERT(NULL != pstLfNode);
                TPST_COMP_NODE *pstRtNode = findNode(m_vG2L[pstGRtNode->eid]);
                DEBUG_ASSERT(NULL != pstRtNode);

                /*DEBUG_PRINTF("BUILD_LAYER self: %d neighbor: tpId: %d %d group: %d %d sup: %d %d\n", iTpId,
                             pstLfNode->iLocalId, pstRtNode->iLocalId,
                             pstLfNode->iGroupId, pstRtNode->iGroupId,
                             vSup[pstLfNode->iLocalId], vSup[pstRtNode->iLocalId]);*/

                if ((iGroupId != pstLfNode->iGroupId) &&
                    (pstGLfNode->iTrussness < m_iDesK))
                {
                    continue;
                }
                if ((iGroupId != pstRtNode->iGroupId) &&
                    (pstGRtNode->iTrussness < m_iDesK))
                {
                    continue;
                }
                vector <int> vTpQ;
                if (iGroupId == pstLfNode->iGroupId)
                {
                    if (vRmd[pstLfNode->iLocalId])
                    {
                        continue;
                    }
                    vTpQ.push_back(pstLfNode->iLocalId);
                }
                if (iGroupId == pstRtNode->iGroupId)
                {
                    if (vRmd[pstRtNode->iLocalId])
                    {
                        continue;
                    }
                    vTpQ.push_back(pstRtNode->iLocalId);
                }
                for (int iLocalId : vTpQ)
                {
                    /*DEBUG_PRINTF("BUILD_LAYER decrease neighbor: tpId: %d\n",
                                 iLocalId);*/
                    DEBUG_ASSERT(0 < vSup[iLocalId]);
                    vSup[iLocalId]--;
                    bin[vSup[iLocalId]].push_back(iLocalId);
                }
            }
        }
    }

    //DEBUG_PRINTF("BUILD_LAYER end peeling\n");
    /* save result */
    for (int iTpId = 0; iTpId < iTpIdMax; ++iTpId)
    {
        int iEid = pstCurGroup->vEdges[iTpId];
        int iGLayer = v_L[iTpId];

        TPST_COMP_NODE *pstNode = findNode(iEid);
        DEBUG_ASSERT(NULL != pstNode);
        pstNode->iLayer = iGLayer;

        if (0 >= iGLayer)
        {
            DEBUG_PRINTF("ERROR tpId: %d eid: %d\n", iTpId, iEid);
            DEBUG_ASSERT(0);
        }
    }
    //DEBUG_PRINTF("BUILD_LAYER end save\n");

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
int Comp::construct()
{
    int iEid = 0;
    TPST_MAP_BY_EID *pstGNode = NULL;

    for (iEid = 1; iEid <= m_poG->m_iMaxEId; ++iEid)
    {
        pstGNode = m_poG->findNode(iEid);
        DEBUG_ASSERT(NULL != pstGNode);

        if (iEid != pstGNode->eid)
        {
            continue;
        }

        if (pstGNode->iTrussness >= m_iDesK)
        {
            continue;
        }
        if (pstGNode->iTrussness < m_iCurK)
        {
            continue;
        }
        add(iEid);
    }
    return 0;
}

/*****************
input:
        int eid
description:
        find node
******************/
TPST_COMP_NODE * Comp::findNode(int iEid)
{
    if (!((0 <= iEid) && (iEid <= m_iMaxEId)))
    {
        printf("COMP ERROR eid: %d maxEid: %d\n",
               iEid, m_iMaxEId);
        DEBUG_ASSERT(0);
    }
    return &((*m_pvComp)[iEid]);
}













