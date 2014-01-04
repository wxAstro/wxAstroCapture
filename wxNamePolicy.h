#ifndef WXNAMEPOLICY_H
#define WXNAMEPOLICY_H

#include <wx/string.h>
#include <wx/arrstr.h>

#include <wx/config.h>

class wxNamePolicy {
public:
   wxNamePolicy();
   virtual ~wxNamePolicy();

   // returns current computed name
   virtual wxString GetName();

   // commit use of current computed name,
   // prepare for computation of next name
   virtual void CommitName();

   // Set the name template used when computing names
   /*
       prefix         : file name prefix
       object         : Fits OBJECT value
       binning        : Current camera binning
       filter         : Fits FILTER value
       imagetyp       : Fits IMAGETYP value
       timestamp      : Current time (UTC) YYYYMMDD_HHMM_SS      (at end of exposure)
       timestamp_msec : Current time (UTC) YYYYMMDD_HHMM_SS.sss  (at end of exposure)
       increment      : Current file increment
   */

   // Defines the template to be used for computing name
   void SetNameTemplate(const wxArrayString& name_template);
   const wxArrayString& GetNameTemplate();

   // defines unused, but available templates
   void SetAvailableTemplate(const wxArrayString& name_template);
   const wxArrayString& GetAvailableTemplate();

   // true if a template item such as e.g. "increment" is mandatory in current context
   virtual bool IsMandatoryItem(const wxString& item);

   // return the number of entries containing "item" substring
   virtual int  HasItem(const wxString& item);

   // true if a template item is currently in use
   virtual bool IsActiveItem(const wxString& item);

   // true when calculating names in preview mode, will keep increment constant
   virtual void SetPreviewMode(bool on);

   // flag to indicate increment is to be reset to 1 at start of each capture
   virtual void SetAutoResetIncrement(bool reset);

   // save and restore of name policy info to config
   virtual bool SaveToConfig(wxConfig* config) = 0;
   virtual bool RestoreFromConfig(wxConfig* config) = 0;

protected:
   // helper function: concatenate template array into one string
   wxString GetTemplateString(const wxArrayString& name_template);
   bool GetTemplateArray(const wxString& tstring, wxArrayString& name_template);
   virtual  void SetDefaultPolicy() = 0;

   // Based on current settings, compute the next name
   void ComputeName(wxNamePolicy* policy);

   virtual bool ReplaceTemplate(wxString& item_text);
   void SetDirty(bool isdirty);

private:
   bool          m_dirty;          // true when m_name must be recomputed
   wxString      m_name;           // Holds computed name
   wxArrayString m_name_template;  // Holds templates for computing name
   wxArrayString m_avail_template; // Holds available, but unused templates for computing name
};

class wxNamePolicyFile : public wxNamePolicy {
public:
   wxNamePolicyFile();
   virtual ~wxNamePolicyFile();

   virtual bool ReplaceTemplate(wxString& item_text);
   virtual bool IsMandatoryItem(const wxString& item);

   void SetDefaultPolicy();

   void SetPreviewMode(bool on);

   size_t GetIncrement();
   void   SetIncrement(size_t incr);
   bool   GetAutoResetIncrement();
   void   SetAutoResetIncrement(bool reset);

   bool SaveToConfig(wxConfig* config);
   bool RestoreFromConfig(wxConfig* config);
private:
   size_t m_increment;
   bool   m_preview;
   bool   m_autoReset;
};

class wxNamePolicyContainer : public wxNamePolicy {
public:
   wxNamePolicyContainer();
   virtual ~wxNamePolicyContainer();
   virtual bool IsMandatoryItem(const wxString& item);

   void SetDefaultPolicy();

   bool SaveToConfig(wxConfig* config);
   bool RestoreFromConfig(wxConfig* config);
};

#endif // WXNAMEPOLICY_H
