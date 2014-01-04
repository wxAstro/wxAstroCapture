#include "wxFactory.h"

#include "wxIO/wxPortManager.h"
#include "wxIO/wxLongExpoPort.h"
#include "wxIO/wxGuiderPort.h"
#include "wxIO/wxPort.h"
#include "wxWebcam/fits/wxFitsKeyword.h"
#include "wxGuider.h"
#include "wxGuiderFileInterface.h"
#include "wxGuiderPortInterface.h"
#include "wxGuiderBridgeInterface.h"
#include "wxGuidingConfig.h"
#include "wxConfigPath.h"
#include <wx/stdpaths.h>
#include "wxWebcam/wxWebcam.h"
#include "wxWebcam/wxArtcam.h"

#include "BatchJobManager.h"

wxFactory* wxFactory::m_self = 0;

wxFactory* wxFactory::singleton()
{
   return m_self;
}

/// BM modified to use /.wxAstroCapture directory (due to GetUserDataDir())
/// the config file is named wxAstroCapture.conf per default?!
/// so make sure the old one is reallocated in the next setup !!!
wxFactory::wxFactory()
:m_config(_T("wxAstroCapture"),_T("Rustadgrenda"), wxEmptyString, wxEmptyString,
            wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_SUBDIR)
,m_camera(0)
,m_portManager(0)
,m_lePort(0)
,m_guidePort(0)
,m_index_port_entry(-1)
,m_capture_prefix(_T("wxAC"))
,m_guider(0)
,m_cntPointSize(CNT_HUGE)
,m_ftsSize(FTS_OFF)
,m_batchJobManager(0)
{
   m_self = this;


   Init();
}

wxFactory::~wxFactory()
{
   if (m_guider) delete m_guider;
   if (m_lePort) delete m_lePort;
   if (m_guidePort) delete m_guidePort;
   if (m_portManager) delete m_portManager;
   // don't delete cameras, it is not owned here

   m_policy_file.SaveToConfig(&m_config);
   m_policy_container.SaveToConfig(&m_config);
   if(m_batchJobManager) delete m_batchJobManager;
}

void wxFactory::Init()
{

   // Here we have to make sure the GetUserDataDir() exists
   // On Linux the config file is stored in this dir
   // ~/.appname on Linux
   // C:\Documents and Settings\username\Application Data\appname
   // We cannot rely on the OS or wx to create this directory

   wxStandardPaths std_paths;
   wxString userDataDir = std_paths.GetUserDataDir();
   wxString appname = m_config.GetAppName();
   userDataDir.Replace(wxT("wxastrocapture"),appname);
   if(!::wxDirExists(userDataDir)) {
      ::wxMkdir(userDataDir);
   }


   restoreFromConfig();
}

// enumerate any camera on the system
size_t wxFactory::EnumerateCameras(wxWindow* parent,wxArrayString& cameras, CameraTypeVector& ctypes)
{
    cameras.clear();
    ctypes.clear();

    // construct two temporary camera objects to help enumerate cameras
    wxWebcam* tmp1 = wxWebcam::Construct(parent);
    wxArtcam* tmp2 = wxArtcam::Construct(parent);

    size_t nweb = tmp1->EnumerateCameras(cameras);
    for(size_t i=0;i<nweb;i++)ctypes.push_back(ECT_Webcam);

    tmp2->EnumerateCameras(cameras);
    size_t nart = cameras.size() - nweb;
    for(size_t i=0;i<nart;i++)ctypes.push_back(ECT_Artcam);

    tmp1->Destroy();
    tmp2->Destroy();

    return (nweb+nart);
}

void wxFactory::setCamera(wxCamera* camera)
{
   if(wxWebcam* webcam = dynamic_cast<wxWebcam*>(camera))setWebcam(webcam);
   else if(wxArtcam* artcam = dynamic_cast<wxArtcam*>(camera))setArtcam(artcam);
}

void wxFactory::setWebcam(wxWebcam* webcam)
{
   // the camera is not owned here, any existing camera is deleted elsewhere
   m_camera = webcam;
}

void wxFactory::setArtcam(wxArtcam* artcam)
{
   // the camera is not owned here, any existing camera is deleted elsewhere
   m_camera = artcam;
}

