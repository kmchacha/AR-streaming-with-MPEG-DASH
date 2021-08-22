/*
	MCNL-ARstreaming Capston Project - Client
*/

#include <atomic>
#include <chrono>
#include <mutex>
#include <random>
#include <thread>
#include <vector>
#include <experimental/filesystem>
#include <iostream>
#include <string>
#include <algorithm>
#include "open3d/Open3D.h"
#include "libdash.h"
//#include "../../include/libdash_networkpart/TestChunk.h"
//#include "../../include/libdash_networkpart/PersistentHTTPConnection.h"
#include "TestChunk.h"
#include "PersistentHTTPConnection.h"

#include <fstream>
#include <pthread.h>
#include <cstring>
#include <semaphore.h>
#include <cstdlib>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>

#include "IMPD.h"
#include "INode.h"

using namespace open3d;
using namespace open3d::visualization;
using namespace std;
using namespace dash;
using namespace dash::network;
using namespace libdashtest;
using namespace dash::mpd;

const int WIDTH = 1024;
const int HEIGHT = 1024; // 768
const int PLY_COUNT_PER_BIN = 150;
const int PLY_PER_DIRECTORY = 10;

//std::string MPD_DATA_PATH = "/video/loot.mpd";
std::string MPD_DATA_PATH;
int idx = 0;
//const Eigen::Vector3f CENTER_OFFSET(0.0f, 0.0f, -3.0f);
//const std::string CLOUD_NAME = "points";

typedef struct {
	sem_t filled;
	sem_t empty;
	pthread_mutex_t lock;
	char ** elem;
	int capacity;
	int num;
	int front;
	int rear;
} bounded_buffer;

bounded_buffer * buf1 = 0x0;
bounded_buffer * buf2 = 0x0;

void bounded_buffer_init(bounded_buffer * buf, int capacity) {
	sem_init(&(buf->filled), 0, 0);
	sem_init(&(buf->empty), 0, capacity);
	pthread_mutex_init(&(buf->lock), 0x0);
	buf->capacity = capacity;
	buf->elem = (char **) calloc(sizeof(char *), capacity);
	buf->num = 0;
	buf->front = 0;
	buf->rear = 0;
}

void bounded_buffer_queue(bounded_buffer * buf, char * msg) {
	sem_wait(&(buf->empty));
	pthread_mutex_lock(&(buf->lock));
		buf->elem[buf->rear] = msg;
		buf->rear = (buf->rear + 1) % buf->capacity;
		buf->num += 1;
	pthread_mutex_unlock(&(buf->lock));
	sem_post(&(buf->filled));
}

char * bounded_buffer_dequeue(bounded_buffer * buf) {
	char * r = 0x0;
	
	sem_wait(&(buf->filled));
	pthread_mutex_lock(&(buf->lock));
		r = buf->elem[buf->front];
		buf->front = (buf->front + 1) % buf->capacity;
		buf->num -= 1;
	pthread_mutex_unlock(&(buf->lock));
	sem_post(&(buf->empty));
	return r;
}

void download(IConnection *connection, IChunk *chunk, ofstream *file)
{
	int len = 32768;
	uint8_t *p_data = new uint8_t[32768];

	int ret = 0;
	do
	{
		ret = connection->Read(p_data, len, chunk);
		if(ret > 0)
			file->write((char *)p_data, ret);
	}while(ret > 0);
}

