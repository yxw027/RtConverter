#ifndef INCLUDE_BAMBOO_CONST_H_
#define INCLUDE_BAMBOO_CONST_H_
#include <string>
#include <cmath>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <map>
#include <list>
#include <iostream>
#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#include <condition_variable>
#else
#include <pthread.h>
#include <unistd.h>
#endif
#include "../Rtklib/rtklib.h"
using namespace std;
namespace bamboo {
	const unsigned char RTCM2PREAMB = 0x66;       /* rtcm ver.2 frame preamble */
	const unsigned char RTCM3PREAMB = 0xD3;      /* rtcm ver.3 frame preamble */
	const unsigned char ATOMPREAMB = 0xDB;      /* atom preamble */
	const int MAXFREQ = 3;
	const int MAXSYS = 7;
	const int MAXSAT = 130;
	const int MAXSIT = 500;
	const int MAXOC = 120;
	const int MAXOBSTYP = 50;
	const int MAXPORT = 10;
	const int MAXICS = 21;
	const int MAXECLIP = 120;
	const int MAXCOEFPAR = 3 + MAXSYS + 1 + 1 + 2 + 1; // XYZ RECCLK ZTD AMB ION DCB (ION maybe two)
	const int MAXDGR = 7;
	const int MAXPNT = 2 * (MAXDGR + 1);
	const int MAXVARS = 3 * (MAXICS + 1);

	const int MINCMNSIT = 4;
	const int MAX_GRID = 73;
	const int MeanEarthRadius = 6371000;
	const int IONO_SHELL_H = 450000;
	const int MAXTABLE = 20;
	const int MAXBASE = 20;

	const int MAXREFBLINE = 20;
	const int MAXBLINELEN = 1024;

	const char SYS[] = "GRECSJI";
	const char SYSNAME[][4] = { "gps","glo","gal","bds","","","" };
	const double MAXWND = 1E-2;
	const char OBSTYPE[] = "PWCIXSAQLDBYMZN "; //"WPCIXSAQLDBYMZN ";

	const double PI = 3.1415926535897932384626433832795028841971693993;
	const double VEL_LIGHT = 299792458.0;
	const double E_MAJAXIS = 6378136.60;

	const double DEG2RAD = PI / 180.0;
	const double RAD2DEG = 180.0 / PI;

	const double ARCSEC2RAD = PI / (3600.0*180.0);
	const double CONE = 2.7182818284590452353602874713526624977572470936999;

	const double EARTH_A = 6378137;
	const double EARTH_ALPHA = 1 / 298.257223563;
	const double ESQUARE = (EARTH_A * EARTH_A - (EARTH_A - EARTH_A * EARTH_ALPHA) * (EARTH_A - EARTH_A * EARTH_ALPHA)) / (EARTH_A * EARTH_A);

	const double OFF_GPS2TAI = 19.0;
	const double OFF_TAI2TT = 32.184;
	const double OFF_GPS2TT = OFF_GPS2TAI + OFF_TAI2TT;
	const double OFF_MJD2JD = 2400000.50;

	const double E_ROTATE = 7.2921151467E-5;
	const double GME = 3.986004415E14; //TT-compatible
	const double GPS_FREQ = 10230000.0;
	const double GPS_L1 = 154 * GPS_FREQ;
	const double GPS_L2 = 120 * GPS_FREQ;
	const double GPS_L5 = 115 * GPS_FREQ;

	const double GLS_FREQ = 178000000.0;
	const double GLS_L1 = 9 * GLS_FREQ;
	const double GLS_L2 = 7 * GLS_FREQ;
	const double GLS_dL1 = 562500.0;
	const double GLS_dL2 = 437500.0;

	const double GAL_E1 = GPS_L1;
	const double GAL_E5 = 1191795000.0;
	const double GAL_E5a = GPS_L5;
	const double GAL_E5b = 1207140000.0;
	const double GAL_E6 = 1278750000.0;

	const double BDS_B1 = 1561098000.0;
	const double BDS_B2 = 1207140000.0;
	const double BDS_B3 = 1268520000.0;
	const double QZS_L1 = GPS_L1;
	const double QZS_L2 = GPS_L2;
	const double QZS_L5 = GPS_L5;
	const double QZS_LEX = 1278750000.0;
	class Locker {
	public:
		Locker(lock_t* in) { mlock = in; if(mlock) lock(mlock); }
		~Locker() { if(mlock) unlock(mlock); }
	private:
		lock_t* mlock;
	};
}
#endif /* INCLUDE_BAMBOO_CONST_H_ */
