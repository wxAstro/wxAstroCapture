

#include "wxFitsKeyword.h"

/// ////////////////////////////////////////////////////////////////////
/// IMPLEMENT wxFitsKeyword

// general Spec is:
// Keyword (bytes 1-8)           - The keyword shall be a left justifed, 8-character, blank-filled,
// Value Indicator (bytes 9-10)  - If this field contains the ASCII characters "= " it indicates the presence of a value field
//  If the value indicator is not present or if it is a commentary keyword then columns 9{80 may contain any ASCII text.
// Value/Comment (bytes 11-80)   - This field, when used, shall contain the value, if any, of the keyword, followed by optional comments.
//  If a comment is present, it must be preceded by a slash (hexadecimal 2F, "/").

// NOTE the strings must match the enums
static const wxChar* s_kwdString[] = {
   wxT("SIMPLE  "),    wxT("BITPIX  "),    wxT("NAXIS   "),    wxT("NAXIS1  "),    wxT("NAXIS2  "),    wxT("NAXIS3  "),
   wxT("BSCALE  "),    wxT("BZERO   "),    wxT("DATE-OBS"),    wxT("TIMESYS "),    wxT("TELESCOP"),    wxT("INSTRUME"),    wxT("OBSERVER"),
   wxT("OBJECT  "),    wxT("EXPTIME "),    wxT("CCD_TEMP"),    wxT("PSZX    "),    wxT("PSZY    "),    wxT("XBINNING"),    wxT("YBINNING"),
   wxT("CAMSETUP"),    wxT("EGAIN   "),    wxT("FOCALLEN"),    wxT("APTDIA  "),    wxT("SWCREATE"),    wxT("FILTER  "),
   wxT("IMAGETYP"),    wxT("OBJCTRA "),    wxT("OBJCTDEC"),    wxT("SITELAT "),    wxT("SITELONG"),    wxT("FRAMENO "),    wxT("COMMENT "),
   wxT("HISTORY "),    wxT("END     "),
};

// the static lookup map from keyword string to enum
wxFitsKeyword::KeywordMap wxFitsKeyword::m_map;

wxFitsKeyword::wxFitsKeyword()
: m_kwd(EF_UNDEFINED)
{
}

wxFitsKeyword::wxFitsKeyword(EFitsKeyword kwd)
: m_kwd(EF_UNDEFINED)
{
   SetKeyword(kwd);
}

wxFitsKeyword::EFitsKeyword wxFitsKeyword::lookupKeyword(const wxString& keyword)
{
   if(m_map.size() == 0) {

      // empty map, must initialised
      int nkw = sizeof(s_kwdString)/(sizeof(wxChar*));
      for(int ikw=0; ikw<nkw; ikw++) {
         wxString kwd = s_kwdString[ikw];
         m_map[kwd.Trim()] = (wxFitsKeyword::EFitsKeyword)(ikw);
      }
   }

   // loop up the string to get the enum
   KeywordMap::iterator ikw = m_map.find(keyword);
   if(ikw == m_map.end())return EF_UNDEFINED;

   return ikw->second;
}

wxFitsKeyword::wxFitsKeyword(EFitsKeyword kwd, const wxString& kwdString, const wxString& comment)
: m_kwd(EF_UNDEFINED)
{
   SetKeyword(kwd,kwdString,comment);
}

wxFitsKeyword::wxFitsKeyword(EFitsKeyword kwd, long kwdInt, const wxString& comment)
: m_kwd(EF_UNDEFINED)
{
   SetKeyword(kwd,kwdInt,comment);
}

wxFitsKeyword::wxFitsKeyword(EFitsKeyword kwd, double kwdDbl, int digits, const wxString& comment)
: m_kwd(EF_UNDEFINED)
{
   SetKeyword(kwd,kwdDbl,comment,digits);
}


// If the value is a fixed format integer, the ASCII representation shall be right justified in
//  columns 11-30. An integer consists of a + (hexadecimal 2B) or - (hexadecimal 2D)
//  sign, followed by one or more ASCII digits (hexadecimal 30 to 39), with no embedded
//  spaces. The leading + sign is optional.
bool wxFitsKeyword::SetKeyword(EFitsKeyword kwd)
{
   if (kwd<EF_SIMPLE || kwd>EF_END) return false;
   m_kwd = kwd;
   m_sKwdString = s_kwdString[kwd];
   m_sKwdString.Append(wxT(' '), 72); // fill with Blanks (according to standard)
   m_sKwdString.Truncate(80); // trunc 80 chars (according to standard)
   return true;
}

