#ifndef WXGUIDINGCONFIG_H
#define WXGUIDINGCONFIG_H

#include <wx/string.h>
#include "wxGuider.h"
/*
   A helper class for storing/retrieving configuration info
   related to guiding
*/

class wxGuider;

class wxGuidingConfig
{
   public:
      wxGuidingConfig();
      virtual ~wxGuidingConfig();

      static bool getGuidingConfig(int& guideBoxSize,    // size in pixels
                                   int& bacgroundCutoff, // [0,255]
                                   int& guideInterval,   // [ms]
                                   double& imageScale,   // "/pixel
                                   double& cameraAngle); // [d] counterclockwise

      static bool saveGuidingConfig(int guideBoxSize,
                                   int bacgroundCutoff,
                                   int guideInterval, // [ms]
                                   double imageScale,
                                   double cameraAngle);

      enum DECguiding { NoDecGuiding=0, NorthAndSouth, NorthOnly, SouthOnly};
      static bool saveGuidingOptions(bool reverseRA,      // If RA shall be reversed
                                     bool reverseDEC,     // If RA shall be reversed
                                     DECguiding dec);     // DEC

      static bool getGuidingOptions(bool& reverseRA,
                                    bool& reverseDEC,
                                    DECguiding& dec);

      // getCachedGuidingOptions returns cached values from last call to getGuidingOptions
      static bool getCachedGuidingOptions(bool& reverseRA,
                                          bool& reverseDEC,
                                          DECguiding& dec);

      static wxString getFileInterfacePath();
      static bool saveFileInterfacePath(const wxString& path);

      // retrieve saved interface type
      static wxGuider::guider_interface getGuiderInterface();

      // saves current guide interface
      static bool saveGuiderInterface();

      static bool saveBridgeParameters(const wxString& host, short port);
      static bool getBridgeParameters(wxString& host,short& port);

      static bool saveGuiderTuningParams(wxGuider* guider);
      static bool restoreGuiderTuningParams(wxGuider* guider);

   protected:
   private:
      static bool       m_cache_init;
      static bool       m_cached_reverseRA;
      static bool       m_cached_reverseDEC;
      static DECguiding m_cached_dec;
};



#endif // WXGUIDINGCONFIG_H
