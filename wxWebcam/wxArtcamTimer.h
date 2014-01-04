
#ifndef _WXARTCAMTIMER_H__
#define _WXARTCAMTIMER_H__

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

class wxArtcamTimerCB;
class wxArtcamTimer : public wxTimer
{

public:
    wxArtcamTimer(wxArtcamTimerCB* p_callback);
    virtual ~wxArtcamTimer();

public:
    // override
    void Notify();
    void Stop();

    // our own added interface

    // general start of this timer
    bool Start(int milliseconds = 50);

    bool StartExposure(unsigned long milliseconds);

    // Kill the cycle
    void AbortExposure();
    // in which intervals is the ProgressEvent fired
    // 0= not at all
    bool ProgressIntervall(unsigned long seconds);

    bool IsTimerOn() {return m_timerOn;};

    void InitRealExposureSeconds();
    float GetRealExposureSeconds();

private:
    wxArtcamTimer() :m_callback(NULL) {};

    wxArtcamTimerCB* m_callback;

    bool           m_timerOn;
    int            m_timerIntervall; // the interval of the general timer
    bool           m_abortFlag;

    // all in milliseconds
    long m_expoDurationSet;         // ms exposure time as given by the caller
    wxLongLong m_endTimeOfExposure; // ms time when the exposure ends
    bool m_exposing;                // true when exposure is running

    long m_progressIntervall;       // ms interval time at which progress events are triggered
    wxLongLong m_progressNextReport;// the ms time when a exposure progress report must be issued

    wxLongLong m_realExpotime;      // used to track the real exposure time [ms]
};


#endif  // _WXARTCAMTIMER_H__