bool wxFactory::restoreFromConfig()
{
   if(!m_config.Read(_T("CaptureFolder"),&m_capture_folder)) {
      m_capture_folder = wxGetHomeDir();
   }

   m_config.Read(_T("CapturePrefix"),&m_capture_prefix,m_capture_prefix);

   int ifont_size(2),ifont_size_def(2);
   m_config.Read(_T("FrameCounterFontSize"),&ifont_size,ifont_size_def);
   wxFactory::FrameCounterSize font_size = wxFactory::CNT_OFF;
   switch(ifont_size) {
      case 0:  {font_size = wxFactory::CNT_OFF;     break; }
      case 1:  {font_size = wxFactory::CNT_SMALL;   break; }
      case 2:  {font_size = wxFactory::CNT_MEDIUM;  break; }
      case 3:  {font_size = wxFactory::CNT_LARGE;   break; }
      case 4:  {font_size = wxFactory::CNT_HUGE;    break; }
      default: {font_size = wxFactory::CNT_MEDIUM;}
   };
   SetFrameCounterFontSize(font_size);

   int iframe_timestamp,iframe_timestamp_def(0);
   m_config.Read(_T("FrameTimeStampSize"),&iframe_timestamp,iframe_timestamp_def);
   wxFactory::FrameTimeStampSize fts_size = FrameTimeStampFontSize();
   switch(iframe_timestamp) {
      case 0:  {fts_size = wxFactory::FTS_OFF;     break; }
      case 1:  {fts_size = wxFactory::FTS_SMALL;   break; }
      case 2:  {fts_size = wxFactory::FTS_MEDIUM;  break; }
      default: {fts_size = wxFactory::FTS_MEDIUM;}
   };
   SetFrameTimeStampFontSize(fts_size);

   //20100703:BM - added new settings
   ifont_size=2; ifont_size_def=2;
   m_config.Read(_T("FpsFontSize"),&ifont_size,ifont_size_def);
   wxFactory::FrameCounterSize ffont_size = wxFactory::CNT_OFF;
   switch(ifont_size) {
      case 0:  {ffont_size = wxFactory::CNT_OFF;     break; }
      case 1:  {ffont_size = wxFactory::CNT_SMALL;   break; }
      case 2:  {ffont_size = wxFactory::CNT_MEDIUM;  break; }
      case 3:  {ffont_size = wxFactory::CNT_LARGE;   break; }
      case 4:  {ffont_size = wxFactory::CNT_HUGE;    break; }
      default: {ffont_size = wxFactory::CNT_MEDIUM;}
   };
   wxF()->SetFpsFontSize(ffont_size);

   //20100703:BM - added new settings
   int iperf_level,iperf_level_def(0);
   m_config.Read(_T("PerformanceLevel"),&iperf_level,iperf_level_def);
   wxFactory::PerfPollIntervall perf_level = wxFactory::PPL_LOW;
   switch(iperf_level) {
      case 0:  {perf_level = wxFactory::PPL_LOW;   break; }
      case 1:  {perf_level = wxFactory::PPL_MID;   break; }
      case 2:  {perf_level = wxFactory::PPL_HIGH;  break; }
      default: {perf_level = wxFactory::PPL_LOW;}
   };
   wxF()->SetPerformanceLevel(perf_level);

   // Set the chosen guider interface
   setGuiderInterface(wxGuidingConfig::getGuiderInterface());

   // restore name policies
   m_policy_file.RestoreFromConfig(&m_config);
   m_policy_container.RestoreFromConfig(&m_config);

   // Do not instantiate the guide port yet
   // wait until someone asks for it, see guidePort().
   // This is because if the guide port is a serial port, it may be in use
   // from another program at this stage (typically for GOTO purposes).
   // We don't want error messages each time we start the program because of
   // a busy guide port that we may not even want to use in this session.

   return true;
}

wxWebcam*  wxFactory::webcam()
{
   return dynamic_cast<wxWebcam*>(m_camera);
}

wxArtcam*  wxFactory::artcam()
{
   return dynamic_cast<wxArtcam*>(m_camera);;
}

wxCamera*  wxFactory::cam()
{
   return m_camera;
}

wxConfig*  wxFactory::config()
{
   return &m_config;
}

wxLongExpoPort*    wxFactory::lePort()
{
   if(!m_lePort) {
      m_lePort = new wxLongExpoPort();

      // restore long exposure port info
      m_lePort->RestoreFromConfigLongExpo(&m_config);
   }

   return m_lePort;
}

void wxFactory::lePortRelease(bool force_release)
{
   if(m_lePort) {

      bool release_port = true;

      // Release the LE port only if no webcam is connected
      if(wxWebcam* webcam = this->webcam()) {
         // we have a webcam, is it connected?
         if(webcam->IsConnected()) {
            // cannot release port
            if(!force_release)release_port = false;
         }
      }

      if(release_port) {
         delete m_lePort; // delete implies a PClose()
         m_lePort = 0;
      }
   }
}

wxGuiderPort*    wxFactory::guidePort()
{
   if(!m_guidePort) {
      m_guidePort = new wxGuiderPort();

      // restore guider port info
      m_guidePort->RestoreFromConfigGuidePort(&m_config);
   }

   return m_guidePort;
}

