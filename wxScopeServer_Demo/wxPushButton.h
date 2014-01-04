/**
 * @file wxPushButton.h
 *****************************************************************************
 * wxPushButton
 *
 * Definition of a wxWidgets pushbutton
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

#ifndef WXPUSHBUTTON_H
#define WXPUSHBUTTON_H

/*
Author: Carsten A. Arnholm

This work is inspired from
Abutton.h   : http://audacity.sourceforge.net/dox/html/AButton_8h-source.html
awxButton.h : http://www.iftools.com/awx.en.html

wxPushButton a custom pushbutton class designed to emulate the functionality
of physical spring loaded buttons, used in for example door bells:
Pressed state : As long the button is pushed, the bell rings.
Released state: When button is released the bell stops ringing.

The class is designed to be pretty much construction and event compatible
with a wxButton so that it can be used with wxSmith GUI rad tool. Simply
make a wxButton as normal, but specify wxPushButton as class

The button has 2 functional states:
Up   : The button is unpressed
Down : The button is pressed

In addition there are 2 special highlight states
Over : The mouse is over the button
Dis  : The button is disabled

The highlight states only control the visual aspect
of the button, not the up/down funtional state. The
highlight states have no visual effect unless the
functional state is "Up".

wxButton uses wxIcons for all of its visual states:
up, down, Over and disabled,
The icons musts be rectangular and opaque. Typical dimensions are 32x32
*/


#include <wx/window.h>
#include <wx/wx.h>

class wxPushButton : public wxWindow {
public:
   // Constructor. Any wxIcons passed are owned by wxPushButton
   wxPushButton(wxWindow * parent,
                wxWindowID id,
                const wxString& label = wxEmptyString,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize &size = wxDefaultSize,
                long style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = _T("PushButton"),
                wxIcon* up   = 0,
                wxIcon* down = 0,
                wxIcon* over = 0,
                wxIcon* dis  = 0);

   // Set the icons in a separate call. wxIcons passed are owned by wxPushButton
   virtual void SetIcons( wxIcon* up  ,
                          wxIcon* down,
                          wxIcon* over,
                          wxIcon* dis
                          );

   // Set the label, it may be empty
   virtual void SetLabel(const wxString& label);

   virtual bool Disable();
   virtual void Enable();
   virtual bool Enable(bool enable);

   // Return the current button state: true = button is pressed down
   virtual bool IsDown();

   // Event handling follows
   // The following functions generate events if call result in a state change
   virtual void SetButtonState(bool down);

   // OnMouseEvent handles mouse events and
   virtual void OnMouseEvent(wxMouseEvent& event);

   virtual void OnPaint(wxPaintEvent &event);
   virtual void OnEraseBackground(wxEraseEvent &event);
   virtual void OnSizeEvent(wxSizeEvent& event);

   virtual ~wxPushButton();

protected:
   void clear();
   void Redraw();

   enum FState {  // Functional State
      FS_Up = 0,
      FS_Down
   };

   enum VState {  // Visual State
      VS_Up = 0,
      VS_Down,
      VS_Over,
      VS_Dis
   };

   enum BorderType {
      Border_Sunken,
      Border_Flat,
      Border_High
   };

   FState FunctionalState();
   VState VisualState();

   void DrawOnBitmap();
   void DrawBorder(wxDC& dc, BorderType border);

private:
   // the following objects are owned here
   wxIcon*   m_icons[4];  // Icons for visual button states
   wxBitmap* m_bitmap;    // Bitmap used for painting
   bool      m_painted;   // True when bitmap has been painted
   wxFont*   m_font;      // Font used for text
   FState    m_fstate;
   VState    m_vstate;

   bool      m_enabled;   // true when button enabled

   wxString  m_label;     // button text label
   int m_width;
   int m_height;
   int m_dx;
   int m_dy;

   DECLARE_EVENT_TABLE()
};



#endif // WXPUSHBUTTON_H