// If the value is a fixed format character string, column 11 shall contain a single quote (hexadecimal code 27, "'");
// the string shall follow, starting in column 12, followed by a closing single quote (also hexadecimal code 27)
// that should not occur before column 20 and must occur in or before column 80.
bool wxFitsKeyword::SetKeyword(EFitsKeyword kwd, const wxString& kwdString, const wxString& comment)
{
   if (kwd<EF_SIMPLE || kwd>EF_END) return false;
   m_kwd = kwd;
   m_sKwdString = s_kwdString[kwd];
   if ( kwd==EF_COMMENT || kwd==EF_HISTORY ) {
      // COMMENT and HISTORY Keyword
      // This keyword shall have no associated value; columns 9-80 may contain any ASCII text.
      m_sKwdString += wxT("  "); // btw next is char 11
      m_sKwdString += kwdString.Mid(0, 69);
   }
   else {
      // string is keyword content
      // btw next is char 9
      m_sKwdString += wxT("= '"); // btw next is char 11
      m_sKwdString += kwdString.Mid(0, 67);
      m_sKwdString += wxT("'");
      if (!comment.IsEmpty()) {
         m_sKwdString += wxT(" /");
         m_sKwdString += comment;
      }
   }
   m_sKwdString.Append(wxT(' '), 72); // fill with Blanks (according to standard)
   m_sKwdString.Truncate(80); // trunc 80 chars (according to standard)
   return true;
}

// If the value is a fixed format integer, the ASCII representation shall be right justified in
//  columns 11-30. An integer consists of a + (hexadecimal 2B) or - (hexadecimal 2D)
//  sign, followed by one or more ASCII digits (hexadecimal 30 to 39), with no embedded
//  spaces. The leading + sign is optional.
bool wxFitsKeyword::SetKeyword(EFitsKeyword kwd, long kwdInt, const wxString& comment)
{
   if (kwd<EF_SIMPLE || kwd>EF_END) return false;
   m_kwd = kwd;
   m_sKwdString = s_kwdString[kwd];
   wxString n;
   m_sKwdString += wxT("= "); // btw next is char 11
   n.Printf(wxT("%20d"), kwdInt);
   m_sKwdString += n;

   if (!comment.IsEmpty()) {
      m_sKwdString += wxT(" /");
      m_sKwdString += comment;
   }
   m_sKwdString.Append(wxT(' '), 72); // fill with Blanks (according to standard)
   m_sKwdString.Truncate(80); // trunc 80 chars (according to standard)
   return true;
}

// If the value is a fixed format real floating point number, the ASCII representation shall
// appear, right justified, in columns 11-30.
bool wxFitsKeyword::SetKeyword(EFitsKeyword kwd, double kwdDbl, const wxString& comment, int digits)
{
   if (kwd<EF_SIMPLE || kwd>EF_END) return false;
   m_kwd = kwd;
   m_sKwdString = s_kwdString[kwd];
   wxString n;
   m_sKwdString += wxT("= "); // btw next is char 11
   n.Printf(wxT("%#20.*G"), digits, kwdDbl);
   m_sKwdString += n;
   if (!comment.IsEmpty()) {
      m_sKwdString += wxT(" /");
      m_sKwdString += comment;
   }
   m_sKwdString.Append(wxT(' '), 72); // fill with Blanks (according to standard)
   m_sKwdString.Truncate(80); // trunc 80 chars (according to standard)
   return true;
}

// If the value is a fixed format logical constant, it shall appear as a T or F in column 30.
bool wxFitsKeyword::SetKeyword(EFitsKeyword kwd, bool kwdLogical, const wxString& comment)
{
   if (kwd<EF_SIMPLE || kwd>EF_END) return false;
   m_kwd = kwd;
   m_sKwdString = s_kwdString[kwd];
   wxString n;
   m_sKwdString += wxT("= "); // btw next is char 11
   if (kwdLogical)
      n.Printf(wxT("%20s"), wxT("T"));
   else
      n.Printf(wxT("%20s"), wxT("F"));
   m_sKwdString += n;
   if (!comment.IsEmpty()) {
      m_sKwdString += wxT(" /");
      m_sKwdString += comment;
   }
   m_sKwdString.Append(wxT(' '), 72); // fill with Blanks (according to standard)
   m_sKwdString.Truncate(80); // trunc 80 chars (according to standard)
   return true;
}



/// Implement some specific keywords

// specific keywords
void wxFitsKeywordEx::ExposureTimeKwd(double expoSeconds)
{
   SetKeyword(wxFitsKeyword::EF_EXPTIME, expoSeconds, wxT("Total integration time (s)"));
}

//BM:20090620 - added
void wxFitsKeywordEx::DateObsKwd(wxDateTime dateObs)
{
   // assuming we use wxDateTime::UNow() to get the timestamp
   unsigned short msec = dateObs.GetMillisecond(wxDateTime::UTC);
   wxString dts = wxString::Format(wxT("%s.%03hu"), dateObs.Format(wxT("%Y-%m-%dT%H:%M:%S"), wxDateTime::UTC).c_str(), msec);
   SetKeyword(wxFitsKeyword::EF_DATE_OBS, dts, wxT("UTC of start exposure"));
}

void wxFitsKeywordEx::TemperatureKwd(double ccdTemperature)
{
   SetKeyword(wxFitsKeyword::EF_CCD_TEMP, ccdTemperature, wxT("CCD temperature [degC]"));
}

