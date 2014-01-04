#include "wxNamePolicy.h"
#include "wxFactory.h"
#include "wxWebcam/wxWebcam.h"
#include "wxWebcam/wxArtcam.h"

#include <wx/datetime.h>
#include "wxConfigPath.h"



wxNamePolicy::wxNamePolicy()
: m_dirty(true)
{
   //ctor
}

wxNamePolicy::~wxNamePolicy()
{
   //dtor
}


// returns current computed name
wxString wxNamePolicy::GetName()
{
   if(m_dirty)ComputeName(this);
   return m_name;
}

// commit use of current computed name,
// prepare for computation of next name
void wxNamePolicy::CommitName()
{
   m_dirty = true;
}

void wxNamePolicy::SetDirty(bool dirty)
{
   m_dirty = dirty;
}

// Set the name template used when computing names
void wxNamePolicy::SetNameTemplate(const wxArrayString& name_template)
{
   m_name_template = name_template;
   m_dirty = true;
}

const wxArrayString& wxNamePolicy::GetNameTemplate()
{
   return m_name_template;
}

void wxNamePolicy::SetAvailableTemplate(const wxArrayString& avail_template)
{
   m_avail_template = avail_template;
}

const wxArrayString& wxNamePolicy::GetAvailableTemplate()
{
   return m_avail_template;
}

bool wxNamePolicy::ReplaceTemplate(wxString& item_text)
{
   bool replaced=false;

   // see if it can be replaced with something more explicit
   if(item_text == _("prefix")) {
      item_text = wxF()->capturePrefix();
      replaced  = true;
   }
   else if(item_text == _("object")) {
      wxString rep;
      if(wxF()->getFitsHeaderValue(_("OBJECT"),rep)) {
         item_text = rep;
         replaced  = true;
      }
   }
   else if(item_text == _("binning")) {

      unsigned short binning = 1;
      if(wxF()->artcam()) binning =  wxArtcam::DefaultBinning();
      item_text = wxString::Format(_T("%dx%d"),binning,binning);
      replaced  = true;
   }
   else if(item_text == _("filter")) {
      wxString rep;
      if(wxF()->getFitsHeaderValue(_("FILTER"),rep)) {
         item_text = rep;
         replaced  = true;
      }
   }
   else if(item_text == _("imagetyp")) {
      wxString rep;
      if(wxF()->getFitsHeaderValue(_("IMAGETYP"),rep)) {
         item_text = rep;
         replaced  = true;
      }
   }
   else if(item_text == _("timestamp")) {

      wxDateTime now = wxDateTime::Now().ToUTC();
      item_text = wxString::Format(wxT("%.2d%.2d%.2d_%.2d%.2d_%.2d"),(int)now.GetYear(),(int)(now.GetMonth()+1),(int)now.GetDay(),
                                                                     (int)now.GetHour(),(int)now.GetMinute(),(int)now.GetSecond());
      replaced  = true;
   }
   else if(item_text == _("timestamp_msec")) {

      wxDateTime now = wxDateTime::UNow().ToUTC();
      wxDateTime::wxDateTime_t ms = now.GetMillisecond();
      item_text = wxString::Format(wxT("%.2d%.2d%.2d_%.2d%.2d_%.2d.%.3d"),(int)now.GetYear(),(int)(now.GetMonth()+1),(int)now.GetDay(),
                                                                     (int)now.GetHour(),(int)now.GetMinute(),(int)now.GetSecond(), ms);
      replaced  = true;
   }
   return replaced;
}

// Based on current settings, compute the next name
void wxNamePolicy::ComputeName(wxNamePolicy* policy)
{
   wxString name = _T("");
   size_t nitems = m_name_template.GetCount();
   for(size_t item =0; item<nitems; item++) {

      // get the current item text
      wxString item_text = m_name_template[item];

      if(item > 0) name += _T("_");
      if(policy->ReplaceTemplate(item_text)) name += item_text;
      else name += _T("%") + item_text + _T("%");
   }

   m_name = name;
   m_dirty = false;
}

void wxNamePolicy::SetPreviewMode(bool WXUNUSED(on))
{
   // dummy
}

