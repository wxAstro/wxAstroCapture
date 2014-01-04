#include "BatchJobManager.h"

#include "wxFactory.h"
#include "wxConfigPath.h"
#include "wxWebcam/wxCamera.h"

BatchJobManager::BatchJobManager()
: m_active(false)
{
   //ctor
}

BatchJobManager::~BatchJobManager()
{
   //dtor
}

void BatchJobManager::DefineBatchJobs(const BatchList& batch_list)
{
   // keep the GUI list
   m_batch_list = batch_list;

   // produce the list that will be consumed during exposure
   m_batch_jobs.clear();
   BatchList::iterator i = m_batch_list.begin();
   while(i != m_batch_list.end()) {
      const ExpNumSpec& expnum = *i++;
      if(expnum.active) {
         m_batch_jobs.push_back(BatchJob(expnum.exptime,expnum.numexp));
      }
   }

   // Job confirmed, save to config at this stage
   SaveToConfig();
}

BatchJobManager::iterator BatchJobManager::begin()
{
   return m_batch_list.begin();
}

BatchJobManager::iterator BatchJobManager::end()
{
   return m_batch_list.end();
}

void BatchJobManager::setActive(bool active)
{
   m_active = active;
}

bool BatchJobManager::active()
{
   return m_active;
}


bool BatchJobManager::JobAvailable()     // true if one is available with ActiveJob()
{
   return (m_batch_jobs.size()>0);
}

BatchJob& BatchJobManager::ActiveJob()   // get ref to Job on QueueHead
{
   return m_batch_jobs.front();
}

bool BatchJobManager::CommitJob()       // job is done, remove it
{
   m_batch_jobs.pop_front();
   return true;
}

// return total number of batch jobs
size_t BatchJobManager::size()
{
   return m_batch_list.size();
}

size_t BatchJobManager::active_size()
{
   size_t n_active = 0;
   BatchList::iterator ib = m_batch_list.begin();
   while(ib != m_batch_list.end()) {
      const ExpNumSpec& expnum = *ib++;
      if(expnum.active)n_active++;
   }
   return n_active;
}

// return number of remaining jobs
size_t BatchJobManager::remaining()
{
   if(m_active) return m_batch_jobs.size();
   else return 0;
}

void BatchJobManager::SaveToConfig()
{
   if(wxCamera* cam = wxF()->cam()) {
      wxConfig* config = wxF()->config();

      // delete old batch job data
      DeleteConfigJobs(config,cam);

      // set path to new config dir
      wxConfigPath path(config,ConfigPath(cam));

      // write batch job entries
      int counter = 0;
      BatchList::iterator ib = m_batch_list.begin();
      while(ib != m_batch_list.end()) {
         const ExpNumSpec& expnum = *ib++;
         int iactive = (expnum.active)? 1 : 0;
         wxString keyword = wxString::Format(wxT("Batch%03i"),++counter);
         wxString value = wxString::Format(wxT("%f/%d/%d"), expnum.exptime, expnum.numexp,iactive);
         config->Write(keyword,value);
      }
   }
}

bool BatchJobManager::RestoreFromConfig()
{
   // erase current content
   m_batch_list.clear();

   if(wxCamera* cam = wxF()->cam()) {
      wxConfig* config = wxF()->config();
      wxConfigPath path(config,ConfigPath(cam));

      int counter = 0;
      bool value_found = true;
      while(value_found) {
         wxString keyword;
         keyword.Printf(wxT("Batch%03i"),++counter);
         wxString value;
         if(value_found = config->Read(keyword,&value,wxT(" "))) {

            long numexp = 0;
            double exptime = 0.0;
            long iactive = 0;

            // find the forward slash and extract the FITS keyword
            int s1 = value.Find('/');
            wxString s_exptime = value.Mid(0,s1);
            s_exptime.Trim();
            s_exptime.ToDouble(&exptime);

            // extract the remaining string
            value = value.Mid(s1+1);
            value.Trim();
            s1 = value.Find('/');

            wxString s_numexp = value.Mid(0,s1);
            s_numexp.Trim();
            s_numexp.ToLong(&numexp);

            wxString s_active = value.Mid(s1+1);
            s_active.ToLong(&iactive);
            bool active = (iactive == 1)? true : false;

            // push the entry
            m_batch_list.push_back(ExpNumSpec(exptime,numexp,active));
         }
      }
   }
   return (m_batch_list.size() > 0);
}

void BatchJobManager::DeleteConfigJobs(wxConfig* config, wxCamera* cam)
{
   wxConfigPath path(config,ConfigPath(cam));
   config->SetPath(wxT(".."));
   config->DeleteGroup(wxT("Jobs"));
}

wxString BatchJobManager::ConfigPath(wxCamera* cam)
{
   if(!cam)return _T("/Batch/Jobs");
   wxString path = _T("/Batch/") + cam->CameraName() +_T("(")+  cam->CameraSerial() + _T(")/Jobs");
   return path;
}
