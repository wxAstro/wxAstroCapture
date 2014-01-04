

#ifndef _WXFITSKEYWORD_H__
#define _WXFITSKEYWORD_H__


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <vector>
#include <map>

/// Define wxFitsKeyword
// handles the FITS keywords
// currently only the enumerated ones are available
class wxFitsKeyword
{
public:
   // Keywords and description from SBFITSEXT 1r0  (SBIG 'Standard')
   enum EFitsKeyword {
      EF_UNDEFINED = -1, // NOTE this must be -1 and the FIRST
      EF_SIMPLE = 0, // NOTE this must be 0 and the SECOND
      EF_BITPIX,
      EF_NAXIS,
      EF_NAXIS1,
      EF_NAXIS2,
      EF_NAXIS3,
      EF_BSCALE,     // BSCALE Floating Point - The last two are for 16 bit images. Typically BSCALE = 1.0
      EF_BZERO,      // BZERO Floating Point - Typically 32768.0

      EF_DATE_OBS,   // DATE-OBS String - The UTC date and time at the start of the exposure in
                     // the ISO standard 8601 format: '2002-09-07T15:42:17.123'
      EF_TIMESYS,    // Time system used
      EF_TELESCOP,   // TELESCOP String - The model Telescope, ie LX200
      EF_INSTRUME,   // INSTRUME String - The model Camera used, ie ST-7
      EF_OBSERVER,   // OBSERVER String - The name of the Observer, ie John Doe
      EF_OBJECT,     // OBJECT String - The name of Object Imaged, ie M57
      EF_EXPTIME,    // EXPTIME Floating Point - The total exposure time in seconds. For Track
                     // and Accumulate Images this is the sum of all the exposure times.
      EF_CCD_TEMP,   // CCD-TEMP Floating Point - Temperature of CCD when exposure taken
      EF_PSZX,       // PSZX Floating Point - detector pixel width in microns
      EF_PSZY,       // PSZY Floating Point - detector pixel heigth in microns
      EF_XBINNING,   // XBINNING Integer - Binning factor in width
      EF_YBINNING,   // YBINNING Integer - Binning factor in height
      EF_CAMSETUP,   // CAMSETUP (wxAC added) camera setting string
      EF_EGAIN,      // EGAIN Floating Point - Electronic gain in e-/ADU
      EF_FOCALLEN,   // FOCALLEN Floating Point - Focal Length of the Telescope used in millimeters
      EF_APTDIA,     // APTDIA Floating Point - Aperture Diameter of the Telescope used in millimeters.
      EF_SWCREATE,   // SWCREATE String - This indicates the name and version of the Software that
                     // initially created this file ie SBIGs CCDOps Version 5.10
      EF_FILTER,     // FILTER String - Optical Filter used to take image, ie H-Alpha. If this
                     // keyword is not included there was no filter used to take the image.
      EF_IMAGETYP,   // IMAGETYP String - This indicates the type of image and should be one of the following:
                     // Light Frame, Dark Frame, Bias Frame, Flat Field
      EF_OBJCTRA,    // OBJCTRA String - This is the Right Ascension of the center of the image in hours, minutes and seconds.
                     // The format for this is 12 24 23.123 (HH MM SS.SSS)using a space as the separator.
      EF_OBJCTDEC,    // OBJCTDEC String - This is the Declination of the center of the image in degrees.
                     // The format for this is +25 12 34.111 (SDD MM SS.SSS) using a space as the separator.
                     // For the sign, North is + and South is -.
      EF_SITELAT,    // SITELAT String - Latitude of the imaging location in degrees. Format is the same as the OBJCTDEC keyword.
      EF_SITELONG,   // SITELONG String - Longitude of the imaging location in degrees. Format is the same as the OBJCTDEC keyword.

      EF_FRAMENO,    //20080819:BM sequence number (NNNN) The sequence number typically increments by one after
                     // each saved CCD readout, but the observer is allowed to modify this sequence number
                     // such that more than one FITS file has the same value of FRAMENO.

      EF_COMMENT,    // Any comment - can include multiple copies of this keyword
      EF_HISTORY,    // HISTORY String - Include multiple copies of this keyword for every
                     // modification made to the image in the order made.

      EF_END         //  // NOTE this must be 0 and the LAST
   };

