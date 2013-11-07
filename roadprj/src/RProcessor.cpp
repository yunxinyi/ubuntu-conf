/*
 *            File: processor.cpp
 *     Description: Main Pre Processor Source File
 *    Last-updated: 2013-11-07 15:25:53 CST
 *          Author: Oxnz
 *         Version: 0.1
 */


#include "RProcessor.h"
#include "RsidGen.h"
#include "RConstant.h"
#include "RHelper.h"

#include <cstring>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <exception>
#include <stdexcept>
#include <algorithm>

using namespace std;

Processor::Processor(const char* indir, const char* outdir, size_t minPerTS,
                     size_t bufsize)
    : m_indir(indir),
      m_outdir(outdir),
      m_nMinPerTS(minPerTS),
      m_nTSCnt(24*60/minPerTS),
      m_tsp(0),
      m_nBufSize(bufsize)
{
    if (m_indir.length() * m_outdir.length() == 0)
        throw logic_error("invalid parameter: indir or outdir is null");
    if (m_nMinPerTS < 2)
        throw logic_error("invalid parameter: MinPerTS too small");
    else if (24*60%m_nMinPerTS)
        throw logic_error("invalid paramter: 24*60 mod MinPerTS != 0");
    if (m_nBufSize <= 1024*1024)
        throw logic_error("invalid paremter: buf size is too small");
    if (*m_indir.rbegin() != '/')
        m_indir.append("/");
    if (*m_outdir.rbegin() != '/')
        m_outdir.append("/");
    m_pFileBuffer = new char[m_nBufSize];
    m_pTSPool = new map<const orec_key, void*>[24*60/minPerTS];
}

/*
 * @description: get time slot index
 */
inline size_t Processor::getTSIndex(const gps_time& time) {
    uint16_t h = static_cast<uint16_t>(time % 1000000/10000);
    uint16_t m = time % 10000/100;
    uint16_t s = time % 100;
    return (h*60*60+m*60+s)/(m_nMinPerTS*60);
}

inline int Processor::processOrigRecord(const in_rec& rec) {
#ifdef DEBUG
	cout << "DEBUG: (" << rec.cid << "," << rec.event << ","
		<< rec.status << "," << rec.time << ","
		<< rec.x << "," << rec.y << "," << rec.speed << ","
		<< rec.direct << "," << rec.valid << ")" << endl;
#endif
    gps_coord coord = {static_cast<gps_x>(rec.x * 10000000),
                       static_cast<gps_y>(rec.y * 10000000)};
    orec_key key = {get_rsid2(coord), rec.cid};
    /*
     * @advice: skip the wrong road id
     */
    if (key.rsid == 0) { cerr << "WARNING: invalid road id" << endl; return 0; }
    if (abs(static_cast<int64_t>((rec.time - m_tsp)/10000))) {
#ifdef WARNING
        cout << "WARNING: invalid timestamp : " << rec.time
             << " current time: " << m_tsp << endl;
#endif
        return 0;
    }
     
    m_pTSPool[getTSIndex(rec.time)
              ].insert(make_pair(key, static_cast<void*>(0)));
#ifdef DEBUG
	cout << "INFO: roadseg_id: " << key.rsid << " car_id: " << key.cid << endl;
#endif
    
    return 0;
}

int Processor::processFileBuffer() {
	char* p;
#ifdef DEBUG
	int i = 0;
#endif
	for (in_rec irec; m_pCurFBufPos < m_pFileBufEnd - 1; ++m_pCurFBufPos) {
		irec.cid = strtoul(m_pCurFBufPos, &p, 10);
		m_pCurFBufPos = p;
		irec.event = strtoul(++m_pCurFBufPos, &p, 10);
		m_pCurFBufPos = p;
		irec.status = strtoul(++m_pCurFBufPos, &p, 10);
		m_pCurFBufPos = p;
		irec.time = strtoull(++m_pCurFBufPos, &p, 10);
		m_pCurFBufPos = p;
		irec.x = strtod(++m_pCurFBufPos, &p);
		m_pCurFBufPos = p;
		irec.y = strtod(++m_pCurFBufPos, &p);
		m_pCurFBufPos = p;
		irec.speed = strtoul(++m_pCurFBufPos, &p, 10);
		m_pCurFBufPos = p;
		irec.direct = strtoul(++m_pCurFBufPos, &p, 10);
		m_pCurFBufPos = p;
		irec.valid = strtoul(++m_pCurFBufPos, &p, 10);
		m_pCurFBufPos = p;
#ifdef DEBUG
		if (++i == 1 || i == 20200)
			cout << "DEBUG: (" << irec.cid << "," << irec.event << ","
				<< irec.status << "," << irec.time << ","
				<< irec.x << "," << irec.y << "," << irec.speed << ","
				<< irec.direct << "," << irec.valid << ")" << endl;
#endif
		if (!irec.valid || irec.status != NON_OCCUPIED) continue;
        if(processOrigRecord(irec)) {
            cerr << "ERROR: process record failed, record:"
                 << "(" << irec.cid << "," << irec.event << ","
                 << irec.status << "," << irec.time << ","
                 << irec.x << "," << irec.y << "," << irec.speed << ","
                 << irec.direct << "," << irec.valid << ")" << endl;

            return -1;
        }
	}

	return 0;
}

