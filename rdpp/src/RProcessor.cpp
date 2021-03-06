/*
 * File: RProcess.h
 * Description: main pre processor source file
 * Author: Oxnz
 * Version: @VERSION@
 * Mail: yunxinyi@gmail.com
 * Copying: Copyright (C) 2013, All rights reserved.
 * This file is part of the @PACKAGE@ project
 *
 * Revision: -*-
 * Last-update: 2013-11-07 22:10:12
 */

#include "RProcessor.h"
#include "RsidGen.h"
#include "RConstant.h"
#include "RHelper.h"

#include "../libnz/NZLogger.h"

#include <cstring>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <exception>
#include <stdexcept>
#include <algorithm>

using namespace std;
using NZ::NZLogger;
using RHelper::getTSIndex;

/*
 * @description: default constructor for RDProcessor
 * @params:
 * 	-i indir: input dir which contains original records file
 * 	-i outdir: output dir which used to store the output files
 * 	-i minPerTS: minute(s) per Time Slot
 * 	-i bufsize: buffer size used to store original records file
 * 	-i process: whether to process the buffer with RDPool
 */
R::Processor::Processor(const char* indir, const char* outdir,
                        size_t minPerTS, size_t bufsize, bool process)
throw(invalid_argument, bad_alloc)
try
    : m_indir(indir),
      m_outdir(outdir),
      m_pTSPool(new set<orec_key>[24*60/minPerTS]),
      m_nMinPerTS(minPerTS),
      m_nTSCnt(24*60/minPerTS),
      m_tsp(0),
      m_nBufSize(bufsize),
	  m_pFileBuffer(new char[bufsize]),
      m_bProcess(process),
      m_pRDPool(process ? new RDP::RDPool(RsidGen::MAX_RSID, 24*60/minPerTS)
                : nullptr)
{
    if (m_indir.length() * m_outdir.length() == 0)
        throw invalid_argument("invalid parameter: indir or outdir is null");
    if (m_nMinPerTS < 2)
        throw invalid_argument("invalid parameter: MinPerTS too small");
    else if (24*60%m_nMinPerTS)
        throw invalid_argument("invalid paramter: 24*60 mod MinPerTS != 0");
    if (m_nBufSize <= 1024*1024)
        throw invalid_argument("invalid paremter: buf size is too small");
    if (*m_indir.rbegin() != '/')
        m_indir.append("/");
    if (*m_outdir.rbegin() != '/')
        m_outdir.append("/");
} catch (...) {
	/* handle the constructor exceptions
	 * clean up and rethrow the exception
	 */
	if (m_pTSPool)
		delete m_pTSPool;
	if (m_pFileBuffer)
		delete m_pFileBuffer;
	if (m_pRDPool)
		delete m_pRDPool;
	throw;
}

/*
 * @description: process every single orignal record
 * @params:
 * 	-i rec: const original record references
 * 	-i echo: echo the record content for debug use
 * @returns:
 * 	-o int: 0 for success, other stands for fails
inline int R::Processor::processOrigRecord(const in_rec& rec, bool echo) {
	if (echo)
    std::cout << "(" << rec.cid << "," <<rec.event
             << "," << rec.status << "," << (rec.time)
             << "," << rec.x << "," << rec.y
             << "," << rec.speed << "," << rec.direct
             << "," << rec.valid << ")" << endl;
    orec_key key(RsidGen::get_rsid(rec.x*10000000, rec.y*10000000, false));
     // @advice: skip the wrong road id
	if (key == RsidGen::INVALID_RSID) {
//        NZLog(NZLogger::LogLevel::WARNING, "invalid road segment ID");
		return 0;
	}
	key = (key << 32) | rec.cid;
    if (abs(static_cast<int64_t>(rec.time - m_tsp)/10000)) {
        NZLog(NZLogger::LogLevel::DEBUG, "invalid timestamp: " + to_string(rec.time)
                      + " - current time: " + to_string(m_tsp) + " = "
                      + to_string(abs(static_cast<int64_t>(rec.time - m_tsp)
                                      /10000)));
        return 0;
    } else {
        m_tsp = rec.time;
    }
     
    m_pTSPool[getTSIndex(m_nMinPerTS, rec.time)].insert(key);

    return 0;
}*/

/*
 * @description: parse the contents in file buffer
 * @params:
 * 	none
 * @returns:
 * 	-o int: 0 for success and others for failure
 */
