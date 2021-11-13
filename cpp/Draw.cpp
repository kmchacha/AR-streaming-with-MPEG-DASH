// ----------------------------------------------------------------------------
// -                        Open3D: www.open3d.org                            -
// ----------------------------------------------------------------------------
// The MIT License (MIT)
//
// Copyright (c) 2018-2021 www.open3d.org
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------------------------------------------------------

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

using namespace open3d;
using namespace open3d::visualization;
using namespace std;

const int WIDTH = 1024;
const int HEIGHT = 768;
//std::string DATA_PATH = "/home/mcnl/project/sample-project/dataset/Thaidancer/downsample/Thaidancer.ply";
std::string DATA_PATH;
int idx=0;
vector<string> files;
const Eigen::Vector3f CENTER_OFFSET(0.0f, 0.0f, -3.0f);
const std::string CLOUD_NAME = "points";

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
			new_vis->SetupCamera(60, center, center + CENTER_OFFSET,
					{0.0f, -1.0f, 0.0f});
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
				std::string full_path = DATA_PATH + files[idx++];

				if(file_size <= idx) {
					idx=0; // For repetition
					//OnMainWindowClosing();
					//break;
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

		int main(int argc, char *argv[]) {
			std::cout << "EX: /home/mcnl/project/sample-project/dataset/Longdress/longdress/downsample/" << std::endl;
			std::cout << "PATH=";
			std::cin >> DATA_PATH;
			for(auto & p : std::experimental::filesystem::directory_iterator(DATA_PATH))
			{
				files.push_back(p.path().filename().string());
			}
			sort(files.begin(), files.end());
			//for(auto& f : files) cout << f << endl;

			MultipleWindowsApp().Run();
			return 0;
		}
