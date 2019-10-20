#ifndef __version_h_included__
#define __version_h_included__

#define VERSION_MAJOR    1
#define VERSION_MINOR    0
#define VERSION_REVISION 1

#define xstr(s) str(s)
#define str(s) #s

#define VERSION_STRING xstr(VERSION_MAJOR) "." xstr(VERSION_MINOR) "." xstr(VERSION_REVISION)

#endif
