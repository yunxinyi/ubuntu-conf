/*
 * File: RDPool.cpp
 * Description: Road Data Pool impl
 * Author: Oxnz
 * Mail: yunxinyi@gmail.com
 * This file is part of the @PACKAGE@ project
 * Copying: Copyright (C) 2013, All rights reserved.
 *
 * Version: @VERSION@
 * Revision: -*-
 * Last-update: 2013-11-07 22:10:12
 */

#include "RDPool.h"
#include "RsidGen.h"
#include "NZLogger.h"
#include "RHelper.h"

#include <cmath>
#include <ctime>
#include <fstream>
#include <stdexcept>
#include <exception>

#ifdef DEBUG
#include <cassert>
#endif

using NZ::NZLogger;
/*
using std::cout;
using std::endl;
using std::to_string;
using std::ifstream;
using std::ofstream;
using std::ios;
using std::string;
*/
using namespace std;

RDP::RDPool::RDPool(size_t nrs, size_t nts, const char* fpath)
    : m_nrs(nrs),
      m_nts(nts),
      m_pp(new car_count[nrs*nts])
{
    NZLogger::log(NZ::DEBUG, "creating RDP (nts = %u, nrs = %u) ...",
			m_nts, m_nrs);
	if (fpath) {
		ifstream infile(fpath);
		if (!infile.is_open())
			throw runtime_error(string("cannot open file: ") + fpath);
		infile.read(reinterpret_cast<char*>(m_pp),
				m_nrs*m_nts*sizeof(car_count));
		infile.close();
	} else {
		for (uint64_t* p = reinterpret_cast<uint64_t*>(m_pp);
				p < reinterpret_cast<uint64_t*>(m_pp+(m_nrs*m_nts/(sizeof(uint64_t)/sizeof(car_count))));
				++p)
			*p = 0;
	}
    NZLogger::log(NZ::DEBUG, "RDP created");
}

car_count* RDP::RDPool::operator[](roadseg_id rsid) {
    if (rsid <= 0 || rsid > m_nrs) {
        NZLogger::log(NZ::FATAL, "invalid rsid: %u", rsid);
        return nullptr;
    }
    return m_pp + (rsid-1) * m_nts;
}

const car_count& RDP::RDPool::operator()(roadseg_id rsid, ts_index tsi) const {
    if (rsid <= 0 || rsid > m_nrs || tsi < 0 || tsi >= m_nts)
        NZLogger::log(NZ::FATAL,
				"invalid rsid or TS index: rsid(%u), tsi(%u)", rsid, tsi);
	return m_pp[(rsid-1)*m_nts+tsi];
}

car_count& RDP::RDPool::operator()(roadseg_id rsid, ts_index tsi) {
    if (rsid <= 0 || rsid > m_nrs || tsi < 0 || tsi >= m_nts)
        NZLogger::log(NZ::FATAL,
				"invalid rsid or TS index: rsid(%u), tsi(%u)", rsid, tsi);
	return m_pp[(rsid-1)*m_nts+tsi];
}

int RDP::RDPool::process(const std::set<orec_key>* ptsm) {
	roadseg_id rsid;
    NZLogger::log(NZ::INFO, "RDP processing ...");
    for (ts_index i = 0; i < m_nts; ++i) {
        for (std::set<orec_key>::const_iterator it =
                 ptsm[i].begin(); it != ptsm[i].end(); ++it) {
			/* three lines below is to play safe, could be removed in release
			 * version
			 */
			rsid = *it >> 32;
            if (rsid <= 0 || rsid >= m_nrs) {
                NZLogger::log(NZ::ERROR, "invalid rsid: %u", *it);
            } else {
                ++((m_pp + (rsid-1) * m_nts)[i]);
				/* debug use
				if (rsid == 110488)
					cout << endl << "INCRESE: tsi=" << i << " count="
						<< (*this)(rsid, i) << endl;
				*/
			}
        }
    }
    NZLogger::log(NZ::INFO, "RDP process done");    
    return 0;
}

