/***************
my class G
****************/

#include "common.h"
#include "DAG.h"
/*****************
input:
        int iName
description:
        return pid
******************/
int DAG::addNode(int iName, int iWeight)
{
    TPST_DAG_NODE stTp = {0};
    TPST_DAG_NODE *pstCurN = NULL;
    unordered_map<int, int>::iterator itN;

    itN = m_umpName.find(iName);
    if (itN == m_umpName.end())
    {
        /* new */
        ++m_iMaxPid;
        m_pvDAG->push_back(stTp);
        pstCurN = findNode(m_iMaxPid);
        DEBUG_ASSERT(NULL != pstCurN);
        pstCurN->iPid = m_iMaxPid;
        pstCurN->iName = iName;
        pstCurN->iWeight = iWeight;
        pstCurN->bSrc = false;
        //pstCurN->bAnchord = false;
        pstCurN->bVistd = false;
        pstCurN->bShow = false;
        pstCurN->bInQ = false;
        pstCurN->bActive = false;
        pstCurN->bAban = false;
        m_umpName[iName] = pstCurN->iPid;
        m_iTotalScore += iWeight;
        return pstCurN->iPid;
    }
    else
    {
        return itN->second;
    }
}
/*****************
input:
        int x
        int y
description:
        add new edge
        init stage
******************/
int DAG::add(int iPName, int iCName, int iWeight)
{
    int iPtPid = 0;
    int iCdPid = 0;
    TPST_DAG_NODE *pstCurPt = NULL;
    TPST_DAG_NODE *pstCurCd = NULL;
    TPST_DAG_EDGE stEdge = {0};

    /* 0: not needed */
    iPtPid = addNode(iPName, 0);
    iCdPid = addNode(iCName, 0);

    ++m_iMaxEid;
    stEdge.iEid = m_iMaxEid;
    stEdge.iWeight = iWeight;
    stEdge.iPPid = iPtPid;
    stEdge.iCPid = iCdPid;
    m_pvE->push_back(stEdge);
    DEBUG_ASSERT(m_iMaxEid == m_pvE->at(m_iMaxEid).iEid);

    pstCurPt = findNode(iPtPid);
    DEBUG_ASSERT(NULL != pstCurPt);
    pstCurPt->vChildren.push_back(m_iMaxEid);

    pstCurCd = findNode(iCdPid);
    DEBUG_ASSERT(NULL != pstCurCd);

    pstCurCd->vParents.push_back(m_iMaxEid);

    return m_iMaxPid;
}
/*****************
input:
        int x
        int y
description:
        add new edge
        init stage
******************/
int DAG::addS(int iCName, int iWeight)
{
    int iCdPid = 0;
    TPST_DAG_NODE *pstCurPt = NULL;
    TPST_DAG_NODE *pstCurCd = NULL;
    TPST_DAG_EDGE stEdge = {0};

    /* 0: not needed */
    iCdPid = addNode(iCName, 0);

    ++m_iMaxEid;
    stEdge.iEid = m_iMaxEid;
    stEdge.iWeight = iWeight;
    stEdge.iPPid = DAG_S_ID;
    stEdge.iCPid = iCdPid;
    m_pvE->push_back(stEdge);
    DEBUG_ASSERT(m_iMaxEid == m_pvE->at(m_iMaxEid).iEid);

    pstCurPt = findNode(DAG_S_ID);
    DEBUG_ASSERT(NULL != pstCurPt);
    pstCurPt->vChildren.push_back(m_iMaxEid);

    pstCurCd = findNode(iCdPid);
    DEBUG_ASSERT(NULL != pstCurCd);
    pstCurCd->vParents.push_back(m_iMaxEid);

    return m_iMaxPid;
}
/*****************
input:
        int x
        int y
description:
        add new edge
        init stage
******************/
int DAG::addT(int iPName, int iWeight)
{
    int iPtPid = 0;
    TPST_DAG_NODE *pstCurPt = NULL;
    TPST_DAG_NODE *pstCurCd = NULL;
    TPST_DAG_EDGE stEdge = {0};

    /* 0: not needed */
    iPtPid = addNode(iPName, 0);
    pstCurPt = findNode(iPtPid);
    DEBUG_ASSERT(NULL != pstCurPt);
    /* check whether added */
    for (int iEid : pstCurPt->vChildren)
    {
        TPST_DAG_EDGE *pstE = findE(iEid);
        if (DAG_T_ID == pstE->iCPid)
        {
            /* added */
            pstE->iWeight += iWeight;
            return pstE->iEid;
        }
    }

    ++m_iMaxEid;
    stEdge.iEid = m_iMaxEid;
    stEdge.iWeight = iWeight;
    stEdge.iPPid = iPtPid;
    stEdge.iCPid = DAG_T_ID;
    m_pvE->push_back(stEdge);
    DEBUG_ASSERT(m_iMaxEid == m_pvE->at(m_iMaxEid).iEid);

    pstCurPt->vChildren.push_back(m_iMaxEid);

    pstCurCd = findNode(DAG_T_ID);
    DEBUG_ASSERT(NULL != pstCurCd);
    pstCurCd->vParents.push_back(m_iMaxEid);

    return m_iMaxEid;
}

/*****************
input:
        none
description:
        construct graph
******************/
int DAG::construct()
{
    vector <int> vVst;
    list <int> lsQ;
    TPST_DAG_NODE *pstCur = NULL;
    TPST_DAG_NODE *pstTp = NULL;

    /* add source node */
    for (auto atNode : (*m_pvDAG))
    {
        if (DAG_T_ID >= atNode.iPid)
        {
            continue;
        }
        if (atNode.vParents.empty())
        {
            /* add source node */
            addS(atNode.iName, 0);
        }
    }

    /* build layer */
    pstCur = findNode(DAG_T_ID);
    DEBUG_ASSERT(NULL != pstCur);

    pstCur->bVistd = true;
    pstCur->iLayer = 0;
    vVst.push_back(DAG_T_ID);
    lsQ.push_back(DAG_T_ID);

    while (!lsQ.empty())
    {
        int iCurP = lsQ.front();
        lsQ.pop_front();

        pstCur = findNode(iCurP);
        DEBUG_ASSERT(NULL != pstCur);

        /* parents must be higher layer*/
        for (auto atParent : pstCur->vParents)
        {
            TPST_DAG_EDGE *pstE = findE(atParent);
            if (DAG_S_ID == pstE->iPPid)
            {
                /* root node is ignored */
                continue;
            }
            pstTp = findNode(pstE->iPPid);
            DEBUG_ASSERT(NULL != pstTp);
            if (!pstTp->bVistd)
            {
                pstTp->bVistd = true;
                pstTp->iLayer = pstCur->iLayer + 1;
                m_mpLayer[pstTp->iLayer].push_back(pstTp->iPid);
                vVst.push_back(pstTp->iPid);
                lsQ.push_back(pstTp->iPid);
            }
        }
    }
    if(m_mpLayer.empty())
    {
        show();
        DEBUG_ASSERT(0);
    }
    m_iMaxLayer = m_mpLayer.rbegin()->first;
    pstCur = findNode(DAG_S_ID);
    pstCur->iLayer = m_iMaxLayer + 1;
    /* restore */
    for (auto atI : vVst)
    {
        pstCur = findNode(atI);
        DEBUG_ASSERT(NULL != pstCur);
        pstCur->bVistd = false;
    }
#if 0
    /* init stack */
    m_pvStack = new vector<TPST_STACK_BY_DEPTH>(m_iMaxLayer);
    for (int i = 0; i < m_iMaxLayer; ++i)
    {
        m_pvStack->at(i).iDepth = i;
        m_pvStack->at(i).iLayer = m_iMaxLayer - i;
        m_pvStack->at(i).iPos = 0;
    }
#endif
    return 0;
}

