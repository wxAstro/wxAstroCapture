
#ifndef _WXWEBCAMTIMERCB_H__
#define _WXWEBCAMTIMERCB_H__

#include "captypes.h"

class wxWebcamTimerCB
{

public:
    wxWebcamTimerCB() {};
    virtual ~wxWebcamTimerCB() {};

    // regular timer event at the nominal pace
    virtual void TimerEvent() =0;

    // special timer event
    virtual void TimerXEvent() =0;

    // calls back the AmpLeadTime before the exposure ends
    virtual void TriggerExposureEvent(bool aborted) =0;
    // calls back if the exposure ends
    virtual void ReadExposureEvent(bool aborted) =0;
    // returns seconds left until exposure ends
    virtual void ProgressEvent(unsigned long secondsLeft) =0;
};


#endif  // _WXWEBCAMTIMERCB_H__
