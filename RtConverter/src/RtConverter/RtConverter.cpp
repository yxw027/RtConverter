#include "../../include/RtConverter/RtConverter.h"
#include "../../include/RtConverter/Const.h"
#include "../../include/Rtklib/rtklib_fun.h"
#include "../../include/Rnxobs/RtRinexStream.h"
#include "../../include/VrsObs/VrsNtripClient.h"
using namespace bamboo;
RtConverter::RtConverter() {
	initlock(&_mutex);
	obsdelay = 5;
	port = -1;
}
RtConverter::RtConverter(int delay,unsigned int port_in) {
	initlock(&_mutex);
	obsdelay = delay;
	port = port_in;
}
RtConverter::~RtConverter() {
}
/// which will be called in the other threads
int RtConverter::inputObs(RtConvItem& item) {
	def_lock(&_mutex);
	if (obsqueue.empty()) {
		obsqueue.push_back(item);
		def_unlock(&_mutex);
		return 1;
	}
	RtConvItem& front = this->obsqueue.front();
	RtConvItem& end = this->obsqueue.back();
	if (item.curt.time < front.curt.time) {
		def_unlock(&_mutex);
		return 0;
	}
	if (item.curt.time > end.curt.time) {
		this->obsqueue.push_back(item);
		def_unlock(&_mutex);
		return 1;
	}
	list<RtConvItem>::iterator itrlist;
	for (itrlist = obsqueue.begin(); itrlist != obsqueue.end(); ++itrlist) {
		if (item.curt.time <= (*itrlist).curt.time)
			break;
	}
	if ((*itrlist).curt.time != item.curt.time) {
		obsqueue.insert(itrlist, item);
		def_unlock(&_mutex);
		return 1;
	}
	/// queue already exist the same time,will add into the memory
	std::map<std::string, std::map<std::string, RtSatObs> >::iterator itr, obitr_in = item.obslist.begin();
	std::map<std::string, std::map<std::string, RtSatObs> >::iterator obitr_mem = (*itrlist).obslist.begin();
	while (obitr_in != item.obslist.end()) {
		itr = (*itrlist).obslist.find((*obitr_in).first);
		if (itr != (*itrlist).obslist.end()) {
			++obitr_in;
			continue;
		}
		(*itrlist).obslist[(*obitr_in).first] = (*obitr_in).second;
		++obitr_in;
	}
	def_unlock(&_mutex);
	return 1;
}
void RtConverter::beginProcess() {
	def_thread_t pid_handle;
#ifdef _WIN32
	CreateThread(NULL, 0, s_pthProcess_win, this, 0, NULL);
#else
	if (0 != pthread_create(&pid_handle, NULL, &s_pthProcess, this)) {
		cout
			<< "***ERROR(s_VRSMain):cant create thread to handle request!"
			<< endl;
		exit(1);
	}
#endif
}
#ifdef _WIN32
DWORD WINAPI RtConverter::s_pthProcess_win(LPVOID args) {
	RtConverter* svr = (RtConverter*)args;
	svr->routineCheck();
	return NULL;
}
#endif
void* RtConverter::s_pthProcess(void* args) {
	RtConverter* svr = (RtConverter*)args;
	svr->routineCheck();
	return NULL;
}
void RtConverter::routineCheck() {
	time_t now;
	int nbyte;
	char cmd[256];
	unsigned char buff[1024];
	list<RtConvItem> item_sd;
	sprintf(cmd, ":%d", port);
	strinit(&svr);
	stropen(&svr, STR_TCPSVR, STR_MODE_RW, cmd);
	while (true) {
		/* test for re-read configures*/
		item_sd.clear();
		def_lock(&_mutex);
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
		def_unlock(&_mutex);

		/// here to sending the observation 
		list<RtConvItem>::iterator itr = item_sd.begin();
		while (itr != item_sd.end()) {
			char strbuf[1024] = { 0 };
			time2str((*itr).curt, strbuf, 2);
			printf("Thread %010d,begin to sending observation %s\n", def_pthread_id_self(), strbuf);
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
			sprintf(buffin + strlen(buffin), " %s %15.3lf", obs.fob[ifreq].c_str(), obs.obs[ifreq]);
		}
	}
	strcat(buffin, "\r\n");
	nbyte = strlen(buffin);
	return;
}
int main(int argc, char* args[]) {
	Deploy::s_initInstance(argc,args);
	list<RtConverter*> rt_svrs;
	Deploy dly = Deploy::s_getConfigures();
	RtConverter rtconv_nrtk(5,dly.nrtkport); 
	RtConverter rtconv_rtk(10,dly.rtkport); 
	RtRinexStream poststr;
	VrsNtripClient vrsstr;

	rt_svrs.push_back(&rtconv_nrtk);
	rt_svrs.push_back(&rtconv_rtk);
	/// post observation 
	//poststr.openStream(rt_svrs);
	vrsstr.openStream(rt_svrs);

	rtconv_nrtk.beginProcess();
	rtconv_rtk.beginProcess();
	while (true) {
		Deploy::s_updateConfigures();
		sleepms(5000);
	}
}