bool wxFactory::setGuiderInterface(wxGuider::guider_interface intfc)
{
   delete m_guider;
   m_guider = 0;

   switch(intfc) {
      case wxGuider::PORT_INTERFACE:   { m_guider = new wxGuiderPortInterface(); break; }
      case wxGuider::BRIDGE_INTERFACE: { m_guider = new wxGuiderBridgeInterface(); break; }
      default:                         { m_guider = new wxGuiderFileInterface(); break; }
   };

   if(m_guider) {
      // save interface type to configuration
      wxGuidingConfig::saveGuiderInterface();
      return true;
   }
   return false;
}

wxGuider*  wxFactory::guider()
{
   if(!m_guider)setGuiderInterface(wxGuider::FILE_INTERFACE);
   return m_guider;
}

wxString  wxFactory::captureFolder()
{
   return m_capture_folder;
}

wxString  wxFactory::capturePrefix()
{
   return m_capture_prefix;
}

// returns the portmanager
wxPortManager* wxFactory::portManager()
{
   if(!m_portManager) m_portManager = new wxPortManager();

   return m_portManager;
}

bool  wxFactory::setCaptureFolder(const wxString& folder, const wxString& prefix)
{
    m_capture_folder = folder;
    m_capture_prefix = prefix;
    m_config.Write(_T("CaptureFolder"),m_capture_folder);
    m_config.Write(_T("CapturePrefix"),m_capture_prefix);

    // Mark the name policy objects dirty so they get recomputed
    if(m_policy_file.IsActiveItem(wxT("prefix")))m_policy_file.CommitName();
    if(m_policy_container.IsActiveItem(wxT("prefix")))m_policy_container.CommitName();
    return true;
}

EVideoSampleFormat wxFactory::sampleFormat()
{
   int format,defVal(EVF_YUV); // see captypes.h EVideoSampleFormat
   m_config.Read(_T("CaptureFormat"),&format,defVal);
   EVideoSampleFormat EVformat = (EVideoSampleFormat)format;
   return EVformat;
}

EContainerFormat wxFactory::containerFormat()
{
   int format,defVal(ECF_AVI); // see captypes.h EContainerFormat
   m_config.Read(_T("ContainerFormat"),&format,defVal);
   EContainerFormat ECformat = (EContainerFormat)format;
   return ECformat;
}

bool wxFactory::saveFitsHeaderEntries(wxPropGrid::Entries& entries)
{
   wxString FitsHeader = wxT("FitsHeader");
   // remove any existing content
   m_config.DeleteGroup(FitsHeader);

   // recreate the group and write to it
   wxConfigPath config(&m_config,FitsHeader);

   int counter = 0;
   wxPropGrid::Entries::iterator ie = entries.begin();
   while(ie != entries.end()) {
      wxString keyword;
      keyword.Printf(wxT("key%03i"),++counter);
      wxString value   = ie->keyword + wxT('/') + ie->value + wxT('/') + ie->comment;
      ie++;
      m_config.Write(keyword,value);
   }
   return true;
}

bool wxFactory::getFitsHeaderValue(const wxString& kw, wxString& value)
{
   wxPropGrid::Entries entries;
   getFitsHeaderEntries(entries);
   wxPropGrid::Entries::iterator i = entries.begin();
   while(i != entries.end()) {
      if(kw == i->keyword) {
         value = i->value;
         value.Trim(true);
         value.Trim(false);
         return true;
      }
      i++;
   }
   return false;
}

bool wxFactory::saveFitsHeaderValue(const wxString& kw, wxString& value)
{
   wxPropGrid::Entries entries;
   getFitsHeaderEntries(entries);
   wxPropGrid::Entries::iterator i = entries.begin();
   while(i != entries.end()) {
      if(kw == i->keyword) {
         i->value = value;
         saveFitsHeaderEntries(entries);
         return true;
      }
      i++;
   }
   return false;
}

