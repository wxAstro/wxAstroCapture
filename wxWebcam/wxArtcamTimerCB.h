
#ifndef _WXARTCAMTIMERCB_H__
#define _WXARTCAMTIMERCB_H__

#include "ArtTypes.h"

class wxArtcamTimerCB
{

public:
    wxArtcamTimerCB() {};
    virtual ~wxArtcamTimerCB() {};

    // regular timer event at the nominal pace
    virtual void TimerEvent() =0;

    // returns seconds left until exposure ends
    virtual void ProgressEvent(unsigned long secondsLeft) =0;
};


#endif  // _WXARTCAMTIMERCB_H__