/*
class MultipleWindowsApp {
	public:
		MultipleWindowsApp() {
			is_done_ = false;

			gui::Application::GetInstance().Initialize();
		}

		void Run() {
			main_vis_ = std::make_shared<visualizer::O3DVisualizer>(
					"Open3D - MCNL Demo", WIDTH, HEIGHT);
			main_vis_->AddAction(
					"Take snapshot in new window",
					[this](visualizer::O3DVisualizer &) { this->OnSnapshot(); });
			main_vis_->SetOnClose([this]() { return this->OnMainWindowClosing(); });
			
			gui::Application::GetInstance().AddWindow(main_vis_);
			auto r = main_vis_->GetOSFrame();
			snapshot_pos_ = gui::Point(r.x, r.y);


			std::thread read_thread([this]() { this->ReadThreadMain(); });
			gui::Application::GetInstance().Run();
			read_thread.join();
		}

	private:
		void OnSnapshot() {
			n_snapshots_ += 1;
			snapshot_pos_ = gui::Point(snapshot_pos_.x + 50, snapshot_pos_.y + 50);
			auto title = std::string("Open3D - MCNL Demo (Snapshot #") +
				std::to_string(n_snapshots_) + ")";
			auto new_vis = std::make_shared<visualizer::O3DVisualizer>(title, WIDTH,
					HEIGHT);

			geometry::AxisAlignedBoundingBox bounds;
			{
				std::lock_guard<std::mutex> lock(cloud_lock_);
				auto mat = rendering::Material();
				mat.shader = "defaultUnlit";
				new_vis->AddGeometry(
						CLOUD_NAME + " #" + std::to_string(n_snapshots_), cloud_,
						&mat);
				bounds = cloud_->GetAxisAlignedBoundingBox();
			}

			new_vis->ResetCameraToDefault();
			auto center = bounds.GetCenter().cast<float>();
			//new_vis->SetupCamera(60, center, center + CENTER_OFFSET,
			//		{0.0f, -1.0f, 0.0f});
			gui::Application::GetInstance().AddWindow(new_vis);
			auto r = new_vis->GetOSFrame();
			new_vis->SetOSFrame(
					gui::Rect(snapshot_pos_.x, snapshot_pos_.y, r.width, r.height));
		}

		bool OnMainWindowClosing() {
			// Ensure object is free so Filament can clean up without crashing.
			// Also signals to the "reading" thread that it is finished.
			main_vis_.reset();
			return true;  // false would cancel the close
		}

	private:
		void ReadThreadMain() {
			// This is NOT the UI thread, need to call PostToMainThread() to
			// update the scene or any part of the UI.
			geometry::AxisAlignedBoundingBox bounds;
			char * msg;	

			while (main_vis_) {
				msg = bounded_buffer_dequeue(buf2);
				std::string full_path = string(msg);

				Eigen::Vector3d extent;
				{
					std::lock_guard<std::mutex> lock(cloud_lock_);
					cloud_ = std::make_shared<geometry::PointCloud>();
					io::ReadPointCloudFromPLY(full_path, *cloud_, {"auto",false,false,false,{}});
					bounds = cloud_->GetAxisAlignedBoundingBox();
					extent = bounds.GetExtent();
				}

				
				auto mat = rendering::Material();
				mat.shader = "defaultUnlit";

				gui::Application::GetInstance().PostToMainThread(
						main_vis_.get(), [this, bounds, mat]() {
						std::lock_guard<std::mutex> lock(cloud_lock_);
						main_vis_->RemoveGeometry(CLOUD_NAME);
						main_vis_->AddGeometry(CLOUD_NAME, cloud_, &mat);
							
						//main_vis_->ResetCameraToDefault();
						//Eigen::Vector3f center = bounds.GetCenter().cast<float>();
						//main_vis_->SetupCamera(60, center, center + CENTER_OFFSET,
						//		{0.0f, -1.0f, 0.0f});
						});

				Eigen::Vector3d magnitude = 0.005 * extent;
				auto seed = std::random_device()();
				std::mt19937 gen_algo(seed);
				std::uniform_real_distribution<> random(-0.5, 0.5);

				std::this_thread::sleep_for(std::chrono::milliseconds(100));


				// Perturb the cloud with a random walk to simulate an actual read
				{
					std::lock_guard<std::mutex> lock(cloud_lock_);
					for (size_t i = 0; i < cloud_->points_.size(); ++i) {
						Eigen::Vector3d perturb(magnitude[0] * random(gen_algo),
								magnitude[1] * random(gen_algo),
								magnitude[2] * random(gen_algo));
						cloud_->points_[i] = perturb;
					}
				}
				
				if(idx++ == 1) main_vis_->ResetCameraToDefault(); 

				if (!main_vis_) {  // might have changed while sleeping
					break;
				}

			}
		}

	private:
		std::mutex cloud_lock_;
		std::shared_ptr<geometry::PointCloud> cloud_;

		std::atomic<bool> is_done_;
		std::shared_ptr<visualizer::O3DVisualizer> main_vis_;
		int n_snapshots_ = 0;
		gui::Point snapshot_pos_;
};
*/

