/***************
insertNode function
****************/

#include <unordered_map>

#include "common.h"
#include "myG.h"
#include "insert.h"

/*extern long g_lInitTime;
extern long g_lInitTimeA;
extern long g_lInitTimeB;
extern long g_lInitTimeC;
extern long g_lInitTimeCB;
extern long g_lInitTimeD;
extern long g_lInitTimeE;
extern long g_lBackInitTime;
extern long g_upgradeTime;
extern long g_findTrussTime;
extern long g_lUpgradeInitTime;
extern long g_lUpgradeDelTime;
extern long g_lUpgradeSaveTime;
extern long g_lRestoreTime;*/

//long g_lBFSCnt;
//long g_lUgCnt;

/*****************
input:
        myG &mpG
        int iSelfId
        int iLtId
        int iRtId
description:
        check whether increase seSup
******************/
bool insertPart::checkInsSeSup(myG &mpG, int iSelfId, int iLtId, int iRtId)
{
    TPST_MAP_BY_EID *pstNode = NULL;
    TPST_MAP_BY_EID *pstLfNode = NULL;
    TPST_MAP_BY_EID *pstRtNode = NULL;

    pstNode = mpG.findNode(iSelfId);
    //DEBUG_ASSERT(NULL != pstNode);
    pstLfNode = mpG.findNode(iLtId);
    //DEBUG_ASSERT(NULL != pstLfNode);
    pstRtNode = mpG.findNode(iRtId);
    //DEBUG_ASSERT(NULL != pstRtNode);

    if ((pstLfNode->iTrussness != pstNode->iTrussness) ||
        (pstLfNode->iLayer <= pstNode->iLayer))
    {
        return false;
    }
    if (pstLfNode->iLayer <= pstRtNode->iLayer)
    {
        if (pstRtNode->iTrussness > pstNode->iTrussness)
        {
            return true;
        }
        else if (pstRtNode->iTrussness == pstNode->iTrussness)
        {
            if (pstRtNode->iLayer > pstNode->iLayer)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        if (pstRtNode->iTrussness > pstNode->iTrussness)
        {
            return true;
        }
        else if (pstRtNode->iTrussness < pstNode->iTrussness)
        {
            return false;
        }
        else
        {
            if (pstRtNode->iLayer > pstNode->iLayer)
            {
                return false;
            }
            else if (pstRtNode->iLayer < pstNode->iLayer)
            {
                if (pstRtNode->bDoneQFlag)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                if (pstRtNode->eid < pstNode->eid)
                {
                    if ((pstRtNode->bVisQFlag) &&
                        (pstRtNode->bUsedDoneFlag) &&
                        (pstRtNode->bInsLFlag))
                    {
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
                    if ((pstRtNode->bCanQFlag) ||
                        (pstRtNode->bDoneQFlag) ||
                        ((pstRtNode->bVisQFlag) &&
                         (pstRtNode->bUsedDoneFlag)))
                    {
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
            }
        }
    }
    return false;
}
/*****************
input:
        myG &mpG
        int iSelfId
        int iLtId
        int iRtId
description:
        check whether decrease seSup
******************/
bool insertPart::checkDesSeSup(myG &mpG, int iSelfId, int iLtId, int iRtId)
{
    TPST_MAP_BY_EID *pstNode = NULL;
    TPST_MAP_BY_EID *pstLfNode = NULL;
    TPST_MAP_BY_EID *pstRtNode = NULL;

    pstNode = mpG.findNode(iSelfId);
    DEBUG_ASSERT(NULL != pstNode);
    pstLfNode = mpG.findNode(iLtId);
    DEBUG_ASSERT(NULL != pstLfNode);
    pstRtNode = mpG.findNode(iRtId);
    DEBUG_ASSERT(NULL != pstRtNode);

    if (pstNode->bUsedDoneFlag)
    {
        if (pstLfNode->iTrussness != pstNode->iTrussness)
        {
            return false;
        }
        if (pstLfNode->iLayer <= pstNode->iLayer)
        {
            if (pstLfNode->bCanQFlag || pstLfNode->bDoneQFlag)
            {
                if (pstRtNode->iTrussness < pstNode->iTrussness)
                {
                    return false;
                }
                else if (pstRtNode->iTrussness > pstNode->iTrussness)
                {
                    return true;
                }
                else
                {
                    if (pstRtNode->iLayer < pstNode->iLayer)
                    {
                        if (pstRtNode->bDoneQFlag)
                        {
                            return true;
                        }
                        else
                        {
                            return false;
                        }
                    }
                    else if (pstRtNode->iLayer > pstNode->iLayer)
                    {
                        return true;
                    }
                    else
                    {
                        if (pstRtNode->bCanQFlag ||
                            pstRtNode->bDoneQFlag ||
                            (pstRtNode->bVisQFlag &&
                             pstRtNode->bUsedDoneFlag))
                        {
                            return true;
                        }
                        else
                        {
                            return false;
                        }
                    }
                }
            }
            else
            {
                return false;
            }
        }
        else if (pstLfNode->iLayer - 1 == pstNode->iLayer)
        {
            if (pstRtNode->iTrussness > pstNode->iTrussness)
            {
                return true;
            }
            else if (pstRtNode->iTrussness == pstNode->iTrussness)
            {
                if (pstRtNode->iLayer > pstNode->iLayer)
                {
                    return true;
                }
                else if (pstRtNode->iLayer < pstNode->iLayer)
                {
                    if (pstRtNode->bDoneQFlag)
                    {
                        return true;
                    }
                    return false;
                }
                else
                {
                    if (pstRtNode->bCanQFlag ||
                        pstRtNode->bDoneQFlag ||
                        (pstRtNode->bVisQFlag &&
                         pstRtNode->bUsedDoneFlag))
                    {
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
            }
            else
            {
                return false;
            }
        }
        else
        {
            /** pstLfNode->iLayer - 1 > pstNode->iLayer */
            if (pstRtNode->iTrussness > pstNode->iTrussness)
            {
                return true;
            }
            else if (pstRtNode->iTrussness == pstNode->iTrussness)
            {
                if (pstRtNode->iLayer > pstNode->iLayer)
                {
                    if (pstLfNode->bCanQFlag)
                    {
                        if (pstRtNode->bCanQFlag)
                        {
                            if (pstLfNode->iLayer > pstRtNode->iLayer)
                            {
                                return false;
                            }
                            else
                            {
                                return true;
                            }
                        }
                        else if (pstRtNode->bVisQFlag && pstRtNode->bUsedDoneFlag)
                        {
                            return true;
                        }
                        else
                        {
                            if (pstLfNode->iLayer > pstRtNode->iLayer)
                            {
                                return false;
                            }
                            else
                            {
                                return true;
                            }
                        }
                    }
                    else if (pstLfNode->bVisQFlag && pstLfNode->bUsedDoneFlag)
                    {
                        if (pstRtNode->bCanQFlag ||
                            (pstRtNode->bVisQFlag && pstRtNode->bUsedDoneFlag))
                        {
                            return true;
                        }
                        else
                        {
                            if (pstLfNode->bInsLFlag)
                            {
                                if (pstLfNode->iLayer - 1 == pstRtNode->iLayer)
                                {
                                    return true;
                                }
                                else
                                {
                                    return false;
                                }
                            }
                            else
                            {
                                if (pstLfNode->iLayer > pstRtNode->iLayer)
                                {
                                    return false;
                                }
                                else
                                {
                                    return true;
                                }
                            }
                        }
                    }
                    else
                    {
                        /** none or visited queue not from done queue */
                        if (pstLfNode->iLayer > pstRtNode->iLayer)
                        {
                            if (pstRtNode->bVisQFlag && pstRtNode->bUsedDoneFlag)
                            {
                                return true;
                            }
                            else
                            {
                                return false;
                            }
                        }
                        else
                        {
                            return true;
                        }
                    }
                }
                else if (pstRtNode->iLayer < pstNode->iLayer)
                {
                    if (pstRtNode->bDoneQFlag)
                    {
                        return true;
                    }
                    return false;
                }
                else
                {
                    if (pstRtNode->bCanQFlag ||
                        pstRtNode->bDoneQFlag ||
                        (pstRtNode->bVisQFlag &&
                         pstRtNode->bUsedDoneFlag))
                    {
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
            }
            else
            {
                return false;
            }
        }
    }
    else
    {
        if (pstLfNode->iTrussness != pstNode->iTrussness)
        {
            return false;
        }
        if (pstLfNode->iLayer <= pstNode->iLayer)
        {
            if (pstLfNode->bCanQFlag || pstLfNode->bDoneQFlag)
            {
                if (pstRtNode->iTrussness < pstNode->iTrussness)
                {
                    return false;
                }
                else if (pstRtNode->iTrussness > pstNode->iTrussness)
                {
                    return true;
                }
                else
                {
                    if (pstRtNode->iLayer < pstNode->iLayer)
                    {
                        if (pstRtNode->bDoneQFlag)
                        {
                            return true;
                        }
                        else
                        {
                            return false;
                        }
                    }
                    else if (pstRtNode->iLayer > pstNode->iLayer)
                    {
                        return true;
                    }
                    else
                    {
                        if (pstRtNode->bCanQFlag ||
                            pstRtNode->bDoneQFlag ||
                            (pstRtNode->bVisQFlag &&
                             pstRtNode->bUsedDoneFlag))
                        {
                            return true;
                        }
                        else
                        {
                            if (pstRtNode->eid > pstNode->eid)
                            {
                                return true;
                            }
                            else
                            {
                                return false;
                            }
                        }
                    }
                }
            }
            else
            {
                return false;
            }
        }
        else if (pstLfNode->iLayer - 1 == pstNode->iLayer)
        {
            if (pstLfNode->bVisQFlag &&
                pstLfNode->bUsedDoneFlag &&
                pstLfNode->bInsLFlag)
            {
                if (pstRtNode->iTrussness < pstNode->iTrussness)
                {
                    return false;
                }
                else if (pstRtNode->iTrussness > pstNode->iTrussness)
                {
                    return true;
                }
                else
                {
                    if (pstRtNode->iLayer < pstNode->iLayer)
                    {
                        if (pstRtNode->bDoneQFlag)
                        {
                            return true;
                        }
                        else
                        {
                            return false;
                        }
                    }
                    else if (pstRtNode->iLayer > pstNode->iLayer)
                    {
                        return true;
                    }
                    else
                    {
                        if (pstRtNode->bCanQFlag ||
                            pstRtNode->bDoneQFlag ||
                            (pstRtNode->bVisQFlag &&
                             pstRtNode->bUsedDoneFlag))
                        {
                            return true;
                        }
                        else
                        {
                            if (pstRtNode->eid > pstNode->eid)
                            {
                                return true;
                            }
                            else
                            {
                                return false;
                            }
                        }
                    }
                }

            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    return false;
}

/*****************
input:
        myG &mpG
        int iCurK
        int iCurL
        myPriQueue &myVstQ
description:
        rm edge in visited Queue
******************/
int insertPart::rmEInVisQEasy(myG &mpG, int iCurK, int iCurL, myPriQueueLess<int, int> &myVstQ)
{
    int iCurEid = 0;
    int iMinT = 0;
    bool bLeftFlag = false;
    bool bRightFlag = false;
    int iState = 0;
    int iMinLayer = 0;
    int iCnt = 0;
    TPST_MAP_BY_EID *pstNode = NULL;
    TPST_MAP_BY_EID *pstLfNode = NULL;
    TPST_MAP_BY_EID *pstRtNode = NULL;
    vector <int>::iterator itLfE;
    vector <int>::iterator itRtE;

    /*struct timeval tv;
	long lStartTime = 0;
	long lCurTime = 0;*/
	bool res = false;

    /*gettimeofday(&tv, NULL);
    lStartTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;*/
    while (!myVstQ.empty())
    {
        iCurEid = myVstQ.getTop();
        pstNode = mpG.findNode(iCurEid);
        DEBUG_ASSERT(NULL != pstNode);

        if (!pstNode->bVisQFlag)
        {
            /* should be removed */
            myVstQ.pop();
            pstNode->bVisQFlag = false;
            pstNode->bUsedVisFlag = false;
            continue;
        }

        if (mergeKey(pstNode->iLayer, pstNode->bUsedDoneFlag) != myVstQ.getKey())
        {
            /* appear later */
            DEBUG_ASSERT(mergeKey(pstNode->iLayer, pstNode->bUsedDoneFlag) > myVstQ.getKey());
            myVstQ.pop();
            myVstQ.insertByOrder(mergeKey(pstNode->iLayer, pstNode->bUsedDoneFlag),
                                   pstNode->eid);
            continue;
        }

        if (pstNode->iLayer > iCurL)
        {
            break;
        }

        /*DEBUG_PRINTF("DEBUG -----visited queue: (%d, %d) k: %d layer: %d seSup: %d done: %d key: %d\n",
                     pstNode->paXY.first, pstNode->paXY.second,
                     pstNode->iTrussness, pstNode->iLayer,
                     pstNode->iSeSup, pstNode->bUsedDoneFlag,
                     myVstQ.getDy());*/
        ++iCnt;

        itLfE = pstNode->vLfE.begin();
        itRtE = pstNode->vRtE.begin();
        for (; itLfE != pstNode->vLfE.end(); ++itLfE, ++itRtE)
        {
            pstLfNode = mpG.findNode(*itLfE);
            //DEBUG_ASSERT(NULL != pstLfNode);
            pstRtNode = mpG.findNode(*itRtE);
            //DEBUG_ASSERT(NULL != pstRtNode);

            /*DEBUG_PRINTF("DEBUG pop: (%d, %d) (%d, %d) k: %d %d layer: %d %d seSup: %d %d self: (%d, %d) layer: %d\n",
                         pstLfNode->paXY.first, pstLfNode->paXY.second,
                         pstRtNode->paXY.first, pstRtNode->paXY.second,
                         pstLfNode->iTrussness, pstRtNode->iTrussness,
                         pstLfNode->iLayer, pstRtNode->iLayer,
                         pstLfNode->iSeSup, pstRtNode->iSeSup,
                         pstNode->paXY.first, pstNode->paXY.second,
                         pstNode->iLayer);*/

            bLeftFlag = checkDesSeSup(mpG, pstNode->eid, pstLfNode->eid, pstRtNode->eid);
            if (bLeftFlag)
            {
                pstLfNode->iSeSup--;
                /*DEBUG_PRINTF("DEBUG decrease : (%d, %d) k: %d layer: %d seSup: %d\n",
                             pstLfNode->paXY.first, pstLfNode->paXY.second,
                             pstLfNode->iTrussness, pstLfNode->iLayer,
                             pstLfNode->iSeSup);*/
                if (pstLfNode->iSeSup + 2 == iCurK)
                {
                    if (pstLfNode->bDoneQFlag)
                    {
                        //DEBUG_ASSERT(!pstLfNode->bInsLFlag);
                        if (pstLfNode->iLayer < pstNode->iLayer + 1)
                        {
                            pstLfNode->bInsLFlag = true;
                            pstLfNode->iLayer = pstNode->iLayer + 1;
                        }
                        pstLfNode->bDoneQFlag = false;
                    }
                    else
                    {
                        //DEBUG_ASSERT(pstLfNode->bCanQFlag);
                        //DEBUG_ASSERT(!pstLfNode->bUsedDoneFlag);
                        pstLfNode->bCanQFlag = false;
                    }
                    //DEBUG_ASSERT(!pstLfNode->bVisQFlag);
                    pstLfNode->bVisQFlag = true;
                    if (!pstLfNode->bUsedVisFlag)
                    {
                        pstLfNode->bUsedVisFlag = true;
                        myVstQ.insertByOrder(mergeKey(pstLfNode->iLayer, pstLfNode->bUsedDoneFlag),
                                               pstLfNode->eid);
                    }
                    /*DEBUG_PRINTF("VISIT push back (%d, %d) layer: %d\n",
                                 pstLfNode->paXY.first, pstLfNode->paXY.second,
                                 pstLfNode->iLayer);*/
                }
            }

            bRightFlag = checkDesSeSup(mpG, pstNode->eid, pstRtNode->eid, pstLfNode->eid);
            if (bRightFlag)
            {
                pstRtNode->iSeSup--;
                /*DEBUG_PRINTF("DEBUG decrease : (%d, %d) k: %d layer: %d seSup: %d\n",
                             pstRtNode->paXY.first, pstRtNode->paXY.second,
                             pstRtNode->iTrussness, pstRtNode->iLayer,
                             pstRtNode->iSeSup);*/
                if (pstRtNode->iSeSup + 2 == iCurK)
                {
                    if (pstRtNode->bDoneQFlag)
                    {
                        //DEBUG_ASSERT(!pstRtNode->bInsLFlag);
                        if (pstRtNode->iLayer < pstNode->iLayer + 1)
                        {
                            pstRtNode->bInsLFlag = true;
                            pstRtNode->iLayer = pstNode->iLayer + 1;
                        }
                        pstRtNode->bDoneQFlag = false;
                    }
                    else
                    {
                        //DEBUG_ASSERT(pstRtNode->bCanQFlag);
                        //DEBUG_ASSERT(!pstRtNode->bUsedDoneFlag);
                        pstRtNode->bCanQFlag = false;
                    }
                    //DEBUG_ASSERT(!pstRtNode->bVisQFlag);
                    pstRtNode->bVisQFlag = true;
                    if (!pstRtNode->bUsedVisFlag)
                    {
                        pstRtNode->bUsedVisFlag = true;
                        myVstQ.insertByOrder(mergeKey(pstRtNode->iLayer, pstRtNode->bUsedDoneFlag),
                                               pstRtNode->eid);
                    }
                    /*DEBUG_PRINTF("VISIT push back (%d, %d) layer: %d\n",
                                 pstRtNode->paXY.first, pstRtNode->paXY.second,
                                 pstRtNode->iLayer);*/
                }
            }
        }
        DEBUG_ASSERT(itRtE == pstNode->vRtE.end());
        pstNode->bUsedDoneFlag = false;
        pstNode->bInsLFlag = false;
        myVstQ.pop();
        pstNode->bVisQFlag = false;
        pstNode->bUsedVisFlag = false;
    }

    /*gettimeofday(&tv, NULL);
    lCurTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    g_lUpgradeDelTime += lCurTime - lStartTime;*/
    return iCnt;
}

/*****************
input:
        myG &mpG
        int iCurT
        vector <int> &vSeed
        vector <int> &vUpGE
        vector <int> &vUpdSeed
description:
        increase k directly
******************/
int insertPart::insKByBFSEasy(myG &mpG, int iCurT, vector <int> &vSeed, vector <int> &vUpGE, vector <int> &vUpdSeed, map<int, int> &mpOLChgE)
{
    TPST_MAP_BY_EID *pstNode = NULL;
    TPST_MAP_BY_EID *pstLfNode = NULL;
    TPST_MAP_BY_EID *pstRtNode = NULL;
    list<int>::iterator itE;
    vector <int> vVisit;
    vector<int>::iterator itvE;
    vector <int>::iterator itLfE;
    vector <int>::iterator itRtE;

    int iCurEid = 0;
    int iCurLayer = 0;
    int iMinT = 0;
    int iCurK = iCurT;
    int iMinLayer = 0;
    int iOldLayer = 0;
    int iNewLayer = 0;
    int iState = 0;

    bool bLeftFlag = false;
    bool bRightFlag = false;
    int iCnt = 0;

    struct timeval tv;
	long lStartTime = 0;
	long lLcStartTime = 0;
	long lCurTime = 0;

	bool res = false;

    myPriQueueLess<int, int> myCanQ;
    vector <int> vDoneQ;
    myPriQueueLess<int, int> myVstQ;

    DEBUG_ASSERT(vUpdSeed.empty());
    /*DEBUG_PRINTF("DEBUG start BFS k: %d\n",
                 iCurT);*/
    for (itvE = vSeed.begin(); itvE != vSeed.end(); ++itvE)
    {
        pstNode = mpG.findNode(*itvE);
        DEBUG_ASSERT(NULL != pstNode);
        mpOLChgE[pstNode->eid] = 1;

        if (!pstNode->bVstFlag)
        {
            pstNode->bVstFlag = true;
            //++g_lBFSCnt;
            vVisit.push_back(pstNode->eid);
            myCanQ.insertByOrder(pstNode->iLayer, pstNode->eid);
            pstNode->bCanQFlag = true;
            pstNode->bUsedCanFlag = true;

            /*DEBUG_PRINTF("DEBUG seed: (%d, %d) k: %d layer: %d seSup: %d\n",
                         pstNode->paXY.first, pstNode->paXY.second,
                         pstNode->iTrussness, pstNode->iLayer,
                         pstNode->iSeSup);*/
        }
    }

    //DEBUG_PRINTF("DEBUG start queue\n");
    while (!myCanQ.empty())
    {
        iCurEid = myCanQ.getTop();

        pstNode = mpG.findNode(iCurEid);
        DEBUG_ASSERT(NULL != pstNode);
        if (!pstNode->bCanQFlag)
        {
            /* has been removed, ignore */
            myCanQ.pop();
            pstNode->bUsedCanFlag = false;
            continue;
        }
        DEBUG_ASSERT(pstNode->iLayer == myCanQ.getKey());

        if (pstNode->iLayer > iCurLayer)
        {
            iCnt = 0;
            iCurLayer = pstNode->iLayer;
            //DEBUG_PRINTF("DEBUG =========layer: %d\n", iCurLayer);
            iCnt = rmEInVisQEasy(mpG, iCurK, iCurLayer - 1, myVstQ);
            if (0 < iCnt)
            {
                continue;
            }
        }

        /*DEBUG_PRINTF("DEBUG ------queue: (%d, %d) k: %d layer: %d seSup: %d\n",
                     pstNode->paXY.first, pstNode->paXY.second,
                     pstNode->iTrussness, pstNode->iLayer,
                     pstNode->iSeSup);*/

        DEBUG_ASSERT(iCurEid == myCanQ.getTop());
        myCanQ.pop();
        pstNode->bCanQFlag = false;
        pstNode->bUsedCanFlag = false;
        itLfE = pstNode->vLfE.begin();
        itRtE = pstNode->vRtE.begin();
        for (; itLfE != pstNode->vLfE.end(); ++itLfE, ++itRtE)
        {
            /*DEBUG_PRINTF("DEBUG neighbor %d, %d\n",
                         *itLfE, *itRtE);*/
            pstLfNode = mpG.findNode(*itLfE);
            //DEBUG_ASSERT(NULL != pstLfNode);
            pstRtNode = mpG.findNode(*itRtE);
            //DEBUG_ASSERT(NULL != pstRtNode);

            mpOLChgE[pstLfNode->eid] = 1;
            mpOLChgE[pstRtNode->eid] = 1;

            iMinT = COMMON_MIN(pstLfNode->iTrussness, pstRtNode->iTrussness);
            if (iMinT != iCurK)
            {
                /* not meet, ignore */
                continue;
            }

            /* visited Queue check */
            iNewLayer = COMMON_TRG_MIN(pstNode->iLayer,
                            pstLfNode->iLayer,
                            (pstLfNode->bCanQFlag ||
                             pstLfNode->bUsedDoneFlag ||
                             pstLfNode->iTrussness > iCurK),
                            pstRtNode->iLayer,
                            (pstRtNode->bCanQFlag ||
                             pstRtNode->bUsedDoneFlag ||
                             pstRtNode->iTrussness > iCurK));
            if (iNewLayer == pstNode->iLayer)
            {
                /* not meet, ignore */
                /* add to  visit queue */
                if ((pstLfNode->iTrussness == iCurK) &&
                    (iNewLayer == pstLfNode->iLayer) &&
                    (!pstLfNode->bCanQFlag) &&
                    (!pstLfNode->bUsedDoneFlag))
                {
                    if (!pstLfNode->bVisQFlag)
                    {
                        pstLfNode->bVisQFlag = true;
                        if (!pstLfNode->bUsedVisFlag)
                        {
                            pstLfNode->bUsedVisFlag = true;
                            myVstQ.insertByOrder(mergeKey(pstLfNode->iLayer, 0),
                                                   pstLfNode->eid);

                            if (!pstLfNode->bVstFlag)
                            {
                                pstLfNode->bVstFlag = true;
                                //++g_lBFSCnt;
                                vVisit.push_back(pstLfNode->eid);
                            }
                        }

                    }
                }
                if ((pstRtNode->iTrussness == iCurK) &&
                    (iNewLayer == pstRtNode->iLayer) &&
                    (!pstRtNode->bCanQFlag) &&
                    (!pstRtNode->bUsedDoneFlag))
                {
                    if (!pstRtNode->bVisQFlag)
                    {
                        pstRtNode->bVisQFlag = true;
                        if (!pstRtNode->bUsedVisFlag)
                        {
                            pstRtNode->bUsedVisFlag = true;
                            myVstQ.insertByOrder(mergeKey(pstRtNode->iLayer, 0),
                                                   pstRtNode->eid);
                            if (!pstRtNode->bVstFlag)
                            {
                                pstRtNode->bVstFlag = true;
                                //++g_lBFSCnt;
                                vVisit.push_back(pstRtNode->eid);
                            }
                        }
                    }
                }

                continue;
            }

            bLeftFlag = checkInsSeSup(mpG, pstNode->eid, pstLfNode->eid, pstRtNode->eid);
            if (bLeftFlag)
            {
                pstLfNode->iSeSup++;

                if (pstLfNode->iSeSup + 1 == iCurK)
                {
                    if (pstLfNode->bVisQFlag)
                    {
                        /* in visited queue */
                        pstLfNode->bVisQFlag = false;
                        pstLfNode->bInsLFlag = false;
                    }
                    pstLfNode->bCanQFlag = true;
                    if (!pstLfNode->bUsedCanFlag)
                    {
                        pstLfNode->bUsedCanFlag = true;
                        myCanQ.insertByOrder(pstLfNode->iLayer, pstLfNode->eid);

                        if (!pstLfNode->bVstFlag)
                        {
                            pstLfNode->bVstFlag = true;
                            //++g_lBFSCnt;
                            vVisit.push_back(pstLfNode->eid);
                        }
                    }
                }
                else if (pstLfNode->iSeSup + 1 < iCurK)
                {
                    if (!pstLfNode->bVisQFlag)
                    {
                        pstLfNode->bVisQFlag = true;
                        if (!pstLfNode->bUsedVisFlag)
                        {
                            pstLfNode->bUsedVisFlag = true;
                            myVstQ.insertByOrder(mergeKey(pstLfNode->iLayer, 0),
                                                   pstLfNode->eid);
                        }
                    }
                }
            }

            bRightFlag = checkInsSeSup(mpG, pstNode->eid, pstRtNode->eid, pstLfNode->eid);
            if (bRightFlag)
            {
                pstRtNode->iSeSup++;

                if (pstRtNode->iSeSup + 1 == iCurK)
                {
                    if (pstRtNode->bVisQFlag)
                    {
                        /* in visited queue */
                        pstRtNode->bVisQFlag = false;
                        pstRtNode->bInsLFlag = false;
                    }
                    pstRtNode->bCanQFlag = true;
                    if (!pstRtNode->bUsedCanFlag)
                    {
                        pstRtNode->bUsedCanFlag = true;
                        myCanQ.insertByOrder(pstRtNode->iLayer, pstRtNode->eid);

                        if (!pstRtNode->bVstFlag)
                        {
                            pstRtNode->bVstFlag = true;
                            //++g_lBFSCnt;
                            vVisit.push_back(pstRtNode->eid);
                        }

                    }
                }
                else if (pstRtNode->iSeSup + 1 < iCurK)
                {
                    if (!pstRtNode->bVisQFlag)
                    {
                        pstRtNode->bVisQFlag = true;
                        if (!pstRtNode->bUsedVisFlag)
                        {
                            pstRtNode->bUsedVisFlag = true;
                            myVstQ.insertByOrder(mergeKey(pstRtNode->iLayer, 0),
                                                   pstRtNode->eid);
                        }
                    }
                }
            }
        }
        DEBUG_ASSERT(!pstNode->bDoneQFlag);
        pstNode->bDoneQFlag = true;
        if (!pstNode->bUsedDoneFlag)
        {
            pstNode->bUsedDoneFlag = true;
            vDoneQ.push_back(pstNode->eid);
        }
    }

    //DEBUG_PRINTF("DEBUG =========rm all visited queue\n");
    while (!myVstQ.empty())
    {
        iCurEid = myVstQ.getTop();
        pstNode = mpG.findNode(iCurEid);
        DEBUG_ASSERT(NULL != pstNode);

        rmEInVisQEasy(mpG, iCurK, pstNode->iLayer, myVstQ);
    }

    if (!vDoneQ.empty())
    {
        /* upgrade */
        DEBUG_ASSERT(vUpGE.empty());
        DEBUG_ASSERT(vUpdSeed.empty());
        /* increase k */
        ++iCurK;
        //DEBUG_PRINTF("DEBUG ins k: %d\n", iCurK);
        /* upgrade trussness */
        for (auto atIt : vDoneQ)
        {
            pstNode = mpG.findNode(atIt);
            DEBUG_ASSERT(NULL != pstNode);

            if (!pstNode->bDoneQFlag)
            {
                /* has been removed, ignore */
                continue;
            }

            //++g_lUgCnt;
            pstNode->iTrussness = iCurK;
            pstNode->bUgdFlag = true;
            pstNode->iLayer = 1;
            pstNode->bDoneQFlag = false;
            pstNode->bUsedDoneFlag = false;
            /*DEBUG_PRINTF("DEBUG increase k: (%d, %d) k: %d layer: %d seSup: %d\n",
                         pstNode->paXY.first, pstNode->paXY.second,
                         pstNode->iTrussness, pstNode->iLayer,
                         pstNode->iSeSup);*/

            itLfE = pstNode->vLfE.begin();
            itRtE = pstNode->vRtE.begin();
            for (; itLfE != pstNode->vLfE.end(); ++itLfE, ++itRtE)
            {
                pstLfNode = mpG.findNode(*itLfE);
                //DEBUG_ASSERT(NULL != pstLfNode);
                pstRtNode = mpG.findNode(*itRtE);
                //DEBUG_ASSERT(NULL != pstRtNode);

                iMinT = COMMON_MIN(pstLfNode->iTrussness, pstRtNode->iTrussness);
                if (iMinT != iCurK)
                {
                    /* not meet, ignore */
                    continue;
                }

                if ((pstLfNode->iTrussness == iCurK) &&
                    (pstLfNode->iLayer <= pstNode->iLayer) &&
                    (!pstLfNode->bUgdFlag))
                {
                    pstLfNode->iSeSup++;
                    if (pstLfNode->iSeSup + 1 == iCurK)
                    {
                        vUpdSeed.push_back(pstLfNode->eid);
                    }
                }
                if ((pstRtNode->iTrussness == iCurK) &&
                    (pstRtNode->iLayer <= pstNode->iLayer) &&
                    (!pstRtNode->bUgdFlag))
                {
                    pstRtNode->iSeSup++;
                    if (pstRtNode->iSeSup + 1 == iCurK)
                    {
                        vUpdSeed.push_back(pstRtNode->eid);
                    }
                }

            }
            DEBUG_ASSERT(itRtE == pstNode->vRtE.end());

            vUpGE.push_back(pstNode->eid);
            if (pstNode->iSeSup + 2 > iCurK)
            {
                vUpdSeed.push_back(pstNode->eid);
            }
        }
    }

    /* restore */
    for (itvE = vVisit.begin(); itvE != vVisit.end(); ++itvE)
    {
        pstNode = mpG.findNode(*itvE);
        DEBUG_ASSERT(NULL != pstNode);

        /*DEBUG_PRINTF("RESTORE (%d, %d) layer: %d\n",
                     pstNode->paXY.first, pstNode->paXY.second,
                     pstNode->iLayer);*/
        pstNode->bVstFlag = false;
        DEBUG_ASSERT(!pstNode->bInsLFlag);
        DEBUG_ASSERT(!pstNode->bVisQFlag);
        DEBUG_ASSERT(!pstNode->bCanQFlag);
        DEBUG_ASSERT(!pstNode->bDoneQFlag);
        DEBUG_ASSERT(!pstNode->bUsedCanFlag);
        DEBUG_ASSERT(!pstNode->bUsedDoneFlag);
        DEBUG_ASSERT(!pstNode->bUsedVisFlag);
    }

    //DEBUG_PRINTF("DEBUG end BFS\n");
    return 0;
}

/*****************
input:
        int iType
description:
        upgrade and save trussness
******************/
int insertPart::upgradeForAllK(myG &mpG, list<int> &lsCdtE, map<int, int> &mpOLChgE)
{
    list<int>::iterator itE;
    vector <int>::iterator itvE;
    /* k, eid */
    map<int, vector <int> > mpKLevel;
    map<int, vector <int> >::iterator itmpNode;
    map<int, vector <int> >::reverse_iterator ritmpNode;
    /* eid, none */
    TPST_MAP_BY_EID* pstNode = NULL;
	/*struct timeval tv;
	long lStartTime = 0;
	long lCurTime = 0;*/
    vector <int> vUpd;
    vector <int> vSeed;
    int iCurK = 0;
    vector <int> vCurUpd;
    vector <int> vUpdSeed;
    int iSeSup = 0;

    for (itE = lsCdtE.begin(); itE != lsCdtE.end(); ++itE)
    {
        pstNode = mpG.findNode(*itE);
        debug_assert(NULL != pstNode);
        mpKLevel[pstNode->iTrussness].push_back(*itE);
    }

    /* kmax -> 3 avoid repeat */
    for (ritmpNode = mpKLevel.rbegin(); ritmpNode != mpKLevel.rend(); ++ritmpNode)
    {
        iCurK = ritmpNode->first;

        //DEBUG_PRINTF("UPGRADE K: %d \n", iCurK);
        /*gettimeofday(&tv, NULL);
        lStartTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;*/
        DEBUG_ASSERT(vCurUpd.empty());
        DEBUG_ASSERT(vUpdSeed.empty());
        insKByBFSEasy(mpG, iCurK, ritmpNode->second, vCurUpd, vUpdSeed, mpOLChgE);
        vUpd.insert(vUpd.end(), vCurUpd.begin(), vCurUpd.end());
        vCurUpd.clear();
        if (!vUpdSeed.empty())
        {
            /* maintain layer */
            vSeed.swap(vUpdSeed);
            insKByBFSEasy(mpG, iCurK + 1, vSeed, vCurUpd, vUpdSeed, mpOLChgE);
            DEBUG_ASSERT(vCurUpd.empty());
        }
        vSeed.clear();
        /*gettimeofday(&tv, NULL);
        lCurTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        g_lInitTimeE += lCurTime - lStartTime;*/
    }

    /* restore */
    for (itvE = vUpd.begin(); itvE != vUpd.end(); ++itvE)
    {
        pstNode = mpG.findNode(*itvE);
        debug_assert(NULL != pstNode);
        pstNode->bUgdFlag = false;

        //iSeSup = pstNode->iSeSup;
        //mpG.updateSeSup(*itvE);
        //DEBUG_ASSERT(iSeSup == pstNode->iSeSup);
    }

    /*DEBUG_PRINTF("DEBUG BFS total: %ld upgraded: %ld\n",
           g_lBFSCnt, g_lUgCnt);*/
    return 0;
}

/*****************
input:
        myG &mpG
        int x
        int y
description:
        simply add
******************/
int insertPart::simpleAdd(myG &mpG, int x, int y)
{
    TPST_MAP_BY_EID* pstENode = NULL;
    int iEid = 0;
    vector<int>::iterator itLfE;
    vector<int>::iterator itRtE;

    /* add and init */
    DEBUG_ASSERT(x < y);
    iEid = mpG.add(x, y);
    DEBUG_ASSERT(0 != iEid);
    pstENode = mpG.findNode(iEid);
    DEBUG_ASSERT(NULL != pstENode);
    //pstENode->bNewFlag = true;

    pstENode->vLfE.clear();
    pstENode->vRtE.clear();
    //pstENode->iSup = mpG.findNeb(x, y, pstENode->vLfE, pstENode->vRtE);
    mpG.findNeb(x, y, pstENode->vLfE, pstENode->vRtE);

    itLfE = pstENode->vLfE.begin();
    itRtE = pstENode->vRtE.begin();
    for (; itLfE != pstENode->vLfE.end(); ++itLfE, ++itRtE)
    {
        TPST_MAP_BY_EID* pstLfNode = NULL;
        TPST_MAP_BY_EID* pstRtNode = NULL;

        pstLfNode = mpG.findNode(*itLfE);
        DEBUG_ASSERT(NULL != pstLfNode);
        pstRtNode = mpG.findNode(*itRtE);
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

        /*DEBUG_PRINTF("DEBUG add vector %d %d %d\n",
               *itRtE, *itLfE, iEid);
        DEBUG_PRINTF("DEBUG add vector size %d %d %d\n",
               pstRtNode->vLfE.size(), pstLfNode->vLfE.size(),
                     pstENode->vLfE.size());*/
    }
    DEBUG_ASSERT(itRtE == pstENode->vRtE.end());

    return iEid;
}
/*****************
input:
        myG &mpG
        int iEid
description:
        find low bound for edge
******************/
int insertPart::edgeDec(myG &mpG, int iEid, vector <int> &vOLChgE)
{
    list<int> lstCdtE;
    TPST_MAP_BY_EID* pstNode = NULL;
    TPST_MAP_BY_EID *pstLfNode = NULL;
    TPST_MAP_BY_EID *pstRtNode = NULL;
    vector <int>::iterator itLfE;
    vector <int>::iterator itRtE;

    vector <int> vNebK;
    int iNebKSize = 0;
    int iNebKCnt = 0;
    int iTriT = 0;

    vector <int> vNebL;
    int iNebLSize = 0;
    int iNebLCnt = 0;
    int iTriL = 0;
    int iHighL = 0;

	struct timeval tv;
	long lStartTime = 0;
	long lCurTime = 0;

	bool bLfFlag = false;
	bool bRtFlag = false;

	int i = 0;
	/* eid, none */
	map<int, int> mpOLChgE;
	map<int, int>::iterator itmpE;

	mpOLChgE[iEid] = 1;

    pstNode = mpG.findNode(iEid);
    DEBUG_ASSERT(NULL != pstNode);

    /* fill neighbor truss vector */
    itLfE = pstNode->vLfE.begin();
    itRtE = pstNode->vRtE.begin();
    for (; itLfE != pstNode->vLfE.end(); ++itLfE, ++itRtE)
    {
        pstLfNode = mpG.findNode(*itLfE);
        DEBUG_ASSERT(NULL != pstLfNode);
        pstRtNode = mpG.findNode(*itRtE);
        DEBUG_ASSERT(NULL != pstRtNode);

        mpOLChgE[pstLfNode->eid] = 1;
        mpOLChgE[pstRtNode->eid] = 1;

        if (pstLfNode->iTrussness < 3)
        {
            /* must be 3-truss */
            pstLfNode->iTrussness = 3;
        }
        if (pstRtNode->iTrussness < 3)
        {
            /* must be 3-truss */
            pstRtNode->iTrussness = 3;
        }

        iTriT = COMMON_MIN(pstLfNode->iTrussness, pstRtNode->iTrussness);
        while (iTriT >= iNebKSize)
        {
            vNebK.push_back(0);
            ++iNebKSize;
        }
        if (!(iTriT < vNebK.size()))
        {
            printf("ERROR %d %d %d %d %d\n", iTriT, iNebKSize, vNebK.size(),
                   pstLfNode->iTrussness, pstRtNode->iTrussness);
            DEBUG_ASSERT(0);
        }
        vNebK[iTriT]++;
    }
    DEBUG_ASSERT(itRtE == pstNode->vRtE.end());

    /* count neighbor truss vector, find trussness */
    for (i = iNebKSize - 1; i > 0; --i)
    {
        iNebKCnt += vNebK[i];
        /*DEBUG_PRINTF("DEBUG (%d, %d) k: %d number: %d\n", pstNode->paXY.first, pstNode->paXY.second,
                     i, vNebK[i]);*/
        if (iNebKCnt + 2 >= i)
        {
            /* meet truss requirement */
            break;
        }
    }
    pstNode->iTrussness = COMMON_MAX(i, ((iNebKSize > 0)?3:2));
    /*DEBUG_PRINTF("DEBUG set (%d, %d) k: %d\n", pstNode->paXY.first, pstNode->paXY.second,
                 pstNode->iTrussness);*/

    pstNode->iLayer = 1;
    /* fill neighbor layer vector */
    itLfE = pstNode->vLfE.begin();
    itRtE = pstNode->vRtE.begin();
    for (; itLfE != pstNode->vLfE.end(); ++itLfE, ++itRtE)
    {
        pstLfNode = mpG.findNode(*itLfE);
        DEBUG_ASSERT(NULL != pstLfNode);
        pstRtNode = mpG.findNode(*itRtE);
        DEBUG_ASSERT(NULL != pstRtNode);

        iTriT = COMMON_MIN(pstLfNode->iTrussness, pstRtNode->iTrussness);
        if (iTriT < pstNode->iTrussness)
        {
            continue;
        }
        else if (iTriT > pstNode->iTrussness)
        {
            ++iHighL;
            continue;
        }
        iTriL = COMMON_TRG_MIN(pstNode->iLayer,
                               pstLfNode->iLayer,
                               (pstLfNode->iTrussness > pstNode->iTrussness),
                               pstRtNode->iLayer,
                               (pstRtNode->iTrussness > pstNode->iTrussness));
        while (iTriL >= iNebLSize)
        {
            vNebL.push_back(0);
            ++iNebLSize;
        }
        DEBUG_ASSERT(iTriL < vNebL.size());
        vNebL[iTriL]++;
    }
    DEBUG_ASSERT(itRtE == pstNode->vRtE.end());

    /* count neighbor layer vector, find layer */
    iNebLCnt = iHighL;
    if (0 == iNebLSize)
    {
        pstNode->iLayer = 1;
        pstNode->iSeSup = iNebLCnt;
    }
    else
    {
        for (i = iNebLSize - 1; i > 0; --i)
        {
            /*DEBUG_PRINTF("DEBUG (%d, %d) l: %d number: %d\n", pstNode->paXY.first, pstNode->paXY.second,
                         i, vNebL[i]);*/
            if (iNebLCnt + vNebL[i] + 2 > pstNode->iTrussness)
            {
                /* meet truss requirement */
                break;
            }
            iNebLCnt += vNebL[i];
        }
        if (0 == iNebLCnt)
        {
            pstNode->iLayer = i;
            pstNode->iSeSup = iNebLCnt + vNebL[i];
            if (pstNode->iSeSup + 2 > pstNode->iTrussness)
            {
                lstCdtE.push_back(pstNode->eid);
            }
        }
        else
        {
            pstNode->iLayer = i + 1;
            pstNode->iSeSup = iNebLCnt;
        }
    }
    /*DEBUG_PRINTF("DEBUG set (%d, %d) l: %d\n", pstNode->paXY.first, pstNode->paXY.second,
                 pstNode->iLayer);
    DEBUG_PRINTF("DEBUG set (%d, %d) seSup: %d\n", pstNode->paXY.first, pstNode->paXY.second,
                 pstNode->iSeSup);*/

    /* find candidature */
    itLfE = pstNode->vLfE.begin();
    itRtE = pstNode->vRtE.begin();
    for (; itLfE != pstNode->vLfE.end(); ++itLfE, ++itRtE)
    {
        pstLfNode = mpG.findNode(*itLfE);
        DEBUG_ASSERT(NULL != pstLfNode);
        pstRtNode = mpG.findNode(*itRtE);
        DEBUG_ASSERT(NULL != pstRtNode);

        bLfFlag = false;
        bRtFlag = false;

        /*DEBUG_PRINTF("DEBUG visit: (%d, %d) (%d, %d) k: %d %d layer: %d %d seSup: %d %d eid: %d %d self: (%d, %d) layer: %d\n",
                     pstLfNode->paXY.first, pstLfNode->paXY.second,
                     pstRtNode->paXY.first, pstRtNode->paXY.second,
                     pstLfNode->iTrussness, pstRtNode->iTrussness,
                     pstLfNode->iLayer, pstRtNode->iLayer,
                     pstLfNode->iSeSup, pstRtNode->iSeSup,
                     pstLfNode->eid, pstRtNode->eid,
                     pstNode->paXY.first, pstNode->paXY.second,
                     pstNode->iLayer);*/

        iTriT = COMMON_MIN(pstLfNode->iTrussness, pstRtNode->iTrussness);
        if (iTriT > pstNode->iTrussness)
        {
            continue;
        }
        else if (iTriT < pstNode->iTrussness)
        {
            if (pstLfNode->iTrussness == pstRtNode->iTrussness)
            {
                iTriL = COMMON_MIN(pstLfNode->iLayer, pstRtNode->iLayer);
                if (pstLfNode->iLayer == iTriL)
                {
                    bLfFlag = true;
                }
                if (pstRtNode->iLayer == iTriL)
                {
                    bRtFlag = true;
                }
            }
            else if (iTriT == pstRtNode->iTrussness)
            {
                bRtFlag = true;
            }
            else if (iTriT == pstLfNode->iTrussness)
            {
                bLfFlag = true;
            }
            else
            {
                DEBUG_ASSERT(0);
            }
        }
        else
        {
            iTriL = COMMON_TRG_MIN(pstNode->iLayer,
                                   pstLfNode->iLayer,
                                   (pstLfNode->iTrussness > pstNode->iTrussness),
                                   pstRtNode->iLayer,
                                   (pstRtNode->iTrussness > pstNode->iTrussness));
            if (iTriL > pstNode->iLayer)
            {
                /* no change, ignore */
                continue;
            }

            if ((pstLfNode->iTrussness == pstNode->iTrussness) &&
                (pstLfNode->iLayer == iTriL))
            {
                bLfFlag = true;
            }
            if ((pstRtNode->iTrussness == pstNode->iTrussness) &&
                (pstRtNode->iLayer == iTriL))
            {
                bRtFlag = true;
            }
        }

        if (bLfFlag)
        {
            pstLfNode->iSeSup++;
            /*DEBUG_PRINTF("DEBUG increase left: (%d, %d) k: %d layer: %d seSup: %d\n",
                         pstLfNode->paXY.first, pstLfNode->paXY.second,
                         pstLfNode->iTrussness, pstLfNode->iLayer,
                         pstLfNode->iSeSup);*/
            if (pstLfNode->iSeSup + 2 > pstLfNode->iTrussness)
            {
                /* may increase truss or layer */
                lstCdtE.push_back(pstLfNode->eid);
            }
        }
        if (bRtFlag)
        {
            pstRtNode->iSeSup++;
            /*DEBUG_PRINTF("DEBUG increase right: (%d, %d) k: %d layer: %d seSup: %d\n",
                         pstRtNode->paXY.first, pstRtNode->paXY.second,
                         pstRtNode->iTrussness, pstRtNode->iLayer,
                         pstRtNode->iSeSup);*/
            if (pstRtNode->iSeSup + 2 > pstRtNode->iTrussness)
            {
                /* may increase truss or layer */
                lstCdtE.push_back(pstRtNode->eid);
            }
        }
    }
    DEBUG_ASSERT(itRtE == pstNode->vRtE.end());

    upgradeForAllK(mpG, lstCdtE, mpOLChgE);
    //DEBUG_PRINTF("DEBUG update done\n");
    DEBUG_ASSERT(vOLChgE.empty());
    for (itmpE = mpOLChgE.begin(); itmpE != mpOLChgE.end(); ++itmpE)
    {
        vOLChgE.push_back(itmpE->first);
    }

    return 0;
}

/*****************
input:
        myG &mpG
        int iNodeX
        int iNodeY
        vector <int> &vOLChgE
description:
        insert an edge
******************/
int insertPart::insertOne(myG &mpG, int iNodeX, int iNodeY, vector <int> &vOLChgE)
{
	/*struct timeval tv;
	long lStartTime = 0;
	long lCurTime = 0;*/

    int x = 0;
    int y = 0;
    int iEid = 0;
    TPST_MAP_BY_EID* pstENode = NULL;

    /*gettimeofday(&tv, NULL);
    lStartTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;*/

    if (iNodeX < iNodeY)
    {
        x = iNodeX;
        y = iNodeY;
    }
    else
    {
        x = iNodeY;
        y = iNodeX;
    }
    if (mpG.m_mpBasicG.find(pair<int, int>(x, y)) != mpG.m_mpBasicG.end())
    {
        /* has joined, ignore */
        return -1;
    }
    iEid = simpleAdd(mpG, x, y);

    pstENode = mpG.findNode(iEid);
    DEBUG_ASSERT(NULL != pstENode);

    /*gettimeofday(&tv, NULL);
    lCurTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    g_lInitTime += lCurTime - lStartTime;*/

    /*gettimeofday(&tv, NULL);
    lStartTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;*/
    edgeDec(mpG, iEid, vOLChgE);
    //DEBUG_PRINTF("INSERT done\n");
    /*gettimeofday(&tv, NULL);
    lCurTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    g_lInitTimeA += lCurTime - lStartTime;*/

    return iEid;
}

