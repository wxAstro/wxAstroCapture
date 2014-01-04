#ifndef WXMAINTOOLBAR_H
#define WXMAINTOOLBAR_H

#include <wx/toolbar.h>
#include "wxWebcam/wxCameraTypes.h"
class wxChoice;
class wxSpinCtrlDbl;
class wxSpinEvent;
class wxStaticText;

class wxMainToolBar : public wxToolBar
{
   public:
      wxMainToolBar(wxWindow* parent,
                    wxWindowID id, const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize,
                    long style = 0,
                    const wxString& name = wxToolBarNameStr);
      virtual ~wxMainToolBar();
      static wxMainToolBar* singleton() { return m_self; }

      void InitToolBar();
      void UpdateToolBar();

      void UpdateObject();
      void UpdateZoom();
      void UpdateType();
      void UpdateTemp();

   protected:

      int InsertObject(int pos);
      int InsertType(int pos);
      int InsertZoom(int pos);
      int InsertBin(int pos);
      int InsertSubframe(int npos);
      int InsertCooling(int npos);
      int InsertRestartExposure(int npos);

      void CamEnable();
      void EnableBin(bool enable);
      void EnableZoom(bool enable);
      void EnableSubframe(bool enable);


   protected:
      static const long ID_OBJECT;
      static const long ID_TYPE_CHOICE;
      static const long ID_ZOOM_SPIN;
      static const long ID_UNZOOM;
      static const long ID_BIN_CHOICE;
      static const long ID_FULLFRAME;
      static const long ID_SET_TEMP;
      static const long ID_WARMUP;
      static const long ID_RESTART;

      void OnBinChoiceSelect(wxCommandEvent& event);
      void OnTypeChoiceSelect(wxCommandEvent& event);
      void OnZoomSpinUpdated(wxSpinEvent& event);
      void OnUnZoom(wxCommandEvent& event);
      void OnFullFrame(wxCommandEvent& event);
      void OnObject(wxCommandEvent& event);
      void OnSetTemp(wxCommandEvent& event);
      void OnWarmUp(wxCommandEvent& event);
      void OnRestartExposure(wxCommandEvent& event);

      DECLARE_EVENT_TABLE()
   private:
      static wxMainToolBar* m_self;

      size_t m_first_dpos;
      wxChoice* m_binChoice;
      wxSpinCtrlDbl* m_zoomSpin;
      wxChoice* m_typeChoice;
      wxStaticText* m_object_text;
      wxStaticText* m_temp_text;

      ECoolingStatus m_cstat;
};



#endif // WXMAINTOOLBAR_H