ssize_t Processor::readFileIntoMem(const char* fpath) {
    cout << "INFO: reading [" << fpath << "] ..." << endl;
    ifstream infile(fpath);
    if (!infile.is_open()) {
        cerr << "ERROR: open file [" << fpath << " ] failed" << endl;
        return -1;
    }
    infile.seekg(0, ios::end);
    ssize_t fsize = static_cast<ssize_t>(infile.tellg());
    if (fsize > m_nBufSize) {
        cerr << "ERROR: file size larger than buffer size" << endl;
        return -1;
    }
    m_pFileBufEnd = m_pFileBuffer + fsize;
    m_pCurFBufPos = (char *)m_pFileBuffer;
    cout << "INFO: file size: " << fsize << endl;
    infile.seekg(0, ios::beg);
    infile.read((char *)m_pFileBuffer, fsize);
    infile.close();
    return fsize;
}

int Processor::dumpRecords() {
    string fpath = m_outdir + to_string(m_tsp/1000000) + ".dat";
    ofstream outfile(fpath.c_str(), ios::out|ios::binary);
    if (!outfile.is_open()) {
        cerr << "CRITICAL: can't open file [" << fpath << "]" << endl;
        return -1;
    }
    cout << "INFO: dumping to file [" << fpath << "] ...";
    size_t cnt;
    roadseg_id x;
    for (size_t i = 0; i < m_nTSCnt; ++i) {
        cnt = 0;
        for (map<const orec_key, void*>::iterator it = m_pTSPool[i].begin();
             it != m_pTSPool[i].end(); ++it) {
            ++cnt;
            outfile.write(reinterpret_cast<const char*>(&it->first.rsid),
                          sizeof(roadseg_id));
        }
        m_pTSPool[i].clear();
        x = INVALID_RSID;
        outfile.write(reinterpret_cast<const char*>(&x),
                      sizeof(roadseg_id));
        x = roadseg_id(i);
        outfile.write(reinterpret_cast<const char*>(&x),
                      sizeof(roadseg_id));
        x = roadseg_id(cnt);
        outfile.write(reinterpret_cast<const char*>(&x),
                      sizeof(roadseg_id));
        if (i % 10 == 0)
            cout << endl << setw(6) << setfill('0') << left << i;
        cout << setw(6) << setfill(' ') << left << cnt << " ";
    }
    cout << endl << "INFO: dump to file [" << fpath << "] successfully" << endl;
    outfile.close();
    return 0;
}

int Processor::process(uint32_t date, size_t len) {
    string indir;
    int ret;
    for (size_t i = 0; i < len; ++i) {
        m_tsp = (date+i) * 1000000;
        indir = m_indir + to_string(date);
        ret = find_files((m_indir + to_string(date)).c_str(),
                         to_string(date/100).c_str(),
                         m_fileList);
        if (ret == -1) {
            cerr << "FATAL ERROR: find files error" << endl;
            return -1;
        } else if (!ret) {
            cerr << "ERROR: no file was found" << endl;
            return -1;
        }

        while (m_fileList.size()) {
            cout << "INFO: ========> processing " << m_fileList.front() << endl;
            if (readFileIntoMem(m_fileList.front().c_str()) <= 0) {
                cerr << "ERROR: read file into mem failed" << endl;
                return -1;
            }
            m_fileList.pop_front();
            ret = processFileBuffer();
            if (ret == -1) {
                cerr << "ERROR: process file buffer failed" << endl;
                return -1;
            }
        }
        
        if (dumpRecords()) {
            cerr << "FATAL ERROR: dump to file failed" << endl;
            return -1;
        }
	}
    
	return 0; // indicate there's no more files to be processed
}

Processor::~Processor() {
    if (m_fileList.size())
        m_fileList.clear();
    for (int i = 0; i < 480; ++i) {
        if (m_pTSPool[i].size())
            m_pTSPool[i].clear();
    }
    delete[] m_pTSPool;
}
