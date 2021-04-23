/*
MIT License

Copyright(c) 2021 frto027

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

#include <iostream>
#include <cstring>

/*

目前只是个简易的字体处理
1.手动将原始fnt文件加一个下划线后缀（比如luamini.fnt改成luamini_.fnt）
2.手动将原始png文件中所有文件名字的_换成-(比如luamini_0.png改成luamini-0.png)
3.将新的字体放进当前文件夹（luamini.fnt）
在同一文件夹内，执行exe文件，并输入字体名字（luamini.fnt），输入y，回车，会自动使用新的fnt替换luamini.fnt
5.最后留下来的有luamini.fnt(合并后的fnt)，luamini-0.png（合并前的png），luamini_0.png（自己补充的png，其中原始字体有的字符位置不会被使用），原始的luamini_.fnt可以删除。
（C语言写输入输出真费劲）


*/

#define ERROR(...) do{fprintf(stderr,__VA_ARGS__);exit(-1);}while(0)

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

class FntFile {
public:
    info* info_ptr = NULL;
    common* common_ptr = NULL;
    pages* pages_ptr = NULL;
    chars* chars_ptr = NULL;
    kerning* kerning_ptr = NULL;
    unsigned int info_sz, common_sz, pages_sz, chars_sz, kerning_sz;

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
                fread(&sz,sizeof(int),1,f);                         \
                if(sz < sizeof(type))                               \
                    ERROR("size is too small(%s)",fname);           \
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
        printf("font name:%s\n", info_ptr->fontName);
    }
    void PrintPageNames() {
        if (common_ptr == NULL)
            ERROR("common is null!");
        if (pages_ptr == NULL)
            ERROR("Can't print empty page!");
        int n = 0;
        printf("page count:%d\n", common_ptr->pages);
        while (n < pages_sz) {
            putchar(pages_ptr->pageNames[n] == '\0' ? '\n' : pages_ptr->pageNames[n]);
            n++;
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
    void ReplaceCharsUse(FntFile* other) {
        int myCount = CharCount();
        int oCount = other->CharCount();
        for (int i = 0; i < myCount; i++) {
            for (int j = 0; j < oCount; j++) {
                if (chars_ptr[i].id == other->chars_ptr[j].id) {
                    //replace
                    memcpy(&chars_ptr[i], &(other->chars_ptr[j]), sizeof(chars));
                    chars_ptr[i].page = AddOrGetPage(other->GetPageName(chars_ptr[i].page));
                }
            }
        }
    }
};

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
    char file_name[4096];

    printf("1.add a '_' to original fnt file.(e.g. rename luamini.fnt -> luamini_.fnt)\n\n");
    printf("2.replace all '_' in png file with '-'.(e.g. luamini_0.png -> luamini-0.png ........)\n\n");
    printf("3.paste your new font here\n(e.g. now, current folder contains 'luamini_.fnt','luamini-0.png','luamini.fnt','luamini_0.png')\n\n");
    printf("4.now, input your font name(e.g. input 'luamini.fnt'):\n");
    scanf("%s", file_name);
    int len = strlen(file_name);
    if (len < 4 || (strcmp(&file_name[len-4],".fnt") != 0)) {
        ERROR("wrong file name(must *.fnt)");
    }
    char file_name2[4097];
    strcpy(file_name2, file_name);
    strcpy(&file_name2[len - 4], "_.fnt");
    printf("parse font file: %s = (%s - %s) + %s\n", file_name, file_name, file_name2, file_name2);
    printf("input 'y' to continue, 'n' to exit:");
    int ch;
    while (true) {
        ch = getchar();
        if (ch == '\n' || ch == ' ')
            continue;
        if (ch == 'y')
            break;
        if (ch == 'n')
            return 0;
        printf("input 'y' to continue, 'n' to exit:");
    }
    {
        FntFile fontA, fontB;
        fontA.parseFntFile(file_name);
        fontB.parseFntFile(file_name2);
        fontB.ApplyReplacedRule();
        fontA.ReplaceCharsUse(&fontB);
        fontA.SaveToFile(file_name);
    }
    //    for(int i=0;i<255;i++){
    //        printf("char:id=%u,%u\n",font.chars_ptr[i].id,font.chars_ptr[i].page);
    //    }
    printf("over\n");
    system("pause");
    return 0;
}
