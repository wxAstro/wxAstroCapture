#include "BatchJob.h"

BatchJob::BatchJob()
: m_exptime(0.0)
, m_nframes(0)
, m_framesCaptured(0)
, m_inProcess(false)
{}

BatchJob::BatchJob(double exptime, int nframes)
: m_exptime(exptime)
, m_nframes(nframes)
, m_framesCaptured(0)
, m_inProcess(false)
{}


BatchJob::~BatchJob()
{
}

double BatchJob::ExpoTime()
{
   return m_exptime;
}

int BatchJob::NumFrames()
{
   return m_nframes;
}

void BatchJob::StartJob()
{
   m_inProcess = true;
}

int  BatchJob::FramesRemaining()
{
   return (m_nframes - m_framesCaptured);
}

void BatchJob::OneFrameDone()
{
   m_framesCaptured++;
}
