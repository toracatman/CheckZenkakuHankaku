#define _CRT_NON_CONFORMING_SWPRINTFS

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <wchar.h>
#include <Windows.h>
#include <stdlib.h>

WCHAR full_name[32] = L"";	//フォント名
LONG weight = FW_BLACK;		//太さ
BYTE italic = 255;			//斜体

UINT cell_height;			//高さ
UINT size_em;				//EMの大きさ

int CALLBACK EnumFontFamExProc(const ENUMLOGFONTEX* lpelfe, const NEWTEXTMETRICEX* lpntme, DWORD FontType, LPARAM lParam) {
	//フォント名が同じならスキップ
	if (wcscmp(lpelfe->elfFullName, full_name) == 0) return 1;
	wcscpy_s(full_name, sizeof(full_name), lpelfe->elfFullName);

	//標準を優先
	if (italic == 0 && lpntme->ntmTm.tmItalic != 0) return 1;
	if (abs(lpntme->ntmTm.tmWeight - FW_REGULAR + 1) > abs(weight - FW_REGULAR + 1)) return 1;

	//更新
	if (italic != 0 && lpntme->ntmTm.tmItalic == 0) italic = 0;
	weight = lpntme->ntmTm.tmWeight;
	cell_height = lpntme->ntmTm.ntmCellHeight;
	size_em = lpntme->ntmTm.ntmSizeEM;

	//標準であれば決定
	if (weight == FW_REGULAR && italic == 0) return 0;

	return 1;
}

