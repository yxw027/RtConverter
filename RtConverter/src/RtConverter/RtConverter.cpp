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
	int nbyte,ns;
	unsigned char buff[1024];
	list<RtConvItem> item_sd;
	strinit(&svr);
	stropen(&svr, STR_TCPSVR, STR_MODE_RW, ":8009");
	while (true) {
		/* TEST FOR OUTPUT */
		test_input();

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
					itr = obsqueue.erase(itr);
				}
			}
		}
		/// here to sending the observation 
		list<RtConvItem>::iterator itr = item_sd.begin();
		while (itr != item_sd.end()) {
			cout << "begin to sending observation here " << (*itr).curt.time << endl;
			/// generate the buff here
			memset(buff, 0, sizeof(buff));
			makeupTimeBuffer((*itr).curt, (char*)buff, nbyte);
			strwrite(&svr, buff, nbyte);
			std::map<std::string, std::map<std::string, RtSatObs> >::iterator obitr_tosend = (*itr).obslist.begin();
			while (obitr_tosend != (*itr).obslist.end()) {
				map<string, RtSatObs>::iterator satitr = (*obitr_tosend).second.begin();
				while (satitr != (*obitr_tosend).second.end()) {
					memset(buff, 0, sizeof(buff));
					makeupObsBuffer((*obitr_tosend).first.c_str(), (*satitr).first.c_str(), (*satitr).second, (char*)buff, nbyte);
					strwrite(&svr, buff, nbyte);
					++satitr;
				}
				obitr_tosend++;
			}
			++itr;
		}
		sleepms(1000);
	}
	strclose(&svr);
}
void RtConverter::makeupTimeBuffer(gtime_t tt, char* buffin, int& nbyte) {
	int week;
	double sow;
	sow = time2gpst(tt, &week);
	sprintf(buffin, ">%04d %14.7lf\r\n", week, sow);
	nbyte = strlen(buffin);
	return;
}
void RtConverter::makeupObsBuffer(const char* sitname, const char* cprn, RtSatObs& obs, char* buffin, int& nbyte) {
	int ifreq;
	sprintf(buffin, "%s %s", sitname, cprn);
	for (ifreq = 0; ifreq < 2 * MAXFREQ; ifreq++) {
		if (obs.obs[ifreq] != 0.0) {
			sprintf(buffin + strlen(buffin), " %s %12.3lf", obs.fob[ifreq].c_str(), obs.obs[ifreq]);
		}
	}
	strcat(buffin, "\r\n");
	nbyte = strlen(buffin);
	return;
}
void RtConverter::test_input() {
	static gtime_t beg = { 0 };
	if (beg.time == 0) {
		double ep[] = { 2019,9,7,10,25,00 };
		beg = epoch2time(ep);
	}
	else
		beg.time++;
	/* make up observation here */
	RtConvItem item;
	item.curt = beg;
	map<string, RtSatObs> obs;

	RtSatObs sat;
	sat.obs[0] = 127172623.353;
	sat.obs[1] = 111961948.919;
	sat.obs[MAXFREQ] = 20981549.381;
	sat.obs[MAXFREQ + 1] = 20981552.597;

	sat.fob[0] = "L1C";
	sat.fob[1] = "L2C";

	sat.fob[MAXFREQ] = "C1C";
	sat.fob[MAXFREQ + 1] = "C2P";

	obs["G01"] = sat;
	obs["G02"] = sat;
	obs["G06"] = sat;

	item.obslist["HKNP"] = obs;
	item.obslist["HKTK"] = obs;

	inputObs(item);

	cout << "input observation " << beg.time << endl;
}
int main(int argc, char* args[]) {
	RtConverter rtconv;
	rtconv.routineCheck();
}