void wxNamePolicy::SetAutoResetIncrement(bool WXUNUSED(reset))
{
   // dummy
}

wxString wxNamePolicy::GetTemplateString(const wxArrayString& name_template)
{
   wxString template_string = _T("");
   size_t nitems = name_template.GetCount();
   for(size_t item =0; item<nitems; item++) {

      // get the current item text
      wxString item_text = name_template[item];

      if(item > 0) template_string += _T("_");
      template_string += _T("%") + item_text + _T("%");
   }
   return template_string;
}

bool wxNamePolicy::GetTemplateArray(const wxString& tstring, wxArrayString& name_template)
{
   size_t i = 0;
   while( i<tstring.Length() ) {
      if(_T('%') == tstring[i++]) {
         wxString item = _T("");
         if(i >= tstring.Length())return false;
         wxChar ch = tstring[i++];
         while(_T('%') != ch ) {
            item += ch;
            if(i >= tstring.Length())return false;
            ch = tstring[i++];
         }
         if(item.Length() > 0)name_template.Add(item);
         item = _T("");
      }
   }

   return true;
}

bool wxNamePolicy::SaveToConfig(wxConfig* config)
{
   // do not set config path here,
   // it is done by derived class before calling this function

   config->Write(_T("ComputedName"), m_name);
   config->Write(_T("NameTemplate"), GetTemplateString(m_name_template));
   config->Write(_T("NameTemplateAvailable"), GetTemplateString(m_avail_template));

   return true;
}

bool wxNamePolicy::RestoreFromConfig(wxConfig* config)
{
   // do not set config path here,
   // it is done by derived class before calling this function

   config->Read(_T("ComputedName"), &m_name,_T(""));

   // first, take copies of the default settings,
   // so we can add items that may have been introduced in a version
   // later than the config entry

   this->SetDefaultPolicy();

   // keep all templates in one array
   wxArrayString all_templates  = m_name_template;
   for(size_t it=0;it< m_avail_template.Count();it++) all_templates.Add(m_avail_template[it]);

   // then restore from config
   wxString tstring;
   if(config->Read(_T("NameTemplate"),&tstring,_T(""))) {
      m_name_template.Clear();
      if(!GetTemplateArray(tstring,m_name_template)) this->SetDefaultPolicy();
   }

   tstring.Clear();
   if(config->Read(_T("NameTemplateAvailable"),&tstring,_T(""))) {
      m_avail_template.Clear();
      if(!GetTemplateArray(tstring,m_avail_template)) this->SetDefaultPolicy();
   }

   // at this stage there might be some entries in "all_templates"
   // that are not in m_name_template or m_avail_template
   for(size_t it=0;it<m_name_template.Count();it++) all_templates.Remove(m_name_template[it]);
   for(size_t it=0;it<m_avail_template.Count();it++) all_templates.Remove(m_avail_template[it]);

   // add anything remaining to the available ones
   for(size_t it=0;it<all_templates.Count();it++) m_avail_template.Add(all_templates[it]);

   m_dirty = true;

   return true;
}


bool wxNamePolicy::IsActiveItem(const wxString& itemtext)
{
   size_t nitems = m_name_template.GetCount();
   for(size_t item =0; item<nitems; item++) {
      // compare with the current item text
      if(itemtext == m_name_template[item])return true;
   }
   return false;
}



wxNamePolicyFile::wxNamePolicyFile()
: m_increment(1)
, m_preview(false)
, m_autoReset(false)
{
  SetDefaultPolicy();
}

wxNamePolicyFile::~wxNamePolicyFile()
{}

void wxNamePolicyFile::SetDefaultPolicy()
{
   m_increment = 1;
   m_preview   = false;
   m_autoReset = false;

   wxArrayString templates;
   templates.Add(_("object"));
   templates.Add(_("increment"));
   SetNameTemplate(templates);

   wxArrayString avail;
   avail.Add(_("prefix"));
   avail.Add(_("imagetyp"));
   avail.Add(_("binning"));
   avail.Add(_("filter"));
   avail.Add(_("timestamp"));
   avail.Add(_("timestamp_msec"));
   SetAvailableTemplate(avail);
}