/*****************
input:
        none
description:
        construct graph
******************/
int DAG::constructGoldberg()
{
    vector <int> vVst;
    list <int> lsQ;
    TPST_DAG_NODE *pstCur = NULL;
    TPST_DAG_NODE *pstTp = NULL;

    m_iM = 0;
    m_iW1 = 1;
    m_iW2 = 1;
    m_fG = 0;
    m_iMaxPWeight = 0;
    m_iTotalScore = 0;

    /* count edge weights */
    for (int iNode = DAG_T_ID + 1; iNode <= m_iMaxPid; ++iNode)
    {
        int iOutWeight = 0;
        TPST_DAG_NODE *pstParent = findNode(iNode);
        m_iMaxPWeight = COMMON_MAX(m_iMaxPWeight, pstParent->iWeight);
        m_iTotalScore += pstParent->iWeight;

        for (int iTpId : pstParent->vChildren)
        {
            TPST_DAG_EDGE *pstTpE = findE(iTpId);
            iOutWeight += pstTpE->iWeight;
        }
        m_iM = COMMON_MAX(m_iM, iOutWeight);
    }

    /* build layer */
    pstCur = findNode(DAG_T_ID);
    DEBUG_ASSERT(NULL != pstCur);

    pstCur->bVistd = true;
    pstCur->iLayer = 0;
    pstCur->bAban = true;
    vVst.push_back(DAG_T_ID);
    lsQ.push_back(DAG_T_ID);

    while (!lsQ.empty())
    {
        int iCurP = lsQ.front();
        lsQ.pop_front();

        pstCur = findNode(iCurP);
        DEBUG_ASSERT(NULL != pstCur);

        /* parents must be higher layer*/
        for (auto atParent : pstCur->vParents)
        {
            TPST_DAG_EDGE *pstE = findE(atParent);
            if (DAG_S_ID == pstE->iPPid)
            {
                /* root node is ignored */
                continue;
            }
            pstTp = findNode(pstE->iPPid);
            DEBUG_ASSERT(NULL != pstTp);
            if (!pstTp->bVistd)
            {
                pstTp->bVistd = true;
                pstTp->iLayer = pstCur->iLayer + 1;
                m_mpLayer[pstTp->iLayer].push_back(pstTp->iPid);
                vVst.push_back(pstTp->iPid);
                lsQ.push_back(pstTp->iPid);
            }
        }
    }
    if(m_mpLayer.empty())
    {
        show();
        DEBUG_ASSERT(0);
    }
    m_iMaxLayer = m_mpLayer.rbegin()->first;
    pstCur = findNode(DAG_S_ID);
    pstCur->iLayer = m_iMaxLayer + 1;

    /* add source node */
    for (int iNode = DAG_T_ID + 1; iNode <= m_iMaxPid; ++iNode)
    {
        pstCur = findNode(iNode);
        /* from source node */
        addS(pstCur->iName, m_iM);
    }
    /* add sink node */
    for (int iNode = DAG_T_ID + 1; iNode <= m_iMaxPid; ++iNode)
    {
        pstCur = findNode(iNode);
        /* to sink node */
        int iSinkEid = addT(pstCur->iName, 0);
        pstCur->iSinkEid = iSinkEid;
    }
    /* restore */
    for (auto atI : vVst)
    {
        pstCur = findNode(atI);
        DEBUG_ASSERT(NULL != pstCur);
        pstCur->bVistd = false;
    }
    return 0;
}

