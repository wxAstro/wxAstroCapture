#!/usr/bin/perl
#
# wxAstroCapture device check script
# run from shell and log to file 
# e.g.  testAtk.pl > atklog.txt
#
# 20080322: Martin Burri
# it is completely non intrusive, does not change anything
#
use Shell(bash);

sub CheckUSBx()
{
   my (@arg)=@_;
   my ($i, $res, @res, $TX);
   $TX="/sys/bus/usb/drivers/*/*-*/ttyUSB@arg[0]";
   $res=bash("-c \"find -P  $TX -maxdepth 0\""); chomp($res);

   if ($res) {
      printf "** list USB serial devices ttyUSB@arg[0]\n%s\n", $res;
      $res=substr $res,length("/sys/bus/usb/drivers/ftdi_sio/"), 3; chomp($res);
      if ($res) {
         $res="/sys/bus/usb/devices/$res";
         @res=bash("-c \"ls $res\""); chomp @res;
         foreach $i (@res) {
#            print "$i\n";
            if ($i eq 'idVendor')     {printf "idVendor:     %s", bash("-c \"cat $res/$i\"");}
            if ($i eq 'idProduct')    {printf "idProduct:    %s", bash("-c \"cat $res/$i\"");}
            if ($i eq 'manufacturer') {printf "manufacturer: %s", bash("-c \"cat $res/$i\"");}
            if ($i eq 'product')      {printf "product:      %s", bash("-c \"cat $res/$i\"");}
            if ($i eq 'serial')       {printf "serial:       %s", bash("-c \"cat $res/$i\"");}
         }
         print "\n"
      }
   }
}

sub ListUSBx ()
{
   my (@arg)=@_;
   my ($i, $res, @res, $TX);
   printf "** list USB devices @arg[0]\n";
   if (@arg[0]) {
      @res=bash("-c \"ls @arg[0]\""); chomp @res;
      foreach $i (@res) {
         if ($i eq 'idVendor')     {printf "idVendor:     %s", bash("-c \"cat @arg[0]/$i\"");}
         if ($i eq 'idProduct')    {printf "idProduct:    %s", bash("-c \"cat @arg[0]/$i\"");}
         if ($i eq 'manufacturer') {printf "manufacturer: %s", bash("-c \"cat @arg[0]/$i\"");}
         if ($i eq 'product')      {printf "product:      %s", bash("-c \"cat @arg[0]/$i\"");}
         if ($i eq 'serial')       {printf "serial:       %s", bash("-c \"cat @arg[0]/$i\"");}
      }
      print "\n"
   }
}

sub ListFTDIknowns()
{
   my ($i, $res, @res, $NewATK);
   # Vid of Art/Atk is v0403 Pid is pDF*
   $NewATK=0;
   @res=bash("-c \"/sbin/modprobe -c | grep -i -e ^alias | grep -i v0403pDF\"");
   foreach $i (@res) {
      chomp $i;
      $res=substr(lc $i, index($i, ":")+1,10);
      if ($res eq 'v0403pdf28') {printf "$res:  Artemis";}
      if ($res eq 'v0403pdf34') {printf "$res:  Mini Artemis";}
      if ($res eq 'v0403pdf30') {printf "$res:  ATIK ATK-16 Grayscale Camera";}
      if ($res eq 'v0403pdf31') {printf "$res:  ATIK ATK-16HR Grayscale Camera";}
      if ($res eq 'v0403pdf32') {printf "$res:  ATIK ATK-16C Colour Camera";}
      if ($res eq 'v0403pdf33') {printf "$res:  ATIK ATK-16HRC Colour Camera";}
      if ($res eq 'v0403pdf35') {printf "$res:  ATIK ATK-16IC Grayscale Camera"; $NewATK+=1;}
      if ($res eq 'v0403pdf36') {printf "$res:  ATIK ATK-16IC-C Colour Camera"; $NewATK+=1;}
      if ($res eq 'v0403pdf2c') {printf "$res:  ATIK ATK-16IC-S Grayscale Camera"; $NewATK+=1;}
      if ($res eq 'v0403pdf2d') {printf "$res:  ATIK ATK-16IC-S-C Colour Camera"; $NewATK+=1;}

      print "\n";
   }
   print "\n";
   if ( $NewATK eq 0 ) {
      printf("WARNING: ATIK ATK-IC16 line cameras are not known by the linux driver module,\n");
   }
   else {
      printf("NOTE: At least %1d ATIK ATK-IC16 line cameras are known by the linux driver module.\n", $NewATK);
   }
   printf "NOTE: If you are missing your cam here the FTDI USB chip support has to be updated!\n";
   printf("      For more information visit the wxAstroCapture Yahoo group. \n");
   print "\n";
}


