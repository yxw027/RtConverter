#include "../../include/RtConverter/RtConverter.h"
#include "../../include/RtConverter/Const.h"
#include "../../include/Rtklib/rtklib_fun.h"
using namespace bamboo;
RtConverter::RtConverter() {
	initlock(&_mutex);
}
RtConverter::~RtConverter() {

}
/// which will be called in the other threads
int RtConverter::inputObs(RtConvItem& item) {
	Locker(&this->_mutex);
	if (obsqueue.empty()) {
		obsqueue.push_back(item);
		return 1;
	}
	RtConvItem& front = this->obsqueue.front();
	RtConvItem& end = this->obsqueue.back();
	if (item.curt.time < front.curt.time) return 0;
	if (item.curt.time > end.curt.time) {
		this->obsqueue.push_back(item);
		return 1;
	}
	list<RtConvItem>::iterator itrlist, itrfind = obsqueue.end();
	itrlist = obsqueue.begin();
	while (item.curt.time > (*itrlist).curt.time) {
		itrlist++;
	}
	if ((*itrlist).curt.time != item.curt.time) {
		obsqueue.insert(itrlist,item);
		return 1;
	}
	/// queue already exist the same time,will add into the memory
	std::map<std::string, std::map<std::string, RtSatObs> >::iterator itr,obitr_in = item.obslist.begin();
	std::map<std::string, std::map<std::string, RtSatObs> >::iterator obitr_mem = (*itrlist).obslist.begin();
	while (obitr_in != item.obslist.end()) {
		itr = (*itrlist).obslist.find((*obitr_in).first);
		if (itr != (*itrlist).obslist.end()) continue;
		(*itrlist).obslist[(*obitr_in).first] = (*obitr_in).second;
		++obitr_in;
	}
	return 1;
}
void RtConverter::routineCheck() {
	time_t now;
	int nbyte;
	unsigned char buff[1024];
	list<RtConvItem> item_sd;
	strinit(&svr);
	while (true) {
		item_sd.clear();
		{
			Locker(&this->_mutex);
			time(&now);
			list<RtConvItem>::reverse_iterator itrlist = obsqueue.rbegin();
			while (itrlist != obsqueue.rend()) {
				if (now - (*itrlist).gent >= obsdelay) {
					break;
				}
				++itrlist;
			}
			if (itrlist != obsqueue.rend()) {
				gtime_t sd_t = (*itrlist).curt;
				list<RtConvItem>::iterator itr = obsqueue.begin();
				while (itr != obsqueue.end()) {
					if ((*itr).curt.time > sd_t.time)  break;
					item_sd.push_back(*itr);
				}
			}
		}
		/// here to sending the observation 
		list<RtConvItem>::iterator itr = item_sd.begin();
		while (itr != item_sd.end()) {
			/// generate the buff here
			strwrite(&svr, buff, nbyte);
		}
		sleepms(100);
	}
}
int main(int argc, char* args[]) {
	RtConverter rtconv;
	rtconv.routineCheck();
}