/*****************
input:
        vector<pair<int, int> > &vInsE
description:
        return score
******************/
bool DAG::findPathDFS(int iSrc, vector <int> &vPath)
{
    vector <int> vVst;
    vector <int> vStack;
    TPST_DAG_NODE *pstN = NULL;
    TPST_DAG_NODE *pstChild = NULL;
    TPST_DAG_NODE *pstParent = NULL;
    bool bRes = false;
    int iPreLayer = 0;

    pstN = findNode(iSrc);
    pstN->bVistd = true;
    DEBUG_ASSERT(vPath.empty());
    //vPath.resize(pstN->iLayer + 1);
    vVst.push_back(iSrc);
    vStack.push_back(iSrc);

    while (!vStack.empty())
    {
        int iNode = vStack.back();
        //bool bFind = false;
        //vStack.pop_back();
        pstN = findNode(iNode);
        //DEBUG_PRINTF("DFS node: %d layer: %d Q Flag: %d\n", pstN->iName, pstN->iLayer, pstN->bInQ);
        if (pstN->bInQ)
        {
            /* handled */
            if (vPath.back() != iNode)
            {
                for (auto atN : vStack)
                {
                    DEBUG_PRINTF("DFS stack node: %d layer: %d\n", findNode(atN)->iName, findNode(atN)->iLayer);
                }
                for (auto atN : vPath)
                {
                    DEBUG_PRINTF("DFS path node: %d layer: %d\n", findNode(atN)->iName, findNode(atN)->iLayer);
                }
                DEBUG_ASSERT(0);
            }
            vStack.pop_back();
            vPath.pop_back();
            continue;
        }
        else
        {
            pstN->bInQ = true;
        }
        vPath.push_back(pstN->iPid);
        //vPath[pstN->iLayer] = pstN->iPid;

        if (DAG_T_ID == pstN->iPid)
        {
            /* bottom */
            bRes = true;
            break;
        }
        for (auto atParent : pstN->vParents)
        {
            TPST_DAG_EDGE *pstE = findE(atParent);
            /*DEBUG_PRINTF("DFS parent: %d flow: %d cap: %d\n",
                         findNode(pstE->iPPid)->iName, pstE->iFlow, pstE->iCap);*/
            /* reverse flow */
            if (pstE->iFlow <= -pstE->iCap)
            {
                /* full */
                continue;
            }
            pstParent = findNode(pstE->iPPid);
            if (!pstParent->bVistd)
            {
                /* free */
                pstParent->bVistd = true;
                vVst.push_back(pstParent->iPid);
                vStack.push_back(pstParent->iPid);
                //bFind = true;
            }
        }
        for (auto atChild : pstN->vChildren)
        {
            TPST_DAG_EDGE *pstE = findE(atChild);
            /*DEBUG_PRINTF("DFS child: %d flow: %d cap: %d\n",
                         findNode(pstE->iCPid)->iName, pstE->iFlow, pstE->iCap);*/
            if (pstE->iFlow >= pstE->iCap)
            {
                /* full */
                continue;
            }
            pstChild = findNode(pstE->iCPid);
            if (!pstChild->bVistd)
            {
                /* free */
                pstChild->bVistd = true;
                vVst.push_back(pstChild->iPid);
                vStack.push_back(pstChild->iPid);
                //bFind = true;
            }
        }
        /*if (!bFind)
        {
            vPath.pop_back();
        }*/
    }

    /* restore */
    for (int iNode : vVst)
    {
        pstN = findNode(iNode);
        pstN->bVistd = false;
        pstN->bInQ = false;
    }
    return bRes;
}
/*****************
input:
        vector<pair<int, int> > &vInsE
description:
        return score
******************/
float DAG::blockFlow(vector <int> &vPath)
{
    float iMaxFlow = m_iTotalBgt;
    int iMaxLen = vPath.size() - 1;
    TPST_DAG_NODE *pstN = NULL;
    TPST_DAG_NODE *pstNext = NULL;
    vector <int> vPosPath;
    vector <int> vNegPath;

    /* ignore target node */
    for (int i = 0; i < iMaxLen; ++i)
    {
        int iNode = vPath[i];
        int iNext = vPath[i + 1];

        pstN = findNode(iNode);
        pstNext = findNode(iNext);
        if (pstN->iLayer > pstNext->iLayer)
        {
            for (auto atChild : pstN->vChildren)
            {
                TPST_DAG_EDGE *pstE = findE(atChild);
                //DEBUG_PRINTF("BLOCK_FLOW child: %d\n", pstE->iCPid);
                if (pstE->iCPid == iNext)
                {
                    vPosPath.push_back(pstE->iEid);
                    iMaxFlow = COMMON_MIN(iMaxFlow, pstE->iCap - pstE->iFlow);
                }
            }
        }
    }
    for (int iEid : vPosPath)
    {
        TPST_DAG_EDGE *pstE = findE(iEid);
        pstE->iFlow += iMaxFlow;
        /*DEBUG_PRINTF("SHOW_DINIC block (%d, %d) positive flow: %d cap: %d \n",
                     m_pvDAG->at(pstE->iPPid).iName, m_pvDAG->at(pstE->iCPid).iName,
                     pstE->iFlow, pstE->iCap);*/

    }
    for (int iEid : vNegPath)
    {
        TPST_DAG_EDGE *pstE = findE(iEid);
        pstE->iFlow -= iMaxFlow;
        /*DEBUG_PRINTF("SHOW_DINIC block (%d, %d) negative flow: %d cap: %d \n",
                     m_pvDAG->at(pstE->iPPid).iName, m_pvDAG->at(pstE->iCPid).iName,
                     pstE->iFlow, pstE->iCap);*/

    }
    DEBUG_ASSERT(0 < iMaxFlow);

    //DEBUG_PRINTF("BLOCK_FLOW find flow: %d\n", iMaxFlow);
    return iMaxFlow;
}

/*****************
input:
        vector<pair<int, int> > &vInsE
description:
        return score
******************/
int DAG::active(int iSrc, vector <int> &vFirst)
{
    vector <int> vStack;
    vector <int> vVst;
    TPST_DAG_NODE *pstN = NULL;
    TPST_DAG_NODE *pstChild = NULL;

    pstN = findNode(iSrc);
    DEBUG_ASSERT(!pstN->bActive);
    pstN->bActive = true;
    vStack.push_back(iSrc);

    while (!vStack.empty())
    {
        int iNode = vStack.back();
        vStack.pop_back();
        pstN = findNode(iNode);

        if (1 == pstN->iLayer)
        {
            vFirst.push_back(pstN->iPid);
        }

        for (auto atChild : pstN->vChildren)
        {
            TPST_DAG_EDGE *pstE = findE(atChild);

            pstChild = findNode(pstE->iCPid);
            if ((!pstChild->bVistd) && (!pstChild->bActive))
            {
                pstChild->bVistd = true;
                pstChild->bActive = true;
                vVst.push_back(pstChild->iPid);
                vStack.push_back(pstChild->iPid);
            }
        }
    }
    /* restore */
    for (int iNode : vVst)
    {
        pstN = findNode(iNode);
        pstN->bVistd = false;
    }

    return 0;
}
/*****************
input:
        vector<pair<int, int> > &vInsE
description:
        return score
******************/
int DAG::activeFromAban(int iSrc, vector <int> &vActiveP, vector <int> &vFirst)
{
    vector <int> vStack;
    TPST_DAG_NODE *pstN = NULL;
    TPST_DAG_NODE *pstChild = NULL;

    pstN = findNode(iSrc);
    if (pstN->bActive)
    {
        return 0;
    }
    pstN->bActive = true;
    pstN->bAban = false;
    vStack.push_back(iSrc);
    vActiveP.push_back(iSrc);

    while (!vStack.empty())
    {
        int iNode = vStack.back();
        vStack.pop_back();
        pstN = findNode(iNode);

        if (1 == pstN->iLayer)
        {
            vFirst.push_back(pstN->iPid);
        }

        for (auto atChild : pstN->vChildren)
        {
            TPST_DAG_EDGE *pstE = findE(atChild);

            pstChild = findNode(pstE->iCPid);
            if (!pstChild->bActive)
            {
                pstChild->bActive = true;
                pstChild->bAban = false;
                vStack.push_back(pstChild->iPid);
                vActiveP.push_back(pstChild->iPid);
            }
        }
    }

    return 0;
}
/*****************
input:
        vector<pair<int, int> > &vInsE
description:
        return score
******************/
int DAG::anchorEdge(vector <int> &vFirst, vector <int> &vAbaP)
{
    int iAbanScore = 0;
    list <int> lsQ;
    vector <int> vVst;
    vector <int> vRawAncE;
    TPST_DAG_NODE *pstN = NULL;

    pstN = findNode(DAG_T_ID);
    pstN->bAban = true;

    for (int iNode : vFirst)
    {
        pstN = findNode(iNode);
        pstN->bVistd = true;
        pstN->bInQ = true;
        lsQ.push_back(pstN->iPid);
        vVst.push_back(pstN->iPid);
        //DEBUG_PRINTF("ANCHOR_EDGE first: %d\n", pstN->iName);
    }

    while (!lsQ.empty())
    {
        int iNode = lsQ.front();
        bool bFull = true;
        lsQ.pop_front();
        pstN = findNode(iNode);
        pstN->bInQ = false;

        //DEBUG_PRINTF("ANCHOR visit: %d children number: %d\n", pstN->iName, pstN->vChildren.size());

        for (auto atChild : pstN->vChildren)
        {
            TPST_DAG_EDGE *pstE = findE(atChild);
            TPST_DAG_NODE *pstChild = findNode(pstE->iCPid);
            /*DEBUG_PRINTF("ANCHOR edge (%d, %d) flow: %d cap: %d\n",
                         pstN->iName, pstChild->iName, pstE->iFlow, pstE->iCap);*/
            if (pstChild->bAban && (pstE->iFlow < pstE->iCap))
            {
                bFull = false;
                break;
            }

        }
        if (!bFull)
        {
            /* abandon all connected nodes */
            list <int> lsAban;
            lsAban.push_back(pstN->iPid);
            while (!lsAban.empty())
            {
                int iAbanP = lsAban.front();
                lsAban.pop_front();
                TPST_DAG_NODE *pstAban = findNode(iAbanP);
                //DEBUG_PRINTF("ANCHOR_EDGE abandon: %d\n", pstAban->iName);

                pstAban->bAban = true;
                //pstAban->bAnchord = false;
                if (pstAban->bSrc)
                {
                    //show();
                    DEBUG_PRINTF("ERROR abandon source node: %d\n", pstAban->iName);
                    DEBUG_ASSERT(0);
                }
                vAbaP.push_back(pstAban->iPid);
                iAbanScore += pstAban->iWeight;
                /* abandon children */
                for (auto atChild : pstAban->vChildren)
                {
                    TPST_DAG_EDGE *pstE = findE(atChild);
                    TPST_DAG_NODE *pstChild = findNode(pstE->iCPid);

                    if (!pstChild->bAban)
                    {
                        //pstChild->bAban = true;
                        //pstChild->bAnchord = false;
                        lsAban.push_back(pstChild->iPid);
                        //vAbaP.push_back(pstChild->iPid);
                        //DEBUG_PRINTF("ANCHOR_EDGE abandon child: %d\n", pstChild->iName);

                        //iAbanScore += pstChild->iWeight;
                    }
                }
                /* abandon parent */
                for (auto atParent : pstAban->vParents)
                {
                    TPST_DAG_EDGE *pstE = findE(atParent);
                    TPST_DAG_NODE *pstParent = findNode(pstE->iPPid);
                    if (pstParent->bActive && (!pstParent->bInQ))
                    {
                        pstParent->bInQ = true;
                        lsQ.push_back(pstParent->iPid);
                        if (!pstParent->bVistd)
                        {
                            pstParent->bVistd = true;
                            vVst.push_back(pstParent->iPid);
                        }
                    }
                }
            }

        }
    }

    /* restore */
    for (int iNode : vVst)
    {
        pstN = findNode(iNode);
        pstN->bVistd = false;
    }
    return iAbanScore;
}

