/**
 * @file wxPushButton.cpp
 *****************************************************************************
 * wxPushButton
 *
 * Implementation of a wxWidgets pushbutton
 *
 *<hr>
 *
 * @b Project	wxScopeServer Demo Project<br>
 *
 * @author     C.A. Arnholm
 * @date       05-Dec-2007
 * Copyright	2007 - C.A. Arnholm<br>
 * rev:			1.0<br>
 *
 * This is copyrighted material - NOT freeware NOR GPL NOR anything else !!!!!
 *****************************************************************************
 *<hr>
 * @b Updates
 *
 *****************************************************************************
 *<hr>
 * License:
 * You are allowed to use the files included in the wxScopeServer Project for
 * any NON commercial (NON shareware etc.) purposes only.
 * If you want to use it for any other purpose you have to obtain a proper license
 * - just call <ca _at_ arnholm _dot_ org>
 *
 * You may distribute the files included in the wxScopeServer Demo project included
 * in projects that you have made yourself providing the above paragraph is respected
 * and the copyright and limited warranty is left included in the source files.
 * Please make sure using an appropriate installer to maintain your customers
 * PC in a healthy condition - thanks
 *<br>
 *
 * No warranties whatsoever
 *<br>
 * LIMITED WARRANTY AND DISCLAIMER OF WARRANTY
 * THE Copyright Holder EXPRESSLY DISCLAIMS ANY WARRANTY FOR THE SOFTWARE. THIS SOFTWARE
 * AND THE ACCOMPANYING FILES ARE GIVEN FOR FREE "AS IS" AND WITHOUT WARRANTIES AS TO
 * PERFORMANCE OF MERCHANTABILITY OR ANY OTHER WARRANTIES WHETHER EXPRESSED OR
 * IMPLIED, OR NONINFRINGEMENT.
 *<br>
 * THIS SOFTWARE IS NOT FAULT TOLERANT AND SHOULD NOT BE USED IN ANY ENVIRONMENT
 * WHICH REQUIRES THIS.
 *<br>
 * The Licensed Software is provided AS IS without warranty of any kind. The Copyright Holder
 * makes no further representations or warranties expressly or implied such as by way
 * of example but not of limitation regarding merchant ability or fitness for
 * any particular purpose, or that Licensed Software is error free, or that the use
 * of the Licensed Software or any copies thereof will not infringe any patent,
 * copyright or trademark of third parties.
 *<br>
 * NO LIABILITY FOR DAMAGES. In no event shall the Copyright Holder or agents or its
 * suppliers be liable to you for any consequential, incidental or indirect damages
 * whatsoever (including, without limitation, damages for loss of business profits,
 * business interruption, loss of business information, or any other pecuniary loss)
 * arising out of the use of or inability to use this SOFTWARE EVEN IF the
 * Copyright Holder HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 *<br>
 * The entire risk arising out of use or performance of the SOFTWARE remains with you.
 *<br>
 * Because of the various hardware and software environments into which this software
 * may be put, NO WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE IS OFFERED.
 *<br>
 * The liability of the Copyright Holder for actual damages from any cause whatsoever,
 * and regardless of the form of action (whether in contract, tort (including negligence),
 * product liability or otherwise) WILL BE LIMITED TO ZERO, NOTHING, NIENTE, NADA, NICHTS.
 *<br>
 ******************************************************************************/

#include "wxPushButton.h"

#define wxMB_TEXT_MARGIN 8
#define wxMB_COLOR_OVER wxColour(0xE8,0xE8,0xE8)
#define wxMB_COLOR_BG wxColour(0xD7,0xD7,0xD7)

#ifdef __WXMSW__
    #define wxLABEL_FONT_SIZE 8
#else
    #define wxLABEL_FONT_SIZE 10
#endif


