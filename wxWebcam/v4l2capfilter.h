// CV4L2capFilter.h : Declaration of the CV4L2capFilter
//
//---------------------------------------------------------------------------

#ifndef __CV4L2capFilter_H_
#define __CV4L2capFilter_H_


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if defined(__UNIX__)

#include <linux/videodev2.h>


/////////////////////////////////////////////////////////////////////////////
// CV4L2capFilterItem:  one capture filter object
class CV4LcapFilterItem
{
friend class CV4LcapFilter; // has all access granted

public:
	CV4LcapFilterItem();
	virtual ~CV4LcapFilterItem();

// IVCapFilter
public:
	CV4LcapFilterItem(const CV4LcapFilterItem& other);
	CV4LcapFilterItem& operator= (const CV4LcapFilterItem& other);
	bool operator== (const CV4LcapFilterItem& other) const;

	// just expose the data members
	const long FIndex ()  const {return m_FIndex;};
	const wxString& FName ()  const {return m_friendlyName;};
	const wxString& DevSerial ()  const  {return m_devSerial;};
	const wxString& DevPath ()  const {return m_devPath;};
   wxUint32 PixFormat()  const {return m_fourCC;};
   wxUint32 FrameSize_Width()  const {return m_width;};
   wxUint32 FrameSize_Height()  const {return m_height;};
   wxUint32 Fps_Num()  const {return m_num;};
   wxUint32 Fps_Denom()  const {return m_denom;};

private:
	bool IsSimilar(const CV4LcapFilterItem& other) const;
	void put_FIndex (long newVal);
	void put_FName (const wxString &newVal);
	void put_DevSerial (const wxString &newVal);
	void put_DevPath (const wxString &newVal);

	void put_PixFormat (wxUint32 fourCC);
	void put_FrameSize (wxUint32 width, wxUint32 height);
	void put_Fps  (wxUint32 num, wxUint32 denom);
	void MergeFps (wxUint32 num, wxUint32 denom);

	void Clear();

private:
	long            m_FIndex;
	wxString        m_friendlyName;
	wxString        m_devPath;
	wxString        m_devSerial;

   wxUint32        m_fourCC;
   wxUint32        m_width;
   wxUint32        m_height;
   wxUint32        m_num;
   wxUint32        m_denom;
};

	/////////////////////////////////////////////////////////////////////////////
// Singleton Instance helper
#define CAPTURE_ENUMERATOR( vname )  CV4LcapFilter& (vname) = CV4LcapFilter::Instance()

/////////////////////////////////////////////////////////////////////////////
// CV4L2capFilter implemented as Singleton
class CV4LcapFilter
{
public:
	static CV4LcapFilter& Instance();

	virtual ~CV4LcapFilter();

public:

	typedef CV4LcapFilterItem       *PCV4LcapFilterItem;


public:
	const CV4LcapFilterItem& GetCapFilter(long cfIndex) const;
	bool EnumerateCapFilter();
	long NumItems() {return m_numFilter;};


private:
	CV4LcapFilter();
	CV4LcapFilter(const CV4LcapFilter&);

	void DropAll();

   void EnumPixFormat(int v4l2_dev);
   void EnumFrameSize(int v4l2_dev, const v4l2_fmtdesc& fmt);
   void EnumFPS(int v4l2_dev, const v4l2_frmsizeenum& fsize);

   void AddSupport(int v4l2_dev, const v4l2_frmivalenum& fival);

   bool IsSupportedPixFormat(const v4l2_fmtdesc& fmt);
   bool IsSupportedFrameSize(const v4l2_frmsizeenum& fsize);
   bool IsSupportedFps(const v4l2_frmivalenum& fival);

	enum MAXLIST {MAXITEM = 50}; // bug workaround
	long                m_numFilter;
	PCV4LcapFilterItem  m_pFilters[MAXITEM];
	CV4LcapFilterItem   m_nullFilter;

	wxString            m_actualDevPath;
	v4l2_capability     m_actualCap;
};

#endif // defined __UNIX__

#endif //__CV4L2capFilter_H_
