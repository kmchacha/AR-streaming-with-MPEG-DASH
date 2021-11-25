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
#include <cstdlib>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUF_SIZE 10
void error_handling(char *message);

#if defined _WIN32 || defined _WIN64
    #include <Windows.h>
#endif

using namespace dash;
using namespace dash::network;
using namespace libdashtest;
using namespace std;
using namespace dash::mpd;

//const string filepath = "/video/loot.mpd";
const int BIN_COUNT = 10;

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
	int sock;
	char ret[BUF_SIZE];
	socklen_t adr_sz;
	struct sockaddr_in serv_adr, from_adr;

	sock = socket(PF_INET, SOCK_DGRAM, 0);
	if(sock == -1)
		error_handling("socket() error");

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_adr.sin_port = htons(atoi(argv[3]));

    IDASHManager    *manager        = CreateDashManager();
    HTTPConnection  *httpconnection = new HTTPConnection();
	
    ofstream file;
	int number = atoi(argv[1]);
	char *quality = argv[2];

	if(number == 0) {
    	TestChunk test1chunk("203.252.121.219", 80, filepath, 0, 0, false);

    	httpconnection->Init(&test1chunk);
    	httpconnection->Schedule(&test1chunk);

    	cout << "*****************************************" << endl;
    	cout << "* Download files with external HTTP 1.0 *" << endl;
    	cout << "*****************************************" << endl;
    	cout << "Testing download of MPD file:\t";
    	file.open("mcnl.mpd", ios::out | ios::binary);
    	download(httpconnection, &test1chunk, &file);
		file.close();
    	cout << "finished!" << endl;
    	delete(httpconnection);
	}

	cout << "*****************************************" << endl;
    cout << "*********  MPD  files  Anaysis **********" << endl;
    cout << "*****************************************" << endl;	
		
	IMPD *mpd = manager->Open("mcnl.mpd");
	std::string baseUrl;
	baseUrl = mpd->GetBaseUrls().at(0)->GetUrl();
	string high[BIN_COUNT], mid[BIN_COUNT], low[BIN_COUNT];
	for(int i=0;i<BIN_COUNT;i++){
		high[i] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(0)->GetSegmentList()->GetSegmentURLs().at(i)->GetMediaURI();
		mid[i] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(1)->GetSegmentList()->GetSegmentURLs().at(i)->GetMediaURI();
		low[i] = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(2)->GetSegmentList()->GetSegmentURLs().at(i)->GetMediaURI();
	}

	double HIGH_QUALITY = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(0)->GetBandwidth();
	double MID_QUALITY = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(1)->GetBandwidth();
	double LOW_QUALITY = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(2)->GetBandwidth();

	std::string  fileName, urls;
	if(!strcmp(quality,"High")) fileName = high[number];
	else if(!strcmp(quality,"Mid")) fileName = mid[number];
	else fileName = low[number];

	int idx = fileName.find("/");
	fileName= fileName.substr(idx + 1);
	cout << "fileName : " << fileName << endl;

	if(!strcmp(quality,"High")) urls = baseUrl + high[number];
	else if(!strcmp(quality,"Mid")) urls = baseUrl + mid[number];
	else urls = baseUrl + low[number];

	cout << "*****************************************" << endl;
	cout << "* Download files with external HTTP 1.0 *" << endl;
	cout << "*****************************************" << endl;
	TestChunk test2chunk("203.252.121.219", 80, urls, 0, 0, false);
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
	
	if(Bandwidth >= HIGH_QUALITY) strcpy(ret, "High");
	else if(Bandwidth >= MID_QUALITY) strcpy(ret, "Mid");
	else strcpy(ret, "Low");
	
	cout << "JINA: " << ret << endl;

	sendto(sock, ret, strlen(ret), 0, (struct sockaddr*)&serv_adr, sizeof(serv_adr));
	ret[strlen(ret)] = 0;
	adr_sz = sizeof(from_adr);

	close(sock);
	return 0;
}

void error_handling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