void wxFitsKeywordEx::BinningKwdX(unsigned short xBin)
{
   SetKeyword(wxFitsKeyword::EF_XBINNING, long(xBin), wxT("X binning factor"));
}

void wxFitsKeywordEx::BinningKwdY(unsigned short yBin)
{
   SetKeyword(wxFitsKeyword::EF_YBINNING, long(yBin), wxT("Y binning factor"));
}


void wxFitsKeywordEx::TelescopeDescriptionKwd(const wxString& telescope)
{
   SetKeyword(wxFitsKeyword::EF_TELESCOP, telescope);
}

void wxFitsKeywordEx::CameraSettingKwd(const wxString& setting)
{
   SetKeyword(wxFitsKeyword::EF_CAMSETUP, setting);
}


void wxFitsKeywordEx::CameraDescriptionKwd(const wxString& camera)
{
   SetKeyword(wxFitsKeyword::EF_INSTRUME, camera);
}

void wxFitsKeywordEx::PixelSizeKwdX(float xPix_um)
{
   SetKeyword(wxFitsKeyword::EF_PSZX, double(xPix_um), wxT("detector X cell size [microns]"));
}

void wxFitsKeywordEx::PixelSizeKwdY(float yPix_um)
{
   SetKeyword(wxFitsKeyword::EF_PSZY, double(yPix_um), wxT("detector Y cell size [microns]"));
}

void wxFitsKeywordEx::FilterDescriptionKwd(const wxString& filter)
{
   SetKeyword(wxFitsKeyword::EF_FILTER, filter);
}

void wxFitsKeywordEx::ObjectDescriptionKwd(const wxString& object)
{
   SetKeyword(wxFitsKeyword::EF_OBJECT, object);
}

void wxFitsKeywordEx::ObserverDescriptionKwd(const wxString& observer)
{
   SetKeyword(wxFitsKeyword::EF_OBSERVER, observer);
}

void wxFitsKeywordEx::ImageTypeKwd(EImageType imageType)
{
   wxFitsKeyword k;
   wxString it;
   switch (imageType) {
      case EIT_Light_Frame: it = wxT("Light Frame"); break;
      case EIT_Dark_Frame:  it = wxT("Dark Frame"); break;
      case EIT_Bias_Frame:  it = wxT("Bias Frame"); break;
      case EIT_Flat_Field:  it = wxT("Flat Field"); break;
      default: it = wxT("Unknown");
   }//switch
   SetKeyword(wxFitsKeyword::EF_IMAGETYP, it);
}

void wxFitsKeywordEx::FrameNumberKwd(size_t frameNo)
{
   SetKeyword(wxFitsKeyword::EF_FRAMENO, long(frameNo), wxT("Frame sequence number"));
}


/// Implement wxFitsKeywordList
//
wxFitsKeywordList::wxFitsKeywordList()
{
}


wxFitsKeywordList::wxFitsKeywordList(const wxFitsKeyword &arg)
{
   *this += arg;
}

wxFitsKeywordList::wxFitsKeywordList(const wxFitsKeywordList &arg)
{
   *this = arg;
}


wxFitsKeywordList::~wxFitsKeywordList()
{
   m_keywordList.clear();
}

void wxFitsKeywordList::Update(const wxFitsKeyword& kwd)
{
   wxFitsKeywordList::iterator h_it;

   for (h_it = m_keywordList.begin(); h_it != m_keywordList.end(); h_it++) {
      if ( (*h_it).Keyword()==kwd.Keyword()) {
         *h_it = kwd;
         break;
      }
   }//for
   if ( h_it==m_keywordList.end() ) {
      m_keywordList.push_back(kwd);
   }
}

wxFitsKeywordList& wxFitsKeywordList::operator+= (const wxFitsKeyword &arg)
{
   m_keywordList.push_back(arg);
   return *this;
}

wxFitsKeywordList wxFitsKeywordList::operator+ (const wxFitsKeyword &arg) const
{
   wxFitsKeywordList tx(arg);
   tx += arg;
   return tx;
}



wxFitsKeywordList& wxFitsKeywordList::operator= (const wxFitsKeywordList &arg)
{
   if (this!=&arg) {
      m_keywordList.clear();
      ArrayOfFitsKeywords::const_iterator it;
      for (it=arg.m_keywordList.begin(); it!=arg.m_keywordList.end(); it++) {
         m_keywordList.push_back(*it);
      }//for
   }
   return *this;
}


wxFitsKeywordList wxFitsKeywordList::operator+ (const wxFitsKeywordList &arg) const
{
   wxFitsKeywordList tx(*this);
   tx += arg;
   return tx;
}


wxFitsKeywordList& wxFitsKeywordList::operator+= (const wxFitsKeywordList &arg)
{
   ArrayOfFitsKeywords::const_iterator it;
   for (it=arg.m_keywordList.begin(); it!=arg.m_keywordList.end(); it++) {
      m_keywordList.push_back(*it);
   }//for
   return *this;
}