#define ENV_CNT 4
int RDP::RDPool::query(size_t mpts) {
	ifstream flist[ENV_CNT] = { ifstream("x"),
		ifstream("out/20121101.rsd"),
		ifstream("Z"),
		ifstream("X")
	};
	int dcnt[ENV_CNT] = { 1, 1, 3, 4};
	for (size_t i = 0; i < ENV_CNT; ++i) {
		if (!flist[i].is_open()) {
			NZLogger::log(NZ::ERROR, "%s: cannot open file", __FUNCTION__);
			//return -1;
		}
	}
	clock_t t0;
	size_t nts(24*60/mpts);
	car_count cnt;
	roadseg_id rsid;
	char line[256];
	char buf[256];
	double x, y;
	int yy, mm, dd, h, m, s;
	gps_time t;
	double p;
	ts_index tsi;
	size_t envi;
	cin.getline(line, 256);
	line[strlen(line)-1] = '\0';
	cout << line << "打到车的概率	平均等待时间(分钟)	运行时间(秒)" << endl;
	cin.getline(line, 256);
	while(sscanf(line, "(%lf, %lf)\t%s %d/%d/%d %d:%d:%d\r\n", &x, &y, buf,
				&yy, &mm, &dd, &h, &m, &s) == 9) {
		t0 = clock();
		t = yy*10000000000 + mm*100000000 + dd*1000000 + h*10000 + m*100 + s;
		tsi = RHelper::getTSIndex(mpts, t);
		NZLogger::log(NZ::DEBUG, "%u, %u, %d, tsi = %d\n",
				static_cast<gps_x>(x*10000000), static_cast<gps_y>(y*10000000),
				t, tsi);
		rsid = RsidGen::get_rsid2(x*10000000, y*10000000);
		if (rsid == RsidGen::INVALID_RSID) {
			NZLogger::log(NZ::WARNING, "invalid coordinates, %f %f", x, y);
			continue;
		}
		envi = RHelper::get_envi(t);
		flist[envi].seekg(((rsid-1)*nts+tsi)*sizeof(car_count),
				ios_base::beg);
		flist[envi].read(reinterpret_cast<char*>(&cnt), sizeof(car_count));
		line[strlen(line)-1] = '\0';
		//p = 1 - pow(M_E, (cnt*(-0.1))/dcnt[envi]);
		p = 1 - exp(cnt*(-1.0)/dcnt[envi]);
		printf("%s\t%.2f\t%.2lf\t%.3lf\n", line, p,
				(1.0*dcnt[envi])*mpts/cnt, (1.0*clock() - t0)/CLOCKS_PER_SEC);
		/*
		cout << line << "\t" << cnt//"\t0.85\t5\t1.025"
			<< endl;
			*/
		cin.getline(line, 256);
	}
	for (size_t i = 0; i < ENV_CNT; ++i)
		flist[i].close();
	return 0;
}

/*
car_count RDP::RDPool::query(const roadseg_id rsid, const ts_index tsi) const {
	const char* fpath = nullptr;
	ifstream infile("out/20121101.rsd");
	cout << "opening out/20121101.rsd" << endl;
	if (!infile.is_open()) {
		NZLogger::log(NZ::FATAL, "open file %s failed", fpath);
		return -1;
	}
	car_count cnt;
	infile.seekg(((rsid-1)*m_nts+tsi)*sizeof(car_count), ios_base::beg);
	infile.read(reinterpret_cast<char*>(&cnt), sizeof(car_count));
	infile.close();
	cout << "looking for: " << ((rsid-1)*m_nts + tsi)*sizeof(car_count)
		<< " have car: " << cnt << endl;
	return cnt;
	//return (*this)(rsid, tsi);
}

car_count RDP::RDPool::query_interactive() {
	//cout << "Enter longitude, lantitude & ts index to query: " << endl;
	gps_time t;
	roadseg_id rsid;
	for (double x(0), y(0); std::cin >> x >> y >> t;) {
		rsid = RsidGen::get_rsid2(static_cast<gps_x>(x*10000000),
				static_cast<gps_y>(y*10000000));
		if (rsid == RsidGen::INVALID_RSID) {
			std::cerr << "Invalid longitude or lantitude" << endl;
			continue;
		}
		cout << "rsid: " << rsid << " ts index: " << t << " car count: "
		<< query(rsid, t) << endl;
		getchar();
	}
	return 0;
}
*/

int RDP::RDPool::dump(const string& fpath) {
    NZLogger::log(NZ::NOTICE, "RDP dumping to file [%s]:", fpath);
    ofstream outfile(fpath, ios::out|ios::binary);
    if (!outfile.is_open()) {
        NZLogger::log(NZ::FATAL, "cannot open file [%s]", fpath);
        return -1;
    }
    for (roadseg_id i = 0; i < m_nrs; ++i) {
        RHelper::print_progress(i*100/m_nrs);
        for (ts_index j = 0; j < m_nts; ++j) {
            outfile.write(reinterpret_cast<const char*>(m_pp + i*m_nts + j),
                          sizeof(car_count));
        }
    }
    RHelper::print_progress(100);
    outfile.close();
    NZLogger::log(NZ::INFO, "RDP dump success");
    return 0;
}

RDP::RDPool::~RDPool() {
    delete m_pp;
}
