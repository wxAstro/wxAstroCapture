#include "wxGuidingConfig.h"
#include "wxFactory.h"
#include "wxConfigPath.h"
#include "wxGuider.h"


bool       wxGuidingConfig::m_cache_init = false;
bool       wxGuidingConfig::m_cached_reverseRA = false;
bool       wxGuidingConfig::m_cached_reverseDEC = false;
wxGuidingConfig::DECguiding wxGuidingConfig::m_cached_dec;


wxGuidingConfig::wxGuidingConfig()
{
   //ctor
}

wxGuidingConfig::~wxGuidingConfig()
{
   //dtor
}


bool wxGuidingConfig::getGuidingConfig(int& guideBoxSize,     // size in pixels
                                       int& bacgroundCutoff,  // [0,255]
                                       int& guideInterval,    // [ms]
                                       double& imageScale,    // "/pixel
                                       double& cameraAngle)   // [d] counterclockwise
{
   if(wxConfig* config = wxF()->config()) {
      wxConfigPath path(config,wxT("Guiding"));

      config->Read(_T("GuideBoxSize"),&guideBoxSize,100);
      config->Read(_T("BackgroundCutoff"),&bacgroundCutoff,60);
      config->Read(_T("GuideInterval"),&guideInterval,500);
      config->Read(_T("ImageScale"),&imageScale,1.0);
      config->Read(_T("CameraAngle"),&cameraAngle,0.0);
      return true;
   }
   return false;
}

bool wxGuidingConfig::saveGuidingConfig(int guideBoxSize,
                                        int bacgroundCutoff,
                                        int guideInterval,    // [ms]
                                        double imageScale,
                                        double cameraAngle)
{
   if(wxConfig* config = wxF()->config()) {
      wxConfigPath path(config,wxT("Guiding"));

      config->Write(_T("GuideBoxSize"),guideBoxSize);
      config->Write(_T("BackgroundCutoff"),bacgroundCutoff);
      config->Write(_T("GuideInterval"),guideInterval);
      config->Write(_T("ImageScale"),imageScale);
      config->Write(_T("CameraAngle"),cameraAngle);
      return true;
   }
   return false;
}

bool wxGuidingConfig::saveGuidingOptions(bool reverseRA,      // If RA shall be reversed
                                         bool reverseDEC,     // If RA shall be reversed
                                         DECguiding dec)      // DEC
{
   if(wxConfig* config = wxF()->config()) {
      wxConfigPath path(config,wxT("Guiding"));
      config->Write(_T("ReverseRA"),reverseRA);
      config->Write(_T("ReverseDEC"),reverseDEC);
      config->Write(_T("DECguiding"),dec);

      m_cached_reverseRA  = reverseRA;
      m_cached_reverseDEC = reverseDEC;
      m_cached_dec        = dec;
      m_cache_init = true;
      return true;
   }
   return false;
}

bool wxGuidingConfig::getGuidingOptions(bool& reverseRA,
                                        bool& reverseDEC,
                                        DECguiding& dec)
{
   if(wxConfig* config = wxF()->config()) {
      wxConfigPath path(config,wxT("Guiding"));
      config->Read(_T("ReverseRA"),&reverseRA,false);
      config->Read(_T("ReverseDEC"),&reverseDEC,false);
      long decLocal=0;
      config->Read(_T("DECguiding"),&decLocal,0);
      dec = (DECguiding)decLocal;
      m_cache_init = false;
      return true;
   }
   return false;
}

bool wxGuidingConfig::getCachedGuidingOptions(bool& reverseRA,
                                              bool& reverseDEC,
                                              DECguiding& dec)
{
    if(!m_cache_init) {
       if(getGuidingOptions(m_cached_reverseRA,m_cached_reverseDEC,m_cached_dec)) {
          m_cache_init = true;
       }
    }

    if(m_cache_init) {
       reverseRA  = m_cached_reverseRA;
       reverseDEC = m_cached_reverseDEC;
       dec        = m_cached_dec;
       return true;
    }
    return false;
}

