This is the source code folder of DB-WEAVE, a textile
CAD/CAM system. See http://www.brunoldsoftware.ch for
more information.

The project is written in C++, using the (now ancient)
C++Builder 5 product from Borland. Since it uses the
VCL library extensively, it is not easy to port it to
some newer programming environment.

In addition to C++Builder 5, you need the ComPort 3.3
Component from WinSoft (Erik Salaj). You can download
and install a trial version from the website
http://www.winsoft.sk/comport.htm. The full version
costs about 35$.

The whole project is licensed under the GPL v3 or later.
See LICENSE.txt for details.

A setup suitable for distribution can be created using
the free NSIS setup system. You can download it from
http://nsis.sourceforge.net/Main_Page. The necessary
setup script is included in this folder and named
dbw.nsi. Copy dbw.exe, license.txt, manual.pdf,
handbuch.pdf and source.zip together with dbw.nsi in
a folder, and run the NSIS system to create the
dbw_setup.exe file. This can be distributed and installs
the whole program.
