#include "ImageHeaderDialog.h"
#include "FitsHeader.h"
#include "wx/bookctrl.h"

ImageHeaderDialog::ImageHeaderDialog(wxWindow* parent)
{
   //ctor
    Create(parent, wxID_ANY, _("Image Header"), wxDefaultPosition, wxDefaultSize,
        wxDEFAULT_DIALOG_STYLE|wxPROPSHEET_SHRINKTOFIT |wxRESIZE_BORDER
    );

    FitsHeader*           fh = new FitsHeader(GetBookCtrl());

    m_pages.push_back(fh );

    GetBookCtrl()->AddPage(fh, wxT("FITS Header (Manual)"));
    CreateButtons(wxOK|wxCANCEL);

    LayoutDialog();


}

ImageHeaderDialog::~ImageHeaderDialog()
{
   //dtor
}


