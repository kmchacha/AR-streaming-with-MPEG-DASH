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
#include "./../../include/libdash_networkpart/TestChunk.h"
#include "./../../include/libdash_networkpart/PersistentHTTPConnection.h"
#include <fstream>
#include <pthread.h>
#include <cstring>
#include <semaphore.h>
#include <cstdlib>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace open3d;
using namespace open3d::visualization;
using namespace std;
using namespace dash;
using namespace dash::network;
using namespace libdashtest;
using namespace dash::mpd;

const int WIDTH = 1024;
const int HEIGHT = 768;
std::string MPD_DATA_PATH;
std::string PLY_DATA_PATH = "/home/mcnl/mcnl/project/mcnl/source/cpp/dec_test/";
int idx;
vector<string> files;
const Eigen::Vector3f CENTER_OFFSET(0.0f, 0.0f, -3.0f);
const std::string CLOUD_NAME = "points";

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
			int file_size = files.size();
			geometry::AxisAlignedBoundingBox bounds;
			

			while (main_vis_) {
				std::string full_path = PLY_DATA_PATH + files[idx++];

				if(file_size <= idx) {
				//	idx=0; // For repetition
					//main_vis_RemoveGeometry(CLOUD_NAME);
					main_vis_->Close();
					break;
				}

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
				
				if(idx == 1) main_vis_->ResetCameraToDefault(); 

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

void *
libdash_thread(void *ptr)
{
	cout << "Hello, Lib-dash Thread\n";
	pthread_t tid;
	char msg[256];
	vector<string> v;

	tid = pthread_self();
	for(auto & p : std::experimental::filesystem::directory_iterator(MPD_DATA_PATH))
	{
		string name = p.path().filename().string();
		v.push_back(name);
	}
	
	sort(v.begin(), v.end());
	for(int i = 0 ; i < v.size() ; i++) {
		//snprintf(msg, 256, "[%ld, %s]", (unsigned long) tid, v[i].c_str());
		snprintf(msg, v[i].size()+1, "%s", v[i].c_str());
		cout << msg << endl;
		bounded_buffer_queue(buf1, strdup(msg));
	}

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
	ifstream f1("/home/mcnl/mcnl/project/mcnl/source/cpp/decOpt.txt");
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
			sprintf(path, "../../../../mpeg-vpcc/mpeg-pcc-tmc2/bin/%s", "PccAppDecoder");

			pid_t pid, waitpid;
			int status;
			pid = fork();
			if(pid == 0) {
				sprintf(comprename,"--compressedStreamPath=%s%s",MPD_DATA_PATH.c_str(), msg);
				msg[strlen(msg)-4] = 0x0;
				
				char dir_path[1000];
				sprintf(dir_path, "./../dec_test/%s",msg);
				cout << "dir_path : " << dir_path << endl;
				mkdir(dir_path, 0755);
				sprintf(reconname,"--reconstructedDataPath=../dec_test/%s/%s_dec%%04d.ply", msg ,msg);

				execl(path, "PccAppDecoder", comprename, opt[0].c_str(), opt[1].c_str(), opt[2].c_str(), reconname, NULL);

			}else {
				waitpid = wait(&status);
				//execl("/bin/ls", "ls", "-al", ".ply" , "|", "wc", "-al", NULL);
				
				string str = msg;
				str = str.substr(0, str.length()-4);
				bounded_buffer_queue(buf2, strdup(str.c_str()));
				free(msg);
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
	for(int i = 0 ; i < 5 ; i++) {
		msg = bounded_buffer_dequeue(buf2);
		if(msg != 0x0) {
			//printf("OPEN3D Thread [%ld] reads %s\n", (unsigned long) tid, msg);
			//execl("/bin/ls", "ls", "-al", ".ply" , "|", "wc", "-al", NULL);
			pid_t pid, waitpid;
			int status;
			pid = fork();

			
			if(pid == 0) {
				// file count == 10, Make directory and push the file path
				string directoryCountPath = "./../dec_test/"+ string(msg) + "*.ply";
				execl("/bin/ls", "ls", "-al", directoryCountPath.c_str() , "|", "wc", "-al", NULL);
			}else {
				waitpid = wait(&status);
			}

			/**
			string directory_path;
			directory_path = PLY_DATA_PATH + string(msg); 
			
			for(auto & p : std::experimental::filesystem::directory_iterator(directory_path)) 
			{
				string filepath = string(msg) + "/" + p.path().filename().string();
				files.push_back(filepath);
				cout << filepath << endl;
			}
			sort(files.begin(), files.end());
			MultipleWindowsApp().Run();
			files.clear();
			free(msg);
			idx = 0;
			**/
		}
	}
	
	return 0x0;
}

int main(int argc, char *argv[]) {
	
	pthread_t thread1;
	pthread_t thread2;
	pthread_t thread3;

	buf1 = (bounded_buffer *)malloc(sizeof(bounded_buffer));
	bounded_buffer_init(buf1, 10);
	
	buf2 = (bounded_buffer *)malloc(sizeof(bounded_buffer));
	bounded_buffer_init(buf2, 10);

	// Temporary
	std::cout << "EX: /home/mcnl/mydisk/binfile/low/" << std::endl;
	std::cout << "PATH=";
	std::cin >> MPD_DATA_PATH;
	
	//
	pthread_create(&thread1, 0x0, libdash_thread, 0x0);
	pthread_create(&thread2, 0x0, mpeg_vpcc_thread, 0x0);
	pthread_create(&thread3, 0x0, open3d_thread, 0x0);
	
	cout << "CHECK" << endl;
	
	pthread_join(thread1, 0x0);
	pthread_join(thread2, 0x0);
	pthread_join(thread3, 0x0);
	cout << "END\n";

	/*
		MultipleWindowsApp().Run();
	*/
	return 0;
}
