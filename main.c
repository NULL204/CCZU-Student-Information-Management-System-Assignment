#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sha256.h"

#define INPUT_FIELD_WIDTH 25
#define MAX_USERS 100
#define SHA256_DIGEST_LENGTH 32
#define TOTAL_DATA_LINES totalStudents
#define SCROLLBAR_HEIGHT 25

void setCursorPosition(int x, int y);
void mainUI(const char* username);
void modifyUI(const char *username, int index);

typedef struct {
    char studentId[20];
    char name[50];
    int scores[3]; // 0: ����, 1: ��ѧ, 2: Ӣ��
} StudentInfo;

#define MAX_STUDENTS 100

int readStudentData(const char* username, StudentInfo students[], int maxStudents) {

    char filePath[MAX_PATH];
    sprintf(filePath, "%s\\data", username);

    FILE* file = fopen(filePath, "r");
    if (file == NULL) {
        perror("Unable to open file");
        return 0;
    }

    int count = 0;
    while (count < maxStudents && fscanf(file, "%[^,],%[^,],%d,%d,%d\n",
                                         students[count].studentId, students[count].name,
                                         &students[count].scores[0], &students[count].scores[1],
                                         &students[count].scores[2]) == 5) {
        count++;
    }

    fclose(file);
    return count;
}

#define DISPLAY_HEIGHT 12

void renderDisplay(StudentInfo students[], int totalStudents, int offset) {
    setCursorPosition(0, 4);
    printf("ѧ��\t\t    ����\t\t����\t\t    ��ѧ\t\tӢ��\n");
    for (int i = 0; i < DISPLAY_HEIGHT; i++) {
        int dataIndex = i + offset;
        if (dataIndex < totalStudents) {
            printf("\n");
            printf("%-20s%-20s%-20d%-20d%-5d\n",
                   students[dataIndex].studentId,
                   students[dataIndex].name,
                   students[dataIndex].scores[0],
                   students[dataIndex].scores[1],
                   students[dataIndex].scores[2]);
        }
    }

    // ����Ƿ���Ҫ������
    if (totalStudents > DISPLAY_HEIGHT) {
        int scrollbarPosition =  (offset * (SCROLLBAR_HEIGHT - 1)) / (totalStudents - DISPLAY_HEIGHT);
        for (int i = 0; i < SCROLLBAR_HEIGHT; i++) {
            setCursorPosition(115, 4 + i);
            if (i == scrollbarPosition) {
                printf("  #\n"); // ����λ��
            } else {
                printf("  |\n"); // �������߿�
            }
        }
    } else {
        // �������Ҫ����������ʾһ���̶�λ�õĹ���������ʾ
        for (int i = 0; i < SCROLLBAR_HEIGHT; i++) {
            setCursorPosition(115, 4 + i);
            if (i == 0) {
                printf("  #\n"); // �̶�λ�õĻ���
            } else {
                printf("  |\n"); // �������߿�
            }
        }
    }
}


#define SCORE_RANGES 11 // 0-9, 10-19, ..., 90-100

void countScores(StudentInfo students[], int totalStudents, int count[3][SCORE_RANGES]) {
    memset(count, 0, sizeof(int) * 3 * SCORE_RANGES);

    for (int i = 0; i < totalStudents; i++) {
        for (int j = 0; j < 3; j++) {
            int score = students[i].scores[j];
            if (score >= 0 && score <= 100) {
                int range = score < 100 ? score / 10 : 10; // �� 100 �ֹ鵽���һ������
                count[j][range]++;
            }
        }
    }
}

#define MAX_HISTOGRAM_LENGTH 80

void drawHistogram(int scoreCounts[SCORE_RANGES], const char* subject, int maxScoreCount) {
    setCursorPosition(22, 5);
    printf("%s �ɼ��ֲ���\n", subject);

    for (int i = 0; i < SCORE_RANGES; i++) {
        setCursorPosition(22, 2 * i + 7);
        printf("                                                                                                    ");
        setCursorPosition(22, 2 * i + 7);
        // ��ʾ������
        if (i < SCORE_RANGES - 1) {
            printf("%2d-%2d: ", i * 10, i * 10 + 9);
        } else {
            printf("  100: ");
        }

        // ������״ͼ����
        int length = (scoreCounts[i] * MAX_HISTOGRAM_LENGTH) / maxScoreCount;
        if (length > MAX_HISTOGRAM_LENGTH) {
            length = MAX_HISTOGRAM_LENGTH;
        }

        // ������״ͼ
        for (int j = 0; j < length; j++) {
            printf("\033[47;30m \033[0m");
        }

        printf(" (%d)\n", scoreCounts[i]);
    }
}



