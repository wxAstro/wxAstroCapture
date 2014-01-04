/// Defines a 10x16 font for timestamps
//
//  e.g. 12.04.2009 13:22:23.52
//  or   04/12/2009 13:22:23.52

#ifndef _WXTIMEFONT_H_
#define _WXTIMEFONT_H_

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

class wxTimeFont
{
public:
   enum ELetterCode {
      ELC_0=0,
      ELC_1,
      ELC_2,
      ELC_3,
      ELC_4,
      ELC_5,
      ELC_6,
      ELC_7,
      ELC_8,
      ELC_9,
      ELC_Dot,
      ELC_Colon,
      ELC_Slash,
      ELC_Dash,
      ELC_Blank,
      ELC_SIZE  // dummy to give the 0-bsaed size
   };

   enum EFontSize {
      EFS_Small = 0,
      EFS_Mid,
      EFS_Large,
      EFS_SIZE  // dummy to give the 0-bsaed size
   };

public:
   wxTimeFont(EFontSize fontSize = EFS_Mid);
   ~wxTimeFont();

public:
   EFontSize      FontSize() const {return m_fontSize;};
   unsigned short FontWidth() const {return m_width;};
   unsigned short FontHeight() const {return m_height;};

   const wxUint8* Letter(ELetterCode letter) const;
   const wxUint8* Letter(wxChar letter) const;

private:
   EFontSize      m_fontSize;
   unsigned short m_width;
   unsigned short m_height;
   unsigned short m_size;
   wxUint8       *m_fontPtr;
};


#endif // _WXTIMEFONT_H_
