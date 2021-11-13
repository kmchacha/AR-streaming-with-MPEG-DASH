#include <iostream>
#include <string>
#include <vector>
#include <experimental/filesystem>
#include <algorithm>

using namespace std;

int main()
{
	string path = "";
	cin >> path;

	vector<string> files;

	for(auto & p : experimental::filesystem::directory_iterator(path))
	{
	//	cout << p.path().filename().string() << endl;
		files.push_back(p.path().filename().string());
	}
	sort(files.begin(), files.end());

	for(auto& f : files) cout << f << endl;

	return 0;
}
