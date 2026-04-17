/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/*-----------------------------------------------------------------*/
#include <vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
String GetOsVersion()
{
    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!GetVersionEx ((OSVERSIONINFO*)&osvi))
            return "unbekannt";

    String os = "";

    switch (osvi.dwPlatformId) {
        case VER_PLATFORM_WIN32_NT:
        // Test for the product.
        if (osvi.dwMajorVersion <= 4)
            os = "Microsoft Windows NT ";
        if (osvi.dwMajorVersion == 5) {
            if (osvi.dwMinorVersion == 0) {
                os = "Microsoft Windows 2000 ";
            } else if (osvi.dwMinorVersion == 1) {
                os = "Microsoft Windows XP";
            } else {
                os = "Microsoft Windows XP or newer";
            }
        }
        // Test for workstation versus server.
        HKEY hKey;
        char szProductType[80];
        DWORD dwBufLen;
        RegOpenKeyEx (HKEY_LOCAL_MACHINE,
           "SYSTEM\\CurrentControlSet\\Control\\ProductOptions",
           0, KEY_QUERY_VALUE, &hKey );
        RegQueryValueEx (hKey, "ProductType", NULL, NULL, (LPBYTE)szProductType, &dwBufLen);
        RegCloseKey( hKey );
        if (lstrcmpi("WINNT", szProductType)==0) {
           if (osvi.dwMajorVersion<=4) os += "Workstation ";
           else os += "Professional ";
        }
        if (lstrcmpi("SERVERNT", szProductType)==0)
           os += "Server ";
        // Display version, service pack (if any), and build number.
        char buff[40];
        wsprintf (buff, "Version %d.%d %s (Build %d)", osvi.dwMajorVersion, osvi.dwMinorVersion,
            osvi.szCSDVersion, osvi.dwBuildNumber & 0xFFFF);
        os += String("\n") + buff;
        break;

    case VER_PLATFORM_WIN32_WINDOWS:
        if ((osvi.dwMajorVersion > 4) ||
           ((osvi.dwMajorVersion == 4) && (osvi.dwMinorVersion > 0))) {
            os = "Microsoft Windows 98 ";
        } else if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90) {
            os = "Microsoft Windows Me ";
        } else os = "Microsoft Windows 95 ";
        break;
    }

    return os;
}
/*-----------------------------------------------------------------*/

