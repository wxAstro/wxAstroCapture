
#include "GenericFps.h"


GenericFps::GenericFps(int intervalLength, _llong msDivider)
: m_intervalLength(intervalLength)
, m_msDivider(msDivider)
, m_pScanArrayS(NULL), m_pScanArrayD(NULL)
, m_lastTimeS(0), m_lastTimeD(0)
, m_putIndexS(0), m_putIndexD(0)
{
	m_pScanArrayS = new _llong[m_intervalLength+1];
	::memset(m_pScanArrayS, 1, m_intervalLength+1 * sizeof(_llong));
	m_pScanArrayD = new _llong[m_intervalLength+1];
	::memset(m_pScanArrayD, 1, m_intervalLength+1 * sizeof(_llong));
}


GenericFps::~GenericFps()
{
	delete [] m_pScanArrayS;
	delete [] m_pScanArrayD;
}


void GenericFps::AddSentFrame(long timeStamp)
{
	AddSentFrame((_llong)(timeStamp)); // just forward
}

void GenericFps::AddSentFrame(_llong timeStamp)
{
	m_lastTimeS=timeStamp;
	m_lastTimeD=timeStamp;
	m_pScanArrayS[m_putIndexS] = timeStamp; IncPutIndexS();
}

void GenericFps::AddDroppedFrame(long timeStamp)
{
	AddDroppedFrame((_llong)(timeStamp)); // just forward
}


void GenericFps::AddDroppedFrame(_llong timeStamp)
{
	m_lastTimeD=timeStamp;
	m_lastTimeS=timeStamp;
	m_pScanArrayD[m_putIndexD] = timeStamp; IncPutIndexD();
}


float GenericFps::FpsRate()
{
	float rate = 0;
	// interval is lastAdd - nextPut (oldest)
	_llong intervalTime = (m_lastTimeS - m_pScanArrayS[m_putIndexS]);
	if (intervalTime>0) rate = float(m_intervalLength * 1000.0 * m_msDivider / intervalTime);

	return  rate;
}

float GenericFps::DpsRate()
{
	float rate = 0;
	// interval is lastAdd - nextPut (oldest)
	_llong intervalTime = (m_lastTimeD - m_pScanArrayD[m_putIndexD]);
	if (intervalTime>0) rate = float(m_intervalLength * 1000.0 * m_msDivider / intervalTime);

	return  rate;
}

void GenericFps::IncPutIndexS()
{
	m_putIndexS = ++m_putIndexS % m_intervalLength;
}

void GenericFps::IncPutIndexD()
{
	m_putIndexD = ++m_putIndexD % m_intervalLength;
}