BEGIN_EVENT_TABLE(wxPushButton, wxWindow)
    EVT_MOUSE_EVENTS(wxPushButton::OnMouseEvent)
    EVT_PAINT(wxPushButton::OnPaint)
    EVT_SIZE(wxPushButton::OnSizeEvent)
    EVT_ERASE_BACKGROUND(wxPushButton::OnEraseBackground)
END_EVENT_TABLE()

wxPushButton::wxPushButton(wxWindow * parent,
                   wxWindowID id,
                   const wxString& label,
                   const wxPoint& pos,
                   const wxSize &size,
                   long WXUNUSED(style),
                   const wxValidator& WXUNUSED(validator),
                   const wxString& WXUNUSED(name),
                   wxIcon* up,
                   wxIcon* down,
                   wxIcon* over,
                   wxIcon* dis)
: wxWindow(parent, id, pos, size)
, m_bitmap(new wxBitmap())
, m_label(label)
, m_font(new wxFont(wxLABEL_FONT_SIZE,wxDEFAULT,wxNORMAL,wxBOLD))
, m_dx(0)
, m_dy(0)
, m_fstate(FS_Up)
, m_vstate(VS_Up)
, m_painted(false)
, m_enabled(true)
{
   if(size == wxDefaultSize) {
      m_width = 32;
      m_height = 32;
   }
   else {
      m_width = size.x;
      m_height = size.y;
   }

   for(int i=0;i<4;i++) m_icons[i] = 0;
   SetIcons(up,down,over,dis);
}

wxPushButton::~wxPushButton()
{
   clear();
   delete m_bitmap; m_bitmap = 0;
   delete m_font;   m_font   = 0;
}

void  wxPushButton::clear()
{
   for(int i=0;i<4;i++) {
      delete m_icons[i];
      m_icons[i] = 0;
   }
}


void wxPushButton::SetIcons( wxIcon* up  ,
                             wxIcon* down,
                             wxIcon* over,
                             wxIcon* dis )
{
   clear();
/*
   // make dummy icons for any null pointers
   if(!up)   up   = new wxIcon((const char **)0);
   if(!down) down = new wxIcon((const char **)0);

   if(!over) over = new wxIcon((const char **)0);
   if(!dis)  dis  = new wxIcon(*up); // without a disable image, the up image will be used
*/
   if(up) {
      m_width  = up->GetWidth();
      m_height = up->GetHeight();
   }

   int i = 0;
   m_icons[i++] = up;
   m_icons[i++] = down;  // if the down image is a NULL pointer, use a lower frame instead
   m_icons[i++] = over;  // if the over image is a NULL pointer, use a upper frame instead
   m_icons[i++] = dis;

   // recompute the bitmap by resetting the label
   SetLabel(m_label);
}

void wxPushButton::SetLabel(const wxString& label)
{
   m_dx = 0;
   m_dy = 0;

   m_label = label;
   if(!m_painted) {

      int w,h;
      wxClientDC dc(this);
      dc.SetFont(*m_font);
      dc.GetTextExtent(m_label,&w,&h);
      if(w > m_width) m_width = w + wxMB_TEXT_MARGIN;

      // use the up icon dimensions for the image
      int x = 0;
      if(m_icons[0]) x = m_icons[0]->GetWidth();
      if(x <= m_width) m_dx = (m_width - x) >> 1;
      else m_dx = 0;

      // create a bitmap to match
      if(m_label.Length() > 0){
        // m_height += wxLABEL_FONT_SIZE;
         m_height += m_font->GetPointSize()+2;
      }
      m_bitmap->Create(m_width,m_height);
      SetSize(m_width,m_height);
   }

   Redraw();
}


bool wxPushButton::Enable(bool enable)
{
   if(enable) {
      Enable();
      return true;
   }
   else return Disable();
}

void wxPushButton::Enable()
{
   if(!m_enabled) {
      m_vstate = VS_Up;
      m_enabled = true;
      Redraw();
   }
}

bool wxPushButton::Disable()
{
   if(m_fstate != FS_Down)return false;

   if(m_enabled ) {
      m_enabled = false;
      m_vstate = VS_Dis;
      Redraw();
   }
   return true;
}