sub CheckModule()
{
   my(@arg)=@_;
   my($res);
   $res=bash("-c \"/sbin/modprobe -l | grep @arg[0]\" "); # in SUSE /sbin is not in PATH
   return $res;
}

sub CheckModuleInstalls()
{
# Module installs
   print "\n** Checking Module installs:\n\n";
   printf "PWC       : %s\n", $res=&CheckModule("/pwc.ko");
   if ( $res ) {} else {printf "WARNING: the webcam driver pwc is not installed - wxAstroCapture cannot support webcams!\n";};
   printf "FTDI      : %s\n", $res=&CheckModule("/ftdi_sio.ko");
   if ( $res ) {} else {printf "WARNING: the FTDI chip driver ftdi_sio is not installed - wxAstroCapture cannot support Art/ATK cameras!\n";};
   printf "PPDEV     : %s\n", $res=&CheckModule("/ppdev.ko");
   printf "PARPORT   : %s\n", $res=&CheckModule("/parport.ko");
   if ( $res ) {} else {printf "WARNING: the parallel port driver parport is not installed - wxAstroCapture cannot support parallel port IO!\n";};
   printf "PARPORT PC: %s\n", $res=&CheckModule("/parport_pc.ko");
   print "** END checking Module installs:\n";
}

sub CheckRpmPackage()
{
   my(@arg)=@_;
   my($res);
   $res=rpm("-q --queryformat", '%{NAME}-%{VERSION}-%{RELEASE}\n', "@arg[0]");
   return $res;
}

sub CheckRpmInstalls()
{
# Mandriva (SUSE) style installs
   my($res, @res);
   print "\n** Checking RPM style installs (takes a while):\n\n";
   # find out in which package the wx library resides
   @res=rpm("-qas | grep libwx_gtk2"); $res=rpm("-qf @res[0]"); chomp $res;
   printf "wxGtk     : %s\n", $res=&CheckRpmPackage("$res");
   if ( $res ) {} else {printf "WARNING: wxWidgets library is not installed - wxAstroCapture cannot run!\n";};
   printf "wxAstroCap: %s\n", $res=&CheckRpmPackage("wxastrocapture");
   if ( $res ) {} else {printf "WARNING: wxAstroCapture is not installed and cannot run!\n";};
   print "\n** END checking RPM style installs:\n";
}


sub CheckDebPackage
{
   my(@package)=@_;
   my($res);
   $res=bash("-c \"dpkg -s @package[0] | grep -e Package -e Version\"");
   $res=sprintf "$res%s", bash("-c \"dpkg -s @package[0] | grep -e Status | grep  \"install ok installed\" \"");
   return $res;
}


sub CheckRepository()
{
   my ($i, $res, @res, $TX);
   printf "** checking apt repository for wxWidgets:\n";
   $TX="/etc/apt/sources.list";
   $res=bash("-c \"cat $TX | grep -i \"http://apt.wxwidgets.org/\" \"");
   if ( $res ) {
      return $res;
   }
   return "apt repository not found";
}

sub CheckDebInstalls()
{
# Debian (Ubuntu) style installs
   print "\n** Checking Debian style installs:\n\n";
   printf "Repository: %s\n", $res=&CheckRepository;
   if ( $res ) {} else {printf "NOTE: the wxWidgets repository is not defined - life is easier if this is available!\n";};
   printf "Curl      : %s\n", $res=&CheckDebPackage("curl");
   printf "Gdebi     : %s\n", $res=&CheckDebPackage("gdebi");
   printf "wxGtk     : %s\n", $res=&CheckDebPackage("libwxgtk2.8-0");
   if ( $res ) {} else {printf "WARNING: wxWidgets library is not installed - wxAstroCapture cannot run!\n";};
   printf "wxAstroCap: %s\n", $res=&CheckDebPackage("wxastrocapture");
   if ( $res ) {} else {printf "WARNING: wxAstroCapture is not installed and cannot run!\n";};
   print "\n** END checking Debian style installs:\n";
}


CheckDeviceGroupMember ($devString, \@groups); #prototype
sub CheckDeviceGroupMember
{
   my($devString, $Rgroups)=@_;
   my ($i, @res, @list);

   @res=split "\n", $devString;
   foreach $i (@res) {
      chomp $i;
      @list=split " ",$i;
      if ( grep ($_ eq @list[3], @$Rgroups) ) {} else {
         printf("WARNING: To be able to control the port %s one has to belong to the <%s> group\n",
                   @list[8], @list[3]);
      }
   }
   print "\n";
}


