#ifndef WXFACTORY_H
#define WXFACTORY_H


/*
   The Factory is a singleton object that provides access to
   some common resources in the program.

   The Factory shall also contain "virtual constructors" for
   os-dependent ports and similar. When this is done, an
   abstract interface is returned to the application, which
   does not need to be aware of the OS differences.
*/

#include "wx/wxprec.h"
#include <vector>

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/config.h>
#include "wxGuider.h"
#include "wxcustom/wxPropGrid.h"
#include "wxNamePolicy.h"
#include "wxWebcam/wxCameraTypes.h"
#include "wxIO/wxPort.h"

class wxCamera;
class wxWebcam;
class wxArtcam;
class wxFitsKeywordList;
class wxGuider;
class wxLongExpoPort;
class wxGuiderPort;
class wxPortManager;
class BatchJobManager;


class wxFactory {
public:
   static wxFactory* singleton();

   wxFactory();
   virtual ~wxFactory();

//   void ConstructLEport();
   void Init();

   typedef std::vector<ECType> CameraTypeVector;

   // enumerate any camera on the system
   size_t EnumerateCameras(wxWindow* parent,wxArrayString& cameras, CameraTypeVector& ctypes);

   void setCamera(wxCamera* camera);
   // the main program sets the webcam object after creation ow window
   void setWebcam(wxWebcam* webcam);
   // the main program sets the webcam object after creation ow window
   void setArtcam(wxArtcam* artcam);

   // returns the current generic cam
   wxCamera*  cam();
   // returns the current webcam
   wxWebcam*  webcam();
   // returns the current Artemis camera
   wxArtcam*  artcam();

   // returns the config object for saving/restoring between sessions
   wxConfig*  config();

   // returns the portmanager
   wxPortManager* portManager();

   // returns camera long exposure port
   wxLongExpoPort*   lePort();

   // close and delete the LE port object
   void lePortRelease(bool force_release = false);

   // returns guide port, may be the same as camera long exposure port in some cases
   wxGuiderPort*     guidePort();

   // set or change guider interface
   bool setGuiderInterface(wxGuider::guider_interface intfc);

   // return abstract guider interface
   wxGuider*  guider();

   // the current image capture folder in the file system
   wxString   captureFolder();
   wxString   capturePrefix();
   bool       setCaptureFolder(const wxString& folder, const wxString& prefix);

   // the selected video format
   EVideoSampleFormat sampleFormat();

   // the selected container format
   EContainerFormat containerFormat();

   // save/retrieve the user defined FITS header entries from configuration
   bool saveFitsHeaderEntries(wxPropGrid::Entries& user_fits);
   bool getFitsHeaderEntries(wxPropGrid::Entries& user_fits);
   bool getFitsKeywordList(wxFitsKeywordList& kwlist);
   bool getFitsHeaderValue(const wxString& kw, wxString& value);
   bool saveFitsHeaderValue(const wxString& kw, wxString& value);

   wxNamePolicyFile*       getNamePolicyFile();
   wxNamePolicyContainer*  getNamePolicyContainer();

   // control of graphical frame counter font size
   enum FrameCounterSize { CNT_OFF=0, CNT_SMALL=14, CNT_MEDIUM=28, CNT_LARGE=42, CNT_HUGE=56 };
   virtual void SetFrameCounterFontSize(FrameCounterSize font_size);
   virtual FrameCounterSize FrameCounterFontSize() const;

   // control of graphical frame time stamp visibility/font size
   enum FrameTimeStampSize { FTS_OFF=0, FTS_SMALL=7, FTS_MEDIUM=10 };
   virtual void SetFrameTimeStampFontSize(FrameTimeStampSize fts_size);
   virtual FrameTimeStampSize FrameTimeStampFontSize() const;

   //20100703:BM - added new settings
   // FrameCounterSize enum used for the FPS indication
   virtual void SetFpsFontSize(FrameCounterSize font_size);
   virtual FrameCounterSize FpsFontSize() const;

   // defines the polling interval and the max reachable FPS (level is also poll IV millisec here)
   enum PerfPollIntervall { PPL_LOW=66, PPL_MID=33, PPL_HIGH=6 };
   virtual void SetPerformanceLevel(PerfPollIntervall perf_level);
   virtual PerfPollIntervall PerformanceLevel() const;


   // return manager for use with back captures
   BatchJobManager* batchJobManager();

private:
   bool restoreFromConfig();
//   bool findCamPortIndex(const wxString& port_name, size_t& index);

private:
   wxConfig         m_config;

   // There is only one camera at any single time!
   wxCamera*        m_camera;

   wxPortManager*   m_portManager;
   wxLongExpoPort*  m_lePort;
   wxGuiderPort*    m_guidePort;

   wxPort::PortEntryVector m_port_entries; // the available system IO ports
   int     m_index_port_entry;          // vector index of currently selected port

   wxString         m_capture_folder; // cached
   wxString         m_capture_prefix; // cached
   wxGuider*        m_guider;

   wxNamePolicyFile      m_policy_file;
   wxNamePolicyContainer m_policy_container;

   FrameCounterSize m_cntPointSize;
   FrameTimeStampSize m_ftsSize;
   //20100703:BM - added new settings
   FrameCounterSize m_fpsPointSize;
   PerfPollIntervall m_camPollIntervall;

   BatchJobManager* m_batchJobManager;

private:
   static wxFactory* m_self; // singleton object
};

inline wxFactory* wxF() { return wxFactory::singleton(); }


#endif // WXFACTORY_H
