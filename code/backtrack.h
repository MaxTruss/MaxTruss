#pragma once
/***************
backtrack function
****************/

typedef struct tpstInsG
{
    int iMaxEId;
    int iMaxD;
    int iMaxK;
    /* pid, degree */
    map<int, int> mpChgdD;
    map<int, TPST_MAP_BY_EID> mpChgdE;
    map<pair<int, int>, int> mpNewE;
}TPST_INS_G;

typedef struct tpstvByDepth
{
    int iDepth;
    TPST_INS_G* pstInsG;
    vector<int> vChgE;
    vector<pair<int, int> > vCurNewCanE;
    vector<pair<int, int> > vCurCanE;
    vector<pair<int, int> >::iterator itCurPos;
}TPST_V_BY_DEPTH;

class backtrack
{
public:
    int m_iSize;
    int m_iDesK;
    int m_iBestScore;
    myG* m_pmyG;
    myG* m_pmyBackG;
    vector<pair<int, int> > *m_pvCanE;
    vector<TPST_V_BY_DEPTH> *m_pvStack;
    map<pair<int, int>, int> m_mpNewE;
    vector<pair<int, int> > m_vBestNewCanE;
    TPST_INS_G* m_pstBestInsG;
    vector<pair<int, int> > m_vBestE;

    backtrack(myG &mpInitG, myG &mpBackG, int iBat, vector<pair<int, int> > &vCanE, int iDesK, vector <pair<int, int> > &vInsE);
    ~backtrack();
    int init(myG &mpInitG);
    bool process(int iDepth);
    int run(vector<pair<int, int> > &vCanE, vector<pair<int, int> > &vInsE);
    int tryRun(vector<pair<int, int> > &vCanE, vector<pair<int, int> > &vInsE);
    /* restore */
    static int restoreG(myG &mpRunG, myG &mpBackG, vector<int> &vChgE);
    /* return real score */
    static int tryIns(myG &mpRunG, myG &mpBackG, int iDesK, vector<pair<int, int> > &vInsE);
    /* return score in a limited range */
    static int tryIns(myG &mpRunG, myG &mpBackG, int iDesK, vector<pair<int, int> > &vInsE, vector<int> &vDesE);
    static int tryIns(myG &mpRunG, myG &mpBackG, int iDesK, vector<pair<int, int> > &vInsE, vector<int> &vDesE, vector<pair<int, int> > &vSuccE, int *piRealScore);
    static int batIns(myG &mpInitG, myG &mpBackG, vector<pair<int, int> > &vInsE);

    long long findCanE(myG &mpInitG, vector<pair<int, int> > &vCanE);
    int showCanE(vector<pair<int, int> > &vCanE);

    int keepChg(myG &myResG, TPST_INS_G* pstInsG, vector<int> &vChgE);
    int copyInsG(TPST_INS_G* pstDesG, TPST_INS_G* pstSrcG);
    int clearInsG(myG &mySrcG, TPST_INS_G* pstDesG);
    int addG(myG &myResG, TPST_INS_G* pstInsG);
    int restoreG(myG &myResG, myG &myInitG, TPST_INS_G* pstInsG, vector<int> &vChgE);

    int getScore();
    int saveBest();
};
