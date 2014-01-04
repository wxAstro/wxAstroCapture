

#ifndef _dscaplib_H_
#define _dscaplib_H_


// Folgender ifdef-Block ist die Standardmethode zum Erstellen von Makros, die das Exportieren
// aus einer DLL vereinfachen. Alle Dateien in der DLL werden mit dem DSCAPLIB_EXPORTS-Symbol
// kompiliert, das in der Befehlszeile definiert wurde. Das Symbol darf nicht für ein Projekt definiert werden,
// das diese DLL verwendet. Alle anderen Projekte, deren Quelldateien diese Datei beinhalten, erkennen
// DSCAPLIB_API-Funktionen als aus einer DLL importiert, während die DLL mit diesem Makro
// definierte Symbole als exportiert ansieht.
#ifdef DSCAPLIB_EXPORTS
#define DSCAPLIB_API __declspec(dllexport)
#else
#define DSCAPLIB_API __declspec(dllimport)
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "captypes.h"

// Forwards from internal classes
class CDSstream;
class CDSwcProp;
class CDScapFilterEnum;

// Forwards from this file
class CDScapFilterItem;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MAIN exposed CapLibrary object
// Allocate one and use it - then dispose it properly
class DSCAPLIB_API DScapLib
{
public:
	DScapLib();
	virtual ~DScapLib();

public:
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Exposes the Capture Device Enumeration methods
	bool  EnumerateCapFilter();
	long  NumItems();
	const CDScapFilterItem& GetCapFilter(long cfIndex);


public:
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Exposes a capture stream connected to a specific capture device
	class DSCAPLIB_API CWDSstream
	{
	public:
		CWDSstream();
		~CWDSstream();

	public:
		// Interface

		// create a new capture graph connecting to the capture device submitted
		// submit a window handle which receives the video output
		bool CreateNew(const HANDLE newVal, const CDScapFilterItem* pCapFilter);

		// query the current video frame dimension
		void FrameDimension(long &width, long &height) const;
		// Set the Format of the Capture OutPin (if supported) according to a binning level
		// i.e. level 1 = 640x480; 2 = 320x240; 4 = 160x120; 5 = 352x288 cropped
		bool SetFormat(long binning);
		// make the video output to match the hWnd size of the used window (from CreateNew)
		// call it whenever the video should be readjusted within the control window
		void ScaleVideoOutput();
		// scale the real streaming image (source image xStart etc.) to actual videoWindow rectangle
		void ScaleVideoOutput(long xStart, long xEnd, long yStart, long yEnd);

		void VideoOutputVisible(bool enable);

		//*** capture graph control

		// query the status of the object
		// returns the GetState() result of the graph
		long Status() const;

		// set graph to pause
		bool Pause();
		// set graph to stop
		bool Stop();
		// set graph to run
		bool Run();


		//*** capture handling

		// init and capture images - calls back after countdown # images
		bool CaptureStream(bool enabled, long countdown);
		// set the subframe format (binning factor 1 assumed)
		//  needs two points of a rectangle that makes the subframe in pixel coords
		//  the subframe might get adjusted due to alignment needs
		bool SetSubframe(long &startX, long &startY, long &width, long &height);
		bool SetImageBuffer(void* pBGR, long sizeBGR, void* pYUV, long sizeYUV);

		// query the current type of streaming
		EGraphStreamingType CaptureStatus() const;


		// The app model is either to provide a Wnd Handle that receives WM_CAPTURE
		// events which in turn must trigger the call of OnCapture()
		// If no EventSink is given, the app must poll OnCaptureRequired() and
		// call OnCapture() when it returns true.
		// This flow is needed because the DS Filter must not do any dataprocessing but
		// return asap - so any further capture handling is performed in the context
		// of the application when calling OnCapture()
		// OnCapture returns 0 if a new sample must be collected
		// positive numbers are capture events left until finished
		// negative numbers are error indications

		// submit a window handle to receive the Post message WM_CAPTURE
		bool put_EventSink(HANDLE newVal);
		// query the used window handle
		const HANDLE EventSink() const;

		// returns true if a an OnCapture call is expected
		// supports non event driven apps i.e. EventSink is not provided
		ESYNMode OnCaptureRequired();

		// must be called to gather the sampled image
		// either via WM_CAPTURE handler or polled via OnCaptureRequired
		//
		// OnCapture returns 0 if a new sample must be collected
		// positive numbers are capture events left until finished
		// negative numbers are error indications
		long OnCapture();

		// returns the captured image type and memptr
		// NOTE: the sample is allocated and will be released with the object
		bool CapturedSample(CapSampleType &sample);
		// just reply to ignore that sample and continue
		void DropCapturedSample();

		// used to apply a dark image for collecting mode
	//	bool put_DarkImage(IDispatch* newVal);



		//*** control the DS capture filters behavior
		// query the current conversion type
		const EVideoConversion WCFConversion();
		// set a new conversion type
		void put_WCFConversion(EVideoConversion newVal);
		// query the current pixel order (for raw only)
		const EVideoPixelOrder PixelOrder();
		// set the new pixel order (for raw only)
		void put_PixelOrder(EVideoPixelOrder newVal);
		// get the current FPS/DPS values  //20100702:BM - added
		bool GetRates(float& fps, float& dps);

