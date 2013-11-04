#include "RSIDGen.h"
#include "RType.h"
#include "OutRecord.h"

#include <iostream>
#include <fstream>
#include <cassert>

using namespace std;
#using "C:\Users\xinyi\Documents\Visual Studio 2012\Projects\RDPP\Debug\map_match_CL1.dll"
using namespace map_match_CL1;

int test_get_roadseg_id(void) {
	double x,y,x1,y1;
	x=116.3284302;
	y=39.9685555;
	x1=123.0;
	y1=123.0;
	Match ^f = gcnew Match();
	
	f->OpenMapFile("C:/Users/xinyi/Documents/Visual Studio 2012/Projects/RDPP/Debug/network63.mxd");
	int ID=f->getRoadID(x1,y1);
    cout<<ID;

    return 0;
}

int test_read_preped_file(const char *fpath) {
    ifstream infile(fpath, ios::in|ios::binary);
    if (!infile.is_open()) {
        cerr << "can't open infile" << endl;
        return -1;
    }
    arch_rec rec;
    for (int i = 0; i < 10; ++i) {
        infile.read(reinterpret_cast<char*>(&rec.rsid), sizeof(roadseg_id));
        infile.read(reinterpret_cast<char*>(&rec.orecv.status), sizeof(car_status));
        infile.read(reinterpret_cast<char*>(&rec.orecv.time), sizeof(gps_time));
        cout << "rsid: " << rec.rsid << " orecv status: " << rec.orecv.status
             << "time: " << rec.orecv.time << endl;
    }
    infile.close();
    return 0;
}

/*
int main(int argc, char *argv[]) {
    printf("testing get_roadseg_id:\n");
    test_get_roadseg_id();
	return 0;
    if (test_read_preped_file("outfile") == -1) {
        cerr << "test read preprocessed file failed" << endl;
    }

    return -1;
}
*/