//������Ϊ���
int keyboard(int *x, int *y, int *key, int *wheel) {
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD fdwSaveOldMode, fdwMode;
    INPUT_RECORD irInBuf[128];

    GetConsoleMode(hStdin, &fdwSaveOldMode); // ����ԭ����̨ģʽ

    // �������ͼ�������
    fdwMode = ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
    SetConsoleMode(hStdin, fdwMode);

    while (1) {
        DWORD cNumRead;
        ReadConsoleInput(hStdin, irInBuf, 128, &cNumRead);

        for (DWORD i = 0; i < cNumRead; i++) {
            if (irInBuf[i].EventType == KEY_EVENT ) {
                if (irInBuf[i].Event.KeyEvent.bKeyDown) {
                    KEY_EVENT_RECORD ker = irInBuf[i].Event.KeyEvent;
                    switch (ker.wVirtualKeyCode) {
                        case VK_UP:
                            *key = 0;
                            return -1;
                            break;
                        case VK_DOWN:
                            *key = 1;
                            return -1;
                            break;
                        default:
                            *key = ker.uChar.AsciiChar;
                            break;
                    }
                    SetConsoleMode(hStdin, fdwSaveOldMode); // �ָ�ԭ����̨ģʽ
                    return 0;
                }
            } else if (irInBuf[i].EventType == MOUSE_EVENT) {
                MOUSE_EVENT_RECORD mer = irInBuf[i].Event.MouseEvent;

                if (mer.dwEventFlags == 0) {
                    if (mer.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED) {
                        *x = mer.dwMousePosition.X;
                        *y = mer.dwMousePosition.Y;
                        SetConsoleMode(hStdin, fdwSaveOldMode); // �ָ�ԭ����̨ģʽ
                        return 1;
                    }
                } else if (mer.dwEventFlags == MOUSE_WHEELED) {
                    if (HIWORD(mer.dwButtonState) == 120) {
                        *wheel = 0; // �������Ϲ���
                        SetConsoleMode(hStdin, fdwSaveOldMode); // �ָ�ԭ����̨ģʽ
                        return 2;
                    } else if (HIWORD(mer.dwButtonState) == 65416) { // 65416 is 0xFF88 in hex
                        *wheel = 1; // �������¹���
                        SetConsoleMode(hStdin, fdwSaveOldMode); // �ָ�ԭ����̨ģʽ
                        return 2;
                    }
                }
            }
        }
    }
}

//����ƶ�
void setCursorPosition(int x, int y) {
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = {x, y};

    SetConsoleCursorPosition(hStdout, pos);
}

int createDirectory(const char* name) {
    if (CreateDirectory(name, NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
        return 1; // �ɹ��������Ѵ���
    } else {
        return 0; // ����ʧ��
    }
}

//SHA-256
/****************************** MACROS ******************************/
#define ROTLEFT(a,b) (((a) << (b)) | ((a) >> (32-(b))))
#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))

#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))

/**************************** VARIABLES *****************************/
static const WORD1 k[64] = {
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

/*********************** FUNCTION DEFINITIONS ***********************/
void sha256_transform(SHA256_CTX *ctx, const BYTE data[])
{
    WORD1 a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];

    for (i = 0, j = 0; i < 16; ++i, j += 4)
        m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
    for ( ; i < 64; ++i)
        m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];

    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];

    for (i = 0; i < 64; ++i) {
        t1 = h + EP1(e) + CH(e,f,g) + k[i] + m[i];
        t2 = EP0(a) + MAJ(a,b,c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

void sha256_init(SHA256_CTX *ctx)
{
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0] = 0x6a09e667;
    ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372;
    ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f;
    ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab;
    ctx->state[7] = 0x5be0cd19;
}

void sha256_update(SHA256_CTX *ctx, const BYTE data[], size_t len)
{
    WORD1 i;

    for (i = 0; i < len; ++i) {
        ctx->data[ctx->datalen] = data[i];
        ctx->datalen++;
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

void sha256_final(SHA256_CTX *ctx, BYTE hash[])
{
    WORD1 i;

    i = ctx->datalen;

    // Pad whatever data is left in the buffer.
    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56)
            ctx->data[i++] = 0x00;
    }
    else {
        ctx->data[i++] = 0x80;
        while (i < 64)
            ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }

    // Append to the padding the total message's length in bits and transform.
    ctx->bitlen += ctx->datalen * 8;
    ctx->data[63] = ctx->bitlen;
    ctx->data[62] = ctx->bitlen >> 8;
    ctx->data[61] = ctx->bitlen >> 16;
    ctx->data[60] = ctx->bitlen >> 24;
    ctx->data[59] = ctx->bitlen >> 32;
    ctx->data[58] = ctx->bitlen >> 40;
    ctx->data[57] = ctx->bitlen >> 48;
    ctx->data[56] = ctx->bitlen >> 56;
    sha256_transform(ctx, ctx->data);

    // Since this implementation uses little endian byte ordering and SHA uses big endian,
    // reverse all the bytes when copying the final state to the output hash.
    for (i = 0; i < 4; ++i) {
        hash[i]      = (ctx->state[0] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 4]  = (ctx->state[1] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 8]  = (ctx->state[2] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & 0x000000ff;
    }
}

void hashSHA256(const unsigned char* input, size_t length, unsigned char output[SHA256_DIGEST_LENGTH]) {
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, input, length);
    sha256_final(&ctx, output);
}

int writePasswordHashToFile(const char* directory, const char* password) {
    unsigned char hashedPassword[SHA256_DIGEST_LENGTH];
    char filePath[260]; // �㹻��������·��

    // ��������Ĺ�ϣֵ
    hashSHA256((const unsigned char*)password, strlen(password), hashedPassword);

    // �����ļ�·��
    sprintf(filePath, "%s\\password.txt", directory);

    // ���ļ���д��ģʽ
    FILE* file = fopen(filePath, "w");
    if (file == NULL) {
        perror("fopen");
        return 0; // �ļ���ʧ��
    }

    // ����ϣֵд���ļ�
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        fprintf(file, "%02x", hashedPassword[i]);
    }

    // �ر��ļ�
    fclose(file);
    return 1; // �ɹ�д���ļ�
}

