
#include "PortEnumW32.h"

#include <setupapi.h>
#include <winreg.h>
#include <cfgmgr32.h>

#include "wxPort.h"
#include "ShoestringVidPid.h"


#pragma comment (lib,"setupapi.lib")


/*
 * All hardware I/O resources combined to return data to user.
 */
typedef struct
{
    USHORT min;
    USHORT max;
} DEVICE_IO;

typedef struct
{
    ULONG num_io_ranges;
    DEVICE_IO  io;
} DEVICE_RESOURCES;


typedef PVOID HDEVINFO;

typedef struct
{
    HDEVINFO  class_handle;
    SP_DEVINFO_DATA device;
} DEVICE_DATA;


// SETUPAPI LIB

static BOOL GetNumIODeviceClasses( ULONG* num_classes )
{
    BOOL status;  BOOL error = FALSE;
    ULONG class_list_size;

    if( NULL == num_classes )
        return FALSE;

    status = ::SetupDiBuildClassInfoList( 0, NULL, 0, &class_list_size );
    if( (!status) && (ERROR_INSUFFICIENT_BUFFER != ::GetLastError()) ) {
        error = TRUE;
        class_list_size = 0;
    }

    // Pass data back.
    *num_classes = class_list_size;
    return (!error);
}//GetNumIODeviceClasses


static BOOL GetAllIODeviceClassGuids( GUID** guid_list, ULONG* num_classes )
{
    ULONG class_list_size = 0;
    BOOL status;  BOOL error = FALSE;
    GUID * class_list = NULL;
    ULONG dummy_size;

    // Get number of classes.
    status = ::GetNumIODeviceClasses( &class_list_size );
    if( !status )
        return FALSE;
    // Set up array to hold list of classes.
    class_list = (GUID *) ::malloc( sizeof(GUID) * class_list_size );
    if( NULL == class_list ) {
        error = TRUE;
    }
    else {
        status = ::SetupDiBuildClassInfoList( 0, class_list, class_list_size, &dummy_size );
        if( !status ) {
            error = TRUE;
            ::free( class_list );
        }
    }
    // Pass data back.
    if( error ) {
        *guid_list = NULL;
        *num_classes = 0;
        return FALSE;
    }
    else {
        *guid_list = class_list;
        *num_classes = class_list_size;
        return TRUE;
    }
}// GetAllIODeviceClassGuids


static BOOL GetIODeviceClassNameFromGuidA( GUID* guid, char** name, ULONG* name_len )
{
    BOOL status;
    char * class_name = NULL;
    ULONG class_len = 0; ULONG actual_name_length = 0;

    // Get class name length.
    status = ::SetupDiClassNameFromGuidA( guid, NULL, 0, &class_len );
    if( (!status) && (ERROR_INSUFFICIENT_BUFFER != GetLastError()) )
        return FALSE;

    class_name = (char *) malloc( sizeof(char) * class_len );
    if( NULL == class_name )
        return FALSE;
    // Array is set up, so get class name.
    status = ::SetupDiClassNameFromGuidA( guid, class_name, class_len, &actual_name_length );
    if( (!status) || (actual_name_length > class_len) ) {
        ::free( class_name );
        return FALSE;
    }

    // Pass back data.
    *name = class_name;
    *name_len = actual_name_length;
    return TRUE;
}//GetIODeviceClassNameFromGuidA


