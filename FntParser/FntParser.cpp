/*
MIT License

Copyright(c) 2021-2024 frto027

Permission is hereby granted, free of charge, to any person obtaining a copy
of this softwareand associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright noticeand this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#define _CRT_SECURE_NO_WARNINGS

#define VERSION_MAJOR 0
#define VERSION_MINOR 8

#include <iostream>
#include <cstring>
#include <set>
#include <map>
#define ERROR(...) do{fprintf(stderr,__VA_ARGS__);system("pause");exit(-1);}while(0)

#pragma pack(push, 1)
struct info {
    short fontSize;
    unsigned char bitField;
    unsigned char charSet;
    unsigned short stretchH;
    unsigned char aa;
    unsigned char paddingUp;
    unsigned char paddingRight;
    unsigned char paddingDown;
    unsigned char paddingLeft;
    unsigned char spacingHoriz;
    unsigned char spacingVert;
    unsigned char outline;
    char fontName[0];
};

struct common {
    unsigned short lineHeight;
    unsigned short base;
    unsigned short scaleW;
    unsigned short scaleH;
    unsigned short pages;
    unsigned char bitField;
    unsigned char alphaChnl;
    unsigned char redChnl;
    unsigned char greenChnl;
    unsigned char blueChnl;
};

struct pages {
    char pageNames[0];
};
struct chars {
    unsigned int id;
    unsigned short x,
        y,
        width,
        height;
    short xoffset,
        yoffset,
        xadvance;
    unsigned char
        page,
        chnl;
};
struct kerning {
    unsigned int first;
    unsigned int seconed;
    short amount;
};

char buffer[1024 * 1024];
#include <vector>
#include <string>

class FntFile {
public:
    info* info_ptr = NULL;
    common* common_ptr = NULL;
    pages* pages_ptr = NULL;
    chars* chars_ptr = NULL;
    kerning* kerning_ptr = NULL;
    unsigned int info_sz, common_sz, pages_sz, chars_sz, kerning_sz;

    void pageNameToVec(std::vector<std::string> & vec) {
        char buff[2048];
        int end = 0;
        for (int i = 0; i < pages_sz; i++) {
            char ch = buff[end++] = pages_ptr->pageNames[i];
            if (ch == '\0') {
                vec.push_back(buff);
                end = 0;
            }
        }
        if (end != 0) {
            buff[end++] = '\0';
            vec.push_back(buff);
        }
    }
    void vecToPageName(std::vector<std::string>& vec) {
        int len = 0;
        for (auto &s : vec) {
            len += s.length() + 1;
        }
        if (len != pages_sz) {
            free(pages_ptr);
            pages_ptr = (pages*)malloc(len);
        }
        int i = 0;
        for (auto& s : vec) {
            for (char ch : s) {
                pages_ptr->pageNames[i++] = ch;
            }
            pages_ptr->pageNames[i++] = '\0';
        }
        pages_sz = len;
        if (i != len)
            ERROR("length error");
    }

    void renamePage(std::string from, std::string to) {
        bool found = false;
        std::vector<std::string> vec;
        pageNameToVec(vec);
        for (int i = 0; i < vec.size(); i++) {
            if (vec[i] == from) {
                vec[i] = to;
                found = true;
            }
        }
        vecToPageName(vec);
        if (found)
            std::cout << "replaced\n";
        else
            std::cout << "page not found\n";
    }

    void parseFntFile(const char* fname) {

        FILE* f = fopen(fname, "rb");
        if (f == NULL) {
            ERROR("file open failed.");
        }
        if (!(fgetc(f) == 'B' && fgetc(f) == 'M' && fgetc(f) == 'F' && fgetc(f) == '\3')) {
            fprintf(stderr, "invalid file format for \"%s\":invalid header BMF3", fname);
            exit(-1);
        }
        int end = 0;

        //        info * info_ptr = NULL;
        //        common * common_ptr = NULL;
        //        pages * pages_ptr = NULL;
        //        chars * chars_ptr = NULL;
        //        kerning * kerning_ptr = NULL;
        //        unsigned int info_sz,common_sz,pages_sz,chars_sz,kerning_sz;
        while (!end) {
            int type = fgetc(f);
#define READ_PAIR(id, ptr, type, sz)                                \
            case id:                                                \
                if(ptr)                                             \
                    ERROR("Duplicate " #ptr "at file %s",fname);   \
                printf("reading %d\n", id);fread(&sz,sizeof(int),1,f);                         \
                if(sz < sizeof(type))                               \
                {\
                    printf("Warning: the font you just read has some extra bytes. We will ignore them.\n");\
                    break; \
                }           \
                ptr = (type *)malloc(sz);                           \
                fread(ptr,sz,1,f);                                  \
            break;

            switch (type) {
                READ_PAIR(1, info_ptr, info, info_sz)
                    READ_PAIR(2, common_ptr, common, common_sz)
                    READ_PAIR(3, pages_ptr, pages, pages_sz)
                    READ_PAIR(4, chars_ptr, chars, chars_sz)
                    READ_PAIR(5, kerning_ptr, kerning, kerning_sz)
            case EOF:
                end = 1;
                break;
            default:
                fprintf(stderr, "warring:invalid type format for \"%s\":invalid block type", fname);
                int sz;
                char empty;
                fread(&sz, sizeof(int), 1, f);
                fprintf(stderr, "warring:ignore unknown block(len = %d)\n", sz);
                while (sz--) {
                    if (fread(&empty, sizeof(char), 1, f) == 0) {
                        fprintf(stderr,"invalid file(EOF),emmmm, just ignore it");
                        break;
                    }
                }
                //exit(-1);
#undef READ_PAIR
            }
        }
    }

    ~FntFile() {
        if (info_ptr)
            free(info_ptr);
        if (common_ptr)
            free(common_ptr);
        if (pages_ptr)
            free(pages_ptr);
        if (chars_ptr)
            free(chars_ptr);
        if (kerning_ptr)
            free(kerning_ptr);
    }
    void PrintFontName() {
        if (info_ptr == NULL)
            ERROR("invalid info");
        printf("    font name:%s\n", info_ptr->fontName);
    }
    void PrintPageNames() {
        if (common_ptr == NULL)
            ERROR("common is null!");
        if (pages_ptr == NULL)
            ERROR("Can't print empty page!");
        int n = 0;
        printf("    page count:%d\n", common_ptr->pages);
        std::vector<std::string> pages;
        pageNameToVec(pages);
        for (auto& s : pages) {
            printf("        %s\n", s.c_str());
        }
    }
    char* GetPageName(int i) {
        if (common_ptr == NULL)
            ERROR("common is null!");
        if (pages_ptr == NULL)
            ERROR("Can't print empty page!");
        if (i >= common_ptr->pages)
            ERROR("Page overflow!");
        int n = 0;
        int current_page = 0;
        while (n < pages_sz) {
            if (current_page == i)
                return &pages_ptr->pageNames[n];
            if (pages_ptr->pageNames[n] == '\0')
                current_page++;//next loop, current page move forward
            n++;
        }
        ERROR("No enough page!");
    }

    int AddOrGetPage(const char* pageName) {
        int page_index = 0;
        int page_start = 0;
        while (page_index < common_ptr->pages) {
            int equal = 1;
            int i;
            for (i = page_start; i < pages_sz; i++) {
                if (pageName[i - page_start] != pages_ptr->pageNames[i]) {
                    equal = 0;
                    break;
                }
                if (pageName[i - page_start] == '\0' || pages_ptr->pageNames[i] == '\0') {
                    break;
                }
            }
            if (equal)
                return page_index;
            while (pages_ptr->pageNames[i] != '\0' && i < pages_sz)
                i++;
            //i is end of page_index
            //next_page
            page_index++;
            page_start = i + 1;
        }
        if (pages_ptr->pageNames[pages_sz - 1] != '\0') {
            ERROR("invalid page terminator");
        }
        int new_page_sz = pages_sz + strlen(pageName) + 1;
        char* new_page = (char*)malloc(new_page_sz);
        memcpy(new_page, pages_ptr->pageNames, pages_sz * sizeof(char));
        memcpy(new_page + pages_sz, pageName, new_page_sz - pages_sz);
        free(pages_ptr);
        pages_ptr = (pages*)new_page;
        pages_sz = new_page_sz;
        common_ptr->pages++;
        return page_index;
    }
    void dumpChars() {
        std::cout << "    charset=" << (int)(info_ptr->charSet) << "\n";
        for (int i = 0; i < CharCount(); i++) {
            chars* ch = &chars_ptr[i];
            std::cout << "           id=" << ch->id << " x=" << ch->x << ",y=" << ch->y << ",w=" << ch->width << ",h=" << ch->height << " "
                " xoff=" << ch->xoffset << ",yoff=" << ch->yoffset << ",xadv=" << ch->xadvance << ", page=" << (int)(ch->page) << " chnl=" << (int)(ch->chnl) << "\n";
        }
    }
    void SaveToFile(const char* fileName) {
        FILE* f = fopen(fileName, "wb");
        if (f == NULL) {
            ERROR("Open file failed.(%s)", fileName);
        }

        fputc('B', f); fputc('M', f); fputc('F', f); fputc('\3', f);
        if (info_ptr) {
            fputc('\1', f);
            fwrite(&info_sz, sizeof(int), 1, f);
            fwrite(info_ptr, sizeof(char), info_sz, f);
        }
        if (common_ptr) {
            fputc('\2', f);
            fwrite(&common_sz, sizeof(int), 1, f);
            fwrite(common_ptr, sizeof(char), common_sz, f);
        }
        if (pages_ptr) {
            fputc('\3', f);
            fwrite(&pages_sz, sizeof(int), 1, f);
            fwrite(pages_ptr, sizeof(char), pages_sz, f);
        }
        if (chars_ptr) {
            fputc('\4', f);
            fwrite(&chars_sz, sizeof(int), 1, f);
            fwrite(chars_ptr, sizeof(char), chars_sz, f);
        }
        if (kerning_ptr) {
            fputc('\5', f);
            fwrite(&kerning_sz, sizeof(int), 1, f);
            fwrite(kerning_ptr, sizeof(char), kerning_sz, f);
        }
        fclose(f);
    }

    void ApplyReplacedRule() {
        if (pages_ptr == NULL)
            ERROR("can't apply replaced rule for empty pages");
        for (int i = 0; i < pages_sz; i++) {
            if (pages_ptr->pageNames[i] == '_')
                pages_ptr->pageNames[i] = '-';
        }
    }
    int CharCount() {
        return chars_sz / sizeof(chars);
    }

    template<typename F>
    void forEachChars(F f) {
        int c = CharCount();
        for (int i = 0; i < c; i++) {
            f(chars_ptr[i]);
        }
    }

    template<typename F>
    void updateChars(int idx, F f) {
        forEachChars([&](chars& ch) {
            if (ch.id == idx) {
                f(ch);
            }
            });
    }
    void ReplaceCharsUse(FntFile* other) {
        int myCount = CharCount();
        int oCount = other->CharCount();

        std::vector<std::string> this_page_names, other_page_names;
        pageNameToVec(this_page_names);
        other->pageNameToVec(other_page_names);

        std::map<std::string, int> page_name_to_int;

        std::set<unsigned int> handled;
        for (int i = 0; i < myCount; i++) {
            for (int j = 0; j < oCount; j++) {
                if (chars_ptr[i].id == other->chars_ptr[j].id) {
                    handled.insert(chars_ptr[i].id);
                    //replace
                    memcpy(&chars_ptr[i], &(other->chars_ptr[j]), sizeof(chars));
                    int page;
                    auto other_page_name = other_page_names[other->chars_ptr[j].page];
                    auto it = page_name_to_int.find(other_page_name);
                    if (it == page_name_to_int.end()) {
                        page = this_page_names.size();
                        this_page_names.push_back(other_page_name);
                        page_name_to_int[other_page_name] = page;
                    }
                    else {
                        page = it->second;
                    }
                    chars_ptr[i].page = page;
                }
            }
        }

        std::vector<chars> toBeAdded;
        for (int j = 0; j < oCount; j++) {
            auto idx = other->chars_ptr[j].id;
            if (handled.count(idx))
                continue;
            chars ch = other->chars_ptr[j];
            int page;
            auto other_page_name = other_page_names[ch.page];
            auto it = page_name_to_int.find(other_page_name);
            if (it == page_name_to_int.end()) {
                page = this_page_names.size();
                this_page_names.push_back(other_page_name);
                page_name_to_int[other_page_name] = page;
            }
            else {
                page = it->second;
            }
            ch.page = page;
            toBeAdded.push_back(ch);
        }

        {
            size_t new_char_count = CharCount() + toBeAdded.size();
            chars* new_chars = (chars*)malloc(new_char_count * sizeof(chars));
            if (new_chars == NULL)
                ERROR("malloc failed.");
            memcpy(new_chars, chars_ptr, CharCount() * sizeof(chars));
            for (auto i = 0; i < toBeAdded.size(); i++) {
                new_chars[CharCount() + i] = toBeAdded[i];
            }
            free(chars_ptr);
            chars_ptr = new_chars;
            chars_sz = new_char_count * sizeof(chars);
        }

        vecToPageName(this_page_names);
        common_ptr->pages = this_page_names.size();
    }

    void RemoveKerning() {
        if (kerning_ptr) {
            free(kerning_ptr);
            kerning_ptr = nullptr;
            kerning_sz = 0;
        }
    }
};

#include <map>
#include <vector>

struct MapContent {
    std::string filename;
    FntFile* file = new FntFile();
};
std::vector<MapContent> files;

int main() {
    //just runtime check, instead of static assert!
    if (sizeof(info) != 14) {
        ERROR("invalid info size %d", sizeof(info));
    }
    if (sizeof(common) != 15) {
        ERROR("invalid common size");
    }
    if (sizeof(pages) != 0) {
        //ERROR("invalid pages size %d\n",sizeof(pages));
    }
    if (sizeof(chars) != 20) {
        ERROR("invalid chars size");
    }
    if (sizeof(kerning) != 10) {
        ERROR("invalid kerning size");
    }
    std::cout << "FNT file edit tools ver "<<VERSION_MAJOR<<"."<<VERSION_MINOR<<", by frto027.\ntype 'help' for help.\n";
    while (1) {
        std::cout << ">>> ";
        std::cout.flush();
        char cmdbuff[2048];
        std::cin.getline(cmdbuff, sizeof(cmdbuff));
        std::vector<std::string> args;

        char* p = strtok(cmdbuff, " ");
        while (p) {
            args.emplace_back(p);
            p = strtok(NULL, " ");
        }

        if (args.size() == 0) {
            continue;
        }



        auto cmd = args[0];
        auto m = [&](const char* name, int argcount)->bool {
            return cmd == name && args.size()-1 == argcount;
         };
        
        if (m("help",0)) {
            std::cout << "help\t\t\t\t\t\t print this message\n"
                "read <filename>\t\t\t\t\t load file into memory\n"
                "list\t\t\t\t\t\t list all files in memory\n"
                "merge <idx1> <idx2>\t\t\t\t merge idx2 to idx1: [idx1] = [idx1] <--- [idx2]\n"
                "pagename <idx> <pagename> <new pagename>\t replace pagename to new pagename\n"
                "save <idx> <filename>\t\t\t\t save idx to file\n"

                "xoff <idx> <offset>\t\t\t add <offset> to every chars xoffset inside <idx> e.g. xoff 0 -3\n"
                "yoff <idx> <offset>\t\t\t add <offset> to every chars yoffset inside <idx> e.g. yoff 0 -3\n"
                "xadv <idx> <offset>\t\t\t add <offset> to every chars xadv inside <idx> e.g. xadv 0 -3\n"

                "\n"
                "x= <idx> <char id> <value> \t\t\t set x=<value> for <char id> inside <idx>\n"
                "y= <idx> <char id> <value> \t\t\t set y=<value> for <char id> inside <idx>\n"
                "w= <idx> <char id> <value> \t\t\t set width=<value> for <char id> inside <idx>\n"
                "h= <idx> <char id> <value> \t\t\t set height=<value> for <char id> inside <idx>\n"
                "page= <idx> <char id> <value> \t\t\t set page=<value> for <char id> inside <idx>\n"
                "xoff= <idx> <char id> <value> \t\t\t set xoff=<value> for <char id> inside <idx>\n"
                "yoff= <idx> <char id> <value> \t\t\t set yoff=<value> for <char id> inside <idx>\n"
                "xadv= <idx> <char id> <value> \t\t\t set xadv=<value> for <char id> inside <idx>\n"
                "\n"

                "rmkern <idx> \t\t\t remove kerning information for <idx>\n"
                ""
                "dumpchars <idx>\t\t\t\t print all chars infos\n"
                "exit\n"
                ;
            continue;
        }
        
        if (m("read", 1)) {
            auto idx = files.size();
            files.push_back({});
            files[idx].filename = args[1];
            files[idx].file->parseFntFile(args[1].c_str());
            std::cout << "read success, idx = " << idx << "\n";
            continue;
        }
        if (m("list", 0)) {
            int idx = 0;
            for (auto it = files.begin(), end = files.end(); it != end; ++it) {
                std::cout << "idx = " << idx++ << ", filename = " << it->filename<<'\n';
                it->file->PrintFontName();
                it->file->PrintPageNames();
                std::cout << "    char count = " << it->file->CharCount() << "\n";
                std::cout << "    kerning size = " << it->file->kerning_sz << "\n";
            }
            continue;
        }
        if (m("merge", 2)) {
            int a = atoi(args[1].c_str());
            int b = atoi(args[2].c_str());
            if (a < 0 || b < 0 || a == b || a >= files.size() || b >= files.size())
                ERROR("invalid idx");
            files[a].file->ReplaceCharsUse(files[b].file);
            std::cout << "[" << a << "] = [" << a << "] + [" << b << "]\n";
            continue;
        }
        if (m("pagename", 3)) {
            int idx = atoi(args[1].c_str());
            if (idx < 0 || idx >= files.size())
                ERROR("invalid idx");
            files[idx].file->renamePage(args[2], args[3]);
            continue;
        }
        if (m("save", 2)) {
            int idx = atoi(args[1].c_str());
            if (idx < 0 || idx >= files.size())
                ERROR("invalid idx");
            files[idx].file->SaveToFile(args[2].c_str());
            std::cout << "file saved\n";
            continue;
        }
        if (m("dumpchars", 1)) {
            int idx = atoi(args[1].c_str());
            if (idx < 0 || idx >= files.size())
                ERROR("invalid idx");
            files[idx].file->dumpChars();
            continue;
        }
        if (m("xoff", 2)) {
            int idx = atoi(args[1].c_str());
            if (idx < 0 || idx >= files.size())
                ERROR("invalid idx");
            int num = atoi(args[2].c_str());
            printf("all offset for idx=%d -> xoffset = xoffset + (%d)\n", idx, num);
            files[idx].file->forEachChars([&](chars& ch) {
                ch.xoffset += num;
                });
            continue;
        }
        if (m("yoff", 2)) {
            int idx = atoi(args[1].c_str());
            if (idx < 0 || idx >= files.size())
                ERROR("invalid idx");
            int num = atoi(args[2].c_str());
            printf("all offset for idx=%d -> yoffset = yoffset + (%d)\n", idx, num);
            files[idx].file->forEachChars([&](chars& ch) {
                ch.yoffset += num;
                });
            continue;
        }
        if (m("xadv", 2)) {
            int idx = atoi(args[1].c_str());
            if (idx < 0 || idx >= files.size())
                ERROR("invalid idx");
            int num = atoi(args[2].c_str());
            printf("all offset for idx=%d -> xadvance = xadvance + (%d)\n", idx, num);
            files[idx].file->forEachChars([&](chars& ch) {
                ch.xadvance += num;
                });
            continue;
        }
        if (m("x=", 3)) {
            int idx = atoi(args[1].c_str());
            if (idx < 0 || idx >= files.size())
                ERROR("invalid idx");
            int id = atoi(args[2].c_str());
            int num = atoi(args[3].c_str());
            files[idx].file->updateChars(idx, [&](chars& ch) {
                ch.x = num;
                });
            continue;
        }
        if (m("y=", 3)) {
            int idx = atoi(args[1].c_str());
            if (idx < 0 || idx >= files.size())
                ERROR("invalid idx");
            int id = atoi(args[2].c_str());
            int num = atoi(args[3].c_str());
            files[idx].file->updateChars(idx, [&](chars& ch) {
                ch.y = num;
                });
            continue;
        }
        if (m("w=", 3)) {
            int idx = atoi(args[1].c_str());
            if (idx < 0 || idx >= files.size())
                ERROR("invalid idx");
            int id = atoi(args[2].c_str());
            int num = atoi(args[3].c_str());
            files[idx].file->updateChars(idx, [&](chars& ch) {
                ch.width = num;
                });
            continue;
        }
        if (m("h=", 3)) {
            int idx = atoi(args[1].c_str());
            if (idx < 0 || idx >= files.size())
                ERROR("invalid idx");
            int id = atoi(args[2].c_str());
            int num = atoi(args[3].c_str());
            files[idx].file->updateChars(idx, [&](chars& ch) {
                ch.height = num;
                });
            continue;
        }
        if (m("xoff=", 3)) {
            int idx = atoi(args[1].c_str());
            if (idx < 0 || idx >= files.size())
                ERROR("invalid idx");
            int id = atoi(args[2].c_str());
            int num = atoi(args[3].c_str());
            files[idx].file->updateChars(idx, [&](chars& ch) {
                ch.xoffset = num;
                });
            continue;
        }
        if (m("yoff=", 3)) {
            int idx = atoi(args[1].c_str());
            if (idx < 0 || idx >= files.size())
                ERROR("invalid idx");
            int id = atoi(args[2].c_str());
            int num = atoi(args[3].c_str());
            files[idx].file->updateChars(idx, [&](chars& ch) {
                ch.yoffset = num;
                });
            continue;
        }
        if (m("xadv=", 3)) {
            int idx = atoi(args[1].c_str());
            if (idx < 0 || idx >= files.size())
                ERROR("invalid idx");
            int id = atoi(args[2].c_str());
            int num = atoi(args[3].c_str());
            files[idx].file->updateChars(idx, [&](chars& ch) {
                ch.xadvance = num;
                });
            continue;
        }
        if (m("page=", 3)) {
            int idx = atoi(args[1].c_str());
            if (idx < 0 || idx >= files.size())
                ERROR("invalid idx");
            int id = atoi(args[2].c_str());
            int num = atoi(args[3].c_str());
            if (num < 0 || num >= files[idx].file->common_ptr->pages) {
                printf("page not exists.\n");
                continue;
            }
            files[idx].file->updateChars(idx, [&](chars& ch) {
                ch.page = num;
                });
            continue;
        }
        if (m("rmkern", 1)) {
            int idx = atoi(args[1].c_str());
            if (idx < 0 || idx >= files.size())
                ERROR("invalid idx");
            files[idx].file->RemoveKerning();
            continue;
        }

        if (m("exit", 0))
            return 0;
        std::cout << "invalid command\n";
    }
    return 0;
}