int R::Processor::processFileBuffer() {
    char* p = m_pCurFBufPos;
    for (in_rec irec; p < m_pFileBufEnd - 1; ++p) {
        irec.cid = 0;
        while (*(++p) != ',')
            irec.cid = irec.cid * 10 + *p - 0x30;
        irec.event = *(++p) - 0x30;
        p += 2;
        irec.status = *p - 0x30;
        ++p;
        irec.time = 0;
        while (*(++p) != ',')
            irec.time = irec.time * 10 + *p - 0x30;
        irec.x = 0;
        while (*(++p) != ',')
            if (*p != '.')
                irec.x = irec.x * 10 + *p - 0x30;
        irec.y = 0;
        while (*(++p) != ',')
            if (*p != '.')
                irec.y = irec.y * 10 + *p - 0x30;
        while (*(++p) != ',')
            ;
        while (*(++p) != ',')
            ;
        irec.valid = *(++p) - 0x30;
        if (*(++p) != 0x0d) { // simple check if EOL
			NZLog(NZLogger::LogLevel::ERROR,
					"parse buffer error(%d, %d, %d, %d, %d, %d, %d, %d, %d)"
					", press s to skip or other to abort",
					irec.cid, irec.event, irec.status, irec.time, irec.x,
					irec.y, irec.speed, irec.direct, irec.valid);
            if (getchar() != 's')
				return -1;
			else
				continue;
        }
        if (!irec.valid || irec.status != NON_OCCUPIED) continue;
        // skip invalid ts index
		if (abs(static_cast<int64_t>(irec.time - m_tsp)/10000)) {
			NZLog(NZLogger::LogLevel::WARNING, "invalid time stamp: %d(tsp=%d)",
					irec.time, m_tsp);
			continue;
		} else // update ts pointer
			m_tsp = irec.time;
		// CAUTION: true is equidisant
    	orec_key key(RsidGen::get_rsid(irec.x, irec.y));
		if (key == RsidGen::INVALID_RSID) { // skip invalid rsid
			NZLog(NZLogger::LogLevel::WARNING,
					"invalid rsid: %d(coord: %d %d), skipped",
					key, irec.x, irec.y);
			continue;
		}
		m_pTSPool[getTSIndex(m_nMinPerTS, irec.time)].insert(
				(key << 32) | irec.cid);
	}
    
    return 0;
}

ssize_t R::Processor::readFileIntoMem(const char* fpath) {
    NZLog(NZLogger::LogLevel::INFO, "reading [%s] ...", fpath);
    ifstream infile(fpath);
    if (!infile.is_open()) {
        NZLog(NZLogger::LogLevel::ERROR, "open file [%s] failed", fpath);
        return -1;
    }
    infile.seekg(0, ios::end);
    size_t fsize = infile.tellg();
    if (fsize > m_nBufSize) {
        NZLog(NZLogger::LogLevel::ERROR, "file size is larger than the buffer size");
        return -1;
    }
    m_pFileBufEnd = m_pFileBuffer + fsize;
    m_pCurFBufPos = (char *)m_pFileBuffer;
    NZLog(NZLogger::LogLevel::INFO, "file size: %l", fsize);
    infile.seekg(0, ios::beg);
    infile.read((char *)m_pFileBuffer, fsize);
    infile.close();
    return fsize;
}

/*
 * @description: dump data & js array for analysis and debug use
 * @returns:
 * 	-o int: whether dump succeed
 */