static BOOL CheckIODeviceClassNameMatchA( GUID* guid,
                        char* match_name, BOOL match_start, BOOL* matched )
{
   char * class_name = NULL;
   ULONG class_len; ULONG match_len;
   BOOL status;
   char * class_flyer; char * match_flyer;
   BOOL matched_so_far; ULONG count; BOOL done;

   if( (NULL == guid) || (NULL == match_name) || (NULL == matched) )
      return FALSE;

   // Get length of name to match (including null-terminator).
   match_len = strlen( match_name );
   // Get class name.
   status = GetIODeviceClassNameFromGuidA( guid, &class_name, &class_len );
   if( !status )
      return FALSE;

   if( (!match_start) && (class_len != match_len) ) {
    // If we need an exact match and the names are not the same length, we know they won't match.
      matched_so_far = FALSE;
   }
   else {
      class_flyer = class_name; match_flyer = match_name;
      count = match_len; done = FALSE; matched_so_far = TRUE;

      while( !done ) {
         if( tolower(*class_flyer) != tolower(*match_flyer) ) {
            // Strings don't match here.
            matched_so_far = FALSE;
            done = TRUE;
         }
         // Increment position.
         count --; class_flyer ++; match_flyer ++;
         if( count == 0 ) {
           // Reached end of names.
           done = TRUE;
         }
      }
   }//else
   // Free memory for class name array.
   ::free( class_name );
   // Pass back data.
   *matched = matched_so_far;
   return TRUE;
}//CheckIODeviceClassNameMatchA


static BOOL GetIODeviceClassGuidFromNameA( char * class_name, GUID * guid, BOOL match_start )
{
    GUID * guid_list = NULL;
    ULONG num_classes;
    BOOL status;
    ULONG count;
    BOOL found = FALSE; BOOL error = FALSE; BOOL done;

    if( (NULL == class_name) || (NULL == guid) )
        return FALSE;

    // Get list of all GUIDs.
    status = GetAllIODeviceClassGuids( &guid_list, &num_classes );
    if( !status ) {
        error = TRUE;
    }
    else
    {   // Search GUID list for class with matching name.
        count = 0;
        done = FALSE;
        while( !done ) {
            status = CheckIODeviceClassNameMatchA( &(guid_list[count]),
                                                    class_name, match_start, &found );
            if( status && found ) {
                // Matched class.
                done = TRUE;
            }
            else {
                // Move to next element.
                count ++;
                if( count >= num_classes ) {
                    done = TRUE;
                }
            }
        }//while
        if( found && (!error) ) {
            // If we've found the class, copy the GUID across.
            ::memcpy( guid, &(guid_list[count]), sizeof(GUID) );
        }
        else {
            // Could not find name, which is an error.
            error = TRUE;
        }
    }
    // Free GUID list if needed.
    if( NULL != guid_list ) {
        ::free( guid_list );
    }
    return (!error);
}//GetIODeviceClassGuidFromNameA




static BOOL GetDeviceClassHandleA( char* class_name, HANDLE* class_handle )
{
    BOOL status;
    GUID class_guid;

    // Get device class GUID.
    status = GetIODeviceClassGuidFromNameA( class_name, &class_guid, TRUE );
    if( !status )
        return FALSE;

    // Get handle to class.
    *class_handle = ::SetupDiGetClassDevs( &class_guid, NULL, NULL, DIGCF_PRESENT );
    if( NULL == *class_handle )
        return FALSE;
    // Otherwise handle was read OK.
    return TRUE;
}//GetDeviceClassHandleA


static BOOL GetNumDevices( HDEVINFO class_handle, ULONG* num_devices )
{
    SP_DEVINFO_DATA new_device_data;
    BOOL done = FALSE; BOOL error = FALSE; BOOL status;
    DWORD error_val; DWORD count = 0;

    do {
        // Get data for latest device.
        new_device_data.cbSize = sizeof(SP_DEVINFO_DATA);
        status = ::SetupDiEnumDeviceInfo( class_handle, count, &new_device_data );
        if( !status ) {
            // Could not read device, so find why not.
            error_val = ::GetLastError();
            if( ERROR_NO_MORE_ITEMS == error_val ) {
                done = TRUE;
            }
            else {
                error = TRUE;
            }
        }
        else {
            count ++;
        }
    } while( (!done) && (!error) );

    // Return result.
    *num_devices = count;
    return (!error);
}//GetNumDevices