   // create content

public:
   wxFitsKeyword();
   wxFitsKeyword(EFitsKeyword kwd);
   wxFitsKeyword(EFitsKeyword kwd, const wxString& kwdString, const wxString& comment = wxT(""));
   wxFitsKeyword(EFitsKeyword kwd, long kwdInt, const wxString& comment = wxT(""));
   wxFitsKeyword(EFitsKeyword kwd, double kwdDbl, int digits = 7, const wxString& comment = wxT(""));

   // add an empty keyword (like END)
   bool SetKeyword(EFitsKeyword kwd);
   // add keywords with given value types and optionally a comment
   // note for HISTORY and COMMENT only the kwdString is used, the comment value is just ignored
   bool SetKeyword(EFitsKeyword kwd, const wxString& kwdString, const wxString& comment = wxT(""));
   bool SetKeyword(EFitsKeyword kwd, long kwdInt, const wxString& comment = wxT(""));
   bool SetKeyword(EFitsKeyword kwd, double kwdDbl, const wxString& comment = wxT(""), int digits = 7);
   bool SetKeyword(EFitsKeyword kwd, bool kwdLogical, const wxString& comment = wxT(""));

   // get content
   const EFitsKeyword Keyword() const {return m_kwd;};
   const wxString& KeywordString() const {return m_sKwdString;};

   typedef std::map<wxString, EFitsKeyword> KeywordMap;
   static EFitsKeyword lookupKeyword(const wxString& keyword);

private:
   EFitsKeyword   m_kwd;
   wxString       m_sKwdString;
   static KeywordMap m_map;
};


/// Define some specific keywords

class wxFitsKeywordEx : public wxFitsKeyword
{
public:
   // specific keywords - all are Update types
   // so the original content is overwritten if already existing
   // otherwise they are added at the end
   void ExposureTimeKwd(double expoSeconds);
   void DateObsKwd(wxDateTime dateObs); //BM:20090620 - added
   void TemperatureKwd(double ccdTemperature);
   void BinningKwdX(unsigned short xBin);
   void BinningKwdY(unsigned short yBin);

   void TelescopeDescriptionKwd(const wxString& telescope);
   void CameraDescriptionKwd(const wxString& camera);
   void CameraSettingKwd(const wxString& setting);
   void PixelSizeKwdX(float xPix_um);
   void PixelSizeKwdY(float yPix_um);
   void FilterDescriptionKwd(const wxString& filter);
   void ObjectDescriptionKwd(const wxString& object);
   void ObserverDescriptionKwd(const wxString& observer);
   enum EImageType {
      EIT_Light_Frame = 0,
      EIT_Dark_Frame,
      EIT_Bias_Frame,
      EIT_Flat_Field
   };
   void ImageTypeKwd(EImageType imageType);
   void FrameNumberKwd(size_t frameNo);
};


/// Define a list of wxFitsKeywords
//
class wxFitsKeywordList
{
private:
   // defines a list of keywords
   typedef std::vector<wxFitsKeyword> ArrayOfFitsKeywords;

public:
   wxFitsKeywordList();
   wxFitsKeywordList(const wxFitsKeyword &arg);
   wxFitsKeywordList(const wxFitsKeywordList &arg);
   ~wxFitsKeywordList();

   wxFitsKeywordList& operator+= (const wxFitsKeyword &arg);
   wxFitsKeywordList operator+ (const wxFitsKeyword &arg) const;

   wxFitsKeywordList& operator= (const wxFitsKeywordList &arg);
   wxFitsKeywordList& operator+= (const wxFitsKeywordList &arg);
   wxFitsKeywordList operator+ (const wxFitsKeywordList &arg) const;

   void Update(const wxFitsKeyword& kwd);

   // list ops
   typedef ArrayOfFitsKeywords::iterator iterator;
   const size_t size() const {return m_keywordList.size();};
   const wxFitsKeyword& operator[] (size_t _Pos) const {return m_keywordList[_Pos];};
   wxFitsKeyword& operator[] (size_t _Pos) {return m_keywordList[_Pos];};
   iterator begin() {return m_keywordList.begin();};
   iterator end() {return m_keywordList.end();};
   void clear() {m_keywordList.clear();};

private:
   ArrayOfFitsKeywords  m_keywordList;
};


#endif // _WXFITSKEYWORD_H__

