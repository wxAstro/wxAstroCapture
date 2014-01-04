/**

  Astronomy USB Appliance - Control Library

Copyright (C) 2009 Martin Burri  (bm@burri-web.org)

This program is free software licensed under LGPL;
you can redistribute it and/or modify it under the terms of
the GNU Lesser General Public License as published by the
Free Software Foundation; either version 3 of the License,
or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/**
 * @file           GenericDebHelp.h
 *****************************************************************************
 * class DebHelp
 *
 * Astronomy USB Appliance driver for Linux and Win32
 *
 * This is part of the Driver Library of wxAstroCapture
 *
 * Copyright (C) 2009 Martin Burri  (bm98@burri-web.org)
 *
 *<hr>
 *
 * @b Project      wxAstroCapture<br>
 *
 * @author         M. Burri
 * @date           01-Feb-2009
 *
 *****************************************************************************
 *<hr>
 * @b Updates
 * - dd-mmm-yyyy V. Name: Description
 *
 *****************************************************************************/


#ifndef _GENERICDEBHELP_H_
#define _GENERICDEBHELP_H_

//! Debug Helper class
// verbose printing helper
//   to be used like:  _V_ fprintf(stderr, "something\n);
#define _V_ if ( DebHelp::Verbose()> DebHelp::VL_Quiet)
#define _D_ if ( DebHelp::Verbose()> DebHelp::VL_Log)


class DebHelp
{
public:
   enum VerbosityLevel
   {
         VL_Quiet=0,
         VL_Log,
         VL_Debug

   };
   static VerbosityLevel Verbose() {return m_verbose;};
   static void SetVerboseQuiet() {m_verbose=VL_Quiet;};
   static void SetVerboseLog() {m_verbose=VL_Log;};
   static void SetVerboseDebug() {m_verbose=VL_Debug;};

private:
   static VerbosityLevel   m_verbose;
};

#endif // _GENERICDEBHELP_H_