static BOOL GetNumIODevicesA( char* class_name, ULONG* num_devices )
{
    HANDLE class_handle;
    BOOL status;

    if( (NULL == class_name) || (NULL == num_devices) )
        return FALSE;

    // Get handle to device class.
    status = GetDeviceClassHandleA( class_name, &class_handle );
    if( !status )
        return FALSE;
    // Get number of devices in class.
    status = GetNumDevices( class_handle, num_devices );
    if( !status )
        return FALSE;

    return TRUE;
}//GetNumIODevicesA


static BOOL GetDeviceInfo( HDEVINFO class_handle, ULONG device_index, SP_DEVINFO_DATA* device_info )
{
    SP_DEVINFO_DATA new_device_data;
    BOOL status;

    // Get data for specified device.
    new_device_data.cbSize = sizeof(SP_DEVINFO_DATA);
    status = ::SetupDiEnumDeviceInfo( class_handle, device_index, &new_device_data );
    if( !status )
        return FALSE;
    // Pass back data.
    ::memcpy( device_info, &new_device_data, sizeof(SP_DEVINFO_DATA) );
    return TRUE;
}//GetDeviceInfo


static BOOL GetIODeviceA( char* class_name, ULONG device_index, DEVICE_DATA* device_data )
{
    HDEVINFO c_handle;
    BOOL status;

    if( (NULL == class_name) || (NULL == device_data) )
        return FALSE;
    // Get handle to device class.
    status = GetDeviceClassHandleA( class_name, &c_handle );
    if( !status )
        return FALSE;
    // Get device info.
    device_data->device.cbSize = sizeof(SP_DEVINFO_DATA);
    status = GetDeviceInfo( c_handle, device_index, &(device_data->device) );
    if( !status )
        return FALSE;
    // Else device data read OK.
    device_data->class_handle = c_handle;
    return TRUE;
}//GetIODeviceA


static BOOL GetIODeviceRegistryValueA( DEVICE_DATA* ddata, char* reg_entry_name, void* buffer, ULONG* buffer_size )
{
    LONG status; HKEY key;

    if( (NULL == ddata) || (NULL == reg_entry_name) || (NULL == buffer) || (NULL == buffer_size) )
        return FALSE;
    // Get key (handle) to registry directory for device.
    key = ::SetupDiOpenDevRegKey( ddata->class_handle, &(ddata->device), DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ );
    if( INVALID_HANDLE_VALUE == key ) {
        return FALSE;
    }
    else {
        // Key read OK, so get registry entry.
        status = ::RegQueryValueExA( (HKEY)key, reg_entry_name, 0, NULL, (LPBYTE)buffer, buffer_size );
        (void) ::RegCloseKey( key );
        if( ERROR_SUCCESS == status ) {
            return TRUE;
        }
        else {
            return FALSE;
        }
    }
}//GetIODeviceRegistryValueA

