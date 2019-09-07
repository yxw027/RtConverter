#ifndef RTCONVERTER
#define RTCONVERTER
#include <map>
#include <list>
#include "RtConvItem.h"
#include "../../include/Rtklib/Rtklib.h"
namespace bamboo {
	const int obsdelay = 5;  // 5s delay
	class RtConverter {
	public:
		RtConverter();
		~RtConverter();
		int inputObs(RtConvItem& item);
		void routineCheck();
	private:
		/* test for output */
		void test_input();
		void makeupTimeBuffer(gtime_t,char* buffin,int& nbyte);
		void makeupObsBuffer(const char* sitname, const char* cprn, RtSatObs& obs, char* buffin, int& nbyte);
		stream_t svr;
		std::list<RtConvItem> obsqueue;
		lock_t _mutex;
	};
}
#endif