/*****************
input:
        vector<pair<int, int> > &vInsE
description:
        return score
******************/
int DAG::abandonNodes(vector <int> &vAbaP)
{
    vector<int> vStack;
    int iValueCnt = 0;
    for (int iNode = DAG_T_ID + 1; iNode <= m_iMaxPid; ++iNode)
    {
        TPST_DAG_NODE *pstN = findNode(iNode);
        TPST_DAG_EDGE *pstE = findE(pstN->iSinkEid);

        if (pstE->iCap - pstE->iFlow > 1e-6)
        {
            /* not full */
            pstN->bAban = true;
            vStack.push_back(iNode);
            vAbaP.push_back(iNode);
            iValueCnt += pstN->iWeight;
        }
    }
    while (!vStack.empty())
    {
        int iCurNode = vStack.back();
        vStack.pop_back();
        TPST_DAG_NODE *pstN = findNode(iCurNode);
        for (int iParentEid : pstN->vParents)
        {
            TPST_DAG_EDGE *pstE = findE(iParentEid);
            if (DAG_S_ID == pstE->iPPid)
            {
                continue;
            }
            TPST_DAG_NODE *pstParent = findNode(pstE->iPPid);
            if (!(pstParent->bAban))
            {
                if (pstE->iCap - pstE->iFlow > 1e-6)
                {
                    /* not full */
                    pstParent->bAban = true;
                    vStack.push_back(pstParent->iPid);
                    vAbaP.push_back(pstParent->iPid);
                    iValueCnt += pstParent->iWeight;
                }
            }
        }
    }
    return iValueCnt;
}
/*****************
input:
        vector<pair<int, int> > &vInsE
description:
        return score
******************/
int DAG::getRealFlow()
{
    int iFlow = 0;
    for (int iNode = DAG_T_ID + 1; iNode <= m_iMaxPid; ++iNode)
    {
        TPST_DAG_NODE *pstN = findNode(iNode);
        if (!(pstN->bAban))
        {
            for (int iChildEid : pstN->vChildren)
            {
                TPST_DAG_EDGE *pstE = findE(iChildEid);
                TPST_DAG_NODE *pstChild = findNode(pstE->iCPid);
                if (pstChild->bAban)
                {
                    DEBUG_ASSERT(pstE->iCap - pstE->iFlow < 1e-6);
                    iFlow += pstE->iWeight;
                }
            }
        }
    }
    return iFlow;
}

/*****************
input:
        vector<pair<int, int> > &vInsE
description:
        return score
******************/
int DAG::anchor(int iSrc, vector <int> &vFirst, vector <int> &vAbaP)
{
    list <int> lsQ;
    vector <int> vStack;
    vector <int> vRawAncP;
    vector <int> vVst;
    vector <int> vActive;
    map<int, int> mpAncP;
    TPST_DAG_NODE *pstN = NULL;
    int iAbanScore = 0;

    iAbanScore += anchorEdge(vFirst, vAbaP);
    //DEBUG_PRINTF("ANCHOR first abandon size: %d\n", vAbaP.size());
    /* save abandon */
    for (int i = DAG_T_ID + 1; i <= m_iMaxPid; ++i)
    {
        pstN = findNode(i);
        if (!pstN->bActive)
        {
            DEBUG_ASSERT(!pstN->bAban);
            vAbaP.push_back(pstN->iPid);
            iAbanScore += pstN->iWeight;
        }
        else
        {
            vActive.push_back(pstN->iPid);
        }
    }
    return iAbanScore;
}

