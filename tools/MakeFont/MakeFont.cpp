#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <algorithm>

#include <ft2build.h>

#include FT_FREETYPE_H 
#include FT_OUTLINE_H

using namespace std;

constexpr int FONT_SIZE = 16;
constexpr int CH_CNT = 128;

int main(int argc, char *argv[]) {
	int base_line = 3;
	int bold = 0;

	int arg_idx = 0;
	int arg_tmp;
	char* arg_pend;
	const char* path_ttf = nullptr;

	for(int i = 1; i < argc; i++) {
		if(argv[i][0] == '-') {
			switch(argv[i][1]) {
			case 'h':
				arg_tmp = std::strtol(argv[i] + 2, &arg_pend, 10);
				if(arg_pend == argv[i] + 2 || *arg_pend) {
					cout << "bad parameter, ignore: " << argv[i] << endl;
				} else {
					base_line = arg_tmp;
				}
				break;
			case 'b':
				arg_tmp = std::strtol(argv[i] + 2, &arg_pend, 10);
				if (arg_pend == argv[i] + 2 || *arg_pend || arg_tmp < 0) {
					cout << "bad parameter, ignore: " << argv[i] << endl;
				}
				else {
					bold = arg_tmp;
				}
				break;
			default:
				cout << "Unknow parameter, ignore: " << argv[i] << endl;
			}
		} else {
			switch(arg_idx) {
			case 0:
				path_ttf = argv[i];
				break;
			default:
				cout << "Too more parameter, ignore: " << argv[i] << endl;
			}
			arg_idx++;
		}
	}

	if (!path_ttf) {
		cout << "Usage :\n"
				"\t" "MakeFont [options] ttf_file" "\n"
				"\t" "  options :"  "\n"
				"\t" "      -hY : set vertical position to Y, default is 3" "\n"
				"\t" "            (bigger for higher, could be negative.)\n"
				"\t" "      -bB : set bold value to B. Its unit is 1/64 pixel." "\n"
				"\t" "            Default is 0.\n"
				"\t" "e.g. MakeFont -h4 -b32 test.ttf\n"
			<< endl;
		return 0;
	}

	const char* path_dat = "font.dat";
	const char* path_bmp = "font.bmp";
	
	FT_Library library = NULL;
	FT_Face face = NULL;

	FT_Init_FreeType((FT_Library*)&library);

	if (FT_New_Face((FT_Library)library, path_ttf, 0, (FT_Face*)&face)) {
		cout << "Open ttf file failed!" << endl;
		FT_Done_FreeType(library);
		return -1;
	}

	FT_Set_Pixel_Sizes(face, FONT_SIZE, 0);

	using byte = uint8_t;
	size_t max_size = CH_CNT * 2 + CH_CNT * FONT_SIZE * FONT_SIZE;
	unique_ptr<byte[]> sbuff = make_unique<byte[]>(max_size);
	
	byte* const base = sbuff.get();
	uint16_t *poff = (uint16_t *)base;
	byte* pdata = base + 2 * CH_CNT;

	for (int asc = 0; asc < CH_CNT; asc++) {
		*poff++ = pdata - base;

		FT_UInt glyph_index = FT_Get_Char_Index(face, asc);
		if (FT_Load_Glyph(face, glyph_index, 0)) {
			continue;
		}

		if (bold) {
			FT_Outline_Embolden(&face->glyph->outline, (FT_Pos)(bold));
		}

		if (face->glyph->format != FT_GLYPH_FORMAT_BITMAP) {
			FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
		}

		FT_Bitmap* bitmap = &(face->glyph->bitmap);

		int x0 = face->glyph->bitmap_left;
		int y0 = FONT_SIZE - base_line - face->glyph->bitmap_top;
		int width = face->glyph->advance.x >> 6;

		memset(pdata, 0, width * FONT_SIZE);

		byte* p = bitmap->buffer;
		for(int y = 0; y < (int)bitmap->rows && y < FONT_SIZE - y0; y++) {
			if(y + y0 < 0) continue;
			for(int x = 0; x < (int)bitmap->width && x < width - x0; x++) {
				if(x + x0 < 0) continue;
				pdata[(y + y0) * width + (x + x0)] = p[y * bitmap->width + x];
			}
		}

		pdata += width * FONT_SIZE;
	}

	FT_Done_Face(face);
	FT_Done_FreeType(library);

	int size = pdata - base;
	ofstream ofs(path_dat, ios::binary);
	if (!ofs) {
		cout << "Create output file failed." << endl;
		return -1;
	}

	ofs.write((const char*)base, size);
	ofs.close();

	cout << "Dat Created." << endl;

	int total_bits = size - CH_CNT * 2;
	int total_width = total_bits / FONT_SIZE;
	unique_ptr<byte[]> sbuff_bmp = make_unique<byte[]>(total_bits);
	byte* buff_bmp = sbuff_bmp.get();

	int x = 0;
	poff = (uint16_t *)base;
	for(int i = 0; i < CH_CNT; i++) {
		int cnt_bits = i == CH_CNT - 1 ? size - poff[i] : poff[i+1] - poff[i];
		int w = cnt_bits / FONT_SIZE;
		byte* pdat = (byte*)base + poff[i];
		for(int y = 0; y < FONT_SIZE; y++) {
			for(int xx = 0; xx < w; xx++) {
				buff_bmp[(FONT_SIZE - 1 - y) * total_width + x + xx] = *pdat++;
			}
		}
		x += w;
	}

	using WORD = uint16_t;
	using DWORD = uint32_t;
	using LONG = int32_t;

#pragma pack(1)
	typedef struct tagBITMAPFILEHEADER {
	  WORD  bfType;
	  DWORD bfSize;
	  WORD  bfReserved1;
	  WORD  bfReserved2;
	  DWORD bfOffBits;
	} BITMAPFILEHEADER;
	typedef struct tagBITMAPINFOHEADER {
	  DWORD biSize;
	  LONG  biWidth;
	  LONG  biHeight;
	  WORD  biPlanes;
	  WORD  biBitCount;
	  DWORD biCompression;
	  DWORD biSizeImage;
	  LONG  biXPelsPerMeter;
	  LONG  biYPelsPerMeter;
	  DWORD biClrUsed;
	  DWORD biClrImportant;
	} BITMAPINFOHEADER;
#pragma pack()

	BITMAPFILEHEADER file_header = { 0x4D42 };
	BITMAPINFOHEADER info_header = { sizeof(BITMAPINFOHEADER), total_width, FONT_SIZE,
			1, 8, 0, unsigned(total_width * FONT_SIZE), 0, 0, 256, 0 };
	uint32_t palette[256];
	for(int i = 0; i < 256; i++) palette[i] = (0xFF << 24) | (0x010101 * i);
	file_header.bfOffBits = sizeof(file_header) + sizeof(info_header) + sizeof(palette);
	file_header.bfSize = file_header.bfOffBits + total_width * FONT_SIZE;

	ofstream ofs_bmp(path_bmp, ios::binary);
	if (!ofs) {
		cout << "Create bmp file failed." << endl;
		return -1;
	}
	ofs_bmp.write((char*)&file_header, sizeof(file_header));
	ofs_bmp.write((char*)&info_header, sizeof(info_header));
	ofs_bmp.write((char*)palette, sizeof(palette));
	ofs_bmp.write((char*)buff_bmp, total_width * FONT_SIZE);
	ofs_bmp.close();

	cout << "Bmp Created." << endl;
	return 0;
}
