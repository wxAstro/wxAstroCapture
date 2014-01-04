#ifndef BATCHJOBMANAGER_H
#define BATCHJOBMANAGER_H

#include <wx/config.h>
#include <list>
#include "BatchJob.h"
#include <wx/string.h>
class wxCamera;

class BatchJobManager {
public:
   BatchJobManager();
   virtual ~BatchJobManager();

   void SaveToConfig();
   bool RestoreFromConfig();

   // interface to be used by wxCamera during batch execution
   bool JobAvailable();    // true if one is available with ActiveJob()
   BatchJob& ActiveJob();  // get ref to Job on QueueHead
   bool CommitJob();       // job is done, remove it

   struct ExpNumSpec {
      ExpNumSpec() : exptime(0.0),numexp(0),active(false) {}
      ExpNumSpec(double _exptime, long _numexp, bool _active)
                 : exptime(_exptime),numexp(_numexp),active(_active) {}
      double exptime;
      long   numexp;
      bool   active;
   };

   // interface to be used by GUI follows
   typedef std::list<ExpNumSpec> BatchList;
   typedef BatchList::iterator   iterator;

   // the GUI calls DefineBatchJobs to define the list of exposure specs
   // before actually initiating the batch exposures in wxCamera
   void DefineBatchJobs(const BatchList& batch_list);

   // For GUI USE: STL style iteration over batch jobs
   iterator begin();
   iterator end();

   void setActive(bool active);
   bool active();

   // return total number of batch jobs, active and inactive
   size_t size();

   // return number of active batch jobs
   size_t active_size();

   // return number of remaining jobs
   size_t remaining();

private:
   wxString ConfigPath(wxCamera* cam);
   void DeleteConfigJobs(wxConfig* config, wxCamera* cam);

private:
   typedef std::list<BatchJob>   BatchJobList;
   BatchJobList m_batch_jobs; // this list is modified during batch job execution
   BatchList    m_batch_list; // this list reflects GUI always
   bool         m_active;     // true when batch job is being executed
};

#endif // BATCHJOBMANAGER_H