bool wxPushButton::IsDown()
{
   return (FunctionalState() == FS_Down);
}

wxPushButton::FState wxPushButton::FunctionalState()
{
   return m_fstate;
}

wxPushButton::VState wxPushButton::VisualState()
{
   return m_vstate;
}

void wxPushButton::DrawBorder(wxDC& dc, BorderType border)
{
   // wxColour bg = GetParent()->GetBackgroundColour();
 // wxBrush brush_over(wxMB_COLOR_OVER,wxSOLID);
   wxBrush brush_over(GetParent()->GetBackgroundColour(),wxSOLID);
   dc.SetBrush(brush_over);
   dc.SetPen(*wxTRANSPARENT_PEN);
   dc.DrawRectangle(0,0,m_width,m_height);

   wxPen light(wxColour(0xFF,0xFF,0xFF),1,wxSOLID);
   wxPen dark(wxColour(0x80,0x80,0x80),1,wxSOLID);
 //  wxPen corner(wxMB_COLOR_BG,1,wxSOLID);

   switch(border) {
   case Border_High:
      {
         dc.SetPen(light);
         dc.DrawLine(1,0,m_width-2,0);
         dc.DrawLine(0,1,0,m_height-2);
         dc.SetPen(dark);
         dc.DrawLine(0,m_height-1,m_width-1,m_height-1);
         dc.DrawLine(m_width-1,0,m_width-1,m_height-1);
         break;
      }
   case Border_Sunken:
      {
         dc.SetPen(dark);
         dc.DrawLine(1,0,m_width-2,0);
         dc.DrawLine(0,1,0,m_height-2);
         dc.SetPen(light);
         dc.DrawLine(1,m_height-1,m_width-2,m_height-1);
         dc.DrawLine(m_width-1,1,m_width-1,m_height-2);
         break;
      }
   default:
      {
         break;
      }
   }
}

void wxPushButton::DrawOnBitmap()
{
   wxCoord dx = 0;
   wxCoord dy = 0;
   wxCoord w;
   wxCoord h;

   wxMemoryDC dc;
   dc.SelectObject(*m_bitmap);

   // use the system background colour for buttons (wxSYS_COLOUR_BTNFACE)
   wxBrush brush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE), wxSOLID);
   dc.SetBackground(brush);
   dc.Clear();
   dc.SetFont(*m_font);

   VState vstate = VisualState();

   switch(vstate) {
   case VS_Up:
      {
         if(m_icons[vstate]) dc.DrawIcon(*m_icons[vstate],m_dx,m_dy);
         break;
      }
   case VS_Down:
      {
         DrawBorder(dc,Border_Sunken);
         if(m_icons[vstate]) dc.DrawIcon(*m_icons[vstate],m_dx+1,m_dy+1);
         else {
           if(m_icons[VS_Up]) {
             dc.DrawIcon(*m_icons[VS_Up],m_dx+1,m_dy+1);
           }
         }
         dx = dy = 1;
         break;
      }
   case VS_Over:
      {
         VState vs_icon = (FunctionalState() == FS_Down)? VS_Down : VS_Up;

         DrawBorder(dc,Border_High);
         if(m_icons[vs_icon]) dc.DrawIcon(*m_icons[vs_icon],m_dx,m_dy);
         else {
           if(m_icons[vs_icon]) {
             dc.DrawIcon(*m_icons[vs_icon],m_dx,m_dy);
           }
         }
         break;
      }
   case VS_Dis:
      {
         if(m_icons[vstate]) dc.DrawIcon(*m_icons[vstate],m_dx,m_dy);
         break;
      }
   default:
      {
         dc.SelectObject(wxNullBitmap);
         return;
      }
   }

   if(!m_label.IsEmpty()) {

    //  dy -= (wxLABEL_FONT_SIZE+2);

      dc.GetTextExtent(m_label,&w,&h);
      if(w > m_width) w = m_width;
      if(h > m_height) h = m_height;

	   if(m_enabled) {
//		  dc.SetTextForeground(wxColour(0,0,0));
		   dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
		  dc.DrawText(m_label,((m_width-w)>>1)+1+dx,m_height-h+dy);
	   }
	   else {
		  dc.SetTextForeground(wxColour(255,255,255));
		  dc.DrawText(m_label,((m_width-w)>>1)+1+dx,m_height-h+dy);
		  dc.SetTextForeground(wxColour(128,128,128));
		  dc.DrawText(m_label,((m_width-w)>>1)+dx,m_height-h-1+dy);
	   }
    }

    dc.SelectObject(wxNullBitmap);
}

