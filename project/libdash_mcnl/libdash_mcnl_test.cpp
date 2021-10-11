/*
 * libdash_networkpart_test.cpp
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#include "libdash.h"
#include "TestChunk.h"
#include "PersistentHTTPConnection.h"

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

#if defined _WIN32 || defined _WIN64
    #include <Windows.h>
#endif

using namespace dash;
using namespace dash::network;
using namespace libdashtest;
using namespace std;
using namespace dash::mpd;

const string filepath = "/video/loot.mpd";

void download(IConnection *connection, IChunk *chunk, ofstream *file)
{
    int     len     = 32768;
    uint8_t *p_data = new uint8_t[32768];

    int ret = 0;
    do
    {
        ret = connection->Read(p_data, len, chunk);
        if(ret > 0)
            file->write((char *)p_data, ret);
    }while(ret > 0);
}

int main(int argc, char*argv[])
{
    IDASHManager    *manager        = CreateDashManager();
    HTTPConnection  *httpconnection = new HTTPConnection();
	
    ofstream file;
	int number = atoi(argv[1]);
	int quality = atoi(argv[2]);

	if(number == 0) {
    	TestChunk test1chunk("203.252.121.219", 80, filepath, 0, 0, false);

    	httpconnection->Init(&test1chunk);
    	httpconnection->Schedule(&test1chunk);

    	cout << "*****************************************" << endl;
    	cout << "* Download files with external HTTP 1.0 *" << endl;
    	cout << "*****************************************" << endl;
    	cout << "Testing download of MPD file:\t";
    	file.open("loot.mpd", ios::out | ios::binary);
    	download(httpconnection, &test1chunk, &file);
		file.close();
    	cout << "finished!" << endl;
    	delete(httpconnection);
	}

	cout << "*****************************************" << endl;
    cout << "*********  MPD  files  Anaysis **********" << endl;
    cout << "*****************************************" << endl;	
		
	IMPD *mpd = manager->Open("loot.mpd");
	std::string baseUrl;
	baseUrl = mpd->GetBaseUrls().at(0)->GetUrl();
	string high[10], mid[10], low[10];
	for(int i=0;i<10;i++){
		high[i] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(0)->GetSegmentList()->GetSegmentURLs().at(i)->GetMediaURI();
		mid[i] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(1)->GetSegmentList()->GetSegmentURLs().at(i)->GetMediaURI();
		low[i] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(2)->GetSegmentList()->GetSegmentURLs().at(i)->GetMediaURI();
	}

	double HIGH_QUALITY = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(0)->GetBandwidth();
	double MID_QUALITY = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(1)->GetBandwidth();
	double LOW_QUALITY = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(2)->GetBandwidth();

	std::string  fileName, urls;
	if(quality == 0) fileName = high[number];
	else if(quality == 1) fileName = mid[number];
	else fileName = low[number];

	int idx = fileName.find("/");
	fileName= fileName.substr(idx + 1);
	cout << "fileName : " << fileName << endl;

	if(quality == 0) urls = baseUrl + high[number];
	else if(quality == 1) urls = baseUrl + mid[number];
	else urls = baseUrl + low[number];

	cout << "*****************************************" << endl;
	cout << "* Download files with external HTTP 1.0 *" << endl;
	cout << "*****************************************" << endl;
	TestChunk test2chunk("127.0.0.1", 80, urls, 0, 0, false);
	httpconnection = new HTTPConnection();
	httpconnection->Init(&test2chunk);
	httpconnection->Schedule(&test2chunk);
	std::cout << "Testing download of video file:\t";
	file.open(fileName, ios::out | ios::binary);
	
	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
	download(httpconnection, &test2chunk, &file);
	std::chrono::duration<double> sec = std::chrono::system_clock::now() - start;
	
	file.seekp(0, ios::end);
	double file_size = file.tellp();	
	file.close();
	cout << "finished!" << endl;

	delete(mpd);
	delete(manager);

	double T = sec.count();
	cout << "Time : " << T << " file_size/time: " << (file_size * 8) / T <<  endl;
	double Bandwidth = (file_size * 8) / T;
	if(Bandwidth >= HIGH_QUALITY) return 0;
	else if(Bandwidth >= MID_QUALITY) return 1;
	else return 2;

	return -1;
}