static BOOL GetDeviceLogConfig(DEVINST device_instance, LOG_CONF* log_config, BOOL* log_config_found )
{
    CONFIGRET cm_status;
    LOG_CONF new_lc = 0;
    BOOL found = FALSE; BOOL error = FALSE;

    if( (NULL == log_config) || (NULL == log_config_found) )
        return FALSE;

    // Check for forced configuration first, as the highest priority log config possible.
    cm_status = ::CM_Get_First_Log_Conf( NULL, device_instance, FORCED_LOG_CONF );
    if( CR_SUCCESS == cm_status ) {
        // If a log config exists, read it back.
        // This should always succeed, as we've already checked that one exists.
        cm_status = ::CM_Get_First_Log_Conf( &new_lc, device_instance, FORCED_LOG_CONF );
        if( CR_SUCCESS != cm_status ) {
            error = TRUE;
        }
        else {
            found = TRUE;
        }
    }
    else {
        // If no forced configuration, check for a boot configuration as the next highest priority log config.
        cm_status = ::CM_Get_First_Log_Conf( NULL, device_instance, BOOT_LOG_CONF );
        if( CR_SUCCESS == cm_status ) {
            cm_status = ::CM_Get_First_Log_Conf( &new_lc, device_instance, BOOT_LOG_CONF );
            if( CR_SUCCESS != cm_status ) {
                error = TRUE;
            }
            else {
                found = TRUE;
            }
        }
        else {
            // If no boot configuration, check for an allocated configuration as the lowest priority log config.
            cm_status = ::CM_Get_First_Log_Conf( NULL, device_instance, ALLOC_LOG_CONF );
            if( CR_SUCCESS == cm_status ) {
                // If a log config exists, read it back.  This should always
                // succeed, as we've already checked that one exists.
                cm_status = ::CM_Get_First_Log_Conf( &new_lc, device_instance, ALLOC_LOG_CONF );
                if( CR_SUCCESS != cm_status ) {
                    error = TRUE;
                }
                else {
                    found = TRUE;
                }
            }
            else {
                // No log config found.
                found = FALSE;
            }
        }
    }

    // Pass back result.
    *log_config = new_lc;
    *log_config_found = found;
    return (!error);
}//GetDeviceLogConfig


static BOOL AddResDesToList( RES_DES** rd_list, ULONG* num_rds, RES_DES new_rd )
{
    ULONG total_rds;
    RES_DES * new_rds;

    if( (NULL == rd_list) || (NULL == num_rds) )
        return FALSE;
    // Allocate new list of rds.
    total_rds = (*num_rds) + 1;
    new_rds = (RES_DES *) ::malloc( sizeof(RES_DES) * total_rds );
    if( NULL == new_rds )
        return FALSE;

    if( NULL != *rd_list ) {
        // If old list existed, copy old list into new list.
        ::memcpy( new_rds, *rd_list, (sizeof(RES_DES) * (*num_rds)) );
        ::free( *rd_list );
    }
    // Get resources used.
    new_rds[*num_rds] = new_rd;

    *rd_list = new_rds;
    *num_rds = total_rds;
    ::SetLastError( ERROR_SUCCESS );
    return TRUE;
}//AddResDesToList


static BOOL FreeResDesList( RES_DES * rd_list, ULONG num_rd )
{
    CONFIGRET free_status;
    BOOL error = FALSE;
    ULONG count;  RES_DES * ptr;

    if( 0 == num_rd )
        return TRUE;
    if( NULL == rd_list )
        return FALSE;
    // Free all stored resource descriptor handles.
    ptr = rd_list;
    for( count = 0; count < num_rd; count ++ ) {
        free_status = ::CM_Free_Res_Des_Handle( *ptr );
        ptr ++;
        if( CR_SUCCESS != free_status ) {
            error = TRUE;
        }
    }

    // Deallocate array.
    ::free( rd_list );
    return (!error);
}// FreeResDesList


static BOOL AddIoResToRange( DEVICE_IO* ranges, ULONG * num_ranges, IO_RESOURCE* io_res )
{
    *num_ranges = 1;
    // Get resources used.
    ranges->min = (USHORT) io_res->IO_Header.IOD_Alloc_Base;
    ranges->max = (USHORT) io_res->IO_Header.IOD_Alloc_End;
    return TRUE;
}//AddIoResToRange


