#ifndef RTCONVERTER
#define RTCONVERTER
#include <map>
#include <list>
#include "RtConvItem.h"
#include "../Rtklib/rtklib.h"
#include "../Rnxbrd/OrbitClk.h"
namespace bamboo {
	class RtConverter {
	public:
		enum ConvType {
			obs,
			eph
		};
		RtConverter();
		RtConverter(int,unsigned int,ConvType);
		~RtConverter();
		int inputObs(RtConvItem& item);
		void inputBrdm_G(string cprn,list<GPSEPH>&);
		void inputBrdm_R(string cprn,list<GLSEPH>&);
		void beginProcess();
		ConvType getType() { return type; }
	private:
#ifdef _WIN32
		static DWORD WINAPI s_pthProcess_win(LPVOID lp);
#endif
		static void* s_pthProcess(void*);
		void routineObs();
		void routineEph();
		void makeupTimeBuffer(gtime_t,char* buffin,int& nbyte);
		void makeupObsBuffer(const char* sitname, const char* cprn, RtSatObs& obs, char* buffin, int& nbyte);

		void makeupEphBuffer_G(GPSEPH& eph, char* buffin, int& nbyte);
		void makeupEphBuffer_R(GLSEPH& eph, char* buffin, int& nbyte);
		int obsdelay;
		ConvType type;
		unsigned int port;
		stream_t svr;
		std::list<RtConvItem> obsqueue;
		/// ephemeris to be broadcast int loop
		std::map<string, list<GPSEPH> > ephqueue_G;
		std::map<string, list<GLSEPH> > ephqueue_R;
		lock_t _mutex;
	};
}
#endif