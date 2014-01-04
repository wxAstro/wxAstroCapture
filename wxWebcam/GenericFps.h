
#ifndef _GENERICFPS_H_
#define _GENERICFPS_H_


#ifndef DSCAPLIB_EXPORTS
// this part will not be defined in the dscaplib.dll code

	// For compilers that support precompilation, includes "wx/wx.h".
	#include "wx/wxprec.h"

	#ifndef WX_PRECOMP
	#include "wx/wx.h"
	#endif

	#define _llong unsigned long

#else
	#include <strmif.h>
	#define _llong REFERENCE_TIME

#endif // !DSCAPLIB_EXPORTS

class GenericFps
{
public:
	GenericFps(int intervalLength, _llong msDivider);
	virtual ~GenericFps();

	void AddSentFrame(long timeStamp);
	void AddSentFrame(_llong timeStamp);

	void AddDroppedFrame(long timeStamp);
	void AddDroppedFrame(_llong timeStamp);

	float FpsRate();
	float DpsRate();

private:
	GenericFps(){};
	void IncPutIndexS();			// forward put index
	void IncPutIndexD();			// forward put index

	int		 m_intervalLength;  // how long is the scan array
	_llong   m_msDivider;  // divide timestamps to make millisec units
	_llong   *m_pScanArrayS;
	_llong   *m_pScanArrayD;
	_llong   m_lastTimeS;		// is always the latests add time
	_llong   m_lastTimeD;		// is always the latests add time
	unsigned m_putIndexS;		// where to put next
	unsigned m_putIndexD;		// where to put next

};

#endif //_GENERICFPS_H_
