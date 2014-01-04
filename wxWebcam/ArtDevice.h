
#ifndef _ARTDEVICE_H_
#define _ARTDEVICE_H_

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Declare a ArtemisDevice
//
class ArtDrv;
class ArtBase;
class ArtDevice
{
// Allow initialization of content by owner class
friend class ArtBase;

public:
    // constructor inits the content as an invalid device (index = -1)
	ArtDevice();
	virtual ~ArtDevice();

	// The device index
	long DevIndex(void) const;
	// The friendly device name (any two cams of the same brand have the same name)
	const wxString& DevName(void) const;
	// The MS serial assigned (serves to distinguis between cams of the same brand)
	const wxString& DevSerial(void) const;

   void ControlDevice(bool pCamDrvOK);
   bool IsDeviceControlled() const;
	ArtDrv* PCamDriver() const;

private:
	// hidden from general access, only the provider can use it
	void Init(long pFIndex, const wxString& pCName, const wxString& pDevSerial, ArtDrv* pCamDriver);
	// Capture Device attribute storage
	struct SDevEntry
	{
		SDevEntry(){ Init();}
		void Init(){ s_CIndex=-1; s_CName.Clear(); s_DevSerial.Clear(); s_pCamDriver=NULL;}

		long        s_CIndex;      // The camera index of the entry
		wxString    s_CName;       // The cameras friendly name - show this one
		wxString    s_DevSerial;   // A serial number if we have to cams with the same name
      ArtDrv*     s_pCamDriver;  // the attached driver if this is the controlled entry
	};

	SDevEntry   m_DevEntry;       // the real storage of the cam name etc.
   bool        m_camControlled;
};



#endif // _ARTDEVICE_H_