/*****************
input:
        vector<pair<int, int> > &vInsE
description:
        return score
******************/
int DAG::saveRes(int iFlow, int iScore, vector <int> &vAbaP)
{
    int iPos = 0;
    int iRecordScore = 0;

    if (m_mpScoreTable.find(iFlow) == m_mpScoreTable.end())
    {
        /* new */
        iPos = m_vRes.size();
        m_vRes.resize(iPos + 1);
        m_mpScoreTable[iFlow] = pair<int, int>(iScore, iPos);
    }
    else
    {
        iRecordScore = m_mpScoreTable[iFlow].first;
        iPos = m_mpScoreTable[iFlow].second;
    }
    if (iScore > iRecordScore)
    {
        m_mpScoreTable[iFlow].first = iScore;

        m_vRes[iPos].iScore = iScore;
        m_vRes[iPos].iFlow = iFlow;

        m_vRes[iPos].vAbaP.resize(vAbaP.size());
        /* rename */
        for (int i = 0; i < vAbaP.size(); ++i)
        {
            int iNode = vAbaP[i];
            TPST_DAG_NODE *pstN = findNode(iNode);
            m_vRes[iPos].vAbaP[i] = pstN->iName;
            //DEBUG_ASSERT(!(pstN->bActive && pstN->bSrc));
        }
    }
#if 0
    iPos = m_vRes.size();
    m_vRes.resize(iPos + 1);
    m_vRes[iPos].iScore = iScore;
    m_vRes[iPos].iFlow = iFlow;

    m_vRes[iPos].vAbaP.resize(vAbaP.size());
    /* rename */
    for (int i = 0; i < vAbaP.size(); ++i)
    {
        int iNode = vAbaP[i];
        TPST_DAG_NODE *pstN = findNode(iNode);
        m_vRes[iPos].vAbaP[i] = pstN->iName;
        //DEBUG_ASSERT(!(pstN->bActive && pstN->bSrc));
    }
#endif

    return iPos;
}
/*****************
input:
        vector<pair<int, int> > &vInsE
description:
        return score
******************/
int DAG::clearFlow(int iSrc)
{
    vector <int> vStack;
    vector <int> vVst;
    TPST_DAG_NODE *pstN = NULL;

    pstN = findNode(iSrc);
    pstN->bVistd = true;
    vVst.push_back(iSrc);
    vStack.push_back(iSrc);

    while (!vStack.empty())
    {
        int iNode = vStack.back();
        vStack.pop_back();
        pstN = findNode(iNode);
        //pstN->bAnchord = false;
        pstN->bActive = false;
        pstN->bAban = false;

        for (auto atChild : pstN->vChildren)
        {
            TPST_DAG_EDGE *pstE = findE(atChild);
            pstE->iFlow = 0;
            TPST_DAG_NODE *pstChild = findNode(pstE->iCPid);
            if (!pstChild->bVistd)
            {
                pstChild->bVistd = true;
                vVst.push_back(pstChild->iPid);
                vStack.push_back(pstChild->iPid);
            }
        }

    }
    /* restore */
    for (int iNode : vVst)
    {
        pstN = findNode(iNode);
        pstN->bVistd = false;
    }
    return 0;
}

/*****************
input:
        vector<pair<int, int> > &vInsE
description:
        return score
******************/
int DAG::clearFlow()
{
    TPST_DAG_NODE *pstN = NULL;
    TPST_DAG_EDGE *pstE = NULL;

    for (int i = 1; i <= m_iMaxEid; ++i)
    {
        pstE = findE(i);
        pstE->iFlow = 0;
    }
    for (int i = DAG_T_ID + 1; i <= m_iMaxPid; ++i)
    {
        pstN = findNode(i);
        //pstN->bAnchord = false;
        pstN->bVistd = false;
        pstN->bActive = false;
        pstN->bAban = false;
    }
    return 0;
}
/*****************
input:
        vector<pair<int, int> > &vInsE
description:
        return score
******************/
float DAG::Dinic(int iSrc)
{
    vector <int> vVst;
    /* layer, node */
    vector <int> vPath;
    TPST_DAG_NODE *pstN = NULL;
    bool bRes = true;
    float iFlow = 0;

    while (1)
    {
        vPath.clear();
        bRes = findPathDFS(iSrc, vPath);
        if (bRes)
        {
            iFlow += blockFlow(vPath);
            /* show */
            /*DEBUG_PRINTF("SHOW_DINIC find flow: %d path: \n", iFlow);
            for (int iNode : vPath)
            {
                pstN = findNode(iNode);
                DEBUG_PRINTF("%d ", pstN->iName);
            }
            DEBUG_PRINTF("\n");*/
        }
        else
        {
            break;
        }
    }

    return iFlow;
}

/*****************
input:
        vector<pair<int, int> > &vInsE
description:
        return score
******************/
int DAG::externCap()
{
    vector <int> vStack;
    vector <int> vVst;
    TPST_DAG_NODE *pstN = NULL;
    int iExCnt = 0;

    pstN = findNode(DAG_S_ID);
    pstN->bVistd = true;
    vVst.push_back(DAG_S_ID);
    vStack.push_back(DAG_S_ID);

    while (!vStack.empty())
    {
        int iNode = vStack.back();
        vStack.pop_back();
        pstN = findNode(iNode);

        if (1 == pstN->iLayer)
        {
            continue;
        }

        for (auto atChild : pstN->vChildren)
        {
            TPST_DAG_EDGE *pstE = findE(atChild);
            if ((pstE->iCap < m_iTotalBgt) && (pstE->iFlow >= pstE->iCap))
            {
                pstE->iCap = m_iTotalBgt;
                ++iExCnt;
            }
        }
        /*if (pstN->bAnchord)
        {
            continue;
        }*/
        for (auto atChild : pstN->vChildren)
        {
            TPST_DAG_EDGE *pstE = findE(atChild);
            TPST_DAG_NODE *pstChild = findNode(pstE->iCPid);
            if (!pstChild->bVistd)
            {
                pstChild->bVistd = true;
                vVst.push_back(pstChild->iPid);
                vStack.push_back(pstChild->iPid);
            }
        }

    }
    /* restore */
    for (int iNode : vVst)
    {
        pstN = findNode(iNode);
        pstN->bVistd = false;
    }
    return iExCnt;
}