static BOOL GetIoUsed( LOG_CONF log_config, DEVICE_IO* ranges, ULONG* num_ranges )
{
   CONFIGRET cm_status;
   BOOL done;  BOOL error = FALSE;  BOOL free_error = FALSE;
   RES_DES last_rd; RES_DES new_rd;
   unsigned char * buf = NULL;
   ULONG buf_size = 0; ULONG data_size;
   IO_RESOURCE *io_res;
   RES_DES * rd_list = NULL; ULONG num_rd = 0;

   if( (NULL == ranges) || (NULL == num_ranges) )
     return FALSE;

   // Prepare loop, looking for first resource descriptor for this log config.
   last_rd = log_config;
   done = FALSE;
   // Get next resource descriptor for this log config.
   cm_status = ::CM_Get_Next_Res_Des( &new_rd, last_rd, ResType_IO, NULL, 0 );
   if( CR_SUCCESS != cm_status ) {
      // Couldn't get resource descriptor, so check why.  If no more
      // resource descriptors, this is OK.
      if( CR_NO_MORE_RES_DES != cm_status ) {
          error = TRUE;
      }
      done = TRUE;
   }
   else {
      // Got resource descriptor, so add to list, to be freed later.
      error = !AddResDesToList( &rd_list, &num_rd, new_rd );
      if( !error ) {
          // If added to list OK, start checking for data.
          // Get size of buffer required for resource data.
          cm_status = ::CM_Get_Res_Des_Data_Size( &data_size, new_rd, 0 );
          if( CR_SUCCESS != cm_status ) {
              error = TRUE;
          }
          else {
              // Allocate memory for buffer if required.
              if( data_size > buf_size ) {
                  // Deallocate current buffer if exists.
                  if( NULL != buf ) {
                      ::free( buf );
                  }
                  // Allocate new buffer of required size.
                  buf = (unsigned char *) malloc( data_size );
                  buf_size = data_size;
              }
              if( NULL == buf ) {
                  error = TRUE;
              }
              else {
                  // Get resource data to buffer.
                  io_res = (IO_RESOURCE *) buf;
                  cm_status = ::CM_Get_Res_Des_Data( new_rd, io_res, data_size, 0 );
                  if( CR_SUCCESS != cm_status ) {
                      error = TRUE;
                  }
                  else {
                      // If OK so far, decode resource data and add to array.
                      error = !AddIoResToRange( ranges, num_ranges, io_res );
                  }
              }
          }
      }
   }
   // Free memory.
   free_error = !FreeResDesList( rd_list, num_rd );
   error = error || free_error;
   ::free( buf );

   if( error ) {
      num_ranges = 0;
   }
   return (!error);
}//GetIoUsed


static BOOL GetIODeviceResources( DEVICE_DATA* device_data, DEVICE_RESOURCES* resources )
{
    BOOL status;
    LOG_CONF log_config;
    BOOL log_config_found;
    BOOL io_status;

    if( (NULL == device_data) || (NULL == resources) )
        return FALSE;
    // Get log config used for device.
    status = GetDeviceLogConfig( device_data->device.DevInst, &log_config, &log_config_found );
    if( !status )
        return FALSE;

    if( !log_config_found ) {
        // No resources used.
        resources->num_io_ranges = 0;
        return TRUE;
    }
    // Get resources used.
    io_status = GetIoUsed( log_config, &(resources->io), &(resources->num_io_ranges) );
    if( !io_status )
        return FALSE;
    // Otherwise resources have been got OK.
    return TRUE;
}// GetIODeviceResources


static void FreeIODevice( DEVICE_DATA* device )
{
    if( NULL == device )
        return;
    (void) ::SetupDiDestroyDeviceInfoList( device->class_handle );
}//FreeIODevice


////////////////////////////////////////////////////////////////////////////////////////////
// class implementation

// The Enumerator Singleton

// Singleton
PortEnumW32& PortEnumW32::Instance()
{
	static PortEnumW32 instance;
	return instance;
}


PortEnumW32::PortEnumW32()
: m_numPorts(0)
{
	for (int i=0; i<MAXITEM; i++)
	{
		m_pPorts[i] = NULL;
	}
	m_nullPort = new wxPortDescr();
	// just start with all enumerated
	EnumeratePorts();
}

PortEnumW32::~PortEnumW32()
{
	DropAll();
	delete m_nullPort;
}

void PortEnumW32::DropAll()
{
	for (int i=0; i<MAXITEM; i++)
	{
		if (m_pPorts[i])
			delete m_pPorts[i]; m_pPorts[i] = NULL;
	}
	m_numPorts = 0;
}

