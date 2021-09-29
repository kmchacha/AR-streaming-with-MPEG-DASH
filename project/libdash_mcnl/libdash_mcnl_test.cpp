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

#if defined _WIN32 || defined _WIN64
    #include <Windows.h>
#endif

using namespace dash;
using namespace dash::network;
using namespace libdashtest;
using namespace std;
using namespace dash::mpd;

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
	
	string filepath = "/video/loot.mpd";

    TestChunk test1chunk("127.0.0.1", 80, filepath, 0, 0, false);

    httpconnection->Init(&test1chunk);
    httpconnection->Schedule(&test1chunk);

    ofstream file;
    cout << "*****************************************" << endl;
    cout << "* Download files with external HTTP 1.0 *" << endl;
    cout << "*****************************************" << endl;
    cout << "Testing download of MPD file:\t";
    file.open("loot.mpd", ios::out | ios::binary);
    download(httpconnection, &test1chunk, &file);
    file.close();
    cout << "finished!" << endl;

    delete(httpconnection);
	

	cout << "*****************************************" << endl;
    cout << "*********  MPD  files  Anaysis **********" << endl;
    cout << "*****************************************" << endl;	
		
	IMPD *mpd = manager->Open("loot.mpd");

	std::string baseUrl;
	baseUrl = mpd->GetBaseUrls().at(0)->GetUrl();
	cout << "baseUrl: "<< baseUrl << endl;
	
	string high[10];
	for(int i=0;i<10;i++){
		high[i] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(0)->GetSegmentList()->GetSegmentURLs().at(i)->GetMediaURI();
	}