void wxPushButton::OnPaint(wxPaintEvent& WXUNUSED(event))
{
   wxPaintDC dc(this);
   if(!m_painted) {
      DrawOnBitmap();
      m_painted = true;
   }
   dc.DrawBitmap(*m_bitmap,0,0,false);
}


void wxPushButton::Redraw()
{
   if(m_painted) {
      DrawOnBitmap();
      wxClientDC dc(this);
      dc.DrawBitmap(*m_bitmap,0,0,false);

      // Update is required for linux
      Update();
   }
}

void wxPushButton::OnEraseBackground(wxEraseEvent &event)
{
     Redraw();
/*
#ifdef __WXMSW__
   Redraw();
#else
   // Defer the redraw for later
   wxSizeEvent ev(GetSize(),GetId());
   AddPendingEvent(ev);
#endif
  // event.Skip();
*/
}

void wxPushButton::OnMouseEvent(wxMouseEvent& event)
{
   if(!m_enabled) return;

   // Selected mouse events will be passed as button click
   // command events to the parent of this button.
   // The parent must check the button functional state
   wxCommandEvent ev(wxEVT_COMMAND_BUTTON_CLICKED,GetId());
   ev.SetEventObject(GetParent());

   if(event.ButtonDown()) {
      m_fstate = FS_Down;
      m_vstate = VS_Down;
      GetParent()->ProcessEvent(ev);
   }
   else if(event.ButtonUp()) {
      m_fstate = FS_Up;
	   m_vstate = VS_Up;

	   // send the up event only when the mouse is over the button
	   wxPoint pos = event.GetPosition();
	   if((pos.x < GetSize().GetWidth()) && (pos.y < GetSize().GetHeight())) {
        m_vstate = VS_Over;
		  GetParent()->ProcessEvent(ev);
	   }
   }
   else if (event.Entering()) {
      if(FunctionalState() == FS_Up)m_vstate = VS_Over;
   }
   else if(event.Leaving()) {
      if(FunctionalState() == FS_Down){
         m_fstate = FS_Up;
         m_vstate = VS_Up;
         GetParent()->ProcessEvent(ev);
      }
      else {
         m_vstate = VS_Up;
      }
   }

   Redraw();
}

void wxPushButton::OnSizeEvent(wxSizeEvent& event)
{
   Redraw();
   event.Skip();
}

void wxPushButton::SetButtonState(bool down)
{
   if(down) {
      if(m_fstate != FS_Down) {
         m_fstate = FS_Down;
         wxCommandEvent ev(wxEVT_COMMAND_BUTTON_CLICKED,GetId());
         ev.SetEventObject(GetParent());
         GetParent()->ProcessEvent(ev);
      }
      if(m_vstate != VS_Down) {
         m_vstate = VS_Down;
         Redraw();
      }
   }
   else {
      if(m_fstate != FS_Up) {
         m_fstate = FS_Up;
         wxCommandEvent ev(wxEVT_COMMAND_BUTTON_CLICKED,GetId());
         ev.SetEventObject(GetParent());
         GetParent()->ProcessEvent(ev);
      }
      if(m_vstate != VS_Up) {
         m_vstate = VS_Up;
         Redraw();
      }
   }

}
