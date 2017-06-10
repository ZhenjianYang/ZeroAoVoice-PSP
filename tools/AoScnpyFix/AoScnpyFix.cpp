#include <iostream>
#include <fstream>
#include <unordered_set>
#include <memory>
#include <experimental/filesystem>
#include <cstring>

using namespace std;
namespace fs = std::experimental::filesystem;

int main(int argc, char *argv[]) {
	if (argc <= 1) {
		cout << "Usage :\n"
			<< "\t" "AoScnpyFix dir_py [dir_py_new]" << endl;
		return 0;
	}

	string dir_py = argv[1]; while (!dir_py.empty() && (dir_py.back() == '/' || dir_py.back() == '\\')) dir_py.pop_back();
	string dir_py_new = argc > 2 ? argv[2] : dir_py + ".new";

	dir_py.push_back('/');
	dir_py_new.push_back('/');

	fs::create_directory(dir_py_new);
	unordered_set<string> need_fix_cb = { "c011b.bin.py", "e3600.bin.py" };
	constexpr int buff_size = 10000;

	char buff[buff_size];
	for (auto &p : fs::directory_iterator(argv[1])) {
		if (!fs::is_regular_file(p.status())) continue;
		if (p.path().extension().string() != ".py") continue;

		cout << "Working with " << p.path().filename().string() << endl;

		if (need_fix_cb.find(p.path().filename().string()) != need_fix_cb.end()) {		
			ifstream ifs(p.path().string());
			ofstream ofs(dir_py_new + p.path().filename().string());

			while (ifs.getline(buff, sizeof(buff))) {
				string s = buff;

				auto idx = s.find("OP_CB");
				unsigned param[6];

				if (idx == string::npos 
					|| sscanf(buff + idx + 6, "0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x", 
						&param[0], &param[1], &param[2], &param[3], &param[4], &param[5]) != 6
					|| (param[0] < 0x01 || param[0] > 0x03 || param[1] != 8)
					) {
					ofs << s << '\n';
				}
				else {
					sprintf(buff + idx + 5, "(0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x)",
						param[0], param[1], param[2] / 2, param[3] / 2, param[4] / 2, param[5] / 2);
					ofs << buff << '\n';
				}
			}

			ifs.close();
			ofs.close();
		}
		else {
			fs::copy_file(p.path(), dir_py_new + p.path().filename().string(), fs::copy_options::overwrite_existing);
		}
	}
}
