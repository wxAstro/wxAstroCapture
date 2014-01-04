#ifndef WXPROPERTYPAGE_H
#define WXPROPERTYPAGE_H

#include <wx/panel.h>
class wxPropertyPage : public wxPanel
{
   public:
      wxPropertyPage(): m_init(false) {}
      virtual ~wxPropertyPage() {}

      virtual bool IsInit() {return m_init;}
      virtual void SetInit(bool init) { m_init = init;}
      virtual void Init() = 0;

      // apply the page settings, returns true if changes applied
      virtual bool OnApply() = 0;

      // discard the page settings
      virtual void OnCancel() { } ;
   protected:
      bool m_init;
   private:
};



#endif // WXPROPERTYPAGE_H
