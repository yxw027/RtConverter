#include <iostream>
#include "../../include/VrsObs/ntripc.h"
#include "../../include/Rtklib/rtklib_fun.h"
using namespace bamboo;
ntripc::ntripc() {
	initlock(&_mutex);
}

ntripc::~ntripc() {
}

void ntripc::testchar() {
	unsigned char sat;
	char satid[20];
	memset(satid, '\0', sizeof(satid));
	sat = '1';
	char dump = (char)sat;
	cout << "test for char" << endl;
	strcat(satid, &dump);
	printf("%s\n", satid);
}


void ntripc::rtcm3TOrtsatobs(const char *staname, rtcm_t &rtcm3, RtConvItem &item) {

	map<string, RtSatObs> obs;
	RtSatObs sat;
	char cprn[24] = { 0 };
	char fob[24] = { 0 };
	for (int i = 0; i < rtcm3.obs.n; i++) {
		satno2id((int)rtcm3.obs.data[i].sat, cprn);
		for (int ifreq = 0; ifreq < MAXFREQ; ifreq++) {
			sprintf(fob, "L%s", code2obs(rtcm3.obs.data[i].code[ifreq], NULL));
			sat.fob[ifreq] = fob;
			sprintf(fob, "C%s", code2obs(rtcm3.obs.data[i].code[ifreq], NULL));
			sat.fob[MAXFREQ + ifreq] = fob;

			sat.obs[ifreq] = rtcm3.obs.data[i].L[ifreq];
			sat.obs[MAXFREQ + ifreq] = rtcm3.obs.data[i].P[ifreq];
			sat.snr[ifreq] = 4.0 * (int)rtcm3.obs.data[i].SNR[ifreq];
		}
		obs[cprn] = sat;
		strcpy(cprn, "");
	}
	item.obslist[staname] = obs;
};

void ntripc::beginProcess(Deploy &dly, RtConverter &rtconv) {
	def_thread_t pid_handle;
#ifdef _WIN32
	CreateThread(NULL, 0, s_ntcProcess_win, this, 0, NULL);
#else
	if (0 != pthread_create(&pid_handle, NULL, &s_ntcProcess, this)) {
		cout
			<< "***ERROR(s_VRSMain):cant create thread to handle request!"
			<< endl;
		exit(1);
	}
}
#endif
this->dly = dly;
this->rtconv = rtconv;
}
#ifdef _WIN32
DWORD WINAPI ntripc::s_ntcProcess_win(LPVOID args) {
	ntripc* ntc = (ntripc*)args;
	ntc->sendreq();
	return NULL;
}
#endif
void* ntripc::s_ntcProcess(void* args) {
	ntripc* ntc = (ntripc*)args;
	ntc->sendreq();
	return NULL;
}


void ntripc::sendreq() {
	map<string, list<VrsStaItem>>::iterator svr;
	list<VrsStaItem>::iterator vs;
	RtConvItem item;
	char GGAmsg[1024] = {0};
	unsigned char buff[1024];
	memset(buff, '\0', sizeof(buff));
	for (svr = dly.rt_mounts.begin(); svr != dly.rt_mounts.end(); svr++) {
		rtcm_t *rt_instance = new(rtcm_t);
		stream_t *st_instance = new(stream_t);
		init_rtcm(rt_instance);
		strinit(st_instance);
		rts.push_back(*rt_instance);
		sts.push_back(*st_instance);
		RtConvItem *item = new RtConvItem;
		
	}
	while (true) {
		svr = dly.rt_mounts.begin();
		rt = rts.begin();
		st = sts.begin();
		while (svr!=dly.rt_mounts.end()) {
			cout << "SEND REQUEST:" << endl;
			
			printf("%s\n", (svr->first).c_str());
			stropen(&*st, STR_NTRIPCLI, STR_MODE_RW, (svr->first).c_str());
			for (vs = svr->second.begin(); vs != svr->second.end(); ++vs) {
				// send GGA
				//sprintf(GGAmsg,"$GPGGA,,,",);
				strncpy((*st).msg, GGAmsg, strlen(GGAmsg));
				strwrite(&*st, buff, sizeof(buff));
				strncpy((*st).msg, (char*)buff, sizeof(buff));
				memset(buff, '\0', sizeof(buff));
				int nread = strread(&*st, buff, sizeof(buff));
				for (int i = 0; i < nread; i++) {
					if (input_rtcm3(&*rt, buff[i]) == 1) {
						cout << "recieved RTCM3 message." << endl;
						rtcm3TOrtsatobs(vs->staname.c_str(), *rt, item);
						rtconv.inputObs(item);
					}
				}
				init_rtcm(&*rt);
				memset(buff, '\0', sizeof(buff));
			}
			++svr;
			++rt;
			++st;
		}
		
	}
}