#MAIN
{
   # used variables
   my ($MyName, @MyGroups);
   # widely used temp variables
   my ($i, @res, @list, $TX);
   print "\n\n**** $0 - wxAstroCapture device checks .. \n";
   print "**** plug in any cameras to be used before running this script .. \n\n";

   # we do no want errors to be shown 
   close STDERR;

   print "**** System checks .. \n";

   printf "*** uname command:\n%s\n", $res=bash("-c \"uname -a\"");

   printf "*** cat issue file:\n%s\n", $res=bash("-c \"cat /etc/issue\" ");

   $TX="/etc/*release*";
   printf "*** list and cat release files:\n%s\n", @res=bash("-c \"find -P $TX -maxdepth 0\""); chomp @res;
   foreach $i (@res) {
      if ( -f $i ) {
         printf "%s\n", $TX=bash("-c \"cat $i\"");
      }
   }
   $TX="/etc/*version*";
   printf "*** list and cat version files:\n%s\n", @res=bash("-c \"find -P $TX -maxdepth 0\""); chomp @res;
   foreach $i (@res) {
      if ( -f $i ) {
         printf "%s\n", $TX=bash("-c \"cat $i\"");
      }
   }

   printf "*** whoami command:\n%s\n", $MyName=bash("-c \"whoami\""); chomp $MyName;

   printf "*** list my groups:\n";

   {
   my ($name, $passwd, $gid, $members, @m);
      setgrent:
      do {
         ($name, $passwd, $gid, $members) = getgrent();
         @m=split " ", $members;
         if ( grep ($_ eq $MyName, @m) ) {push @MyGroups, $name;};
      } while ( $name );
      endgrent;

      for ($i=0; $i<@MyGroups; $i++) {
         printf "%s\n", @MyGroups[$i];
      }
   }

   print "\n**** Device checks .. \n\n";

# Parallel Port Check
   printf "*** list parport :\n%s\n", $res=bash("-c \"ls -l /dev/parport*\"");
   CheckDeviceGroupMember($res, \@MyGroups);


# Serial Port Check
   printf "*** list ttySx :\n%s\n", $res=bash("-c \"ls -l /dev/ttyS*\""); 
   CheckDeviceGroupMember($res, \@MyGroups);

# USB Serial Port Check
   printf "*** list ttyUSBx :\n%s\n", $res=bash("-c \"ls -l /dev/ttyUSB*\"");
   CheckDeviceGroupMember($res, \@MyGroups);


# TTY Class Check
   $TX="/sys/class/tty*";
   printf "*** list serial class directory tty:\n%s\n", $res=bash("-c \"find -P $TX -maxdepth 0\"");

# USB TTY Devices 0..9  Check
   print "*** Check ttyUSBx devices:\n";
   foreach $i (0,1,2,3,4,5,6,7,8,9) {
      &CheckUSBx($i);
   }

# USB Devices List
   print "*** List all USB devices:\n\n";
   $TX="/sys/bus/usb/devices";
   @res=bash("-c \"ls $TX\"");
   foreach $i (@res) {
      chomp $i;
      # see if at least an idVendor file is available
      $res=bash("-c \"ls $TX/$i/idVendor\"");
      if ($res) {
         &ListUSBx("$TX/$i");
      }
   }


   print "\n**** Software checks .. \n\n";


   printf "*** List loaded modules with ppdev:\n%s\n", $res=bash("-c \"lsmod | grep ppdev\"");

   printf "*** List loaded modules with usb:\n%s\n", $res=bash("-c \"lsmod | grep usb\"");

   if ( bash("-c \"which /sbin/modprobe\"") ) { 
      &CheckModuleInstalls; 
   } 
   else {
      print "\nCannot check modules !!\n"
   };

   if ( bash("-c \"which dpkg\"") ) { &CheckDebInstalls; } 
   else {
      if ( which("rpm") ) { &CheckRpmInstalls; } 
      else {print "\nCannot check installs !!\n"}
   }
   print "\n";

   printf "*** List known devices of the ftdi_sio module:\n";
   if ( bash("-c \"ls /etc/modprobe.d/artemis.modprobe\"") ) 
   {
      printf "NOTE: FTDI driver support file for ATK-16-IC cameras is installed.\n";
   } 
   else {
      printf "WARNING: FTDI driver support file for ATK-16-IC cameras is NOT installed.\n";
   } 
   print "\n";
   ListFTDIknowns;

   print "\n**** END $0 .. \n";
}