int main() {
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);

	wchar_t buf[256];

	HDC hdc = GetDC(NULL);	//DCの取得

	//高さとEMの大きさの取得
	LOGFONT lf;
	lf.lfCharSet = DEFAULT_CHARSET;

	wprintf(L"フォント名を入力してください: ");
	_getws_s(lf.lfFaceName, sizeof(lf.lfFaceName));

	EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)EnumFontFamExProc, 0, 0);

	if (full_name[0] == L'\0') {
		wprintf(L"フォントが見つかりません。");
		return 1;
	}

	//フォントの作成
	HFONT hFont = CreateFont(
		cell_height,
		0,
		0,
		0,
		FW_DONTCARE,
		FALSE,
		FALSE,
		FALSE,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE,
		lf.lfFaceName
	);
	SelectObject(hdc, hFont);

	wprintf(L"調べたい最初の文字のコードポイントをU+をつけずに入力してください:");
	_getws_s(buf, sizeof(buf));
	int s = (int)wcstol(buf, NULL, 16);
	if (s < 0) {
		wprintf(L"不正なコードポイントです。");
		return 1;
	}

	wprintf(L"調べたい最後の文字のコードポイントをU+をつけずに入力してください:");
	_getws_s(buf, sizeof(buf));
	int e = (int)wcstol(buf, NULL, 16);

	//サロゲートペアは調べられない
	if (s < 0) {
		wprintf(L"不正なコードポイントです。");
		return 1;
	}
	else if (e < s) {
		int tmp = s;
		s = e;
		e = tmp;
	}

	//出力
	FILE* fp;
	swprintf_s(buf, sizeof(buf), L"%s%04X〰%04X.html", lf.lfFaceName, s, e);
	_wfopen_s(&fp, buf, L"w");
	_setmode(_fileno(fp), _O_U8TEXT);

	fwprintf(fp, L"<!DOCTYPE html>\n");
	fwprintf(fp, L"<html lang=\"ja\">\n");
	fwprintf(fp, L"<head>\n");
	fwprintf(fp, L"	<meta charset=\"UTF-8\">\n");
	fwprintf(fp, L"	<title>%sの 全角半角判定結果</title>\n", lf.lfFaceName);
	fwprintf(fp, L"	<style>\n");
	fwprintf(fp, L"		html {\n");
	fwprintf(fp, L"			font-size: 62.5%%;\n");
	fwprintf(fp, L"		}\n");
	fwprintf(fp, L"		body {\n");
	fwprintf(fp, L"			font-size: 1.6rem;\n");
	fwprintf(fp, L"		}\n");
	fwprintf(fp, L"		table {\n");
	fwprintf(fp, L"			border-collapse: collapse;\n");
	fwprintf(fp, L"			width: 100%%;\n");
	fwprintf(fp, L"			table-layout: fixed;\n");
	fwprintf(fp, L"			font-family: \"%s\";\n", lf.lfFaceName);
	fwprintf(fp, L"		}\n");
	fwprintf(fp, L"		td {\n");
	fwprintf(fp, L"			height: 6rem;\n");
	fwprintf(fp, L"			font-size: 3.2rem;\n");
	fwprintf(fp, L"			text-align: center;\n");
	fwprintf(fp, L"		}\n");
	fwprintf(fp, L"		.c {\n");
	fwprintf(fp, L"			background-color: #80ff80;\n");
	fwprintf(fp, L"		}\n");
	fwprintf(fp, L"		.h {\n");
	fwprintf(fp, L"			background-color: #8080ff;\n");
	fwprintf(fp, L"		}\n");
	fwprintf(fp, L"		.f {\n");
	fwprintf(fp, L"			background-color: #ff8080;\n");
	fwprintf(fp, L"		}\n");
	fwprintf(fp, L"		.w {\n");
	fwprintf(fp, L"			background-color: #ffff80;\n");
	fwprintf(fp, L"		}\n");
	fwprintf(fp, L"	</style>\n");
	fwprintf(fp, L"</head>\n");
	fwprintf(fp, L"<body>\n");
	fwprintf(fp, L"	<h3>%s</h3>\n", full_name);
	fwprintf(fp, L"	太さ: %ld<br>\n", weight);
	fwprintf(fp, L"	斜体: %hhu<br>\n", italic);
	fwprintf(fp, L"	セルの 高さ: %u<br>\n", cell_height);
	fwprintf(fp, L"	EMの 大きさ: %u<br>\n", size_em);
	fwprintf(fp, L"	<hr>\n");
	fwprintf(fp, L"	白: 未登録<br>\n");
	fwprintf(fp, L"	<span class=\"h\">青</span>: 半角文字<br>\n");
	fwprintf(fp, L"	<span class=\"f\">赤</span>: 全角文字<br>\n");
	fwprintf(fp, L"	<span class=\"c\">緑</span>: 合成用文字<br>\n");
	fwprintf(fp, L"	<span class=\"w\">黄</span>: その他(1.5倍角文字など)<br>\n");

	fwprintf(fp, L"	<table border=\"1\">\n");

	//文字の数だけループ
	for (int i = s; i <= e; i++) {
		if (i % 32 == 0) fwprintf(fp, L"		<tr>");

		UINT code = i;	//コードポイント
		wchar_t chr[3];	//1文字
		int c;			//サロゲートペアは2、それ以外は1
		SIZE size;		//サイズ（size.cxが幅）

		//サロゲートペアの計算
		if (code >= 0x10000) {
			UINT a = code - 0x10000;
			chr[0] = (wchar_t)((a >> 10) + 0xD800);
			chr[1] = (wchar_t)((a & 0x3FF) + 0xDC00);
			chr[2] = L'\0';
			c = 2;
		}
		else {
			chr[0] = (wchar_t)code;
			chr[1] = L'\0';
			c = 1;
		}

		//グリフが存在するか調べる(サロゲートペアは調べられない)
		WORD pgi[3];
		GetGlyphIndices(hdc, chr, 3, pgi, GGI_MARK_NONEXISTING_GLYPHS);
		if (code < 0x10000 && (pgi[0] == 0xFFFF || pgi[0] == 0)) {
			fwprintf(fp, L"<td>");
		}
		else {
			//幅の取得
			GetTextExtentPoint32(hdc, chr, c, &size);

			//出力
			//fwprintf(fp, L"%s\t", chr);
			//fwprintf(fp, L"U+%04X\t", code);
			//fwprintf(fp, L"%ld\t", size.cx);
			//fwprintf(fp, L"%s\n", (size.cx >= (int)size_em) ? L"全" : L"半");

			int w = size.cx / ((int)size_em / 2);
			wchar_t cl;
			if (size.cx == 0) {
				cl = L'c';
			}
			else if (size.cx < (int)size_em) {
				cl = L'h';
			}
			else if (size.cx == (int)size_em) {
				cl = L'f';
			}
			else {
				cl = 'w';
			}
			fwprintf(fp, L"<td class=\"%lc\">", cl);
		}

		if (chr[0] == L'&') {
			fwprintf(fp, L"&amp;");
		}
		else if (chr[0] == L'<') {
			fwprintf(fp, L"&lt;");
		}
		else if (chr[0] == L'>') {
			fwprintf(fp, L"&gt;");
		}
		else {
			fwprintf(fp, L"%s", chr);
		}
		fwprintf(fp, L"</td>");

		if (i % 32 == 31) fwprintf(fp, L"</tr>\n");
	}

	fwprintf(fp, L"	</table>\n");
	fwprintf(fp, L"</body>\n");
	fwprintf(fp, L"</html>");

	fclose(fp);

	//フォントの削除
	SelectObject(hdc, GetStockObject(SYSTEM_FONT));
	DeleteObject(hFont);
	ReleaseDC(NULL, hdc);	//DCの開放

	return 0;
}