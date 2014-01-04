#ifndef SETTINGSNAMEPOLICY_H
#define SETTINGSNAMEPOLICY_H

//(*Headers(SettingsNamePolicy)
#include <wx/panel.h>
class wxCheckBox;
class wxTextCtrl;
class wxListBox;
class wxStaticText;
class wxBoxSizer;
class wxButton;
//*)

class wxNamePolicy;

#include "wxcustom/wxPropertyPage.h"

class SettingsNamePolicy : public wxPropertyPage
{
	public:

		SettingsNamePolicy(wxWindow* parent,wxWindowID id=wxID_ANY);
		virtual ~SettingsNamePolicy();

		virtual void Init();

      // apply or cancel the name policy setting
		virtual bool OnApply();
      virtual void OnCancel();

		virtual wxNamePolicy* policy() = 0;

	//	virtual bool IsMandatoryComponent(const wxString& label) = 0;

	   virtual void  RefreshNamePreview();
	//	wxString RecomputeName();


		//(*Declarations(SettingsNamePolicy)
		wxButton* UpButton;
		wxListBox* m_list_available;
		wxCheckBox* m_auto_reset_checkbox;
		wxTextCtrl* m_name_preview;
		wxButton* DownButton;
		wxListBox* m_list_used;
		//*)

	protected:

		//(*Identifiers(SettingsNamePolicy)
		static const long ID_STATICTEXT1;
		static const long ID_STATICTEXT2;
		static const long ID_LISTBOX1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		static const long ID_STATICTEXT3;
		static const long ID_LISTBOX2;
		static const long ID_BUTTON3;
		static const long ID_BUTTON4;
		static const long ID_CHECKBOX1;
		static const long ID_STATICTEXT4;
		static const long ID_TEXTCTRL2;
		//*)

	private:

		//(*Handlers(SettingsNamePolicy)
		void OnAddButtonClick(wxCommandEvent& event);
		void OnRemoveButtonClick(wxCommandEvent& event);
		void OnUpButtonClick(wxCommandEvent& event);
		void OnDownButtonClick(wxCommandEvent& event);
		void OnAutoResetCheckboxClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
   bool m_show_increment_checkbox;
};

/// Specialisations for container and file name policies follow

class SettingsNamePolicyContainer : public SettingsNamePolicy
{
	public:
		SettingsNamePolicyContainer(wxWindow* parent,wxWindowID id=wxID_ANY)
		: SettingsNamePolicy(parent,id) { Init(); }
		virtual ~SettingsNamePolicyContainer() {}

		wxNamePolicy* policy();
};

class SettingsNamePolicyFile : public SettingsNamePolicy
{
	public:
		SettingsNamePolicyFile(wxWindow* parent,wxWindowID id=wxID_ANY)
		: SettingsNamePolicy(parent,id) { Init(); }
		void Init();
		virtual ~SettingsNamePolicyFile() {}

      wxNamePolicy* policy();
};

#endif
