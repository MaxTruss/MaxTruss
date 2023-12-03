#pragma once
/***************
my class DAG
****************/

#define DAG_S_ID 1
#define DAG_T_ID 2

#define DAG_DRIFT 10
#define DAG_FRAG 10
typedef struct tpstDAGEdge
{
    int iEid;
    int iPPid;
    int iCPid;
    int iWeight;
    float iFlow;
    float iCap;
}TPST_DAG_EDGE;

typedef struct tpstDAGNode
{
    int iPid;
    int iName;
    int iWeight;
    int iCost;
    int iLayer;
    int iSinkEid;
    bool bSrc;
    //bool bAnchord;
    bool bVistd;
    bool bShow;
    bool bInQ;
    bool bActive;
    bool bAban;
    vector<int> vParents;
    vector<int> vChildren;
}TPST_DAG_NODE;

typedef struct tpstRes
{
    int iScore;
    int iFlow;
    vector <int> vAbaP;
    //vector <int> vPriAncE;
    //vector <pair<int, int> > vPaAncE;
    //vector <int> vSecAncP;
}TPST_RES;

class DAG
{
private:
    int m_iMaxPid;
    int m_iMaxEid;
    int m_iMaxLayer;
    int m_iMaxPWeight;
    int m_iTotalBgt;
    int m_iTotalScore;

    int m_iM;
    int m_iW1;
    int m_iW2;
    float m_fG;
    /* flow, <score, position in result> */
    map<int, pair<int, int> > m_mpScoreTable;

    /* <name, pid > */
    unordered_map<int, int> m_umpName;
    /* layer, <node id> */
    map<int, vector<int> > m_mpLayer;
    vector<TPST_DAG_NODE> *m_pvDAG;
    vector<TPST_DAG_EDGE> *m_pvE;

    int addS(int iCName, int iWeight);

    bool findPathDFS(int iSrc, vector <int> &vPath);
    float blockFlow(vector <int> &vPath);
    int active(int iSrc, vector <int> &vFirst);
    int activeFromAban(int iSrc, vector <int> &vActiveP, vector <int> &vFirst);
    int anchorEdge(vector <int> &vFirst, vector <int> &vAbaP);
    int abandonNodes(vector <int> &vAbaP);
    int getRealFlow();
    int anchor(int iSrc, vector <int> &vFirst, vector <int> &vAbaP);
    float Dinic(int iSrc);
    int saveRes(int iFlow, int iScore, vector <int> &vAbaP);

    int externCap();
    int clearFlow(int iSrc);
    int clearFlow();
    int initMinCut();
    int initGoldberg(int iW1, int iW2, float f_g);

    int expand(map <int, int> &mpAbanPool, int *piTotalCut);

public:

    /* budget */
    vector <TPST_RES> m_vRes;

    DAG();
    ~DAG();

    int addNode(int iName, int iWeight);
    int add(int iPName, int iCName, int iWeight);
    int addT(int iPName, int iWeight);

    int construct();
    int constructGoldberg();
    int show();
    int runMinCut(int iMaxBudget);
    int prim(int iMaxBudget);
    int sizeFirst(int iMaxBudget);
    int Goldberg(int iMaxBudget);

    TPST_DAG_NODE * findNode(int iPid);
    TPST_DAG_EDGE * findE(int iEid);

    int findPid(int iName)
    {
        DEBUG_ASSERT(m_umpName.find(iName) != m_umpName.end());
        return m_umpName[iName];
    }

};