int R::Processor::dumpRecords() {
    string fpath = m_outdir + to_string(m_tsp/1000000) + ".dat";
    ofstream outfile(fpath.c_str(), ios::out|ios::binary);
    if (!outfile.is_open()) {
        NZLog(NZLogger::LogLevel::FATAL, "cannot open file [%s]", fpath.c_str());
        return -1;
    }
    ofstream outjson(fpath.append(".js").c_str(), ios::out);
    if (!outjson.is_open()) {
        NZLog(NZLogger::LogLevel::FATAL, "cannot open file [%s]",
				(fpath+".js").c_str());
        return -1;
    }
    outjson << "var data" << m_tsp/1000000 << " = new Array(" << endl;
    NZLog(NZLogger::LogLevel::INFO, "dumping to file [%s] ...", fpath.c_str());
    NZLog(NZLogger::LogLevel::INFO, "dumping to file [%s.js] ...", fpath.c_str());
	
	cout << "\t\t\tstatistics of day: " << m_tsp/1000000 << endl << 
		"------------------------------------------------------------------------";

    size_t cnt;
    roadseg_id x;
    for (size_t i = 0; i < m_nTSCnt; ++i) {
        cnt = 0;
        x = roadseg_id(i);
        outfile.write(reinterpret_cast<const char*>(&x),
                      sizeof(roadseg_id));
        for (set<orec_key>::iterator it = m_pTSPool[i].begin();
             it != m_pTSPool[i].end(); ++it) {
            ++cnt;
			x = (*it) >> 32;
            outfile.write(reinterpret_cast<const char*>(&x),
                          sizeof(roadseg_id));
        }
        m_pTSPool[i].clear();
        x = RsidGen::INVALID_RSID;
        outfile.write(reinterpret_cast<const char*>(&x),
                      sizeof(roadseg_id));
        x = roadseg_id(cnt);
        outfile.write(reinterpret_cast<const char*>(&x),
                      sizeof(roadseg_id));
        if (i % 10 == 0) {
            //cout << endl << setw(6) << left << i << "  ";
            cout << endl << setw(6) << setfill('0') << i << "  ";
            outjson << endl;
        }
        //cout << setw(6) << setfill(' ') << left << cnt << " ";
        cout << setw(6) << setfill(' ') << cnt << " ";
        outjson << cnt << ",";
    }
	cout << endl <<
	"========================================================================"
    << endl;
    outfile.close();
    outjson << ");" << endl;
    outjson.close();
    NZLog(NZLogger::LogLevel::INFO, "dump to file [%s] successfully",
			fpath.c_str());
    return 0;
}

/*
 * @description: process a single specified by date
 * @params:
 * 	-i date: specify the date to process
 * 	-i progbar: whether show the progress status
 * @returns:
 * 	-o int: { 1: no file was found, 0: success, -1: error }
 */
int R::Processor::process(uint32_t date, bool progbar) {
	NZLog(NZLogger::LogLevel::INFO, "processing day %u", date);
	int ret(0);
	m_tsp = date*1000000;
	string indir = m_indir + to_string(date);
	ret = RHelper::find_files(indir.c_str(), to_string(date/100).c_str(),
			m_fileList);
	if (ret == -1) {
		NZLog(NZLogger::LogLevel::FATAL, "find files error");
		return -1;
	} else if (!ret) {
		NZLog(NZLogger::LogLevel::WARNING, "no file was found");
		return 1;
	}
	size_t fcnt(ret);
	while (m_fileList.size()) {
		if (progbar)
			RHelper::print_progress((fcnt-m_fileList.size())*100/fcnt);
		NZLog(NZLogger::LogLevel::INFO, "processing %s",
				m_fileList.front().c_str());
		if (readFileIntoMem(m_fileList.front().c_str()) <= 0) {
			NZLog(NZLogger::LogLevel::ERROR, "read file failed");
			return -1;
		}
		m_fileList.pop_front();
		ret = processFileBuffer();
		if (ret == -1) {
			NZLog(NZLogger::LogLevel::ERROR, "process file buffer failed");
			return -1;
		}
	}
	if (progbar)
		RHelper::print_progress(100);
	if (m_bProcess && m_pRDPool->process(m_pTSPool)) {
		NZLog(NZLogger::LogLevel::FATAL, "RDP process failed, skipped");
	}
	if (dumpRecords()) {
		NZLog(NZLogger::LogLevel::FATAL, "dump to file failed");
		return -1;
	}
	return 0;
}

/*
 * @description: process multi days specified by dates
 */
