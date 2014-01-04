#ifndef WXGUIDINGINTERFACEPARALLELPORT_H
#define WXGUIDINGINTERFACEPARALLELPORT_H

//(*Headers(wxGuidingInterfaceParallelPort)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/panel.h>
#include <wx/choice.h>
//*)

class wxIoPortCtrl;
#include "wxNoteBookPageEx.h"

class wxGuidingInterfaceParallelPort :  public wxNoteBookPageEx
{
	public:

		wxGuidingInterfaceParallelPort(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~wxGuidingInterfaceParallelPort();

		void Init();

		//InitProtocolChoice fills the protocol choice control with only relevant values
		// and sets the default value to the current setting, if any
		void InitProtocolChoice();

      // similarly, InitPortPinsControl initialises the pin selector
      void InitPortPinsControl();

      /// On apply is called whenever the current page values should be applied
      virtual void OnApply();

		//(*Declarations(wxGuidingInterfaceParallelPort)
		wxIoPortCtrl* GX_RA_neg;
		wxIoPortCtrl* GX_init;
		wxIoPortCtrl* GX_RA_pos;
		wxChoice* ProtocolChoice;
		wxStaticText* StaticTextPinLabels;
		wxIoPortCtrl* GX_DEC_pos;
		wxBoxSizer* BoxSizer1;
		wxIoPortCtrl* GX_DEC_neg;
		wxIoPortCtrl* GX_mask;
		//*)

	protected:

		//(*Identifiers(wxGuidingInterfaceParallelPort)
		static const long ID_STATICTEXT8;
		static const long ID_CHOICE3;
		static const long ID_STATICTEXT10;
		static const long ID_STATICTEXT17;
		static const long ID_STATICTEXT11;
		static const long ID_CUSTOM2;
		static const long ID_STATICTEXT12;
		static const long ID_CUSTOM8;
		static const long ID_STATICTEXT13;
		static const long ID_CUSTOM9;
		static const long ID_STATICTEXT14;
		static const long ID_CUSTOM10;
		static const long ID_STATICTEXT15;
		static const long ID_CUSTOM11;
		static const long ID_STATICTEXT16;
		static const long ID_CUSTOM12;
		//*)

	private:

	   bool ApplyPort();

      unsigned char GXmask() const;       //
      unsigned char GXinit() const;       // init GX
      unsigned char GXRA_pos() const;       // RA+
      unsigned char GXRA_neg() const;       // RA-
      unsigned char GXDEC_pos() const;      // Dec+
      unsigned char GXDEC_neg() const;      // Dec-

		//(*Handlers(wxGuidingInterfaceParallelPort)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
