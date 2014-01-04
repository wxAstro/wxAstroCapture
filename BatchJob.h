#ifndef BATCHJOB_H
#define BATCHJOB_H

/*
   This class defines a series of equal length exposures
*/

class BatchJob {
public:
   BatchJob();
   BatchJob(double exptime, int nframes);
   virtual ~BatchJob();

   // returns the basic specification for this job
   double ExpoTime();
   int    NumFrames();

   void   StartJob();
   int    FramesRemaining();
   void   OneFrameDone();

private:
   double m_exptime;
   int    m_nframes;
   int    m_framesCaptured;
   bool   m_inProcess;
};

#endif // BATCHJOB_H
