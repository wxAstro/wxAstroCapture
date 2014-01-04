#ifndef FITSHEADER_H
#define FITSHEADER_H

//(*Headers(FitsHeader)
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/grid.h>
//*)

#include "wxcustom/wxPropGrid.h"
#include "wxcustom/wxPropertyPage.h"

class FitsHeader: public wxPropertyPage
{
	public:

		FitsHeader(wxWindow* parent,wxWindowID id = -1);
		virtual ~FitsHeader();

     	void Init();
      bool OnApply();

		//(*Identifiers(FitsHeader)
		static const long ID_GRID1;
		//*)

	protected:

		//(*Handlers(FitsHeader)
		//*)

		//(*Declarations(FitsHeader)
		wxPropGrid* grid;
		wxBoxSizer* BoxSizer1;
		//*)

	private:

		DECLARE_EVENT_TABLE()
};

#endif
