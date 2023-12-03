#pragma once
/***************
my list class
add function: insert by order
****************/
#include <stdio.h>
#include <vector>

using namespace std;

class myRandom
{
public:
    static int random(int iMax)
    {
        int iBit = 0;
        int lTpV = iMax;
        int lRanV = 0;
        int iTpRanV = 0;
        static unsigned m_uSeed = 0;

        if (0 == m_uSeed)
        {
            char * pcTp = (char *)malloc(sizeof(char) * 10);
            m_uSeed = (unsigned)time(NULL) & (unsigned)((long long)pcTp % 0xffff);
            free(pcTp);
        }
        srand(m_uSeed);
        m_uSeed = rand();

        while (0 < lTpV)
        {
            /* 12 bits at each time */
            iBit += 12;
            lTpV >>= 12;

            iTpRanV = (rand() & 0xfff);
            lRanV = (lRanV << 12) | (iTpRanV);
        }
        return lRanV % iMax;
    }
    static float random(float fMax)
    {
        int iRes = 0;
        int iRatio = 100;
        int iMax = 0;
        double dTpV = 0;

        dTpV = fMax * iRatio;
        iMax = dTpV;
        if (iMax <= dTpV - 1)
        {
            printf("ERROR get %f, %d %d %f %f\n", fMax, iMax, (int)dTpV, dTpV, dTpV - 1);
            DEBUG_ASSERT(0);
        }
        iRes = random(iMax);
        return iRes / (iRatio + 0.0);
    }
};