/*****************
input:
        vector<pair<int, int> > &vInsE
description:
        return score
******************/
int DAG::initMinCut()
{
    TPST_DAG_EDGE *pstE = NULL;

    for (int i = 1; i <= m_iMaxEid; ++i)
    {
        pstE = findE(i);
        pstE->iCap = pstE->iWeight;
        pstE->iFlow = 0;
    }

    return 0;
}
/*****************
input:
        vector<pair<int, int> > &vInsE
description:
        return score
******************/
int DAG::initGoldberg(int iW1, int iW2, float f_g)
{
    TPST_DAG_EDGE *pstE = NULL;

    for (int i = 1; i <= m_iMaxEid; ++i)
    {
        pstE = findE(i);

        if (DAG_T_ID == pstE->iCPid)
        {
            int iOutWeight = 0;
            TPST_DAG_NODE *pstParent = findNode(pstE->iPPid);
            if (1 < pstParent->iLayer)
            {
                for (int iTpId : pstParent->vChildren)
                {
                    TPST_DAG_EDGE *pstTpE = findE(i);
                    iOutWeight += pstTpE->iWeight;
                }
            }
            /* to sink node */
            float fTp = pstE->iWeight + f_g - iW1 * pstParent->iLayer - iW2 * pstParent->iWeight - iOutWeight;
            pstE->iCap = COMMON_MAX(fTp, 0);
        }
        else
        {
            pstE->iCap = pstE->iWeight;
        }
        pstE->iFlow = 0;
    }

    return 0;
}
/*****************
input:
        vector<pair<int, int> > &vInsE
description:
        return score
******************/
int DAG::runMinCut(int iMaxBudget)
{
    TPST_DAG_NODE *pstN = NULL;
    vector <int> vAbaP;
    vector <int> vFirst;
    int iAbanScore = 0;
    int iCurScore = 0;
    int iFlow = 0;
    //DEBUG_PRINTF("SHOW_DINIC cut begin\n");

    m_iTotalBgt = iMaxBudget;

    initMinCut();
    m_vRes.clear();
    /*m_vRes.resize(iMaxBudget + 1);
    for (int i = 0; i <= iMaxBudget; ++i)
    {
        m_vRes[i].iScore = 0;
    }*/

    /* order */
    /* layer, <node> */
    map<int, vector<int> > mpLayer;
    map<int, vector<int> >::reverse_iterator ritL;
    vector <int> vSrcP;
    pstN = findNode(DAG_S_ID);
    for (auto atChild : pstN->vChildren)
    {
        TPST_DAG_EDGE *pstE = findE(atChild);
        TPST_DAG_NODE *pstChild = findNode(pstE->iCPid);
        mpLayer[pstChild->iLayer].push_back(pstChild->iPid);
        pstChild->bSrc = true;
    }
    for (ritL = mpLayer.rbegin(); ritL != mpLayer.rend(); ++ritL)
    {
        for (int iNode : ritL->second)
        {
            vSrcP.push_back(iNode);
        }
    }
    /* individual */
    for (int iNode : vSrcP)
    {
        iFlow = Dinic(iNode);
        if (iFlow > iMaxBudget)
        {
            clearFlow(iNode);
            continue;
        }
        //DEBUG_PRINTF("SHOW_DINIC individual: %d\n", findNode(iNode)->iName);
        vFirst.clear();
        active(iNode, vFirst);
        vAbaP.clear();
        iAbanScore = anchor(iNode, vFirst, vAbaP);
        iCurScore = m_iTotalScore - iAbanScore;
        saveRes(iFlow, iCurScore, vAbaP);
        clearFlow(iNode);
    }
    /* together */
    int iPreFlow = 0;
    bool bFirst = true;
    //bool bFirstNode = true;
    bool bEnd = false;
    while (1)
    {
        vFirst.clear();
        iFlow = 0;
        for (int iNode : vSrcP)
        {
            iFlow += Dinic(iNode);
            //DEBUG_PRINTF("SHOW_DINIC----------- together: %d flow: %d\n", findNode(iNode)->iName, iFlow);
            if (iFlow > m_iTotalBgt)
            {
                bEnd = true;
                //DEBUG_PRINTF("SHOW_DINIC over flow: %d max: %d\n", iFlow, m_iTotalBgt);
                break;
            }

            active(iNode, vFirst);

            /*if (bFirstNode)
            {
                bFirstNode = false;
                continue;
            }*/

            vAbaP.clear();
            iAbanScore = anchor(iNode, vFirst, vAbaP);
            if (0 == iAbanScore)
            {
                /* all kept */
                bEnd = true;
                break;
            }
            iCurScore = m_iTotalScore - iAbanScore;
            saveRes(iFlow, iCurScore, vAbaP);

            /* restore */
            for (int iAbanP : vAbaP)
            {
                TPST_DAG_NODE *pstAbanP = findNode(iAbanP);
                pstAbanP->bAban = false;
                //DEBUG_PRINTF("SHOW_DINIC reset abandon node: %d\n", pstAbanP->iName);
            }
        }
        //DEBUG_PRINTF("SHOW_DINIC Dinic get flow: %d max: %d\n", iFlow, m_iTotalBgt);
        if (bEnd)
        {
            break;
        }
        DEBUG_ASSERT(iFlow >= iPreFlow);

        /*anchor(vAbaP, vPriAncE);
        if (!vPriAncE.empty())
        {
            vSecAncP.clear();
            TPST_RES stRes;
            stRes.vAbaP = vAbaP;
            stRes.vPriAncE = vPriAncE;
            m_vRes.push_back(stRes);
            ++iInsCnt;
        }*/

        iPreFlow = iFlow;
        if (iFlow > m_iTotalBgt)
        {
            break;
        }
        /* extern cap */
        int iExCnt = 0;
        iExCnt = externCap();
        /*if (bFirst)
        {
            bFirst = false;
            iExCnt = externCap();
        }
        else
        {
            iExCnt = externCap(vPriAncE);
        }*/
        //DEBUG_PRINTF("SHOW_DINIC Dinic extern: %d\n", iExCnt);
        if (0 >= iExCnt)
        {
            break;
        }
        /* Clear */
        clearFlow();
        /* show */
        /*for (int iIns = 0; iIns < m_vRes.size(); ++iIns)
        {
            if (0 >= m_vRes[iIns].iScore)
            {
                continue;
            }
            DEBUG_PRINTF("SHOW_DINIC res %d score: %d prime anchor: ", iIns, m_vRes[iIns].iScore);
            for (auto atE : m_vRes[iIns].vPaAncE)
            {
                DEBUG_PRINTF(" (%d, %d) ", atE.first, atE.second);
            }
            DEBUG_PRINTF("\nSHOW_DINIC res second anchor: ");
            for (int iNode : m_vRes[iIns].vSecAncP)
            {
                DEBUG_PRINTF(" %d ", iNode);
            }
            DEBUG_PRINTF("\n");
        }
        show();*/
    }

    /* show */
    /*for (int iIns = 0; iIns < m_vRes.size(); ++iIns)
    {
        if (0 >= m_vRes[iIns].iScore)
        {
            continue;
        }
        DEBUG_PRINTF("SHOW_DINIC final res %d score: %d prime anchor: ", iIns, m_vRes[iIns].iScore);
        for (auto atE : m_vRes[iIns].vPaAncE)
        {
            DEBUG_PRINTF(" (%d, %d) ", atE.first, atE.second);
        }
        DEBUG_PRINTF("\nSHOW_DINIC res second anchor: ");
        for (int iNode : m_vRes[iIns].vSecAncP)
        {
            DEBUG_PRINTF(" %d ", iNode);
        }
        DEBUG_PRINTF("\n");
    }*/
    return 0;
}