int getUsernames(char usernames[MAX_USERS][MAX_PATH]) {
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(".\\*", &findFileData);
    int count = 0;

    if (hFind == INVALID_HANDLE_VALUE) {
        printf("FindFirstFile failed (%lu)\n", GetLastError());
        return 0;
    }

    do {
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (strcmp(findFileData.cFileName, ".") != 0 && strcmp(findFileData.cFileName, "..") != 0) {
                strcpy(usernames[count], findFileData.cFileName);
                count++;
                if (count >= MAX_USERS) break;
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
    return count;
}

int verifyUsername(const char* username, char usernames[MAX_USERS][MAX_PATH], int count) {
    for (int i = 0; i < count; i++) {
        if (strcmp(username, usernames[i]) == 0) {
            return 1; // �ҵ�ƥ����û���
        }
    }
    return 0; // û���ҵ�ƥ����û���
}

int verifyPassword(const char* username, const char* inputPassword) {
    char filePath[MAX_PATH];
    sprintf(filePath, "%s\\password.txt", username);

    FILE* file = fopen(filePath, "r");
    if (file == NULL) {
        perror("fopen");
        return 0;
    }

    char storedHash[65];
    if (fscanf(file, "%64s", storedHash) != 1) {
        fclose(file);
        return 0;
    }
    fclose(file);

    // ��ϣ�������
    unsigned char hashedPassword[SHA256_DIGEST_LENGTH];
    hashSHA256((const unsigned char*)inputPassword, strlen(inputPassword), hashedPassword);

    // ����ϣֵת��Ϊʮ�������ַ���
    char inputHash[65]; // SHA-256 �Ĺ�ϣֵ����Ϊ 64 �ֽڣ����� null ������
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(&inputHash[i * 2], "%02x", hashedPassword[i]);
    }


    return strcmp(storedHash, inputHash) == 0;
}


//����UI
//��¼����
void loginUI(const char* username, const char* password) {
    system("cls");
    setCursorPosition(52, 9);
    printf("ѧ����Ϣ����ϵͳ\n");
    setCursorPosition(41, 12);
    printf("�˺ţ�[%-25s]", username);
    setCursorPosition(41, 13);
    printf("���룺[%-25s]", password);
    setCursorPosition(50, 16);
    printf("\033[47;30m                    \033[0m");
    setCursorPosition(50, 17);
    printf("\033[47;30m        ��¼        \033[0m");
    setCursorPosition(50, 18);
    printf("\033[47;30m                    \033[0m");
    setCursorPosition(80, 20);
    printf("\033[47;30mע��\033[0m");
}

//ע�����
void registerUI(const char* username, const char* password) {
    system("cls");
    setCursorPosition(52, 9);
    printf("ѧ����Ϣ����ϵͳ\n");
    setCursorPosition(41, 12);
    printf("�˺ţ�[%-25s]", username);
    setCursorPosition(41, 13);
    printf("���룺[%-25s]", password);
    setCursorPosition(50, 16);
    printf("\033[47;30m                    \033[0m");
    setCursorPosition(50, 17);
    printf("\033[47;30m        ע��        \033[0m");
    setCursorPosition(50, 18);
    printf("\033[47;30m                    \033[0m");
    setCursorPosition(80, 20);
    printf("\033[47;30m��¼\033[0m");
}

//���ܽ���
void summaryUI(const char* username) {
    system("cls");
    setCursorPosition(0, 0);
    printf("\033[47;30m                                                                                                                        \n\033[0m");
    printf("\033[47;30m                                                                                                                        \n\033[0m");
    printf("\033[47;30m                                                                                                                        \n\033[0m");
    setCursorPosition(52, 1);
    printf("\033[47;30mѧ����Ϣ����ϵͳ\033[0m");
    setCursorPosition(5, 10);
    printf("\033[47;30m        \033[0m");
    setCursorPosition(5, 11);
    printf("\033[47;30m  ����  \033[0m");
    setCursorPosition(5, 12);
    printf("\033[47;30m        \033[0m");
    setCursorPosition(5, 15);
    printf("\033[47;30m        \033[0m");
    setCursorPosition(5, 16);
    printf("\033[47;30m  ��ѧ  \033[0m");
    setCursorPosition(5, 17);
    printf("\033[47;30m        \033[0m");
    setCursorPosition(5, 20);
    printf("\033[47;30m        \033[0m");
    setCursorPosition(5, 21);
    printf("\033[47;30m  Ӣ��  \033[0m");
    setCursorPosition(5, 22);
    printf("\033[47;30m        \033[0m");
    setCursorPosition(2,1);
    printf("����");


    StudentInfo students[MAX_STUDENTS];
    int totalStudents = readStudentData(username, students, MAX_STUDENTS);
    int scoreCounts[3][SCORE_RANGES];

    countScores(students, totalStudents, scoreCounts);

    const char* subjects[] = {"����", "��ѧ", "Ӣ��"};

    int maxScoreCount = 0;

    for (int subj = 0; subj < 3; subj++) {
        for (int i = 0; i < SCORE_RANGES; i++) {
            if (scoreCounts[subj][i] > maxScoreCount) {
                maxScoreCount = scoreCounts[subj][i];
            }
        }
    }


    drawHistogram(scoreCounts[0], subjects[0], maxScoreCount);

    int x, y, key, wheel;
    int type;
    while (1) {
        type = keyboard(&x, &y, &key, &wheel);
        if (type == 1) {
            if (x >= 5 && x <= 11 && y >= 10 && y <= 12) {
                // ����
                drawHistogram(scoreCounts[0], subjects[0], maxScoreCount);
            } else if (x >= 5 && x <= 11 && y >= 15 && y <= 17) {
                // ��ѧ
                drawHistogram(scoreCounts[1], subjects[1], maxScoreCount);
            } else if (x >= 5 && x <= 11 && y >= 20 && y <= 22) {
                // Ӣ��
                drawHistogram(scoreCounts[2], subjects[2], maxScoreCount);
            } else if (x >= 2 && x <= 6 && y == 1) {
                // ����
                break;
            }
        }
    }
    mainUI(username);
}

void queryStudentScore(StudentInfo students[], int totalStudents, const char* studentId) {
    setCursorPosition(0, 6);
    printf("ѧ��\t\t    ����\t\t����\t\t    ��ѧ\t\tӢ��\n");
    int found = 0;
    for (int i = 0; i < totalStudents; i++) {
        if (strcmp(students[i].studentId, studentId) == 0) {
            found ++;
            printf("%-20s%-20s%-20d%-20d%-5d\n",
                   students[i].studentId,
                   students[i].name,
                   students[i].scores[0],
                   students[i].scores[1],
                   students[i].scores[2]);
        }
    }
    if (found == 0)
        printf("δ�ҵ�ѧ��Ϊ %s ��ѧ����\n", studentId);
}

//��ѯ����
void searchUI(const char* username, const char* studentId) {
    system("cls");
    setCursorPosition(0, 0);
    printf("\033[47;30m                                                                                                                        \n\033[0m");
    printf("\033[47;30m                                                                                                                        \n\033[0m");
    printf("\033[47;30m                                                                                                                        \n\033[0m");
    setCursorPosition(52, 1);
    printf("\033[47;30mѧ����Ϣ����ϵͳ\033[0m");
    setCursorPosition(4, 4);
    printf("������Ҫ��ѯ��ѧ��ѧ��:[%-25s]", studentId);
    setCursorPosition(60, 4);
    printf("\033[47;30m��ѯ\033[0m");
    setCursorPosition(2, 1);
    printf("����");

}

//׷�Ӻ���
void appendStudentData(const char* username, const char* studentId, const char* name, const char* chinese, const char* math, const char* english) {
    char filePath[MAX_PATH];
    sprintf(filePath, "%s\\data", username);

    FILE* file = fopen(filePath, "a");
    if (file == NULL) {
        perror("fopen");
        return;
    }

    fprintf(file, "%s,%s,%s,%s,%s\n", studentId, name, chinese, math, english);
    fclose(file);
}

//�½�����
void createUI(const char *username, const char *studentId, const char *name, const char *chinese, const char *math, const char *english) {
    system("cls");
    printf("\033[47;30m                                                                                                                        \n\033[0m");
    printf("\033[47;30m                                                                                                                        \n\033[0m");
    printf("\033[47;30m                                                                                                                        \n\033[0m");
    setCursorPosition(52, 1);
    printf("\033[47;30mѧ����Ϣ����ϵͳ\033[0m");
    setCursorPosition(43, 9);
    printf("ѧ�ţ�[%-20s]", studentId);
    setCursorPosition(43, 11);
    printf("������[%-20s]", name);
    setCursorPosition(43, 13);
    printf("���ģ�[%-20s]", chinese);
    setCursorPosition(43, 15);
    printf("��ѧ��[%-20s]", math);
    setCursorPosition(43, 17);
    printf("Ӣ�[%-20s]", english);
    setCursorPosition(50, 20);
    printf("\033[47;30m                    \033[0m");
    setCursorPosition(50, 21);
    printf("\033[47;30m        �½�        \033[0m");
    setCursorPosition(50, 22);
    printf("\033[47;30m                    \033[0m");
    setCursorPosition(80, 24);
    printf("\033[47;30mȡ��\033[0m");
}


//�޸Ľ���
void modifyUI(const char *username, int index) {
    StudentInfo students[MAX_STUDENTS];
    int totalStudents = readStudentData(username, students, MAX_STUDENTS);

    char StudentId[20], Name[50], Chinese[10], Math[10], English[10];

    // ��ȡԭʼѧ������
    strcpy(StudentId, students[index].studentId);
    strcpy(Name, students[index].name);
    sprintf(Chinese, "%d", students[index].scores[0]);
    sprintf(Math, "%d", students[index].scores[1]);
    sprintf(English, "%d", students[index].scores[2]);
    int inputMode = 0; // 0: ѧ��, 1: ����, 2: ����, 3: ��ѧ, 4: Ӣ��

    UI:{
        system("cls");
        printf("\033[47;30m                                                                                                                        \n\033[0m");
        printf("\033[47;30m                                                                                                                        \n\033[0m");
        printf("\033[47;30m                                                                                                                        \n\033[0m");
        setCursorPosition(52, 1);
        printf("\033[47;30mѧ����Ϣ����ϵͳ\033[0m");
        setCursorPosition(43, 9);
        printf("ѧ�ţ�[%-20s]", StudentId);
        setCursorPosition(43, 11);
        printf("������[%-20s]", Name);
        setCursorPosition(43, 13);
        printf("���ģ�[%-20s]", Chinese);
        setCursorPosition(43, 15);
        printf("��ѧ��[%-20s]", Math);
        setCursorPosition(43, 17);
        printf("Ӣ�[%-20s]", English);
        setCursorPosition(50, 20);
        printf("\033[47;30m                    \033[0m");
        setCursorPosition(50, 21);
        printf("\033[47;30m        �޸�        \033[0m");
        setCursorPosition(50, 22);
        printf("\033[47;30m                    \033[0m");
        setCursorPosition(80, 24);
        printf("\033[47;30mȡ��\033[0m");

        int x, y, key;
        while (1) {
            int type = keyboard(&x, &y, &key, NULL);
            if (type == 1) {
                if (x >= 43 && x <= 63 && y == 9) {
                    setCursorPosition(50, 9);
                    inputMode = 0; // �л�����ģʽ
                } else if (x >= 43 && x <= 63 && y == 11) {
                    setCursorPosition(50, 11);
                    inputMode = 1; // �л�����ģʽ
                } else if (x >= 43 && x <= 63 && y == 13) {
                    setCursorPosition(50, 13);
                    inputMode = 2; // �л�����ģʽ
                } else if (x >= 43 && x <= 63 && y == 15) {
                    setCursorPosition(50, 15);
                    inputMode = 3; // �л�����ģʽ
                } else if (x >= 43 && x <= 63 && y == 17) {
                    setCursorPosition(50, 17);
                    inputMode = 4; // �л�����ģʽ
                } else if (x >= 50 && x <= 75 && y >= 20 && y <= 22) {
                    // ����

                    strcpy(students[index].studentId, StudentId);
                    strcpy(students[index].name, Name);
                    students[index].scores[0] = atoi(Chinese);
                    students[index].scores[1] = atoi(Math);
                    students[index].scores[2] = atoi(English);
                    //�������Ƿ���д
                    if (strlen(StudentId) == 0 || strlen(Name) == 0 || strlen(Chinese) == 0 || strlen(Math) == 0 || strlen(English) == 0) {
                        MessageBox(NULL, "����д������Ϣ��", "��ʾ", MB_OK);
                        continue;
                    }

                    char filePath[MAX_PATH];
                    sprintf(filePath, "%s\\data", username);
                    FILE* file = fopen(filePath, "w");
                    if (file == NULL) {
                        perror("Unable to open file");
                        return;
                    }
                    for (int i = 0; i < totalStudents; i++) {
                        fprintf(file, "%s,%s,%d,%d,%d\n",
                                students[i].studentId,
                                students[i].name,
                                students[i].scores[0],
                                students[i].scores[1],
                                students[i].scores[2]);
                    }
                    fclose(file);

                    mainUI(username);
                    return;
                } else if (x >= 80 && x <= 84 && y == 24) {
                    // ȡ��
                    mainUI(username);
                    return;
                }
                continue;
            }if (key == '\b') { // Backspace��
                // ɾ���û����������е����һ���ַ�
                if (inputMode == 0 && strlen(StudentId) > 0) {
                    StudentId[strlen(StudentId) - 1] = '\0';
                } else if (inputMode == 1 && strlen(Name) > 0) {
                    Name[strlen(Name) - 1] = '\0';
                } else if (inputMode == 2 && strlen(Chinese) > 0) {
                    Chinese[strlen(Chinese) - 1] = '\0';
                } else if (inputMode == 3 && strlen(Math) > 0) {
                    Math[strlen(Math) - 1] = '\0';
                } else if (inputMode == 4 && strlen(English) > 0) {
                    English[strlen(English) - 1] = '\0';
                }
            } else if (key >= ' ' && key <= '~') {
                // �������ַ�����
                if (inputMode == 0 && strlen(StudentId) < INPUT_FIELD_WIDTH) {
                    int len = strlen(StudentId);
                    StudentId[len] = key;
                    StudentId[len + 1] = '\0';
                } else if (inputMode == 1 && strlen(Name) < INPUT_FIELD_WIDTH) {
                    int len = strlen(Name);
                    Name[len] = key;
                    Name[len + 1] = '\0';
                } else if (inputMode == 2 && strlen(Chinese) < INPUT_FIELD_WIDTH) {
                    int len = strlen(Chinese);
                    Chinese[len] = key;
                    Chinese[len + 1] = '\0';
                } else if (inputMode == 3 && strlen(Math) < INPUT_FIELD_WIDTH) {
                    int len = strlen(Math);
                    Math[len] = key;
                    Math[len + 1] = '\0';
                } else if (inputMode == 4 && strlen(English) < INPUT_FIELD_WIDTH) {
                    int len = strlen(English);
                    English[len] = key;
                    English[len + 1] = '\0';
                }
            }
            // ������ʾ UI ����ӳ����
            goto UI;
        }
    };
}

//ɾ������
void deleteStudentDataByLine(const char* username, int lineNumber) {
    char filePath[MAX_PATH];
    sprintf(filePath, "%s\\data", username);

    StudentInfo students[MAX_STUDENTS];
    int totalStudents = readStudentData(username, students, MAX_STUDENTS);


    // ����������0��ʼ�������Ҫ��1
    lineNumber--;

    // �ƶ�ʣ��Ԫ�ظ���Ҫɾ������
    for (int i = lineNumber; i < totalStudents - 1; i++) {
        students[i] = students[i + 1];
    }
    totalStudents--; // ������ѧ����

    // �����º������д���ļ�
    FILE* file = fopen(filePath, "w");
    if (file == NULL) {
        perror("Unable to open file");
        return;
    }

    for (int i = 0; i < totalStudents; i++) {
        fprintf(file, "%s,%s,%d,%d,%d\n",
                students[i].studentId,
                students[i].name,
                students[i].scores[0],
                students[i].scores[1],
                students[i].scores[2]);
    }

    fclose(file);
}

//������
void mainUI(const char* username) {
    StudentInfo students[MAX_STUDENTS];
    int totalStudents = readStudentData(username, students, MAX_STUDENTS);
    system("cls");
    setCursorPosition(0, 0);
    printf("\033[47;30m                                                                                                                        \n\033[0m");
    printf("\033[47;30m                                                                                                                        \n\033[0m");
    printf("\033[47;30m                                                                                                                        \n\033[0m");
    setCursorPosition(52, 1);
    printf("\033[47;30mѧ����Ϣ����ϵͳ\033[0m");
    setCursorPosition(2, 1);
    printf("����%d��ѧ����Ϣ", totalStudents);
    setCursorPosition(90, 1);
    printf("����");
    setCursorPosition(98, 1);
    printf("��ѯ");
    setCursorPosition(106, 1);
    printf("�½�");

    setCursorPosition(0, 4);
    int offset = 0;

    if (totalStudents == 0) {
        setCursorPosition(0, 6);
        printf("û��ѧ����Ϣ!");
    }

    renderDisplay(students, totalStudents, offset);

    int totalLines = 0;

    if (totalStudents >= 12) {
        totalLines = 12;
        for (int i = 0; i < 12; i++) {
            setCursorPosition(94, 6 + 2 * i);
            printf("\033[47;30m�޸�\033[0m");
            setCursorPosition(102, 6 + 2 * i);
            printf("\033[47;30mɾ��\033[0m");
        }
    } else if (totalStudents >= 0 && totalStudents < 12) {
        totalLines = totalStudents;
        for (int i = 0; i < totalStudents; i++) {
            setCursorPosition(94, 6 + 2 * i);
            printf("\033[47;30m�޸�\033[0m");
            setCursorPosition(102, 6 + 2 * i);
            printf("\033[47;30mɾ��\033[0m");
        }
    }

    while (1) {
        int x, y, key, wheel;
        int type = keyboard(&x, &y, &key, &wheel);

        if (type == -1) {
            if (key == 0 && offset > 0) {
                offset--;
                renderDisplay(students, totalStudents, offset);
            } else if (key == 1 && offset < TOTAL_DATA_LINES - DISPLAY_HEIGHT) {
                offset++;
                renderDisplay(students, totalStudents, offset);
            }
        } else if (type == 1) {
            if (x >= 90 && x <= 94 && y == 1) {
                //����
                summaryUI(username);
            } else if (x >= 98 && x <= 102 && y == 1) {
                //��ѯ
                char studentId[INPUT_FIELD_WIDTH + 1] = {0};
                searchUI(username, studentId);
                while (1) {
                    type = keyboard(&x, &y, &key, NULL);
                    if (type == 1) {
                        if (x >= 60 && x <= 64 && y == 4) {
                            // ��ѯ
                            StudentInfo students[MAX_STUDENTS];
                            int totalStudents = readStudentData(username, students, MAX_STUDENTS);

                            // ���ò�ѯ����
                            queryStudentScore(students, totalStudents, studentId);
                        } else if (x >= 2 && x <= 6 && y == 1) {
                            // ����
                            mainUI(username);
                            return;
                        }
                        continue;
                    }
                    if (key == '\b') { // Backspace��
                        // ɾ���û����������е����һ���ַ�
                        if (strlen(studentId) > 0) {
                            studentId[strlen(studentId) - 1] = '\0';
                        }
                    } else if (key >= ' ' && key <= '~') {
                        // �������ַ�����
                        if (strlen(studentId) < INPUT_FIELD_WIDTH) {
                            int len = strlen(studentId);
                            studentId[len] = key;
                            studentId[len + 1] = '\0';
                        }
                    }
                    searchUI(username, studentId);
                }
            } else if (x >= 106 && x <= 110 && y == 1) {
                //�½�
                int inputMode = 0; // 0: ����ѧ��, 1: ��������, 2: �������ĳɼ�, 3: ������ѧ�ɼ�, 4: ����Ӣ��ɼ�

                char studentId[INPUT_FIELD_WIDTH + 1] = {0};
                char name[INPUT_FIELD_WIDTH + 1] = {0};
                char score1[INPUT_FIELD_WIDTH + 1] = {0};
                char score2[INPUT_FIELD_WIDTH + 1] = {0};
                char score3[INPUT_FIELD_WIDTH + 1] = {0};
                createUI(username, studentId, name, score1, score2, score3);
                while (1) {
                    type = keyboard(&x, &y, &key, NULL);
                    if (type == 1) {
                        if (x >= 43 && x <= 63 && y == 9) {
                            setCursorPosition(50, 9);
                            inputMode = 0; // �л�����ģʽ
                        } else if (x >= 43 && x <= 63 && y == 11) {
                            setCursorPosition(50, 11);
                            inputMode = 1; // �л�����ģʽ
                        } else if (x >= 43 && x <= 63 && y == 13) {
                            setCursorPosition(50, 13);
                            inputMode = 2; // �л�����ģʽ
                        } else if (x >= 43 && x <= 63 && y == 15) {
                            setCursorPosition(50, 15);
                            inputMode = 3; // �л�����ģʽ
                        } else if (x >= 43 && x <= 63 && y == 17) {
                            setCursorPosition(50, 17);
                            inputMode = 4; // �л�����ģʽ
                        } else if (x >= 50 && x <= 75 && y >= 20 && y <= 22) {
                            // �½�
                            //�������Ƿ���д
                            if (strlen(studentId) == 0 || strlen(name) == 0 || strlen(score1) == 0 ||
                                strlen(score2) == 0 || strlen(score3) == 0) {
                                MessageBox(NULL, "����д������Ϣ��", "��ʾ", MB_OK);
                                continue;
                            }
                            appendStudentData(username, studentId, name, score1, score2, score3);
                            mainUI(username);
                            return;
                        } else if (x >= 80 && x <= 84 && y == 24) {
                            // ȡ��
                            mainUI(username);
                            return;
                        }
                        continue;
                    }
                    if (key == '\b') { // Backspace��
                        // ɾ���û����������е����һ���ַ�
                        if (inputMode == 0 && strlen(studentId) > 0) {
                            studentId[strlen(studentId) - 1] = '\0';
                        } else if (inputMode == 1 && strlen(name) > 0) {
                            name[strlen(name) - 1] = '\0';
                        } else if (inputMode == 2 && strlen(score1) > 0) {
                            score1[strlen(score1) - 1] = '\0';
                        } else if (inputMode == 3 && strlen(score2) > 0) {
                            score2[strlen(score2) - 1] = '\0';
                        } else if (inputMode == 4 && strlen(score3) > 0) {
                            score3[strlen(score3) - 1] = '\0';
                        }
                    } else if (key >= ' ' && key <= '~') {
                        // �������ַ�����
                        if (inputMode == 0 && strlen(studentId) < INPUT_FIELD_WIDTH) {
                            int len = strlen(studentId);
                            studentId[len] = key;
                            studentId[len + 1] = '\0';
                        } else if (inputMode == 1 && strlen(name) < INPUT_FIELD_WIDTH) {
                            int len = strlen(name);
                            name[len] = key;
                            name[len + 1] = '\0';
                        } else if (inputMode == 2 && strlen(score1) < INPUT_FIELD_WIDTH) {
                            int len = strlen(score1);
                            score1[len] = key;
                            score1[len + 1] = '\0';
                        } else if (inputMode == 3 && strlen(score2) < INPUT_FIELD_WIDTH) {
                            int len = strlen(score2);
                            score2[len] = key;
                            score2[len + 1] = '\0';
                        } else if (inputMode == 4 && strlen(score3) < INPUT_FIELD_WIDTH) {
                            int len = strlen(score3);
                            score3[len] = key;
                            score3[len + 1] = '\0';
                        }
                    }
                    createUI(username, studentId, name, score1, score2, score3);
                }
            } else if (x >= 94 && x <= 98 && !(y % 2) && y >= 6 && y <= 4 + 2 * totalLines) {
                //�޸�
                int index = (y - 6) / 2 + offset;
                modifyUI(username, index);
                return;
            } else if (x >= 102 && x <= 106 && !(y % 2) && y >= 6 && y <= 4 + 2 * totalLines) {
                //ɾ��
                int index = (y - 6) / 2 + offset + 1;
                deleteStudentDataByLine(username, index);
                mainUI(username);
                return;
            }
        } else if (type == 2) {
            if (wheel == 0 && offset > 0) { // �������Ϲ���
                offset--;
                renderDisplay(students, totalStudents, offset);
            } else if (wheel == 1 && offset < TOTAL_DATA_LINES - DISPLAY_HEIGHT) { // �������¹���
                offset++;
                renderDisplay(students, totalStudents, offset);
            }
        }
    }
}

int main () {
    int x, y, key;

    //��¼
    char username[INPUT_FIELD_WIDTH + 1] = {0};
    char password[INPUT_FIELD_WIDTH + 1] = {0};
    int inputMode = 0; // 0: �����û���, 1: ��������

    signin:

    loginUI(username, password);

    keyboard(&x, &y, &key, NULL);
    if (x >= 42 && x <= 73 && y == 12) {
        setCursorPosition(48, 12);
        inputMode = 0; // �л�����ģʽ
    } else if (x >= 42 && x <= 73 && y == 13) {
        setCursorPosition(48, 13);
        inputMode = 1; // �л�����ģʽ}
    } else if (x >= 50 && x <= 75 && y >= 17 && y <= 19) {
        //��¼
        goto login;
    } else if (x >= 80 && x <= 84 && y == 20) {
        //ע��
        goto signup;
    }

    while (1) {
        int type = keyboard(&x, &y, &key, NULL);
        if (type == 1) {
            if (x >= 42 && x <= 73 && y == 12) {
                setCursorPosition(48, 12);
                inputMode = 0; // �л�����ģʽ
            } else if (x >= 42 && x <= 73 && y == 13) {
                setCursorPosition(48, 13);
                inputMode = 1; // �л�����ģʽ
            } else if (x >= 50 && x <= 75 && y >= 17 && y <= 19) {
                //��¼
                goto login;
            } else if (x >= 80 && x <= 84 && y == 20) {
                //ע��
                goto signup;
            }
            continue;
        }
        if (key == '\b') { // Backspace��
            // ɾ���û����������е����һ���ַ�
            if (inputMode == 0 && strlen(username) > 0) {
                username[strlen(username) - 1] = '\0';
            } else if (inputMode == 1 && strlen(password) > 0) {
                password[strlen(password) - 1] = '\0';
            }
        } else if (key >= ' ' && key <= '~') {
            // �������ַ�����
            if (inputMode == 0 && strlen(username) < INPUT_FIELD_WIDTH) {
                int len = strlen(username);
                username[len] = key;
                username[len + 1] = '\0';
            } else if (inputMode == 1 && strlen(password) < INPUT_FIELD_WIDTH) {
                int len = strlen(password);
                password[len] = key;
                password[len + 1] = '\0';
            }
        }
        loginUI(username, password);
    }

    signup: {

        registerUI(username, password);

        keyboard(&x, &y, NULL, NULL);
        if (x >= 42 && x <= 73 && y == 12) {
            setCursorPosition(48, 12);
            inputMode = 0; // �л�����ģʽ
        } else if (x >= 42 && x <= 73 && y == 13) {
            setCursorPosition(48, 13);
            inputMode = 1; // �л�����ģʽ}
        } else if (x >= 50 && x <= 75 && y >= 17 && y <= 19) {
            //ע��
            //�����ļ���
            createDirectory(username);

            //���û������������й�ϣ�����password.txt
            writePasswordHashToFile(username, password);

            //����data�ļ�
            char filePath[MAX_PATH];
            sprintf(filePath, "%s\\data", username);
            FILE* file = fopen(filePath, "w");
            if (file == NULL) {
                perror("fopen");
                return 0;
            }

            MessageBox(0, "ע����ɣ�", "��ʾ", 0);

            goto signin;

        } else if (x >= 80 && x <= 84 && y == 20) {
            //��¼
            goto signin;
        }

        while (1) {
            int type = keyboard(&x, &y, &key, NULL);
            if (type == 1) {
                if (x >= 42 && x <= 73 && y == 12) {
                    setCursorPosition(48, 12);
                    inputMode = 0; // �л�����ģʽ
                } else if (x >= 42 && x <= 73 && y == 13) {
                    setCursorPosition(48, 13);
                    inputMode = 1; // �л�����ģʽ
                } else if (x >= 50 && x <= 75 && y >= 17 && y <= 19) {
                    //ע��

                    //��ͬĿ¼�´����ļ���
                    //�ļ�������Ϊ�û���
                    //�ļ�����password.txt�����������Ĺ�ϣֵ
                    //���û������������й�ϣ�����password.txt
                    //ע��ɹ������������

                    //�����ļ���
                    createDirectory(username);

                    //���û������������й�ϣ�����password.txt
                    writePasswordHashToFile(username, password);

                    //����data�ļ�
                    char filePath[MAX_PATH];
                    sprintf(filePath, "%s\\data", username);
                    FILE* file = fopen(filePath, "w");
                    if (file == NULL) {
                        perror("fopen");
                        return 0;
                    }

                    MessageBox(0, "ע����ɣ�", "��ʾ", 0);

                    goto signin;

                } else if (x >= 80 && x <= 84 && y == 20) {
                    //��¼
                    goto signin;
                }
                continue;
            }
            if (key == '\b') { // Backspace��
                // ɾ���û����������е����һ���ַ�
                if (inputMode == 0 && strlen(username) > 0) {
                    username[strlen(username) - 1] = '\0';
                } else if (inputMode == 1 && strlen(password) > 0) {
                    password[strlen(password) - 1] = '\0';
                }
            } else if (key >= ' ' && key <= '~') {
                // �������ַ�����
                if (inputMode == 0 && strlen(username) < INPUT_FIELD_WIDTH) {
                    int len = strlen(username);
                    username[len] = key;
                    username[len + 1] = '\0';
                } else if (inputMode == 1 && strlen(password) < INPUT_FIELD_WIDTH) {
                    int len = strlen(password);
                    password[len] = key;
                    password[len + 1] = '\0';
                }
            }
            registerUI(username, password);
        }
    }

    login: {
        //��¼����
        //ͬĿ¼�µ��ļ�������Ϊ�û���
        //�ļ�����password.txt�����������Ĺ�ϣֵ
        //���û������������й�ϣ����password.txt�е����ݽ��бȶ�
        //����ͬ���¼�ɹ��������¼ʧ��
        //��¼�ɹ������������
        //��¼ʧ������ʾ��¼ʧ�ܲ����ص�¼����

        //��ȡͬĿ¼�µ��ļ�����
        //���ļ�������������
        //�������е��������û�������û������бȶ�
        //����ͬ�������һ�������򷵻ص�¼����

        char usernames[MAX_USERS][MAX_PATH];
        int userCount = getUsernames(usernames);
        int usernameExists = verifyUsername(username, usernames, userCount);

        if (!usernameExists) {
            MessageBox(0, "�û��������ڣ�", "����", 0);
            goto signin;
        }

        int passwordCorrect = verifyPassword(username, password);
        if (!passwordCorrect) {
            MessageBox(0, "�������", "����", 0);
            goto signin;
        }

        //��¼�ɹ�
        mainUI(username);

        //��data�ļ��ж�ȡ����
        //���ݸ�ʽΪ��ѧ��,����,���ĳɼ�,��ѧ�ɼ�,Ӣ��ɼ�
        //��ѯ�����г���������
        //���Ҫ�������ݣ�ֱ����data�ļ�ĩβ���
        //���Ҫ�޸����ݣ��޸���һ�е����ݣ�ʵ�ַ�ʽΪ��дһ���У����ͣ����޸Ľ����У����г������е����ݣ�Ȼ�����û�ѡ��Ҫ�޸ĵ����ݣ�Ȼ�������û������µ����ݣ�Ȼ���ٽ���������д���ļ�������ʵ�����Ǹ�д��һ���У�
        //���Ҫɾ�����ݣ�����һ�е�������գ�ͬʱɾ���س���

        //��mainUI���Ѿ�ʵ��

        return 0;
    }
}