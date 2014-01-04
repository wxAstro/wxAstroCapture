

#include "wxWebcamTimer.h"
#include "wxWebcamTimerCB.h"



wxWebcamTimer::wxWebcamTimer(wxWebcamTimerCB* p_callback)
: m_callback(p_callback)
, m_timerOn(false)
, m_timerIntervall(0)
, m_abortFlag(false)
, m_expoDurationSet(2000)
, m_endTimeOfExposure(0)
, m_exposing(false)
, m_ampLeadDurationSet(500)
, m_trigTimeOfAmpLead(0)
, m_ampLeadPending(false)
, m_progressIntervall(1000)
, m_progressNextReport(0)
, m_timerXTimeSet(0)
, m_endTimeOfTimerX(0)
, m_timerX(false)
, m_realExpotime(0)
{
}

wxWebcamTimer::~wxWebcamTimer()
{
    this->Stop();
}


// override parent
void wxWebcamTimer::Notify()
{
   wxLongLong now;

    // do callbacks
    if (m_callback) {
        // first check the exposure timer
        if (m_exposing) {
            // check the amp timeout (if used at all)
            now = ::wxGetLocalTimeMillis(); // hold current time
            if ( m_ampLeadPending && (now>=m_trigTimeOfAmpLead) ) {
                m_ampLeadPending = false; // disable further evaluation
                m_callback->TriggerExposureEvent(m_abortFlag);
            }
            // evaluate the exposure timer
            now = ::wxGetLocalTimeMillis(); // hold current time
            if ( now>=m_endTimeOfExposure ) {
               // now the expotimer has expired
                m_exposing = false; // disable further evaluation
                m_callback->ReadExposureEvent(m_abortFlag);
            } else if (now>=m_progressNextReport) {
               // the report interval time was reached
                m_progressNextReport += m_progressIntervall; // setup the next report time
                //m_callback->ProgressEvent(m_currentExpoTimeLeft/1000); // faster
                m_callback->ProgressEvent(( (m_endTimeOfExposure-now).ToLong() + m_progressIntervall/2)/1000);  // better
            }
        }
        // check special timer
        if (m_timerX) {
            now = ::wxGetLocalTimeMillis(); // hold current time
            if ( now >= m_endTimeOfTimerX ) {
                m_timerX = false; // disable
                m_callback->TimerXEvent();
            }
        }
        // then regular timer
        m_callback->TimerEvent(); // regular one
    }
}


void wxWebcamTimer::Stop()
{
    m_timerOn = false;
    wxTimer::Stop();
    // if there is a capture sequence running - abort it
    this->AbortExposure();
}

// our own added interface

// start of the general timer with a given pace
bool wxWebcamTimer::Start(int milliseconds)
{
    // min intervall is 5ms
    if (milliseconds<5) return false; // ERROR EXIT -

    // kick running exposures
    this->Stop();

    // then restart
    m_abortFlag = false;
    m_timerOn = wxTimer::Start(milliseconds, false);
    // may be they cannot agree on our number so we take what it reports
    m_timerIntervall = wxTimer::GetInterval();
    return m_timerOn;
}

// an independent timer
bool wxWebcamTimer::StartTimerX(unsigned long milliseconds)
{
   if (!m_timerOn) return false; // ERROR EXIT - timer is not set
   if (m_timerX) return false; // ERROR EXIT - already running
   // the callers request
   m_timerXTimeSet = milliseconds;
    // set end time
   m_endTimeOfTimerX = ::wxGetLocalTimeMillis() + m_timerXTimeSet;
   m_timerX = true; // make it running
   return true;
}

// How long will it take before the ReadExposureEvent is fired
// complete exposure time
bool wxWebcamTimer::StartExposure(unsigned long milliseconds)
{
    if (!m_timerOn) return false; // ERROR EXIT - timer is not set
    if (m_exposing) return false; // ERROR EXIT - already exposing

    m_abortFlag = false;
    // that is the callers request
    m_expoDurationSet = milliseconds;
    // downcounter - currently left until end
    m_endTimeOfExposure = ::wxGetLocalTimeMillis() + m_expoDurationSet;
    // if caller wants amp switched, set lead time
    m_trigTimeOfAmpLead = m_endTimeOfExposure - m_ampLeadDurationSet;
    m_ampLeadPending = (m_ampLeadDurationSet>0);

    // the next time to call back for a progress report on exposure
    m_progressNextReport = ::wxGetLocalTimeMillis() + m_progressIntervall;
    // must be last assignement - this will enable downcount in the tick callback
    m_exposing = true;
    return true;
}


void wxWebcamTimer::AbortExposure()
{
   // if there is a capture sequence running - abort it
   if (m_exposing) {
      m_ampLeadPending = false;
      m_endTimeOfExposure = 0;
      m_abortFlag = true;
      this->Notify();
   }
}


// How long before the end will the TriggerExposureEvent be fired
// better this is shorter than the above...
bool wxWebcamTimer::AmpOnLeadTime(unsigned long milliseconds)
{
    if (m_exposing) return false; // ERROR EXIT - cannot change while running

    m_ampLeadDurationSet = milliseconds;
    return true;
}


// in which intervals is the ProgressEvent fired
// 0= not at all
bool wxWebcamTimer::ProgressIntervall(unsigned long seconds)
{
    if (m_exposing) return false; // ERROR EXIT - cannot change while running

    m_progressIntervall = seconds * 1000;
    return true;
}

// this is in fact a stop watch to track a time in seconds
// calling here will save the current time
void wxWebcamTimer::InitRealExposureSeconds()
{
   m_realExpotime = ::wxGetLocalTimeMillis();
}

// calling here will report the time in ms since last Init call
float wxWebcamTimer::GetRealExposureSeconds() const
{
   float retVal = (::wxGetLocalTimeMillis() - m_realExpotime).ToDouble() / 1000.0f;
   return retVal;

}






