#ifndef SETTINGSGUIDEPORTPARALLEL_H
#define SETTINGSGUIDEPORTPARALLEL_H

//(*Headers(SettingsGuidePortParallel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/panel.h>
//*)
#include "wxcustom/wxIoPortCtrl.h"

class SettingsGuidePortParallel: public wxPanel
{
	public:

		SettingsGuidePortParallel(wxWindow* parent,wxWindowID id = -1);
		virtual ~SettingsGuidePortParallel();

		void Init();

      unsigned char GXmask() const;       //
      unsigned char GXinit() const;       // init GX
      unsigned char GXRA_pos() const;       // RA+
      unsigned char GXRA_neg() const;       // RA-
      unsigned char GXDEC_pos() const;      // Dec+
      unsigned char GXDEC_neg() const;      // Dec-


		//(*Identifiers(SettingsGuidePortParallel)
		static const long ID_STATICTEXT1;
		static const long ID_STATICTEXT7;
		static const long ID_STATICTEXT6;
		static const long ID_CUSTOM3;
		static const long ID_STATICTEXT3;
		static const long ID_CUSTOM4;
		static const long ID_STATICTEXT4;
		static const long ID_CUSTOM5;
		static const long ID_STATICTEXT2;
		static const long ID_CUSTOM6;
		static const long ID_STATICTEXT9;
		static const long ID_CUSTOM7;
		static const long ID_STATICTEXT5;
		static const long ID_CUSTOM1;
		//*)

	protected:

		//(*Handlers(SettingsGuidePortParallel)
		//*)

		//(*Declarations(SettingsGuidePortParallel)
		wxIoPortCtrl* GX_RA_neg;
		wxIoPortCtrl* GX_init;
		wxIoPortCtrl* GX_RA_pos;
		wxIoPortCtrl* GX_DEC_pos;
		wxStaticText* StaticText7;
		wxIoPortCtrl* GX_DEC_neg;
		wxFlexGridSizer* FlexGridSizer1;
		wxIoPortCtrl* GX_mask;
		//*)

	private:

		DECLARE_EVENT_TABLE()
};

#endif