		// clear the graphics items
		void Clear();
		// Sets the current pen for drawing (primitive pen...)
		void SetPen(unsigned long rgbColor, unsigned short width);
		// Draws a circle with the given centre and radius.
		bool DrawCircle(short x, short y, short radius);
		// Draws a rectangle with the given top left corner, and with the given size.
		// The current pen is used for the outline (no filling)
		bool DrawRectangle(short x, short y, short width, short height);
		// Draws a line from the first point to the second.
		// The current pen is used for drawing the line.
		// Note that the point (x2, y2) is not part of the line and is not drawn by this function
		bool DrawLine(short x1, short y1, short x2, short y2);


		// provide access to the manufacturers dialogs
		bool ShowSourceDlg(long hwndParent);
		bool ShowFormatDlg(long hwndParent);

	private:
		CDSstream* m_dsStream; // the internal stream object
	};


public:
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Exposes a capture device property handler
	class DSCAPLIB_API CWDSwcProp
	{
	public:
		CWDSwcProp();
		virtual ~CWDSwcProp();

	public:
		bool put_CaptureFilter(const CDScapFilterItem* pCapFilter);

		const EKnownChipsetCat Chipset() const;
		const EVidCapPropertySupport Has(EVidCapProperty Property) const;
		long Prop(EVidCapProperty Property) const;
		bool Range(EVidCapProperty Property, long &pMin, long &pMax, long &pSteppingDelta);
		bool put_Prop(EVidCapProperty Property, long newVal);

		bool ShowSourceDlg(long hwndParent);
		bool ShowFormatDlg(long hwndParent);

	private:
		CDSwcProp*	m_wcProp;	// the internal property handler object
	};

public:
	bool PROP_INIT();
	DScapLib::CWDSwcProp& PROP() {return *m_pProp;};
	bool STREAM_INIT();
	DScapLib::CWDSstream& STREAM() {return *m_pStream;};

private:
	HRESULT m_coResult;
	CDScapFilterEnum* m_pCapFilterEnumerator; // THE ENUMERATOR
	DScapLib::CWDSwcProp*		  m_pProp;
	DScapLib::CWDSstream*		  m_pStream;

}; //class DScapLib

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Encapsulates a CaptureDevice Item
// Get it from the enumerator and use it to connect
//   the stream and/or the property handler
class DSCAPLIB_API CDScapFilterItem
{
friend class CDScapFilterEnum; // an internal class has all access granted

public:
	CDScapFilterItem();
	virtual ~CDScapFilterItem();

// IVCapFilter
public:
	CDScapFilterItem(const CDScapFilterItem& other);
	CDScapFilterItem& operator= (const CDScapFilterItem& other);
	bool operator== (const CDScapFilterItem& other) const;

	// just expose the data members
	const wchar_t* ClsID ()  const {return m_ClsID;};
	const wchar_t* FName ()  const {return m_FriendlyName;};
	const wchar_t* DevSerial ()  const  {return m_devSerial;};
	const wchar_t* DeviceName ()  const {return m_DeviceName;};
	const wchar_t* DeviceVidPid ()  const {return m_DeviceVidPid;};

	const char* ClsIDA ()  const {return m_ClsIDA;};
	const char* FNameA ()  const {return m_FriendlyNameA;};
	const char* DevSerialA ()  const  {return m_devSerialA;};
	const char* DeviceNameA ()  const {return m_DeviceNameA;};
	const char* DeviceVidPidA ()  const {return m_DeviceVidPidA;};

	const long FIndex ()  const {return m_FIndex;};
	void* BaseFilter() const {return m_pBaseFilter;};

	const GUID& PixFormat()  const {return m_mediaT;};
    unsigned long FrameSize_Width()  const {return m_width;};
    unsigned long FrameSize_Height()  const {return m_height;}; // return the abs height of the frame
    unsigned long Bitmap_Width()  const {return m_width;};
    unsigned long Bitmap_Height()  const {return m_height;};	 // return the original bitmap height (can be negative)
    unsigned long Fps_Num()  const {return m_num;};
    unsigned long Fps_Denom()  const {return m_denom;};


private:
	void put_ClsID (const wchar_t* newVal, long slen);
	void put_FName (const wchar_t* newVal, long slen);
	void put_DevSerial (const wchar_t* newVal, long slen);
	void put_DeviceName (const wchar_t* newVal, long slen);
	void put_DeviceVidPid (const wchar_t* newVal, long slen);
	void put_FIndex (long newVal);
	void put_BaseFilter(void* newVal);

	void put_PixFormat (const GUID& mediaT);
	void put_Bitmap_Size (unsigned long width, unsigned long height);
	void put_Fps (unsigned long num, unsigned long denom);

	bool IsSameFriendlyName(const CDScapFilterItem& other) const;
	bool IsSameMediaFormat(const CDScapFilterItem& other) const;
	bool IsSamePixFormat(const CDScapFilterItem& other) const;
	bool IsSameFrameSize(const CDScapFilterItem& other) const;
	bool IsSameFrameFps(const CDScapFilterItem& other) const;

	void Clear();

private:
	long			m_FIndex;
	wchar_t*		m_FriendlyName;
	wchar_t*		m_DeviceName;
	wchar_t*		m_DeviceVidPid;
	wchar_t*		m_ClsID;
	wchar_t*		m_devSerial;

	char*			m_FriendlyNameA;
	char*			m_DeviceNameA;
	char*			m_DeviceVidPidA;
	char*			m_ClsIDA;
	char*			m_devSerialA;

	void*			m_pBaseFilter;

	GUID		    m_mediaT;
    unsigned long   m_width;
    unsigned long   m_height;
    unsigned long   m_num;
    unsigned long   m_denom;

};// class CDScapFilterItem

#endif  //_dscaplib_H_
