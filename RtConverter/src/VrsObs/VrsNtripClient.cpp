#include "../../include/VrsObs/VrsNtripClient.h"
#include "../../include/Rnxobs/Com.h"
using namespace std;
using namespace bamboo;
VrsNtripClient::~VrsNtripClient() {
	/// will delete all the memory
	lcont = false;
}
void VrsNtripClient::openStream() {
	lcont = true;
	configs_sav = Deploy::s_getConfigures();
	time(&lastCheck);
#ifdef _WIN32
	CreateThread(NULL, 0, s_pthVrsStream_win, this, 0, NULL);
#else
	if (0 != pthread_create(&pid_handle, NULL, &s_pthVrsStream, this)) {
		cout
			<< "***ERROR(s_VRSMain):cant create thread to handle request!"
			<< endl;
		exit(1);
	}
#endif
}

void VrsNtripClient::openStream(list<RtConverter*> svrs_in) {
	lcont = true;
	configs_sav = Deploy::s_getConfigures();
	time(&lastCheck);
#ifdef _WIN32
	CreateThread(NULL, 0, s_pthVrsStream_win, this, 0, NULL);
#else
	if (0 != pthread_create(&pid_handle, NULL, &s_pthVrsStream, this)) {
		cout
			<< "***ERROR(s_VRSMain):cant create thread to handle request!"
			<< endl;
		exit(1);
	}
#endif
	this->m_svrs = svrs_in;
}
#ifdef _WIN32
DWORD WINAPI VrsNtripClient::s_pthVrsStream_win(LPVOID lp) {
	VrsNtripClient* svr = (VrsNtripClient*)lp;
	svr->m_routine();
	return NULL;
}
#endif
void* VrsNtripClient::s_pthVrsStream(void* args) {
	VrsNtripClient* svr = (VrsNtripClient*)args;
	svr->m_routine();
	return NULL;
}
void VrsNtripClient::m_routine() {
	time_t tt;
	bool lcheck;
	int nread = 0,i;
	char cmd[1024] = { 0 }, buff[1024] = { 0 };
	list<VrsStaItem>::iterator staItr;
	list<RtConverter*>::iterator rtItr;
	map<string, list<VrsStaItem>>::iterator mapItr;
	map<string, stream_t*>::iterator streamItr;
	map<string, rtcm_t*>::iterator rtcmItr;
	stream_t* str_m;
	rtcm_t* rtcm_m;
	for (mapItr = configs_sav.rt_mounts.begin(); mapItr != configs_sav.rt_mounts.end(); ++mapItr) {
		for (staItr = (*mapItr).second.begin(); staItr != (*mapItr).second.end(); ++staItr) {
			str_m = (stream_t*)calloc(1, sizeof(stream_t));
			strinit(str_m);
			strsettimeout(str_m, 60000, 10000); /// 60s for timeout 10s for reconnect
			rtcm_m = (rtcm_t*)calloc(1, sizeof(rtcm_t));
			init_rtcm(rtcm_m);
			memset(cmd, 0, sizeof(cmd));
			if ((*staItr).type = VrsStaItem::stream_type::stream) {
				if('/' == (*mapItr).first.back())
					sprintf(cmd, "%s%s", (*mapItr).first.c_str(), (*staItr).staname.c_str());
				else
					sprintf(cmd, "%s/%s", (*mapItr).first.c_str(), (*staItr).staname.c_str());
			}else {
				sprintf(cmd, "%s", (*mapItr).first.c_str());
			}
			stropen(str_m, STR_NTRIPCLI, STR_MODE_RW, cmd);
			m_streams[(*staItr).staname] = str_m;
			m_rtcms[(*staItr).staname] = rtcm_m;
			m_stas[(*staItr).staname] = (*staItr);
		}
	}
	while (lcont) {
		lcheck = false;
		time(&tt);
		if (tt - lastCheck > 5) {
			lcheck = true;
			lastCheck = tt;
			m_adaptConfigures();
		}
		for (streamItr = m_streams.begin(); streamItr != m_streams.end(); ++streamItr) {
			if (m_stas[(*streamItr).first].type == VrsStaItem::stream_type::vrs && lcheck) {
				//// sending GGA here
				m_sendGGAReq((*streamItr).second,m_stas[(*streamItr).first]);
			}
			nread = strread((*streamItr).second, (unsigned char*)buff, 1024);
			for (i = 0; i < nread; i++) {
				if (input_rtcm3(m_rtcms[(*streamItr).first], buff[i]) == 1) {
					RtConvItem curitem = m_makeupItems((*streamItr).first,m_rtcms[(*streamItr).first]);
					if (curitem.obslist.size() > 0) {
						for (rtItr = m_svrs.begin(); rtItr != m_svrs.end(); ++rtItr) {
							(*rtItr)->inputObs(curitem);
						}
					}
				}
			}
		}
		sleepms(50);
	}
	/// will delete memory here
	for (streamItr = m_streams.begin(); streamItr != m_streams.end(); ++streamItr) {
		strclose((*streamItr).second);
	}
	for (rtcmItr = m_rtcms.begin(); rtcmItr != m_rtcms.end(); ++rtcmItr) {
		free_rtcm((*rtcmItr).second);
		free((*rtcmItr).second);
	}
	m_streams.clear();
	m_rtcms.clear();
}
void VrsNtripClient::m_sendGGAReq(stream_t* stream, VrsStaItem& item) {


}
RtConvItem VrsNtripClient::m_makeupItems(string staname,rtcm_t* rtcm){
	int iobs,isys,ifreq,gotobs = false;
	RtConvItem item;
	char cprn[16] = { 0 }, code[4] = { 0 };
	map<string, RtSatObs> mapObs;
	for (iobs = 0; iobs < rtcm->obs.n; iobs++) {
		RtSatObs sat;
		satno2id(rtcm->obs.data[iobs].sat,cprn);
		if (-1 == pointer_string(configs_sav.nprn, configs_sav.cprn, cprn))
			continue;
		isys = index_string(SYS, cprn[0]);
		for (ifreq = 0; ifreq < sizeof(rtcm->obs.data[iobs].L) / sizeof(double);ifreq++){
			if (rtcm->obs.data[iobs].P[ifreq] != 0.0)
				gotobs = true;
			strcpy(code, code2obs(rtcm->obs.data[iobs].code[ifreq], NULL));
			sat.obs[ifreq] = rtcm->obs.data[iobs].L[ifreq];
			sat.fob[ifreq] = "L" + string(code);

			sat.obs[MAXFREQ + ifreq] = rtcm->obs.data[iobs].P[ifreq];
			sat.fob[ifreq] = "C" + string(code);

			sat.snr[ifreq] = rtcm->obs.data[iobs].SNR[ifreq];
		}
		if (gotobs) mapObs[cprn] = sat;
	}
	item.curt = rtcm->time;
	if(mapObs.size() > 0)
		item.obslist[staname] = mapObs;
	return item;
}
void VrsNtripClient::m_adaptConfigures() {










}