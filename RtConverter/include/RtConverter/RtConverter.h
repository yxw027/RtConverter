#ifndef RTCONVERTER
#define RTCONVERTER
#include <map>
#include <list>
#include "RtConvItem.h"
#include "../../include/Rtklib/rtklib.h"
namespace bamboo {
	class RtConverter {
	public:
		RtConverter();
		RtConverter(int,unsigned int );
		~RtConverter();
		int inputObs(RtConvItem& item);
		void beginProcess();
	private:
#ifdef _WIN32
		static DWORD WINAPI s_pthProcess_win(LPVOID lp);
#endif
		static void* s_pthProcess(void*);
		void routineCheck();
		void makeupTimeBuffer(gtime_t,char* buffin,int& nbyte);
		void makeupObsBuffer(const char* sitname, const char* cprn, RtSatObs& obs, char* buffin, int& nbyte);
		int obsdelay;
		unsigned int port;
		stream_t svr;
		std::list<RtConvItem> obsqueue;
		lock_t _mutex;
	};
}
#endif