/*
	high[0] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(0)->GetSegmentList()->GetSegmentURLs().at(0)->GetMediaURI();
	high[1] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(0)->GetSegmentList()->GetSegmentURLs().at(1)->GetMediaURI();
	high[2] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(0)->GetSegmentList()->GetSegmentURLs().at(2)->GetMediaURI();
	high[3] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(0)->GetSegmentList()->GetSegmentURLs().at(3)->GetMediaURI();
	high[4] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(0)->GetSegmentList()->GetSegmentURLs().at(4)->GetMediaURI();
	high[5] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(0)->GetSegmentList()->GetSegmentURLs().at(5)->GetMediaURI();
	high[6] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(0)->GetSegmentList()->GetSegmentURLs().at(6)->GetMediaURI();
	high[7] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(0)->GetSegmentList()->GetSegmentURLs().at(7)->GetMediaURI();
	high[8] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(0)->GetSegmentList()->GetSegmentURLs().at(8)->GetMediaURI();
	high[9] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(0)->GetSegmentList()->GetSegmentURLs().at(9)->GetMediaURI();

	cout << "high0 : " << high[0] << endl;
	cout << "high1 : " << high[1] << endl;
	cout << "high2 : " << high[2] << endl;
	cout << "high3 : " << high[3] << endl;
	cout << "high4 : " << high[4] << endl;
	cout << "high5 : " << high[5] << endl;
	cout << "high6 : " << high[6] << endl;
	cout << "high7 : " << high[7] << endl;
	cout << "high8 : " << high[8] << endl;
	cout << "high9 : " << high[9] << endl;
*/	
	string mid[10];
	for(int i=0;i<10;i++){
		mid[i] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(1)->GetSegmentList()->GetSegmentURLs().at(i)->GetMediaURI();
	}
	/*
	mid[0] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(1)->GetSegmentList()->GetSegmentURLs().at(0)->GetMediaURI();
	mid[1] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(1)->GetSegmentList()->GetSegmentURLs().at(1)->GetMediaURI();
	mid[2] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(1)->GetSegmentList()->GetSegmentURLs().at(2)->GetMediaURI();
	mid[3] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(1)->GetSegmentList()->GetSegmentURLs().at(3)->GetMediaURI();
	mid[4] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(1)->GetSegmentList()->GetSegmentURLs().at(4)->GetMediaURI();
	mid[5] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(1)->GetSegmentList()->GetSegmentURLs().at(5)->GetMediaURI();
	mid[6] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(1)->GetSegmentList()->GetSegmentURLs().at(6)->GetMediaURI();
	mid[7] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(1)->GetSegmentList()->GetSegmentURLs().at(7)->GetMediaURI();
	mid[8] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(1)->GetSegmentList()->GetSegmentURLs().at(8)->GetMediaURI();
	mid[9] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(1)->GetSegmentList()->GetSegmentURLs().at(9)->GetMediaURI();
	

	cout << "mid0 : " << mid[0] << endl;
	cout << "mid1 : " << mid[1] << endl;
	cout << "mid2 : " << mid[2] << endl;
	cout << "mid3 : " << mid[3] << endl;
	cout << "mid4 : " << mid[4] << endl;
	cout << "mid5 : " << mid[5] << endl;
	cout << "mid6 : " << mid[6] << endl;
	cout << "mid7 : " << mid[7] << endl;
	cout << "mid8 : " << mid[8] << endl;
	cout << "mid9 : " << mid[9] << endl;
*/
	string low[10];
	for(int i=0;i<10;i++){
		low[i] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(2)->GetSegmentList()->GetSegmentURLs().at(i)->GetMediaURI();
	}
	/*
	low[0] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(2)->GetSegmentList()->GetSegmentURLs().at(0)->GetMediaURI();
	low[1] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(2)->GetSegmentList()->GetSegmentURLs().at(1)->GetMediaURI();
	low[2] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(2)->GetSegmentList()->GetSegmentURLs().at(2)->GetMediaURI();
	low[3] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(2)->GetSegmentList()->GetSegmentURLs().at(3)->GetMediaURI();
	low[4] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(2)->GetSegmentList()->GetSegmentURLs().at(4)->GetMediaURI();
	low[5] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(2)->GetSegmentList()->GetSegmentURLs().at(5)->GetMediaURI();
	low[6] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(2)->GetSegmentList()->GetSegmentURLs().at(6)->GetMediaURI();
	low[7] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(2)->GetSegmentList()->GetSegmentURLs().at(7)->GetMediaURI();
	low[8] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(2)->GetSegmentList()->GetSegmentURLs().at(8)->GetMediaURI();
	low[9] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(2)->GetSegmentList()->GetSegmentURLs().at(9)->GetMediaURI();
	
	cout << "low1 : " << low[0] << endl;
	cout << "low2 : " << low[1] << endl;
	cout << "low3 : " << low[2] << endl;
	cout << "low4 : " << low[3] << endl;
	cout << "low5 : " << low[4] << endl;
	cout << "low6 : " << low[5] << endl;
	cout << "low7 : " << low[6] << endl;
	cout << "low8 : " << low[7] << endl;
	cout << "low9 : " << low[8] << endl;
	cout << "low10 : " << low[9] << endl;
*/
	cout << "argc : " << argc << endl;
	cout << "argv[0]: " << argv[0] << endl;
	cout << "argv[1]: " << argv[1] << endl;
	
	std::string  fileName;
	int number = atoi(argv[1]);
	cout << "number: " << number << endl;

	if(number < 3) fileName = high[number];
	else if(number < 6) fileName = low[number];
	else fileName = mid[number];

	

	int idx = fileName.find("/");
	fileName = fileName.substr(idx + 1);
	cout << "fileName : " << fileName << endl;
	string urls;
	if(number < 3) urls = baseUrl + high[number];
	else if(number < 6) urls = baseUrl + low[number];
	else urls = baseUrl + mid[number];
	cout << "urls : " << urls << endl;

	cout << "*****************************************" << endl;
	cout << "* Download files with external HTTP 1.0 *" << endl;
	cout << "*****************************************" << endl;
	TestChunk test2chunk("127.0.0.1", 80, urls, 0, 0, false);
	httpconnection = new HTTPConnection();
	httpconnection->Init(&test2chunk);
	httpconnection->Schedule(&test2chunk);
	std::cout << "Testing download of video file:\t";
	file.open(fileName, ios::out | ios::binary);
	download(httpconnection, &test2chunk, &file);
	file.close();
	cout << "finished!" << endl;



	delete(mpd);
	delete(manager);

	return 0;
}
