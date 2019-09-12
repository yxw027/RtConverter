#ifndef NTRIPC
#define NTRIP
#include "../RtConverter/RtConverter.h"
#include "../RtConverter/Deploy.h"
namespace bamboo {
	class ntripc
	{
	public:
		ntripc();
		~ntripc();
		void sendreq();
		void rtcm3TOrtsatobs(const char *staname, rtcm_t &rtcm3, RtConvItem &item);
		void testchar();
		void beginProcess(Deploy &dly, RtConverter &rtconv);
	private:
#ifdef _WIN32
		static DWORD WINAPI s_ntcProcess_win(LPVOID lp);
#endif
		static void* s_ntcProcess(void*);
		Deploy dly;
		RtConverter rtconv;
		list<stream_t> sts;
		list<stream_t>::iterator st;
		list<rtcm_t> rts;
		list<rtcm_t>::iterator rt;
		lock_t _mutex;
	};
}

#endif // NTRIPC