/*****************
input:
        vector<pair<int, int> > &vInsE
description:
        return score
******************/
int DAG::expand(map <int, int> &mpAbanPool, int *piTotalCut)
{
    TPST_DAG_NODE *pstN = NULL;
    /* pid, total cut cost */
    map <int, int> mpCanPool;

    *piTotalCut = 0;

    for (auto atN : mpAbanPool)
    {
        pstN = findNode(atN.first);
        for (auto atParent : pstN->vParents)
        {
            TPST_DAG_EDGE *pstE = findE(atParent);
            TPST_DAG_NODE *pstParent = findNode(pstE->iPPid);
            if (pstParent->bAban)
            {
                continue;
            }
            map <int, int>::iterator itmpCanP = mpCanPool.find(pstParent->iPid);
            *piTotalCut += pstE->iWeight;
            if (itmpCanP == mpCanPool.end())
            {
                mpCanPool[pstParent->iPid] = pstE->iWeight;
            }
            else
            {
                itmpCanP->second += pstE->iWeight;
            }
        }
    }
    int iMaxCost = 0;
    int iMaxPid = 0;
    for (auto atN : mpCanPool)
    {
        if (atN.second > iMaxCost)
        {
            iMaxCost = atN.second;
            iMaxPid = atN.first;
        }
    }

    if ( 0 >= iMaxPid)
    {
        show();

        DEBUG_PRINTF("DAG ERROR candidate size: %d\n", mpCanPool.size());
        for (auto atN : mpCanPool)
        {
            DEBUG_PRINTF("DAG ERROR candidate weight: %d\n", atN.second);
        }
        for (int i = DAG_T_ID + 1; i <= m_iMaxPid; ++i)
        {
            pstN = findNode(i);
            if (!pstN->bAban)
            {
                DEBUG_PRINTF("DAG ERROR not abandon %d\n", pstN->iName);
            }
        }
//        for (auto atN : mpAbanPool)
//        {
//            pstN = findNode(atN.first);
//            DEBUG_PRINTF("DAG ERROR abandon %d\n", pstN->iName);
//        }
//        debug_assert(0);
    }

    return iMaxPid;
}
/*****************
input:
        vector<pair<int, int> > &vInsE
description:
        return score
******************/
int DAG::sizeFirst(int iMaxBudget)
{
    TPST_DAG_NODE *pstN = NULL;
    vector <int> vAbaP;
    int iAbanScore = 0;
    int iCurScore = 0;
    int iFlow = 0;

    m_iTotalBgt = iMaxBudget;
    initMinCut();
    m_vRes.clear();

    /* abandon all nodes */
    for (int iNode = DAG_T_ID; iNode <= m_iMaxPid; ++iNode)
    {
        pstN = findNode(iNode);
        pstN->bAban = true;
        pstN->bActive = false;
    }

    /* sort nodes, increasing order */
    /* pid, weight */
    vector <pair<int, int> > vCanP;
    for (int iNode = DAG_T_ID + 1; iNode <= m_iMaxPid; ++iNode)
    {
        pstN = findNode(iNode);
        vCanP.push_back(pair<int, int>(iNode, pstN->iWeight));
    }
    sort(vCanP.begin(), vCanP.end(), [](pair<int, int> &N1, pair<int, int> &N2){
                                    return (N1.second < N2.second);});

    vector <int> vSrcP;
    vector <int> vFirst;
    vector <int> vActiveP;

    while (!vCanP.empty())
    {
        int iNewSrcP = vCanP.back().first;
        vCanP.pop_back();
        vSrcP.push_back(iNewSrcP);
        activeFromAban(iNewSrcP, vActiveP, vFirst);
        DEBUG_PRINTF("DAG SIZE_FIRST add source %d\n", findNode(iNewSrcP)->iName);

        bool bEnd = false;
        for (int iNode : vSrcP)
        {
            iFlow += Dinic(iNode);
            if (iFlow > m_iTotalBgt)
            {
                bEnd = true;
                break;
            }
        }
        if (bEnd)
        {
            break;
        }

        vector <int> vNewAbaP;
        anchorEdge(vFirst, vNewAbaP);

        iCurScore = 0;
        for (int iActiveP : vActiveP)
        {
            TPST_DAG_NODE *pstActiveP = findNode(iActiveP);
            if (!pstActiveP->bAban)
            {
                iCurScore += pstActiveP->iWeight;
            }
        }
        /* find all abandon nodes */
        vAbaP.clear();
        for (int iNode = DAG_T_ID + 1; iNode <= m_iMaxPid; ++iNode)
        {
            pstN = findNode(iNode);
            if (pstN->bAban)
            {
                vAbaP.push_back(iNode);
            }
        }
        saveRes(iFlow, iCurScore, vAbaP);

        /* restore */
        for (int iAbanP : vNewAbaP)
        {
            TPST_DAG_NODE *pstAbanP = findNode(iAbanP);
            pstAbanP->bAban = false;
            DEBUG_PRINTF("DAG SIZE_FIRST new abandon: %d\n", pstAbanP->iName);
        }
        DEBUG_PRINTF("DAG SIZE_FIRST flow: %d size: %d\n", iFlow, iCurScore);

        /* Clear */
        //clearFlow();
    }

    return 0;
}
/*****************
input:
        vector<pair<int, int> > &vInsE
description:
        return score
******************/
int DAG::Goldberg(int iMaxBudget)
{
    TPST_DAG_NODE *pstN = NULL;

    vector<int> vW2 = {1, 10};

    m_iTotalBgt = iMaxBudget;
    m_vRes.clear();

    /* init all nodes */
    for (int iNode = DAG_T_ID + 1; iNode <= m_iMaxPid; ++iNode)
    {
        pstN = findNode(iNode);
        pstN->bAban = false;
        pstN->bActive = false;
    }

    for (int iCurW2 : vW2)
    {
        m_iW2 = iCurW2;
        float fMin = 0;
        float fMax = m_iW1 * m_iMaxLayer + m_iW2 * m_iMaxPWeight + 2 * m_iM;
        priority_queue <pair<float, float>, vector<pair<float, float> >, less<pair<float, float> > > prqQueue;
        prqQueue.push(pair<float, float>(1, fMin));
        prqQueue.push(pair<float, float>(1, fMax));
        map<float, int> mpHandled;
        map<float, int>::iterator itmp;
        int iMpCnt = 0;
        while (!prqQueue.empty())
        {
            float fCur_g = prqQueue.top().second;
            prqQueue.pop();

            initGoldberg(m_iW1, m_iW2, fCur_g);
            /* cut */
            int iFlow = Dinic(DAG_S_ID);
            vector <int> vNewAbaP;
            int iAbanScore = abandonNodes(vNewAbaP);
            int iRealFlow = getRealFlow();
            if (iAbanScore >= m_iTotalScore)
            {
                /* too big */
            }
            else
            {
                /* find right hand */
                itmp = mpHandled.upper_bound(fCur_g);
                if (itmp != mpHandled.end())
                {
                    float fUpper_g = itmp->first;
                    DEBUG_ASSERT(fUpper_g > fCur_g);
                    if (iAbanScore < itmp->second - 1)
                    {
                        /* valuable */
                        //lsQueue.push_back((fCur_g + fUpper_g) / 2);
                        /* deduced when it's in long turn */
                        prqQueue.push(pair<float, float>((itmp->second - iAbanScore) * (1 - (iMpCnt + 0.0)/DAG_FRAG),
                                                         (fCur_g + fUpper_g) / 2));
                    }
                }
            }
            if (0 >= iAbanScore)
            {
                /* too small */
            }
            else
            {
                /* find left hand */
                itmp = mpHandled.upper_bound(fCur_g);
                if (itmp != mpHandled.begin())
                {
                    --itmp;
                    float fLower_g = itmp->first;
                    DEBUG_ASSERT(fLower_g < fCur_g);
                    if (iAbanScore > itmp->second + 1)
                    {
                        /* valuable */
                        //lsQueue.push_back((fCur_g + fLower_g) / 2);
                        prqQueue.push(pair<float, float>((iAbanScore - itmp->second) * (1 - (iMpCnt + 0.0)/DAG_FRAG),
                                                         (fCur_g + fLower_g) / 2));
                    }
                }
            }
            /* insert */
            mpHandled[fCur_g] = iAbanScore;
            if ((0 < iAbanScore) && (iAbanScore < m_iTotalScore))
            {
                saveRes(iRealFlow, m_iTotalScore - iAbanScore, vNewAbaP);
            }

            /* restore */
            for (int iAbanP : vNewAbaP)
            {
                TPST_DAG_NODE *pstAbanP = findNode(iAbanP);
                pstAbanP->bAban = false;
                //DEBUG_PRINTF("DAG SIZE_FIRST new abandon: %d\n", pstAbanP->iName);
            }
            //DEBUG_PRINTF("DAG Goldberg g: %f flow: %d size: %d\n", fCur_g, iRealFlow, m_iTotalScore - iAbanScore);
            ++iMpCnt;
            if (iMpCnt > DAG_FRAG)
            {
                /* end */
                break;
            }
        }
    }

    return 0;
}
/*****************
input:
        vector<pair<int, int> > &vInsE
description:
        return score
******************/
int DAG::prim(int iMaxBudget)
{
    TPST_DAG_NODE *pstN = NULL;
    vector <int> vAbaP;
    /* pid, weight */
    map <int, int> mpAbanPool;
    int iAbanScore = 0;
    int iCurScore = 0;
    int iFlow = 0;

    m_iTotalBgt = iMaxBudget;

    m_vRes.clear();

    pstN = findNode(DAG_T_ID);
    mpAbanPool[DAG_T_ID] = pstN->iWeight;
    pstN->bAban = true;
    int iRemainW = m_iTotalScore - pstN->iWeight;

    bool bFirst = true;

    while (0 < iRemainW)
    {
        int iCut = 0;
        int iNextP = expand(mpAbanPool, &iCut);
        if (0 >= iNextP)
        {
            DEBUG_PRINTF("DAG ERROR remained score: %d\n", iRemainW);
            debug_assert(0);
        }

        if (bFirst)
        {
            bFirst = false;
        }
        else
        {
            if (iCut <= m_iTotalBgt)
            {
                saveRes(iCut, iRemainW, vAbaP);
            }
        }

        pstN = findNode(iNextP);
        iRemainW -= pstN->iWeight;
        vAbaP.push_back(iNextP);
        mpAbanPool[iNextP] = pstN->iWeight;
        pstN->bAban = true;
    }

    return 0;
}
/*****************
input:
        none
description:
        show graph
******************/
int DAG::show()
{
    int iPid = 0;
    list<int> lsQ;
    vector <int> vVst;
    TPST_DAG_NODE *pstCurN = NULL;
    TPST_DAG_NODE *pstTpN = NULL;

    DEBUG_PRINTF("SHOW_DAG begin\n");

    pstCurN = findNode(DAG_S_ID);
    DEBUG_ASSERT(NULL != pstCurN);

    pstCurN->bShow = true;
    vVst.push_back(DAG_S_ID);

    lsQ.push_back(pstCurN->iPid);
    while (!lsQ.empty())
    {
        iPid = lsQ.front();
        lsQ.pop_front();

        pstCurN = findNode(iPid);
        DEBUG_ASSERT(NULL != pstCurN);
        //DEBUG_PRINTF("%d (L: %d score: %d): ", pstCurN->iName, pstCurN->iLayer, pstCurN->iWeight);
        DEBUG_PRINTF("SHOW_DAG,n,%d,%d,%d\n", pstCurN->iName, pstCurN->iLayer, pstCurN->iWeight);
        for (auto atChild : pstCurN->vChildren)
        {
            TPST_DAG_EDGE *pstE = findE(atChild);
            pstTpN = findNode(pstE->iCPid);
            DEBUG_ASSERT(NULL != pstTpN);
            //DEBUG_PRINTF("%d(%d) ", pstTpN->iName, pstE->iWeight);
            //DEBUG_PRINTF("SHOW_DAG,e,%d,%d,%d\n", pstCurN->iName, pstTpN->iName, pstE->iWeight);
            DEBUG_PRINTF("SHOW_DAG,e,%d,%d,%d,%f,%d\n",
                         pstCurN->iName, pstTpN->iName, pstE->iWeight, pstE->iCap, pstTpN->bShow);
            if (!pstTpN->bShow)
            {
                pstTpN->bShow = true;
                vVst.push_back(pstTpN->iPid);
                lsQ.push_back(pstTpN->iPid);
            }
        }
        //DEBUG_PRINTF("\n");
    }
    DEBUG_PRINTF("SHOW_DAG done\n");

    /* restore */
    for (auto atI : vVst)
    {
        pstCurN = findNode(atI);
        DEBUG_ASSERT(NULL != pstCurN);
        pstCurN->bShow = false;
    }

    return 0;
}
/*****************
input:
        none
description:
        init object
        init stage
******************/
DAG::DAG()
{
    TPST_DAG_NODE stTp = {0};
    TPST_DAG_EDGE stTpE = {0};

    m_umpName.clear();

    m_pvDAG = new vector<TPST_DAG_NODE>();
    m_pvE = new vector<TPST_DAG_EDGE>();

    m_iTotalScore = 0;
    m_iMaxPid = DAG_T_ID;

    m_pvDAG->push_back(stTp);
    m_pvDAG->push_back(stTp);
    m_pvDAG->push_back(stTp);
    (*m_pvDAG)[DAG_S_ID].iPid = DAG_S_ID;
    (*m_pvDAG)[DAG_S_ID].iName = 0;
    (*m_pvDAG)[DAG_T_ID].iPid = DAG_T_ID;
    (*m_pvDAG)[DAG_T_ID].iName = -1;

    m_iMaxEid = 0;
    m_pvE->push_back(stTpE);
}
/*****************
input:
        myG &oInitG
description:
        copy map
        calculate stage
******************/
DAG::~DAG()
{
    m_umpName.clear();

    m_iMaxPid = 0;
    m_iMaxEid = 0;

    m_pvDAG->clear();
    m_pvE->clear();

    delete m_pvDAG;
}

/*****************
input:
        int eid
description:
        find node
******************/
TPST_DAG_NODE * DAG::findNode(int iPid)
{
    if (!((0 < iPid) && (iPid <= m_iMaxPid)))
    {
        printf("DAG ERROR pid: %d maxPid: %d\n",
               iPid, m_iMaxPid);
        DEBUG_ASSERT(0);
    }
    return &((*m_pvDAG)[iPid]);
}
/*****************
input:
        int eid
description:
        find node
******************/
TPST_DAG_EDGE * DAG::findE(int iEid)
{
    if (!((0 < iEid) && (iEid <= m_iMaxEid)))
    {
        printf("DAG ERROR eid: %d maxEid: %d\n",
               iEid, m_iMaxEid);
        DEBUG_ASSERT(0);
    }
    return &((*m_pvE)[iEid]);
}