wxString wxGuidingConfig::getFileInterfacePath()
{
   if(wxConfig* config = wxF()->config()) {
      wxConfigPath path(config,wxT("Guiding"));
      wxString fipath;
      config->Read(_T("FileInterfacePath"),&fipath,_T(""));
      return fipath;
   }
   return _T("");
}

bool wxGuidingConfig::saveFileInterfacePath(const wxString& fipath)
{
   if(wxConfig* config = wxF()->config()) {
      wxConfigPath path(config,wxT("Guiding"));
      config->Write(_T("FileInterfacePath"),fipath);
      return true;
   }
   return false;
}

wxGuider::guider_interface wxGuidingConfig::getGuiderInterface()
{
   if(wxConfig* config = wxF()->config()) {
      wxConfigPath path(config,wxT("Guiding"));
      long if_type;
      config->Read(_T("GuiderInterfaceType"),&if_type,wxGuider::FILE_INTERFACE);
      return wxGuider::guider_interface(if_type);
   }
   return wxGuider::FILE_INTERFACE;
}

bool wxGuidingConfig::saveGuiderInterface()
{
   if(wxConfig* config = wxF()->config()) {
      wxConfigPath path(config,wxT("Guiding"));
      long if_type = wxF()->guider()->interface_type();
      config->Write(_T("GuiderInterfaceType"),if_type);
      return true;
   }
   return false;
}


bool wxGuidingConfig::saveBridgeParameters(const wxString& host,short port)
{
   if(wxConfig* config = wxF()->config()) {
      wxConfigPath path(config,wxT("Guiding/Bridge"));
      config->Write(_T("host"),host);
      config->Write(_T("port"),(long)port);
      return true;
   }
   return false;
}

bool wxGuidingConfig::getBridgeParameters(wxString& host,short& port)
{
   if(wxConfig* config = wxF()->config()) {
      wxConfigPath path(config,wxT("Guiding/Bridge"));
      config->Read(_T("host"),&host,_T("localhost"));

      long lport;
      config->Read(_T("port"),&lport,5618);
      port = lport;
      return true;
   }
   return false;
 }


bool wxGuidingConfig::saveGuiderTuningParams(wxGuider* guider)
{
   if(wxConfig* config = wxF()->config()) {
      wxConfigPath path(config,wxT("Guiding"));
      config->Write(_T("RA_DZ"),guider->raDz());
      config->Write(_T("RA_K"),guider->raK());
      config->Write(_T("RA_Q"),guider->raQ());

      config->Write(_T("DEC_DZ"),guider->decDz());
      config->Write(_T("DEC_K"),guider->decK());
      config->Write(_T("DEC_Q"),guider->decQ());
      return true;
   }
   return false;
}

bool  wxGuidingConfig::restoreGuiderTuningParams(wxGuider* guider)
{
   if(wxConfig* config = wxF()->config()) {
      wxConfigPath path(config,wxT("Guiding"));

      double ra_dz = guider->raDz();
      double ra_k  = guider->raK();
      double ra_q  = guider->raQ();

      config->Read(_T("RA_DZ"),&ra_dz,ra_dz);
      config->Read(_T("RA_K"),&ra_k,ra_k);
      config->Read(_T("RA_Q"),&ra_q,ra_q);

      guider->setRaDz(ra_dz);
      guider->setRaK(ra_k);
      guider->setRaQ(ra_q);


      double dec_dz = guider->decDz();
      double dec_k  = guider->decK();
      double dec_q  = guider->decQ();

      config->Read(_T("DEC_DZ"),&dec_dz,dec_dz);
      config->Read(_T("DEC_K"),&dec_k,dec_k);
      config->Read(_T("DEC_Q"),&dec_q,dec_q);

      guider->setDecDz(dec_dz);
      guider->setDecK(dec_k);
      guider->setDecQ(dec_q);

      return true;
   }
   return false;
}
