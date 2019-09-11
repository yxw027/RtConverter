#ifndef DEPLOY_H_
#define DEPLOY_H_
#include "Const.h"
#include <string>
#include <map>
#include <list>
#include "../Rnxobs/Log.h"
using namespace std;
namespace bamboo {
	class VrsStaItem{
	public:
		double x[3];
		string staname;
	};
	class Deploy {
	public:
		Deploy();
		static void s_initInstance(int argc,char* args[]) {
			if (sInstance == NULL)
				sInstance = new Deploy(argc,args);
		}
		~Deploy();
		static Deploy s_getConfigures();
		static bool s_updateConfigures ();
		int mjd0,mjd1;
		double sod0, sod1,seslen,dintv;
		int nfreq[MAXSYS],nprn,rtkport,nrtkport;
		char freq[MAXSYS][MAXFREQ][LEN_FREQ];
		char obsdir[256],outdir[256];
		string cprn[MAXSAT];
		time_t lastAct;
		list<string> post_stalist;
		map<string, list<VrsStaItem>> rt_mounts;
	protected:
		Deploy(int,char*[]);
	
		bool m_checkStation(string);
		void m_readConfiguresJson(bool);
		char f_jsonConfigures[1024];
		static Deploy* sInstance;
		static def_lock_t s_mutex;
		static int s_count;
	};
}
#endif