const wxPortDescr* PortEnumW32::GetPort(size_t cfIndex) const
{

	if (cfIndex>=m_numPorts)
	{
		return m_nullPort;
	}

	return m_pPorts[cfIndex];
}//GetPort

// (Re)-Enumerate the Port devices
bool PortEnumW32::EnumeratePorts()
{
   DWORD  port_items = 0;

	DropAll();

   // Dummy Port
   m_pPorts[port_items] = new wxPortDescr();
   m_pPorts[port_items]->Init(port_items, wxT("!None!"), 0, wxT("n.a."),
                              true, true, false,false,false,false,false);//Gx and Lx supported (to show it)
   port_items++;

   EnumParallel(port_items);
   EnumGPUSB(port_items);
   EnumLXUSB(port_items);
   EnumSerial(port_items);

   m_numPorts = port_items;
   return true;
}

// (Re)-Enumerate the Port devices
void PortEnumW32::EnumParallel(unsigned long& port_items)
{
   BOOL status;
   DWORD  num_items = 0;
   ULONG port_num;
   DEVICE_RESOURCES resources;
   DEVICE_DATA device;

   status = GetNumIODevicesA( "Ports", &num_items );
   // parallel
   if( status && (num_items > 0) ) {
      for( port_num = 0; port_num < num_items; port_num ++ ) {
         status = GetIODeviceA( "Ports", port_num, &device );
         if( status ) {
            char short_name[100];
            ULONG actual_short_name_len = 100;
            status = GetIODeviceRegistryValueA( &device, "PORTNAME", short_name, &actual_short_name_len );
            status = GetIODeviceResources( &device, &resources );
            if( status ) {
               bool par = (strncmp("LPT", short_name, 3)==0);
               if ( par && resources.num_io_ranges ) {
                  wxString pNam;
                  pNam.Printf(wxT("%s (0x%x)"),short_name, resources.io.min);
                  m_pPorts[port_items] = new wxPortDescr();
                  m_pPorts[port_items]->Init(port_items, pNam, resources.io.min, short_name, true,  true, // +guide +LX
                                                                        false, true, false, false, false);
                  port_items++;
               }
            }
            FreeIODevice( &device );
         }
      }
   }
}


