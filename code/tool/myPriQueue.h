#pragma once
/***************
my list class
add function: insert by order
****************/
#include <stdio.h>
#include <vector>
#include <queue>

using namespace std;

template<typename _Key, typename _Data>
class myPriQueueBig
{
private:
struct stData
{
    _Key iKey;
    _Data iData;
    stData(_Key a, _Data b)
    {
        iKey = a;
        iData = b;
    }
    bool operator<(const stData& a) const
    {
        return iKey < a.iKey;
    }
};
    priority_queue<stData> m_prQ;

public:
    bool empty()
    {
        return m_prQ.empty();
    }
    /**
    return true: new insert; false: reinsert
    **/
    bool insertByOrder(_Key a, _Data b)
    {
        m_prQ.emplace(a, b);
        return true;
    }

    /* get fixed key */
    _Data getTop()
    {
        _Data iTpVal = m_prQ.top().iData;
        return iTpVal;
    }
    /* get dynamic key */
    _Key getKey()
    {
        _Key iTpVal = m_prQ.top().iKey;
        return iTpVal;
    }
    bool pop()
    {
        m_prQ.pop();
        return true;
    }
};


template<typename _Key, typename _Data>
class myPriQueueLess
{
private:
struct stData
{
    _Key iKey;
    _Data iData;
    stData(_Key a, _Data b)
    {
        iKey = a;
        iData = b;
    }
    bool operator<(const stData& a) const
    {
        return iKey > a.iKey;
    }
};
    priority_queue<stData> m_prQ;

public:
    bool empty()
    {
        return m_prQ.empty();
    }
    /**
    return true: new insert; false: reinsert
    **/
    bool insertByOrder(_Key a, _Data b)
    {
        m_prQ.emplace(a, b);
        return true;
    }

    /* get fixed key */
    _Data getTop()
    {
        _Data iTpVal = m_prQ.top().iData;
        return iTpVal;
    }
    /* get dynamic key */
    _Key getKey()
    {
        _Key iTpVal = m_prQ.top().iKey;
        return iTpVal;
    }
    bool pop()
    {
        m_prQ.pop();
        return true;
    }
};







