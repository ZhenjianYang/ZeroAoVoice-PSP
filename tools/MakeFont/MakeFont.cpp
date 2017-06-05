#include <iostream>
#include <memory>
#include <fstream>
#include <algorithm>

#include <ft2build.h>

#include FT_FREETYPE_H 
#include FT_OUTLINE_H

using namespace std;

constexpr int FONT_SIZE = 16;
constexpr int CH_CNT = 128;
constexpr int HORI = 3;

int main(int argc, char *argv[]) {
	if (argc <= 2) {
		cout << "Usage :\n"
			<< "\t" "MakeFont font.raw ttf" << endl;
		return 0;
	}

	const char* path_out = argv[1];
	const char* path_ttf = argv[2];
	
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

		if (face->glyph->format != FT_GLYPH_FORMAT_BITMAP) {
			FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
		}

		FT_Bitmap* bitmap = &(face->glyph->bitmap);
		int width = bitmap->width;

		memset(pdata, 0, width * FONT_SIZE);

		int y0 = FONT_SIZE - HORI - face->glyph->bitmap_top;
		int rows = bitmap->rows;
		byte* dst = pdata;
		byte* src = bitmap->buffer;

		if(y0 > 0) {
			dst += y0 * width;
			if(rows > FONT_SIZE - y0) {
				rows = FONT_SIZE - y0;
			}
		}
		else if(y0 < 0){
			src += (-y0) * width;
			rows += y0;
			if(rows > FONT_SIZE) {
				rows = FONT_SIZE;
			}
		}

		memcpy(dst, src, bitmap->width * rows);

		pdata += width * FONT_SIZE;
	}

	FT_Done_Face(face);
	FT_Done_FreeType(library);

	int size = pdata - base;
	ofstream ofs(path_out, ios::binary);
	if (!ofs) {
		cout << "Create output file failed." << endl;
		return -1;
	}

	ofs.write((const char*)base, size);
	ofs.close();

	cout << "Created." << endl;

	return 0;
}