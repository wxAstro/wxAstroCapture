#ifndef WXCONFIGPATH_H
#define WXCONFIGPATH_H

#include <wx/string.h>
#include <wx/config.h>

class wxConfigPath
{
   public:
      wxConfigPath(wxConfig* config,const wxString& newPath);
      virtual ~wxConfigPath();

      wxString OldPath() const;
      wxString GetPath() const;  // the current path
   protected:
   private:
      wxConfig* m_config;
      wxString  m_oldPath;
};



#endif // WXCONFIGPATH_H
