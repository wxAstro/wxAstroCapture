
#ifndef _WXWEBCAMTIMER_H__
#define _WXWEBCAMTIMER_H__

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

class wxWebcamTimerCB;
class wxWebcamTimer : public wxTimer
{

public:
    wxWebcamTimer(wxWebcamTimerCB* p_callback);
    virtual ~wxWebcamTimer();

public:
    // override
    void Notify();
    void Stop();

    // our own added interface

    // general start of this timer
    bool Start(int milliseconds = 10);

    // used to handle web cam LongExposures


    bool StartTimerX(unsigned long milliseconds);

    // How long will it take before the ReadExposureEvent is fired
    // complete exposure time
    bool StartExposure(unsigned long milliseconds);
    // Kill the cycle
    void AbortExposure();
    // How long before the end will the TriggerExposureEvent be fired
    // better this is shorter than the above...
    bool AmpOnLeadTime(unsigned long milliseconds);
    // in which intervals is the ProgressEvent fired
    // 0= not at all
    bool ProgressIntervall(unsigned long seconds);

    bool IsTimerOn() const {return m_timerOn;};

    void InitRealExposureSeconds();
    float GetRealExposureSeconds() const;

private:
    wxWebcamTimer() :m_callback(NULL) {};

    wxWebcamTimerCB* m_callback;

    bool           m_timerOn;
    int            m_timerIntervall; // the interval of the general timer
    bool           m_abortFlag;

    // all in milliseconds
    long m_expoDurationSet;         // ms exposure time as given by the caller
    wxLongLong m_endTimeOfExposure; // ms time when the exposure ends
    bool m_exposing;                // true when exposure is running

    long m_ampLeadDurationSet;      // amp lead time that was given by the caller
    wxLongLong m_trigTimeOfAmpLead; // ms time when a the ampLead must be reported
    bool m_ampLeadPending;          // true if an amp lead is still pending

    long m_progressIntervall;       // ms interval time at which progress events are triggered
    wxLongLong m_progressNextReport;// the ms time when a exposure progress report must be issued

    long m_timerXTimeSet;           // ms lapse time as given by the caller
    wxLongLong m_endTimeOfTimerX;   // ms time when the timer ends
    bool m_timerX;                  // true if timerX is running

    wxLongLong m_realExpotime;      // used to track the real exposure time [ms]
};


#endif  // _WXWEBCAMTIMER_H__
