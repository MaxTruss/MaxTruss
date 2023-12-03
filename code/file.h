#pragma once
/***************
input file output file function
****************/

/*#define DATA_PATH ("../data/test.txt")
#define TRUSS_FILE ("truss.txt")
#define FILE_NAME_BUFFER 100*/
#define ONE_LINE_BUFFER 100
typedef struct save_entry
{
    uint32_t x;
    uint32_t y;
    uint32_t k;
    uint32_t L;
    uint32_t SeSup;
} SaveEntry;
/* read file and fill the list g_lstFindEdgeByP */
/*int file_fillG(char *szFileName, myG &mpG);
int file_readPrivate(char *szFileName, map<int, list<int> > &mpPrivate, int *piMaxPid);
int file_readQuery(char *szFileName, list<pair<int, int> > &lstQuery);*/

int file_saveDeTruss(myG &oMpG, LIST_DECOMP_G &lstDeG, char *pcFile);
int file_saveTrussness(myG &oMpG, char *pcFile);

//int file_saveG(myG &oMpG, char *szGPath, bool bFinal);
int file_saveBitG(myG &oMpG, char *pcFile);
int file_insE(myG &oMpG, char *pcFile, vector<pair<int, int> > &vInsE, int iBudget, int iDesK);
int file_readInsE(char *pcFile, vector<pair<int, int> > &vInsE);
//int file_readG(myG &oMpG, char *szGPath);
int file_readBitG(myG &oMpG, char *pcFile);