bool wxFactory::getFitsHeaderEntries(wxPropGrid::Entries& entries)
{
   wxString FitsHeader = wxT("FitsHeader");
   if(m_config.HasGroup(FitsHeader)) {

      wxConfigPath config(&m_config,FitsHeader);

      // a wxWidgets bug prevents use of GetFirstEntry()/GetNextEntry().
      // They work only the next time the application is started.
      // Instead we use Read() with a generated key, until it fails

      int counter = 0;
      bool value_found = true;
      while(value_found) {
         wxString keyword;
         keyword.Printf(wxT("key%03i"),++counter);
         wxString value;
         if(value_found = m_config.Read(keyword,&value,wxT("//"))) {

            // find the first forward slash and extract the FITS keyword
            int s1 = value.Find('/');
            wxString keyword = value.Mid(0,s1);

            // extract the remaining string
            value = value.Mid(s1+1);

            // find the second forward slash separating value and comment
            int s2 = value.Find('/');
            wxString val = value.Mid(0,s2);
            wxString comment = value.Mid(s2+1);

            // push the entry
            entries.push_back(wxPropGrid::Entry(keyword,val,comment));
         }
      }
   }
   else {
      // nothing in configuration, push empty entries
      entries.push_back(wxPropGrid::Entry(wxT("OBSERVER"),wxT(""),wxT("Observer name")));
      entries.push_back(wxPropGrid::Entry(wxT("SITELAT"),wxT(""),wxT("The site latitude (deg)")));
      entries.push_back(wxPropGrid::Entry(wxT("SITELONG"),wxT(""),wxT("The site longitude (deg)")));
      entries.push_back(wxPropGrid::Entry(wxT("TELESCOP"),wxT(""),wxT("Data acquisition telescope")));
      entries.push_back(wxPropGrid::Entry(wxT("INSTRUME"),wxT(""),wxT("Name of instrument (camera)")));
      entries.push_back(wxPropGrid::Entry(wxT("PSZX"),wxT(""),wxT("Size of a pixel in X direction [m]")));
      entries.push_back(wxPropGrid::Entry(wxT("PSZY"),wxT(""),wxT("Size of a pixel in Y direction [m]")));
      entries.push_back(wxPropGrid::Entry(wxT("FILTER"),wxT(""),wxT("Filter used")));
      entries.push_back(wxPropGrid::Entry(wxT("OBJECT"),wxT("Object"),wxT("Object of observation")));
      entries.push_back(wxPropGrid::Entry(wxT("IMAGETYP"),wxT(""),wxT("LIGHT, BIAS, FLAT or DARK")));
      entries.push_back(wxPropGrid::Entry(wxT("COMMENT"),wxT(""),wxT("Any Comment")));

      // Also save these values to config immediately
      saveFitsHeaderEntries(entries);
   }

   return true;
}

bool wxFactory::getFitsKeywordList(wxFitsKeywordList& kwlist)
{
   wxPropGrid::Entries entries;
   if(getFitsHeaderEntries(entries)) {

	  // convert the grid entries to wxFitsKeywords and add to list
	  wxPropGrid::Entries::iterator ie = entries.begin();
      while(ie != entries.end()) {
         const wxString& keyword = ie->keyword;
         const wxString& value   = ie->value;
         const wxString& comment = ie->comment;

         wxFitsKeyword::EFitsKeyword ekw = wxFitsKeyword::lookupKeyword(keyword);
         if(ekw != wxFitsKeyword::EF_UNDEFINED && value.Length() != 0) {
            if(ekw == wxFitsKeyword::EF_PSZX || ekw == wxFitsKeyword::EF_PSZY ) {

			      // these are of double type
               double dval = 0.0;
               if(value.ToDouble(&dval)) kwlist += wxFitsKeyword(ekw,dval,4,comment);
            }
            else {

			      // normal string type
               kwlist += wxFitsKeyword(ekw,value,comment);
            }
         }
         ie++;
      }

      return true;
   }
   return false;
}


wxNamePolicyFile*       wxFactory::getNamePolicyFile()
{
   return &m_policy_file;
}

wxNamePolicyContainer*  wxFactory::getNamePolicyContainer()
{
   return &m_policy_container;
}


void wxFactory::SetFrameCounterFontSize(FrameCounterSize font_size)
{
   m_cntPointSize = font_size;
}

wxFactory::FrameCounterSize wxFactory::FrameCounterFontSize() const
{
   return m_cntPointSize;
}

void wxFactory::SetFrameTimeStampFontSize(FrameTimeStampSize fts_size)
{
   m_ftsSize = fts_size;
}

wxFactory::FrameTimeStampSize wxFactory::FrameTimeStampFontSize() const
{
   return m_ftsSize;
}


//20100703:BM - added new settings
// FrameCounterSize enum used for the FPS indication
void wxFactory::SetFpsFontSize(FrameCounterSize font_size)
{
   m_fpsPointSize = font_size;
}

wxFactory::FrameCounterSize wxFactory::FpsFontSize() const
{
   return m_fpsPointSize;
}

//20100703:BM - added new settings
// defines the polling interval and the max reachable FPS (level is also poll IV millisec here)
void wxFactory::SetPerformanceLevel(PerfPollIntervall font_size)
{
   m_camPollIntervall = font_size;
}

wxFactory::PerfPollIntervall wxFactory::PerformanceLevel() const
{
   return m_camPollIntervall;
}



BatchJobManager* wxFactory::batchJobManager()
{
   if(!m_batchJobManager) m_batchJobManager = new BatchJobManager();
   return m_batchJobManager;
}
