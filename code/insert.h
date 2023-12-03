#pragma once
/***************
alley function
****************/

class insertPart
{
private:
    static int rmEInVisQEasy(myG &mpG, int iCurK, int iCurL, myPriQueueLess<int, int> &myVstQ);
    static int insKByBFSEasy(myG &mpG, int iCurT, vector <int> &vSeed, vector <int> &vUpGE, vector <int> &vUpdSeed, map<int, int> &mpOLChgE);
    static int upgradeForAllK(myG &mpG, list<int> &lsCdtE, map<int, int> &mpOLChgE);
    static int simpleAdd(myG &mpG, int x, int y);
    static int edgeDec(myG &mpG, int iEid, vector <int> &vOLChgE);

    static bool checkInsSeSup(myG &mpG, int iSelfId, int iLtId, int iRtId);
    static bool checkDesSeSup(myG &mpG, int iSelfId, int iLtId, int iRtId);
public:
    /* find the same alley */
    static int insertOne(myG &mpG, int iNodeX, int iNodeY, vector <int> &vOLChgE);
    static int mergeKey(int iKey1, int iKey2)
    {
        DEBUG_ASSERT(iKey1 <= 0x3fffffff);
        DEBUG_ASSERT(iKey2 <= 0x1);
        return (iKey1 << 1) + iKey2;
    }

};
