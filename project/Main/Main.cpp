/*
	MCNL-ARstreaming Capston Project - Client
	Created by KyungMin Cha, JiNa Choi
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

const string PATH = "/home/mcnl/mcnl/project/mcnl/gitTest2";
const int WIDTH = 1024;
const int HEIGHT = 1024;
const int PLY_COUNT_PER_BIN = 10; // 10 15 30 = frame
const int PLY_PER_DIRECTORY = 10; // 10 or 15
const int BIN_COUNT = 10; // 10 Fix

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
				auto mat = rendering::MaterialRecord();
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
			int cnt = 0;
			std::ofstream writeFile;
			writeFile.open("./timeLog/open3d.txt");
			
			while (main_vis_) {
				msg = bounded_buffer_dequeue(buf2);
				std::string full_path = string(msg);
				std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
				
				Eigen::Vector3d extent;
				{
					std::lock_guard<std::mutex> lock(cloud_lock_);
					cloud_ = std::make_shared<geometry::PointCloud>();
					io::ReadPointCloudFromPLY(full_path, *cloud_, {"auto",false,false,false,{}});
					bounds = cloud_->GetAxisAlignedBoundingBox();
					extent = bounds.GetExtent();
				}

				
				auto mat = rendering::MaterialRecord();
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
				
				cnt++;
				cout << "In Open3D, CNT=" << cnt << endl;
				std::chrono::duration<double> sec = std::chrono::system_clock::now() - start;
				cout << "OPEN-3D Time(sec) : " << sec.count() <<"seconds" <<'\n';
				writeFile << "OPEN-3D Time(sec) : " << sec.count() << "seconds\n";
				
				if(cnt == 1) main_vis_->ResetCameraToDefault(); 
				else if(cnt == PLY_COUNT_PER_BIN * BIN_COUNT - 1) {
					main_vis_->Close();
					writeFile.close();
					break;
				}
				
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
	char msg[256], command[1024];
	vector<string> binaryFile;
	char highfile[128], midfile[128], lowfile[128];	
	int ret = 0;
	std::ofstream writeFile;
	writeFile.open("./timeLog/libdash.txt");

	for(int frame=0;frame<BIN_COUNT;frame++){
		std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
		
		sprintf(command, "./libdash_mcnl_test %d %d", frame, ret);
		ret = system(command);
		cout << "RET: " << ret << endl;
		string buildbinpath = PATH + "/AR-streaming-with-MPEG-DASH/project/build/bin";
		for(auto& p : std::experimental::filesystem::directory_iterator(buildbinpath)) {
			string Filename = p.path().string();
			cout << Filename << endl;
			Filename = Filename.substr(Filename.find("/bin/") + 5);

			sprintf(highfile ,"high_s%d", frame);
			sprintf(midfile ,"mid_s%d", frame);
			sprintf(lowfile ,"low_s%d", frame);
			if(Filename.find(highfile) != string::npos || Filename.find(midfile) != string::npos || Filename.find(lowfile) != string::npos) {
				snprintf(msg, 256, "%s", Filename.c_str());
				bounded_buffer_queue(buf1, strdup(msg));
				cout << Filename << endl;
			}
		}
		std::chrono::duration<double> sec = std::chrono::system_clock::now() - start;
		writeFile << "Lib-Dash Time(sec) : " << sec.count() << "seconds\n";
		//cout << "Lib-DASH Time(sec) : " << sec.count() <<"seconds" <<'\n';
	}
	writeFile.close();

	return 0x0;
}


void * 
mpeg_vpcc_thread(void *ptr)
{
	cout << "Hello, MPEG-VPCC Thraed\n";
	pthread_t tid;
	tid = pthread_self();
	
	char * msg;
	char line[1024] = {0, };
	vector<string> opt;
	string decOptpath = PATH + "/AR-streaming-with-MPEG-DASH/project/Main/decOpt.txt";
	ifstream f1(decOptpath);
	if(!f1) {
		cerr << "file open error\n";
	}
	while(f1.getline(line, 1001)) {
		opt.push_back(line);
	}
	std::ofstream writeFile;
	writeFile.open("./timeLog/mpeg-vpcc.txt");
	
	for(int i = 0 ; i < BIN_COUNT ; i++) {  /// fixing
		msg = bounded_buffer_dequeue(buf1);
		if(msg != 0x0) {
			std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

			printf("MPEG-VPCC Thread [%ld] read %s\n", (unsigned long) tid, msg);
			char path[1024], comprename[1024], reconname[1024];
			sprintf(path, "../../bin/%s", "PccAppDecoder");
			
			msg[strlen(msg)-4] = 0x0;
			char dir_path[1024];
			sprintf(dir_path, "./../../dec_test/%s",msg);
			cout << "dir_path : " << dir_path << endl;
			
			pid_t pid, waitpid;
			int status;
			pid = fork();
			if(pid == 0) {
				mkdir(dir_path, 0755);
				sprintf(comprename,"--compressedStreamPath=%s.bin", msg);	
				sprintf(reconname,"--reconstructedDataPath=./../../dec_test/%s/%s_dec_%%04d.ply", msg ,msg);				
				execl(path, "PccAppDecoder", comprename, opt[0].c_str(), opt[1].c_str(), opt[2].c_str(), reconname, NULL);
			}else {
				char ply_count[101];
				char dir[256];
				int cnt = 0;
				char command[1024];
				char ply_path[1024];
	
				string decTestpath = PATH + "/AR-streaming-with-MPEG-DASH/project/dec_test";
				sprintf(ply_path, "ls -l %s/%s/*.ply | wc -l", decTestpath.c_str(), msg);
				while(1) {
					FILE *fp = popen(ply_path, "r");
					if(fgets(ply_count, 10, fp) == NULL) break;
					//cout << "cnt :" << ply_count << " msg : " << msg <<  " " << atoi(ply_count) << endl;
					if(atoi(ply_count) == PLY_PER_DIRECTORY) { // 10
						sprintf(dir, "%s/%s/ply%02d", decTestpath.c_str() ,msg ,cnt++);
						mkdir(dir, 0755);
						sprintf(command, "mv %s/*.ply %s", dir_path,dir);
						//cout << "Command :" << command << endl;
						system(command);
						
						string directory_path;
						directory_path = string(dir); 
						
						vector<string> files;
						for(auto & p : std::experimental::filesystem::directory_iterator(directory_path)) 
						{
							string filepath = string(dir) + "/" + p.path().filename().string();
							files.push_back(filepath);
						}
						sort(files.begin(), files.end());
						
						for(int i = 0 ; i < files.size() ; i++) {
							string filepath = files[i];
							//cout << "PUSH: " << filepath << endl;
							bounded_buffer_queue(buf2, strdup(filepath.c_str()));
						}
					}
					pclose(fp);
					if(cnt == (PLY_COUNT_PER_BIN / PLY_PER_DIRECTORY)) {
						sprintf(command, "rm -rf %s.bin *.ofl *.opcl *.trc *.txt *.otl", msg);
						system(command);
						break;
					}
					printf("cnt : %d, WEXITSTATUS: %d\n", cnt, WEXITSTATUS(status));
				}
				
				waitpid = wait(&status);
				free(msg);
				kill(pid, SIGKILL);
			}

			std::chrono::duration<double> sec = std::chrono::system_clock::now() - start;
			writeFile << "MPEG-VPCC Time(sec) : " << sec.count() << "seconds\n";
			cout << "MPEG-VPCC Time(sec) : " << sec.count() <<"seconds" <<'\n';
		}

	}

	writeFile.close();
	return 0x0;
}

void * 
open3d_thread(void *ptr)
{
	cout << "Hello, Open3d Thread\n";
	pthread_t tid;
	char * msg;

	tid = pthread_self();
	MultipleWindowsApp().Run();
	cout << "Bye, Open3d Thread\n";
	return 0x0;
}

int main(int argc, char *argv[]) {
	
	pthread_t thread1;
	pthread_t thread2;
	pthread_t thread3;

	buf1 = (bounded_buffer *)malloc(sizeof(bounded_buffer));
	bounded_buffer_init(buf1, 100);
	
	buf2 = (bounded_buffer *)malloc(sizeof(bounded_buffer));
	bounded_buffer_init(buf2, 100);
		
	pthread_create(&thread1, 0x0, libdash_thread, 0x0);
	pthread_create(&thread2, 0x0, mpeg_vpcc_thread, 0x0);
	pthread_create(&thread3, 0x0, open3d_thread, 0x0);
	
	cout << "CHECK" << endl;
	
	pthread_join(thread1, 0x0);
	pthread_join(thread2, 0x0);
	pthread_join(thread3, 0x0);
	cout << "END\n";

	return 0;
}
