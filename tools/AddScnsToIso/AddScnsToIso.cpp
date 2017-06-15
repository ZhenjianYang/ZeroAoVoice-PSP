#include <iostream>
#include <fstream>
#include <map>
#include <memory>
#include <experimental/filesystem>
#include <cstring>

using namespace std;
namespace fs = std::experimental::filesystem;

int main(int argc, char *argv[]) {
	if (argc <= 2) {
		cout << "Usage :\n"
			<< "\t" "AddScnsToIso iso dir_scns" << endl;
		return 0;
	}

	string path_iso = argv[1];
	string dir_scns = argv[2];

	using DInfo = struct {
		char name[8];
		uint32_t size;
		uint32_t lba;
	};
	using FileData = struct {
		unique_ptr<char[]> data;
		size_t size;

		size_t lba;
		DInfo* di_bin;
		DInfo* di_mc1;
	};
	using Scns = map<string, FileData>;
	Scns scns;
	for (auto &p : fs::directory_iterator(dir_scns)) {
		if (fs::is_regular_file(p.status())) {
			ifstream ifs(p.path().string(), ios::in | ios::binary);
			if (!ifs) continue;

			ifs.seekg(0, ios::end);
			size_t size = (size_t)ifs.tellg();
			ifs.seekg(0, ios::beg);
			FileData scn = {
				make_unique<char[]>(size),
				size
			};
			ifs.read(scn.data.get(), size);
			ifs.close();

			scns.emplace(
				p.path().filename().string(),
				std::move(scn) 
			);

			cout << "Scn File Found :" << p.path().filename() << endl;
		}
	}
	if (scns.size() == 0) {
		cout << "No Scn File Found" << endl;
		return 0;
	}

	constexpr uint32_t LbaAlign = 2048;
	constexpr size_t MaxIsoHeadSize = 1024 * LbaAlign;
	constexpr size_t Off_IsoSize = 0x8050;

	constexpr char data_lst[] = "data.lst";
	fstream fstr(path_iso, ios::in | ios::out | ios::binary);
	unique_ptr<char[]> shead = make_unique<char[]>(MaxIsoHeadSize);
	fstr.read(shead.get(), MaxIsoHeadSize);
	const char* head = shead.get();

	uint32_t iso_size = *(uint32_t*)(head + Off_IsoSize);
	uint32_t lba_data_lst = 0, size_data_lst = 0;
	for (size_t i = 0; i < MaxIsoHeadSize - sizeof(data_lst); i++) {
		if (!strcmp(data_lst, head + i)) {
			lba_data_lst = *(uint32_t*)(head + i - 0x1F);
			size_data_lst = *(uint32_t*)(head + i - 0x17);
			break;
		}
	}

	if (lba_data_lst == 0 || size_data_lst == 0) {
		cout << "data.lst not found" << endl;
		return 0;
	}

	constexpr char ext_bin[] = "bin";
	constexpr char ext_mc1[] = "mc1";
	fstr.seekg(lba_data_lst * LbaAlign, ios::beg);
	unique_ptr<char[]> sdlst = make_unique<char[]>(size_data_lst);
	fstr.read(sdlst.get(), size_data_lst);
	char* dlst = sdlst.get();

	uint32_t idx_mc1 = 0;
	uint32_t idx_bin = 0;
	for (auto p = dlst + 4; *p; p += 4) {
		if (!strcmp(ext_bin, p)) idx_bin = (p - dlst) / 4;
		else if (!strcmp(ext_mc1, p)) idx_mc1 = (p - dlst) / 4;
	}
	if (idx_mc1 == 0 || idx_bin == 0) {
		cout << "bin or mc1 not found" << endl;
		return 0;
	}

	uint32_t lba_next = 0;
	for (auto di = (DInfo*)(dlst + 0x400); di < (DInfo*)(dlst + size_data_lst - (sizeof(DInfo) - 1)); di++) {
		unsigned char type = di->lba >> 24;
		if (type == 0) continue;

		uint32_t lba = di->lba & 0xFFFFFF;
		uint32_t size = di->size;

		auto it = scns.end();
		if (type == idx_bin || type == idx_mc1) {
			string file_bin = string(di->name, sizeof(di->name)).c_str();
			file_bin += ".bin";
			it = scns.find(file_bin);
		};
		if (it == scns.end()) {
			lba_next = max(lba_next, lba + (size + LbaAlign) / LbaAlign);
		} else if (type == idx_bin) {
			it->second.di_bin = di;
		}
		else {
			it->second.di_mc1 = di;
		}
	}

	int cnt_added = 0, cnt_not = 0;
	for (auto& scn : scns) {
		if (scn.second.di_bin) {
			scn.second.lba = lba_next;

			lba_next += (scn.second.size + LbaAlign) / LbaAlign;

			scn.second.di_bin->size = scn.second.size;
			scn.second.di_bin->lba = scn.second.lba | (idx_bin << 24);
			if (scn.second.di_mc1) {
				fstr.seekg(scn.second.di_mc1->lba * LbaAlign, ios::beg);
				unique_ptr<char[]> sdata_mc1 = make_unique<char[]>(scn.second.di_mc1->size);
				fstr.read(sdata_mc1.get(), scn.second.di_mc1->size);
				char* data_mc1 = sdata_mc1.get();
				uint32_t cnts = *(uint32_t*)data_mc1;

				struct Mc1Data {
					char name[16];
					uint32_t data[4];
				};

				for (uint32_t i = 0; i < cnts; i++) {
					Mc1Data* md = (Mc1Data*)(data_mc1 + 0x10 + sizeof(Mc1Data) * i);
					if (md->name == scn.first) {
						md->name[0] = '_';
					}
				}

				fstr.seekp(scn.second.di_mc1->lba * LbaAlign, ios::beg);
				fstr.write(sdata_mc1.get(), scn.second.di_mc1->size);
			}

			fstr.seekp(scn.second.lba * LbaAlign, ios::beg);
			fstr.write(scn.second.data.get(), scn.second.size);

			uint32_t gap = lba_next * LbaAlign - (scn.second.lba * LbaAlign + scn.second.size);
			if (gap > 0) {
				unique_ptr<char[]> bub_gap = make_unique<char[]>(gap);
				fill(bub_gap.get(), bub_gap.get() + gap, 0);
				fstr.write(bub_gap.get(), gap);
			}

			fstr.flush();

			++cnt_added;
			cout << "File Added :" << scn.first << endl;
		}
		else {
			++cnt_not;
			cout << "************ File not Added :" << scn.first << "****************" << endl;
		}
	}

	fstr.seekp(lba_data_lst * LbaAlign, ios::beg);
	fstr.write(dlst, size_data_lst);

	if(iso_size < lba_next) {
		char *psize = (char*)&lba_next;
		char size_buff[8];
		for (unsigned i = 0; i < 4; i++) size_buff[i] = psize[i];
		for (unsigned i = 0; i < 4; i++) size_buff[4 + i] = psize[3 - i];
		fstr.seekp(Off_IsoSize, ios::beg);
		fstr.write(size_buff, sizeof(size_buff));
	}
	fstr.close();

	cout << "Done. Added :" << cnt_added << ", not Added :" << cnt_not << endl;
}
