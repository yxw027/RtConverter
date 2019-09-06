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
		stream_t svr;
		std::list<RtConvItem> obsqueue;
		lock_t _mutex;
	};
}
#endif