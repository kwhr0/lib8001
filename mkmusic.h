#include "mml.h"
#include <algorithm>
#include <map>
#include <string>
#define BEGIN \
u8 _buf[0xffff];\
int _ofs = 1;\
bool _f;\
int main() {\
std::map<std::string, int> table;\
std::string s;
#define LABEL(l) \
s = #l;\
std::transform(s.cbegin(), s.cend(), s.begin(), toupper);\
table[s] = _ofs += _f ? mmlEndCompile() : 0;\
_f = true;\
mmlInit();\
mmlBeginCompile(_buf + _ofs, sizeof(_buf) - _ofs);
#define END \
_ofs += mmlEndCompile();\
FILE *fo;\
if (!(fo = fopen("music.h.tmp", "w"))) exit(1);\
for (auto d : table) fprintf(fo, "#define MUSIC_%s\t0x%x\n", d.first.c_str(), d.second);\
fprintf(fo, "extern const unsigned char music[];\n");\
fclose(fo);\
system("diff music.h.tmp music.h > /dev/null 2>&1 || mv -f music.h.tmp music.h");\
if (!(fo = fopen("music.s", "w"))) exit(1);\
fprintf(fo, "\t.module\tmusic\n\t.globl\t_music\n\t.area\t_CODE\n_music:\n");\
for (int i = 0; i < _ofs; i++)\
fprintf(fo, "\t.db\t0x%02x\n", _buf[i]);\
fclose(fo);\
return 0;\
}
