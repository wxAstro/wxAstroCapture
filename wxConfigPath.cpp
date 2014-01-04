#include "wxConfigPath.h"


wxConfigPath::wxConfigPath(wxConfig* config,const wxString& newPath)
: m_config(config)
{
   m_oldPath = m_config->GetPath();
   m_config->SetPath(newPath);
}

wxConfigPath::~wxConfigPath()
{
   m_config->SetPath(m_oldPath);
}


wxString wxConfigPath::OldPath() const
{
   return m_oldPath;
}

wxString wxConfigPath::GetPath() const
{
   return m_config->GetPath();
}