void wxNamePolicyFile::SetPreviewMode(bool on)
{
   m_preview = on;
}

bool wxNamePolicyFile::ReplaceTemplate(wxString& item_text)
{
   if(wxNamePolicy::ReplaceTemplate(item_text)) return true;

   if(item_text == _("increment")) {
      item_text = wxString::Format(wxT("%.4d"),m_increment++);
      if(m_preview) --m_increment;
      return true;
   }
   return false;
}

size_t wxNamePolicyFile::GetIncrement()
{
   return m_increment;
}

void wxNamePolicyFile::SetIncrement(size_t incr)
{
   m_increment = incr;
   SetDirty(true);
}

void wxNamePolicyFile::SetAutoResetIncrement(bool reset)
{
   m_autoReset = reset;
}

bool wxNamePolicyFile::GetAutoResetIncrement()
{
   return m_autoReset;
}

bool wxNamePolicyFile::SaveToConfig(wxConfig* config)
{
   wxConfigPath path(config,wxT("NamePolicyFile"));

   // write base class stuff
   wxNamePolicy::SaveToConfig(config);

   //Then write local additions
   config->Write(_T("Increment"),(int)m_increment);
   config->Write(_T("AutoResetIncrement"),m_autoReset);

   return true;
}

bool wxNamePolicyFile::RestoreFromConfig(wxConfig* config)
{
   wxConfigPath path(config,wxT("NamePolicyFile"));

   // read base class stuff
   wxNamePolicy::RestoreFromConfig(config);

   //Then read local additions
   int incr=0;
   config->Read(_T("Increment"),&incr,0); m_increment = incr;
   config->Read(_T("AutoResetIncrement"),&m_autoReset,false);

   return true;
}

wxNamePolicyContainer::wxNamePolicyContainer()
{
   SetDefaultPolicy();
}

wxNamePolicyContainer::~wxNamePolicyContainer()
{
}

void wxNamePolicyContainer::SetDefaultPolicy()
{
   wxArrayString templates;
   templates.Add(_("object"));
   templates.Add(_("timestamp"));
   SetNameTemplate(templates);

   wxArrayString avail;
   avail.Add(_("prefix"));
   avail.Add(_("imagetyp"));
   avail.Add(_("binning"));
   avail.Add(_("filter"));
   avail.Add(_("timestamp_msec"));
   SetAvailableTemplate(avail);
}

bool wxNamePolicy::IsMandatoryItem(const wxString& WXUNUSED(item))
{
   return false;
}


int wxNamePolicy::HasItem(const wxString& item)
{
   int num = 0;
   for(size_t i=0; i<m_name_template.Count();i++) {
      wxString entry = m_name_template[i];
      if(entry.Contains(item))num++;
   }
   return num;
}

bool wxNamePolicyFile::IsMandatoryItem(const wxString& item)
{
   if(item == wxT("increment")) {
      // mandatory unless timestamp is used
      // side-effect: on screen increment does not work when this is done
      // if(HasTimestamp() > 0)return false;

      // no side-effect
      return true;
   }
   else if(item.Contains( wxT("timestamp"))) {
      // mandatory unless increment or other timestamp is used
      if(HasItem(wxT("increment")) > 0)return false;
      if(HasItem(wxT("timestamp")) > 1)return false;
      return true;
   }

   return false;
}

bool wxNamePolicyContainer::IsMandatoryItem(const wxString& item)
{
   if(item.Contains(wxT("timestamp"))) {
      // mandatory unless other timestamp is used
      if(HasItem(wxT("timestamp")) > 1)return false;
      return true;
   }

   return false;
}

bool wxNamePolicyContainer::SaveToConfig(wxConfig* config)
{
   wxConfigPath path(config,wxT("NamePolicyContainer"));
   return wxNamePolicy::SaveToConfig(config);
}

bool wxNamePolicyContainer::RestoreFromConfig(wxConfig* config)
{
   wxConfigPath path(config,wxT("NamePolicyContainer"));
   return wxNamePolicy::RestoreFromConfig(config);
}