void *
libdash_thread(void *ptr)
{
	cout << "Hello, Lib-dash Thread\n";
	pthread_t tid;
	char msg[256];
	vector<string> v;

	tid = pthread_self();

	IDASHManager *manager = CreateDashManager();
	HTTPConnection *httpconnection = new HTTPConnection();

	TestChunk test1chunk("127.0.0.1", 80, MPD_DATA_PATH, 0, 0, false);

	httpconnection->Init(&test1chunk);
	httpconnection->Schedule(&test1chunk);

	ofstream file;
	cout << "*****************************************" << endl;
	cout << "* Download files with external HTTP 1.0 *" << endl;
	cout << "*****************************************" << endl;
	file.open("test.mpd", ios::out | ios::binary);
	download(httpconnection, &test1chunk, &file);
	file.close();
	cout << "finished!" << endl;
	
	delete(httpconnection);

	cout << "******************************************" << endl;
	cout << "********** MPD Files Analysis ************" << endl;
	cout << "******************************************" << endl;

	IMPD *mpd = manager->Open("./test.mpd");
	if(mpd == NULL) {
		cout << "I have no idea!\n";
		exit(0);
	}

	vector<IAdaptationSet *> adap = mpd->GetPeriods().at(0)->GetAdaptationSets();
	vector<IRepresentation *> rep = adap.at(0)->GetRepresentation();
	cout << "max width: " << adap.at(0)->GetMaxWidth() << "\n";


	string baseUrl;
	baseUrl = mpd->GetBaseUrls().at(0)->GetUrl();
	cout << "\n\n\nbaseUrl : "<< baseUrl << endl;

	cout << "BaseURL Check\n";

	string high1 = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(0)->GetSegmentList()->GetSegmentURLs().at(0)->GetMediaURI();
	string high2 = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(0)->GetSegmentList()->GetSegmentURLs().at(1)->GetMediaURI();

	cout << "File 1 Check\n";
	cout << high1 << endl;
	cout << high2 << endl;
	
	string mid1 = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(1)->GetSegmentList()->GetSegmentURLs().at(0)->GetMediaURI();
	string mid2 = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(1)->GetSegmentList()->GetSegmentURLs().at(1)->GetMediaURI();


	string low1 = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(2)->GetSegmentList()->GetSegmentURLs().at(0)->GetMediaURI();
	string low2 = mpd->GetPeriods().at(0)->GetAdaptationSets().at(0)->GetRepresentation().at(2)->GetSegmentList()->GetSegmentURLs().at(1)->GetMediaURI();

	string fileName, url;
	int idx;

	fileName = high1;
	idx = fileName.find("/");
	fileName = fileName.substr(idx + 1);
	cout << "fileName : " << fileName << endl;
	url = baseUrl + high1;
	cout << "url : " << url << endl;

	cout << "*****************************************\n";
	cout << "* Download files with external HTTP 1.0 *\n";
	cout << "*****************************************\n";
	TestChunk test2chunk("127.0.0.1", 80, url, 0, 0, false);
	httpconnection = new HTTPConnection();
	httpconnection->Init(&test2chunk);
	httpconnection->Schedule(&test2chunk);
	cout << "Testing download of bin file:\t";
	file.open(fileName, ios::out | ios::binary);
	download(httpconnection, &test2chunk, &file);
	file.close();
	v.push_back(fileName);
	cout << "finished!" << endl;



	fileName = high2;
	idx = fileName.find("/");
	fileName = fileName.substr(idx + 1);
	cout << "fileName : " << fileName << endl;
	url = baseUrl + high2;
	cout << "url : " << url << endl;

	cout << "*****************************************\n";
	cout << "* Download files with external HTTP 1.0 *\n";
	cout << "*****************************************\n";
	TestChunk test3chunk("127.0.0.1", 80, url, 0, 0, false);
	httpconnection = new HTTPConnection();
	httpconnection->Init(&test3chunk);
	httpconnection->Schedule(&test3chunk);
	cout << "Testing download of bin file:\t";
	file.open(fileName, ios::out | ios::binary);
	download(httpconnection, &test3chunk, &file);
	file.close();
	v.push_back(fileName);
	cout << "finished!" << endl;

	sort(v.begin(), v.end());
	for(int i = 0 ; i < v.size() ; i++) {
		//snprintf(msg, 256, "[%ld, %s]", (unsigned long) tid, v[i].c_str());
		snprintf(msg, v[i].size()+1, "%s", v[i].c_str());
		cout << msg << endl;
		bounded_buffer_queue(buf1, strdup(msg));
	}
	
	delete(mpd);
	delete(manager);
	
	return 0x0;
}
void * 
mpeg_vpcc_thread(void *ptr)
{
	cout << "Hello, MPEG-VPCC Thraed\n";
	pthread_t tid;
	char * msg;
	char line[1001] = {0, };
	vector<string> opt;
	ifstream f1("/home/mcnl/mcnl/project/mcnl/source_backup/cpp/Main/decOpt.txt");
	if(!f1) {
		cerr << "file open error\n";
	}
	while(f1.getline(line, 1001)) {
		opt.push_back(line);
	}

	tid = pthread_self();
	for(int i = 0 ; i < 5 ; i++) {
		msg = bounded_buffer_dequeue(buf1);
		if(msg != 0x0) {
			printf("MPEG-VPCC Thread [%ld] read %s\n", (unsigned long) tid, msg);

			char path[1001], comprename[1001], reconname[1001];
			sprintf(path, "../../../../../mpeg-vpcc/mpeg-pcc-tmc2/bin/%s", "PccAppDecoder");
			
			msg[strlen(msg)-4] = 0x0;
			char dir_path[101];
			sprintf(dir_path, "./../../dec_test/%s",msg);
			cout << "dir_path : " << dir_path << endl;
			
			pid_t pid, waitpid;
			int status;
			pid = fork();
			if(pid == 0) {
				cout << "1" << endl;
				sprintf(comprename,"--compressedStreamPath=%s.bin", msg);	
				mkdir(dir_path, 0755);
				sprintf(reconname,"--reconstructedDataPath=../../dec_test/%s/%s_dec_%%04d.ply", msg ,msg);
				execl(path, "PccAppDecoder", comprename, opt[0].c_str(), opt[1].c_str(), opt[2].c_str(), reconname, NULL);
			}else {
				char ply_count[101];
				char dir[256];
				int cnt = 0;
				char command[101];
				char ply_path[101];
	
				sprintf(ply_path, "ls -l /home/mcnl/mcnl/project/mcnl/source_backup/cpp/dec_test/%s/*.ply | wc -l", msg);
				while(1) {
					FILE *fp = popen(ply_path, "r");
					fgets(ply_count, 10, fp);
					cout << "\n" << ply_count << endl;
					if(atoi(ply_count) == 10) {
						sprintf(dir, "/home/mcnl/mcnl/project/mcnl/source_backup/cpp/dec_test/%s/ply%02d", msg ,cnt++);
						mkdir(dir, 0755);
						sprintf(command, "mv %s/*.ply %s", dir_path, dir);
						
						system(command);
						
						cout << "\n\n\n\n\nPUSH :" << dir << endl;
				
						string directory_path;
						directory_path = string(dir); 
						
						vector<string> files;
						for(auto & p : std::experimental::filesystem::directory_iterator(directory_path)) 
						{
							string filepath = string(dir) + "/" + p.path().filename().string();
							files.push_back(filepath);
							//cout << filepath << endl;
						}
						sort(files.begin(), files.end());
						
						for(int i = 0 ; i < files.size() ; i++) {
							string filepath = files[i];
							cout << "PUSH: " << filepath << endl;
							bounded_buffer_queue(buf2, strdup(filepath.c_str()));
						}
					}
					pclose(fp);
					if(cnt == (PLY_COUNT_PER_BIN / PLY_PER_DIRECTORY)) break;
					//printf("cnt : %d, WEXITSTATUS: %d\n", cnt, WEXITSTATUS(status));
				}
				
				waitpid = wait(&status);
				free(msg);
				kill(pid, SIGKILL);
			}
		}

	}
	return 0x0;
}