int R::Processor::process(std::list<uint32_t>& dates, bool progbar) {
	string indir;
	int ret(0);
	//int fcnt(0);
	while (!dates.empty()) {
		process(dates.front(), progbar);
		dates.pop_front();
		/*
		m_tsp = dates.front();
		dates.pop_front();
		indir = m_indir + to_string(m_tsp);
		ret = RHelper::find_files(indir.c_str(),
				to_string(m_tsp/100).c_str(),
				m_fileList);
		m_tsp *= 1000000;
		if (ret == -1) {
			NZLog(NZLogger::LogLevel::FATAL, "find files error");
            return -1;
        } else if (!ret) {
            NZLog(NZLogger::LogLevel::WARNING, "no file was found");
            return 1;
        } else {
            NZLog(NZLogger::LogLevel::INFO, "processing day %u, %u files, m_tsp = %u",
					m_tsp/1000000, ret, m_tsp);
        }

        fcnt = m_fileList.size();
        while (m_fileList.size()) {
            if (progbar) {
                RHelper::print_progress((fcnt - m_fileList.size())*100/fcnt);
            }
            NZLog(NZLogger::LogLevel::INFO, "processing %s", m_fileList.front());
            if (readFileIntoMem(m_fileList.front().c_str()) <= 0) {
                NZLog(NZLogger::LogLevel::ERROR, "read file into memory failed");
                return -1;
            }
            m_fileList.pop_front();
            ret = processFileBuffer();
            if (ret == -1) {
                NZLog(NZLogger::LogLevel::ERROR, "process file buffer failed");
                return -1;
            }
        }
        if (progbar)
            RHelper::print_progress(100);
        if (m_bProcess && m_pRDPool->process(m_pTSPool)) {
            NZLog(NZLogger::LogLevel::FATAL, "RDP process failed, skipped");
        }
        if (dumpRecords()) {
            NZLog(NZLogger::LogLevel::FATAL, "dump to file failed");
            return -1;
        }*/
	}
	NZLog(NZLogger::LogLevel::INFO, "DO NOT FORGET TO UNCOMMENT THIS");
    if (m_bProcess && m_pRDPool->dump(m_outdir + to_string(m_tsp/1000000)
                                      + ".rsd")) {
        NZLog(NZLogger::LogLevel::FATAL, "RDP dump failed");
        return ret;
    }
    
	return 0;
}

/*
 * @description: process a sequence of days start by data and have lenght
 * 	of len, normally used to process for the first time
 */
int R::Processor::process(uint32_t date, size_t len, bool progbar) {
    string indir;
    int ret(0);
    int fcnt(0);
    for (size_t i = 0; i < len; ++i, ++date) {
        m_tsp = date;
        m_tsp *= 1000000;
        indir = m_indir + to_string(date);
        ret = RHelper::find_files(indir.c_str(),
                         to_string(date/100).c_str(),
                         m_fileList);
        if (ret == -1) {
            NZLog(NZLogger::LogLevel::FATAL, "find files error");
            return -1;
        } else if (!ret) {
            NZLog(NZLogger::LogLevel::WARNING, "no file was found");
            return 1;
        } else {
            NZLog(NZLogger::LogLevel::INFO, "processing day %u, %u files, m_tsp = %u",
					date, ret, m_tsp);
        }

        fcnt = m_fileList.size();
        while (m_fileList.size()) {
            if (progbar) {
                RHelper::print_progress((fcnt - m_fileList.size())*100/fcnt);
            }
            NZLog(NZLogger::LogLevel::INFO, "processing %s",
					m_fileList.front().c_str());
            if (readFileIntoMem(m_fileList.front().c_str()) <= 0) {
                NZLog(NZLogger::LogLevel::ERROR, "read file into memory failed");
                return -1;
            }
            m_fileList.pop_front();
            ret = processFileBuffer();
            if (ret == -1) {
                NZLog(NZLogger::LogLevel::ERROR, "process file buffer failed");
                return -1;
            }
        }
        if (progbar)
            RHelper::print_progress(100);
        if (m_bProcess && m_pRDPool->process(m_pTSPool)) {
            NZLog(NZLogger::LogLevel::FATAL, "RDP process failed, skipped");
        }
        if (dumpRecords()) {
            NZLog(NZLogger::LogLevel::FATAL, "dump to file failed");
            return -1;
        }
	}
	NZLog(NZLogger::LogLevel::INFO, "DO NOT FORGET TO UNCOMMENT THIS");
    if (m_bProcess && m_pRDPool->dump(m_outdir + to_string(m_tsp/1000000)
                                      + ".rsd")) {
        NZLog(NZLogger::LogLevel::FATAL, "RDP dump failed");
        return ret;
    }
    
	return ret;
}

/*
 * @description: destructor, clean up
 */
R::Processor::~Processor() {
    if (m_fileList.size())
        m_fileList.clear();
    for (int i = 0; i < 480; ++i) {
        if (m_pTSPool[i].size())
            m_pTSPool[i].clear();
    }
    delete[] m_pTSPool;
    delete[] m_pFileBuffer;
	if (m_bProcess)
		delete m_pRDPool;
}
