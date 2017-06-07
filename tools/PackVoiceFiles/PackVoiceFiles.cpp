#include <iostream>
#include <fstream>
#include <map>
#include <memory>
#include <experimental/filesystem>
#include <cstring>

using namespace std;
namespace fs = std::experimental::filesystem;

int main(int argc, char *argv[]) {
	if (argc <= 1) {
		cout << "Usage :\n"
			<< "\t" "PackVoiceFiles dir" << endl;
		return 0;
	}

	using VoicePack = struct {
		int not_added;
		map<uint32_t, string> paths;
	};
	map<string, VoicePack> m_ext_vp = { { ".at3", { } }, { ".ogg",{ } }, { ".wav",{ } } };
	constexpr unsigned MaxVoiceIdLen = 9;
	constexpr unsigned BuffSize = 1 * 1024 * 1024;
	constexpr unsigned MaxVoiceFiles = 50000;

	cout << "Collecting files..." << endl;

	for (auto &p : fs::directory_iterator(argv[1])) {
		if (!fs::is_regular_file(p.status())) continue;

		auto it = m_ext_vp.find(p.path().extension().string());
		if (it == m_ext_vp.end()) continue;

		VoicePack& vp = it->second;

		string name = p.path().filename().string();
		name = name.substr(0, name.length() - it->first.length());

		if (name[0] != 'v' || name.length() > MaxVoiceIdLen + 1) {
			vp.not_added++;
			continue;
		}

		uint32_t voice_id = 0;
		for (size_t i = 1; i < name.length(); i++) {
			if (name[i] >= '0' && name[i] <= '9') {
				voice_id *= 10;
				voice_id += name[i] - '0';
			}
			else {
				voice_id = 0;
				break;
			}
		}

		if (voice_id == 0) {
			vp.not_added++;
			continue;
		}

		if (!vp.paths.insert({ voice_id,  p.path().string() }).second) {
			cout << "Duplicate voice id: " << voice_id << "," << p.path().string() << endl;
			vp.not_added++;
			continue;
		}
	}

	for (const auto& ext_vp : m_ext_vp) {
		auto& vp = ext_vp.second;
		if (vp.paths.empty()) continue;

		string ext = ext_vp.first.substr(1);
		string file_name = ext + ".pak";
		unsigned cnt = vp.paths.size();
		cout << "Creating " << file_name << ", files: " << cnt << endl;
		if (vp.paths.size() > MaxVoiceFiles) {
			cout << "Error, could not create pack with over " << MaxVoiceFiles << " files!" << endl;
			return -1;
		}

		ofstream ofs(file_name, ios::binary);
		using VoiceInfo = struct {
			uint32_t voice_id;
			uint32_t offset;
			uint32_t size;
		};
		using Head = struct {
			uint32_t cnt;
			char ext[4];
		};
		unsigned off_start = sizeof(Head) + cnt * sizeof(VoiceInfo);
		unique_ptr<char[]> shead = make_unique<char[]>(off_start);
		Head* head = (Head*)shead.get();
		VoiceInfo* vi_beg = (VoiceInfo*)(shead.get() + sizeof(Head));
		VoiceInfo* vi = vi_beg;

		memset(head, 0, sizeof(*head));
		head->cnt = cnt;
		for (size_t i = 0; i < ext.length() && i < sizeof(head->ext); i++) head->ext[i] = ext[i];

		ofs.seekp(off_start, ios::beg);
		uint32_t offset = off_start;

		unique_ptr<char[]> sbuff = make_unique<char[]>(BuffSize);
		char* buff = sbuff.get();

		for (auto& path : vp.paths) {
			vi->voice_id = path.first;
			vi->offset = offset;

			ifstream ifs(path.second, ios::binary);
			ifs.seekg(0, ios::end);
			auto size = ifs.tellg();

			if (offset + size > UINT32_MAX) {
				cout << "Error, could not create a file larger than 4GB!" << endl;
				return -1;
			}

			vi->size = (uint32_t)size;
			ifs.seekg(0, ios::beg);

			while (size > 0) {
				auto request = size; 
				if(request > BuffSize) request = BuffSize;
				ifs.read(buff, request);
				ofs.write(buff, request);
				size -= request;
			}
			ifs.close();

			offset += vi->size;
			vi++;

			if ((vi - vi_beg) % 1000 == 0) {
				cout << vi - vi_beg << " files done." << endl;
			}
		}

		ofs.seekp(0, ios::beg);
		ofs.write(shead.get(), off_start);
		ofs.close();
	}

	cout << "All done." << endl;
}