// much easier and catches virtual ports too !!
void PortEnumW32::EnumSerial(unsigned long& port_items)
{
HKEY hkCommMap;

	if (ERROR_SUCCESS != ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
         TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"), 0, KEY_QUERY_VALUE, &hkCommMap))
   {
      wxASSERT(false); return;
	}

	void* pValNameBuff = 0;
	void* pValueBuff = 0;

   DWORD dwValCount, dwMaxCharValNameLen, dwMaxByteValueSize;

   if (ERROR_SUCCESS != ::RegQueryInfoKey(hkCommMap, NULL, NULL, NULL, NULL, NULL, NULL,
         &dwValCount, &dwMaxCharValNameLen, &dwMaxByteValueSize, NULL, NULL))
   {// regkey enum failed !!
		::RegCloseKey(hkCommMap);
      wxASSERT(false);
		return;  // ERROR EXIT
   }

   // The max value name size is returned in TCHARs not including the terminating null character.
	dwMaxCharValNameLen++;
   pValNameBuff = new TCHAR[dwMaxCharValNameLen];
   if (!pValNameBuff) {// no memory !!
		::RegCloseKey(hkCommMap);
      wxASSERT(false); return; // ERROR EXIT
   }
   // The max needed data size is returned in bytes
   dwMaxByteValueSize+=sizeof(TCHAR);
   DWORD dwMaxCharValueLen = (dwMaxByteValueSize/2) * sizeof(TCHAR); // num of TCHARS
   pValueBuff = new TCHAR[dwMaxCharValueLen];
   if (!pValueBuff) { // no memory !!
		::RegCloseKey(hkCommMap); delete pValNameBuff;
      wxASSERT(false); return; // ERROR EXIT
   }

   for (DWORD dwIndex = 0; dwIndex < dwValCount; ++dwIndex) {
      DWORD dwCharValNameSize = dwMaxCharValNameLen;
      DWORD dwByteValueSize   = dwMaxCharValueLen*sizeof(TCHAR);;
      DWORD dwType;
      LONG nRes = ::RegEnumValue(hkCommMap,
                     dwIndex, (LPTSTR)pValNameBuff,&dwCharValNameSize, NULL,
                     &dwType, (LPBYTE)pValueBuff, &dwByteValueSize);
      if (nRes != ERROR_SUCCESS) {
         break; // no more
      }
      if (dwType != REG_SZ) {
         continue; // not expected type - try next
      }
      // now we have name and value in the buffers (TCHAR)
      wxString short_name((LPCTSTR)pValueBuff);
      m_pPorts[port_items] = new wxPortDescr();
      m_pPorts[port_items]->Init(port_items, short_name, 0, short_name, true,  true, // +guide +LX
                                                                        true, false, false, false, false);

      port_items++;
   }//for

   // clean up
	::RegCloseKey(hkCommMap);
   delete pValNameBuff;
   delete pValueBuff;
}

// look for the GPUSB HID device
void PortEnumW32::EnumGPUSB(unsigned long& port_items)
{
BOOLEAN status;

GUID hGuid;
HDEVINFO hDevInfo;
SP_DEVINFO_DATA devInfoData;

DWORD device_index;
DWORD dataSize;
PTSTR propBuffer=NULL;


   // Get device class GUID.
   status = GetIODeviceClassGuidFromNameA( "HIDClass", &hGuid, TRUE );
   if( status == 0 ) return; // no such GUID ..

   // get a devinfo list of the corresponding devices
   hDevInfo = ::SetupDiGetClassDevs(&hGuid, NULL, NULL, DIGCF_PRESENT);

   device_index = 0;		/* init to first index of array */
   devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

   while(status != 0) {
      // Get information about the device with the 'device_index' array entry
      status = ::SetupDiEnumDeviceInfo(hDevInfo, device_index, &devInfoData);
      if(status == 0) {
         // free the memory allocated for DetailData */
         if(propBuffer != NULL) ::free(propBuffer); propBuffer=NULL;
         /* free HID device info list resources */
         ::SetupDiDestroyDeviceInfoList(hDevInfo);
         return;
      }
      // retrieve the DeviceInstanceId i.e. HID\VID_04B4&PID_5A9B\8&1B307ACC&1&0000
      status = ::SetupDiGetDeviceInstanceId(hDevInfo, &devInfoData, NULL, 0, &dataSize); // get size info
      if ( dataSize > 0 ) {
         if(propBuffer != NULL) ::free(propBuffer); propBuffer=NULL;
         propBuffer = (PTSTR)::malloc(dataSize);
         // get with proper size now
         status = ::SetupDiGetDeviceInstanceId(hDevInfo, &devInfoData, propBuffer, dataSize, NULL);
         if ( status != 0) {
            // derive VID and PID
            wxString vid, pid, pNam;
            pNam.Printf(wxT("%s"),propBuffer); // get the first string e.g. HID\VID_04B4&PID_5A9B\8&1B307ACC&1&0000
            int pp=0;
            pp = pNam.Upper().Find(wxT("VID_")); if (pp!=wxNOT_FOUND ) vid = pNam.Mid(pp+4, 4).Upper();
            pp = pNam.Upper().Find(wxT("PID_")); if (pp!=wxNOT_FOUND ) pid = pNam.Mid(pp+4, 4).Upper();

//            /// ATTENTION - REMOVE THE IF BELOW FOR RELEASE !!!!!!!!
//            if ( 1 ) {
            if ( (vid==wxT(SSTRING_VENDOR_VID)) && (pid==wxT(SSTRING_GPUSB_PID)) ) {
               //  seems we have the proper USB device found
               m_pPorts[port_items] = new wxPortDescr();
               m_pPorts[port_items]->Init(port_items, wxT("GPUSB"), 0, pNam, true,  false, // +guide -LX
                                                                             false, false, true, false, false);
               port_items++;
            }
         }
      }
      device_index++;	// increment the array index to search the next entry
   }

   // free the memory allocated for DetailData */
   if (propBuffer != NULL) ::free(propBuffer);
   /* free HID device info list resources */
   ::SetupDiDestroyDeviceInfoList(hDevInfo);

	return;
}

// look for the LXUSB HID device
void PortEnumW32::EnumLXUSB(unsigned long& port_items)
{
BOOLEAN status;

GUID hGuid;
HDEVINFO hDevInfo;
SP_DEVINFO_DATA devInfoData;

DWORD device_index;
DWORD dataSize;
PTSTR propBuffer=NULL;


   // Get device class GUID.
   status = GetIODeviceClassGuidFromNameA( "HIDClass", &hGuid, TRUE );
   if( status == 0 ) return; // no such GUID ..

   // get a devinfo list of the corresponding devices
   hDevInfo = ::SetupDiGetClassDevs(&hGuid, NULL, NULL, DIGCF_PRESENT);

   device_index = 0;		/* init to first index of array */
   devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

   while(status != 0) {
      // Get information about the device with the 'device_index' array entry
      status = ::SetupDiEnumDeviceInfo(hDevInfo, device_index, &devInfoData);
      if(status == 0) {
         // free the memory allocated for DetailData */
         if(propBuffer != NULL) ::free(propBuffer); propBuffer=NULL;
         /* free HID device info list resources */
         ::SetupDiDestroyDeviceInfoList(hDevInfo);
         return;
      }
      // retrieve the DeviceInstanceId i.e. HID\VID_04B4&PID_5A9B\8&1B307ACC&1&0000
      status = ::SetupDiGetDeviceInstanceId(hDevInfo, &devInfoData, NULL, 0, &dataSize); // get size info
      if ( dataSize > 0 ) {
         if(propBuffer != NULL) ::free(propBuffer); propBuffer=NULL;
         propBuffer = (PTSTR)::malloc(dataSize);
         // get with proper size now
         status = ::SetupDiGetDeviceInstanceId(hDevInfo, &devInfoData, propBuffer, dataSize, NULL);
         if ( status != 0) {
            // derive VID and PID
            wxString vid, pid, pNam;
            pNam.Printf(wxT("%s"),propBuffer); // get the first string e.g. USB\Vid_04b4&Pid_5a9b&Rev_0100
            int pp=0;
            pp = pNam.Upper().Find(wxT("VID_")); if (pp!=wxNOT_FOUND ) vid = pNam.Mid(pp+4, 4).Upper();
            pp = pNam.Upper().Find(wxT("PID_")); if (pp!=wxNOT_FOUND ) pid = pNam.Mid(pp+4, 4).Upper();

//            /// ATTENTION - REMOVE THE IF BELOW FOR RELEASE !!!!!!!!
//            if ( 1 ) {
            if ( (vid==wxT(SSTRING_VENDOR_VID)) && (pid==wxT(SSTRING_LXUSB_PID)) ) {
               //  seems we have the proper USB device found
               m_pPorts[port_items] = new wxPortDescr();
               m_pPorts[port_items]->Init(port_items, wxT("LXUSB"), 0, pNam,  false, true, // -guide +LX
                                                                              false, false, false, true, false);
               port_items++;
            }
         }
      }
      device_index++;	// increment the array index to search the next entry
   }

   // free the memory allocated for DetailData */
   if (propBuffer != NULL) ::free(propBuffer);
   /* free HID device info list resources */
   ::SetupDiDestroyDeviceInfoList(hDevInfo);

	return;
}