void * 
open3d_thread(void *ptr)
{
	cout << "Hello, Open3d Thread\n";
	pthread_t tid;
	char * msg;

	tid = pthread_self();
	//MultipleWindowsApp().Run();
	cout << "Bye, Open3d Thread\n";
	return 0x0;
}

int main(int argc, char *argv[]) {
	
	pthread_t thread1;
	pthread_t thread2;
	pthread_t thread3;

	cout << "ex) /video/loot.mpd" << endl;
	cin >>  MPD_DATA_PATH;
	cout << MPD_DATA_PATH << endl;
	buf1 = (bounded_buffer *)malloc(sizeof(bounded_buffer));
	bounded_buffer_init(buf1, 10);
	
	buf2 = (bounded_buffer *)malloc(sizeof(bounded_buffer));
	bounded_buffer_init(buf2, 100);

	pthread_create(&thread1, 0x0, libdash_thread, 0x0);
//	pthread_create(&thread2, 0x0, mpeg_vpcc_thread, 0x0);
//	pthread_create(&thread3, 0x0, open3d_thread, 0x0);
	
	cout << "CHECK" << endl;
	
	pthread_join(thread1, 0x0);
//	pthread_join(thread2, 0x0);
//	pthread_join(thread3, 0x0);
	cout << "END\n";

	return 0;
}
