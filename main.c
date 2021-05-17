#define _XOPEN_SOURCE

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <wchar.h>
#include <time.h>
#include "math.h"

static char helpstr[] = "\n"
                        "Usage: queercat [-f flag_number][-h horizontal_speed] [-v vertical_speed] [--] [FILES...]\n"
                        "\n"
                        "Concatenate FILE(s), or standard input, to standard output.\n"
                        "With no FILE, or when FILE is -, read standard input.\n"
                        "\n"
                        "--flag <d>                , -f <d>: Choose colors to use:\n"
                        "                                    [rainbow: 0, trans: 1, NB: 2, lesbian: 3, \n"
                        "                                    gay: 4, pan: 5, bi: 6, genderfluid: 7]\n"
                        "                                    default is rainbow (0)\n"
                        "--horizontal-frequency <d>, -h <d>: Horizontal rainbow frequency (default: 0.23)\n"
                        "  --vertical-frequency <d>, -v <d>: Vertical rainbow frequency (default: 0.1)\n"
                        "                 --force-color, -F: Force color even when stdout is not a tty\n"
                        "             --no-force-locale, -l: Use encoding from system locale instead of\n"
                        "                                    assuming UTF-8\n"
                        "                      --random, -r: Random colors\n"
                        "                       --24bit, -b: Output in 24-bit \"true\" RGB mode (slower and\n"
                        "                                    not supported by all terminals)\n"
                        "                         --version: Print version and exit\n"
                        "                            --help: Show this message\n"
                        "\n"
                        "Examples:\n"
                        "  queercat f - g      Output f's contents, then stdin, then g's contents.\n"
                        "  queercat            Copy standard input to standard output.\n"
                        "  fortune | queercat  Display a rainbow cookie.\n"
                        "\n"
                        "Report queercat bugs to <https://github.com/elsa002/queercat/issues>\n"
                        "queercat home page: <https://github.com/elsa002/queercat/>\n"
                        "base for code: <https://github.com/jaseg/lolcat/>\n"
                        "Original idea: <https://github.com/busyloop/lolcat/>\n";

#define ARRAY_SIZE(foo) (sizeof(foo) / sizeof(foo[0]))
const unsigned char codes[] = { 39, 38, 44, 43, 49, 48, 84, 83, 119, 118, 154, 148, 184, 178, 214, 208, 209, 203, 204, 198, 199, 163, 164, 128, 129, 93, 99, 63, 69, 33 };
const unsigned char codes_tra[] = {117, 117,  225, 225,  255, 255,  225, 225,  117, 117};
const unsigned char codes_nb[] = {226, 226, 255, 255, 93, 93, 234, 234};
const unsigned char codes_les[] = {196, 208, 255, 170, 128};
const unsigned char codes_gay[] = {36, 49, 121, 255, 117, 105, 92};
const unsigned char codes_pan[] = {200, 200, 200,  227, 227, 227,  45, 45, 45};
const unsigned char codes_bi[] = {162, 162, 162,  129, 129, 27, 27, 27};
const unsigned char codes_gfl[] = {219, 219, 255, 255, 128, 128, 234, 234, 20, 20};

#define FLAG_5_0 (0)
#define FLAG_5_1 (0.4f * M_PI)
#define FLAG_5_2 (0.8f * M_PI)
#define FLAG_5_3 (1.2f * M_PI)
#define FLAG_5_4 (1.6f * M_PI)
#define FLAG_5_5 (2.0f * M_PI)

#define FLAG_4_0 (0)
#define FLAG_4_1 (0.5f * M_PI)
#define FLAG_4_2 (1.0f * M_PI)
#define FLAG_4_3 (1.5f * M_PI)
#define FLAG_4_4 (2.0f * M_PI)

#define FLAG_3_0 (0)
#define FLAG_3_1 (0.667f * M_PI)
#define FLAG_3_2 (1.334f * M_PI)
#define FLAG_3_3 (2.0f * M_PI)

#define TRA_BLU (0xa0e0ff)
#define TRA_PNK (0xffa0e0)
#define TRA_WHT (0xffffff)
#define TRA_FCT (4.0f)

#define NB__YLW (0xffff00)
#define NB__PUR (0xb000ff)
#define NB__WHT (0xffffff)
#define NB__BLK (0x000000)
#define NB__FCT (4.0f)

#define LES_RED (0xff0000)
#define LES_ORG (0xff993f)
#define LES_WHT (0xffffff)
#define LES_PNK (0xff8cbd)
#define LES_PUR (0xff4284)
#define LES_FCT (2.0f)

#define GAY_GR1 (0x00b685)
#define GAY_GR2 (0x6bffb6)
#define GAY_WHT (0xffffff)
#define GAY_BL1 (0x8be1ff)
#define GAY_BL2 (0x8e1ae1)
#define GAY_FCT (6.0f)

#define PAN_PNK (0xff3388)
#define PAN_YLW (0xffea00)
#define PAN_BLU (0x00dbff)
#define PAN_FCT (8.0f)

#define BI__PNK (0xff3b7b)
#define BI__PUR (0xd06bcc)
#define BI__BLU (0x3b72ff)
#define BI__FCT (4.0f)

#define GFL_PNK (0xffa0bc)
#define GFL_WHT (0xffffff)
#define GFL_PUR (0xc600e4)
#define GFL_BLK (0x000000)
#define GFL_BLU (0x4e3cbb)
#define GFL_FCT (2.0f)

typedef enum flag_type_e
{
    FLAG_TYPE_INVALID = -1,
    FLAG_TYPE_RAINBOW = 0,
    FLAG_TYPE_TRANS,
    FLAG_TYPE_NB,
    FLAG_TYPE_LESBIAN,
    FLAG_TYPE_GAY,
    FLAG_TYPE_PAN,
    FLAG_TYPE_BI,
    FLAG_TYPE_GENDERFLUID,
    FLAG_TYPE_END
} flag_type_t;

static void find_escape_sequences(wint_t c, int* state)
{
    if (c == '\033') { /* Escape sequence YAY */
        *state = 1;
    } else if (*state == 1) {
        if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'))
            *state = 2;
    } else {
        *state = 0;
    }
}

static void usage(void)
{
    wprintf(L"Usage: queercat [-h horizontal_speed] [-v vertical_speed] [--] [FILES...]\n");
    exit(1);
}

static void version(void)
{
    wprintf(L"queercat version 1.0, (c) 2021 elsa002\n");
    exit(0);
}

static wint_t helpstr_hack(FILE * _ignored)
{
    (void)_ignored;
    static size_t idx = 0;
    char c = helpstr[idx++];
    if (c)
        return c;
    idx = 0;
    return WEOF;
}

static void gen_color(uint32_t color1, uint32_t color2, float balance, float factor , uint8_t *red, uint8_t *green, uint8_t *blue)
{
    uint8_t red_1   = (color1 & 0xff0000) >> 16;
    uint8_t green_1 = (color1 & 0x00ff00) >>  8;
    uint8_t blue_1  = (color1 & 0x0000ff) >>  0;

    uint8_t red_2   = (color2 & 0xff0000) >> 16;
    uint8_t green_2 = (color2 & 0x00ff00) >>  8;
    uint8_t blue_2  = (color2 & 0x0000ff) >>  0;

    balance = pow(balance, factor);

    *red = lrintf(red_1 * balance + red_2 * (1.0f - balance));
    *green = lrintf(green_1 * balance + green_2 * (1.0f - balance));
    *blue = lrintf(blue_1 * balance + blue_2 * (1.0f - balance));
}

static void get_color(flag_type_t flag_type, float offset, float theta, uint8_t *red, uint8_t *green, uint8_t *blue)
{
    while (theta < 0) theta += 2.0f * (float)M_PI;
    while (theta >= 2.0f * (float)M_PI) theta -= 2.0f * (float)M_PI;
    switch (flag_type)
    {
    default:
    case FLAG_TYPE_RAINBOW:
        *red   = lrintf((offset + (1.0f - offset) * (0.5f + 0.5f * sin(theta + 0            ))) * 255.0f);
        *green = lrintf((offset + (1.0f - offset) * (0.5f + 0.5f * sin(theta + 2 * M_PI / 3 ))) * 255.0f);
        *blue  = lrintf((offset + (1.0f - offset) * (0.5f + 0.5f * sin(theta + 4 * M_PI / 3 ))) * 255.0f);
        break;

    case FLAG_TYPE_TRANS:
        if (FLAG_5_0 <= theta && FLAG_5_1 > theta) 
        { /* blue to pink */
            theta = 1 - ((theta - FLAG_5_0) / (FLAG_5_1 - FLAG_5_0));
            gen_color(TRA_BLU, TRA_PNK, theta, TRA_FCT, red, green, blue);
        } 
        else if (FLAG_5_1 <= theta && FLAG_5_2 > theta) 
        { /* pink to white */
            theta = 1 - ((theta - FLAG_5_1) / (FLAG_5_2 - FLAG_5_1));
            gen_color(TRA_PNK, TRA_WHT, theta, TRA_FCT, red, green, blue);
        } 
        else if (FLAG_5_2 <= theta && FLAG_5_3 > theta) 
        { /* white to pink*/
            theta = 1 - ((theta - FLAG_5_2) / (FLAG_5_3 - FLAG_5_2));
            gen_color(TRA_WHT, TRA_PNK, theta, TRA_FCT, red, green, blue);
        } 
        else if (FLAG_5_3 <= theta && FLAG_5_4 > theta) 
        { /* pink to blue */
            theta = 1 - ((theta - FLAG_5_3) / (FLAG_5_4 - FLAG_5_3));
            gen_color(TRA_PNK, TRA_BLU, theta, TRA_FCT, red, green, blue);
        } 
        else if (FLAG_5_4 <= theta && FLAG_5_5 > theta) 
        { /* blue back to blue */
            theta = 1 - ((theta - FLAG_5_4) / (FLAG_5_5 - FLAG_5_4));
            gen_color(TRA_BLU, TRA_BLU, theta, TRA_FCT, red, green, blue);
        }
        break;

    case FLAG_TYPE_NB:
        if (FLAG_4_0 <= theta && FLAG_4_1 > theta) 
        { /* yellow to white */
            theta = 1 - ((theta - FLAG_4_0) / (FLAG_4_1 - FLAG_4_0));
            gen_color(NB__YLW, NB__WHT, theta, NB__FCT, red, green, blue);
        } 
        else if (FLAG_4_1 <= theta && FLAG_4_2 > theta) 
        { /* white to putple */
            theta = 1 - ((theta - FLAG_4_1) / (FLAG_4_2 - FLAG_4_1));
            gen_color(NB__WHT, NB__PUR, theta, NB__FCT, red, green, blue);
        } 
        else if (FLAG_4_2 <= theta && FLAG_4_3 > theta) 
        { /* purple to balck */
            theta = 1 - ((theta - FLAG_4_2) / (FLAG_4_3 - FLAG_4_2));
            gen_color(NB__PUR, NB__BLK, theta, NB__FCT, red, green, blue);
        } 
        else if (FLAG_4_3 <= theta && FLAG_4_4 > theta) 
        { /* black back to yellow */
            theta = 1 - ((theta - FLAG_4_3) / (FLAG_4_4 - FLAG_4_3));
            gen_color(NB__BLK, NB__YLW, theta, NB__FCT, red, green, blue);
        } 
        break;

    case FLAG_TYPE_LESBIAN:
        if (FLAG_5_0 <= theta && FLAG_5_1 > theta) 
        { /* red to orange */
            theta = 1 - ((theta - FLAG_5_0) / (FLAG_5_1 - FLAG_5_0));
            gen_color(LES_RED, LES_ORG, theta, LES_FCT, red, green, blue);
        } 
        else if (FLAG_5_1 <= theta && FLAG_5_2 > theta) 
        { /* orange to white */
            theta = 1 - ((theta - FLAG_5_1) / (FLAG_5_2 - FLAG_5_1));
            gen_color(LES_ORG, LES_WHT, theta, LES_FCT, red, green, blue);
        } 
        else if (FLAG_5_2 <= theta && FLAG_5_3 > theta) 
        { /* white to pink */
            theta = 1 - ((theta - FLAG_5_2) / (FLAG_5_3 - FLAG_5_2));
            gen_color(LES_WHT, LES_PNK, theta, LES_FCT, red, green, blue);
        } 
        else if (FLAG_5_3 <= theta && FLAG_5_4 > theta) 
        { /* pink to purple */
            theta = 1 - ((theta - FLAG_5_3) / (FLAG_5_4 - FLAG_5_3));
            gen_color(LES_PNK, LES_PUR, theta, LES_FCT, red, green, blue);
        } 
        else if (FLAG_5_4 <= theta && FLAG_5_5 > theta) 
        { /* purple back to red */
            theta = 1 - ((theta - FLAG_5_4) / (FLAG_5_5 - FLAG_5_4));
            gen_color(LES_PUR, LES_RED, theta, LES_FCT, red, green, blue);
        }
        break;

    case FLAG_TYPE_GAY:
        if (FLAG_5_0 <= theta && FLAG_5_1 > theta) 
        { /* green1 to green2 */
            theta = 1 - ((theta - FLAG_5_0) / (FLAG_5_1 - FLAG_5_0));
            gen_color(GAY_GR1, GAY_GR2, theta, GAY_FCT, red, green, blue);
        } 
        else if (FLAG_5_1 <= theta && FLAG_5_2 > theta) 
        { /* green2 to white */
            theta = 1 - ((theta - FLAG_5_1) / (FLAG_5_2 - FLAG_5_1));
            gen_color(GAY_GR2, GAY_WHT, theta, GAY_FCT, red, green, blue);
        } 
        else if (FLAG_5_2 <= theta && FLAG_5_3 > theta) 
        { /* white to blue1 */
            theta = 1 - ((theta - FLAG_5_2) / (FLAG_5_3 - FLAG_5_2));
            gen_color(GAY_WHT, GAY_BL1, theta, GAY_FCT, red, green, blue);
        } 
        else if (FLAG_5_3 <= theta && FLAG_5_4 > theta) 
        { /* blue1 to blue2 */
            theta = 1 - ((theta - FLAG_5_3) / (FLAG_5_4 - FLAG_5_3));
            gen_color(GAY_BL1, GAY_BL2, theta, GAY_FCT, red, green, blue);
        } 
        else if (FLAG_5_4 <= theta && FLAG_5_5 > theta) 
        { /* blue2 back to green1 */
            theta = 1 - ((theta - FLAG_5_4) / (FLAG_5_5 - FLAG_5_4));
            gen_color(GAY_BL2, GAY_GR1, theta, GAY_FCT, red, green, blue);
        }
        break;

    case FLAG_TYPE_PAN:
        if (FLAG_3_0 <= theta && FLAG_3_1 > theta) 
        { /* pink to yellow */
            theta = 1 - ((theta - FLAG_3_0) / (FLAG_3_1 - FLAG_3_0));
            gen_color(PAN_PNK, PAN_YLW, theta, PAN_FCT, red, green, blue);
        } 
        else if (FLAG_3_1 <= theta && FLAG_3_2 > theta) 
        { /* yellow to blue */
            theta = 1 - ((theta - FLAG_3_1) / (FLAG_3_2 - FLAG_3_1));
            gen_color(PAN_YLW, PAN_BLU, theta, PAN_FCT, red, green, blue);
        } 
        else if (FLAG_3_2 <= theta && FLAG_3_3 > theta) 
        { /* blue back to pink */
            theta = 1 - ((theta - FLAG_3_2) / (FLAG_3_3 - FLAG_3_2));
            gen_color(PAN_BLU, PAN_PNK, theta, PAN_FCT, red, green, blue);
        } 
        break;

    case FLAG_TYPE_BI:
        if (FLAG_5_0 <= theta && FLAG_5_1 > theta) 
        { /* pink */
            theta = 1 - ((theta - FLAG_5_0) / (FLAG_5_1 - FLAG_5_0));
            gen_color(BI__PNK, BI__PNK, theta, BI__FCT, red, green, blue);
        } 
        else if (FLAG_5_1 <= theta && FLAG_5_2 > theta) 
        { /* pink to purple */
            theta = 1 - ((theta - FLAG_5_1) / (FLAG_5_2 - FLAG_5_1));
            gen_color(BI__PNK, BI__PUR, theta, BI__FCT, red, green, blue);
        } 
        else if (FLAG_5_2 <= theta && FLAG_5_3 > theta) 
        { /* purple to blue */
            theta = 1 - ((theta - FLAG_5_2) / (FLAG_5_3 - FLAG_5_2));
            gen_color(BI__PUR, BI__BLU, theta, BI__FCT, red, green, blue);
        } 
        else if (FLAG_5_3 <= theta && FLAG_5_4 > theta) 
        { /* blue */
            theta = 1 - ((theta - FLAG_5_3) / (FLAG_5_4 - FLAG_5_3));
            gen_color(BI__BLU, BI__BLU, theta, BI__FCT, red, green, blue);
        } 
        else if (FLAG_5_4 <= theta && FLAG_5_5 > theta) 
        { /* blue back to pink */
            theta = 1 - ((theta - FLAG_5_4) / (FLAG_5_5 - FLAG_5_4));
            gen_color(BI__BLU, BI__PNK, theta, BI__FCT, red, green, blue);
        } 
        break;
    case FLAG_TYPE_GENDERFLUID:
        if (FLAG_5_0 <= theta && FLAG_5_1 > theta) 
        { /* pink to white */
            theta = 1 - ((theta - FLAG_5_0) / (FLAG_5_1 - FLAG_5_0));
            gen_color(GFL_PNK, GFL_WHT, theta, GFL_FCT, red, green, blue);
        } 
        else if (FLAG_5_1 <= theta && FLAG_5_2 > theta) 
        { /* white to purple */
            theta = 1 - ((theta - FLAG_5_1) / (FLAG_5_2 - FLAG_5_1));
            gen_color(GFL_WHT, GFL_PUR, theta, GFL_FCT, red, green, blue);
        } 
        else if (FLAG_5_2 <= theta && FLAG_5_3 > theta) 
        { /* purple to black */
            theta = 1 - ((theta - FLAG_5_2) / (FLAG_5_3 - FLAG_5_2));
            gen_color(GFL_PUR, GFL_BLK, theta, GFL_FCT, red, green, blue);
        } 
        else if (FLAG_5_3 <= theta && FLAG_5_4 > theta) 
        { /* black to blue */
            theta = 1 - ((theta - FLAG_5_3) / (FLAG_5_4 - FLAG_5_3));
            gen_color(GFL_BLK, GFL_BLU, theta, GFL_FCT, red, green, blue);
        } 
        else if (FLAG_5_4 <= theta && FLAG_5_5 > theta) 
        { /* blue back to pink */
            theta = 1 - ((theta - FLAG_5_4) / (FLAG_5_5 - FLAG_5_4));
            gen_color(GFL_BLU, GFL_PNK, theta, GFL_FCT, red, green, blue);
        } 
        break;
    }
}

static size_t get_codes_len(flag_type_t flag_type)
{
    switch (flag_type)
    {
    default:
    case FLAG_TYPE_RAINBOW:
        return ARRAY_SIZE(codes);
        break;

    case FLAG_TYPE_TRANS:
        return ARRAY_SIZE(codes_tra);
        break;

    case FLAG_TYPE_NB:
        return ARRAY_SIZE(codes_nb);
        break;

    case FLAG_TYPE_LESBIAN:
        return ARRAY_SIZE(codes_les);
        break;

    case FLAG_TYPE_GAY:
        return ARRAY_SIZE(codes_gay);
        break;

    case FLAG_TYPE_PAN:
        return ARRAY_SIZE(codes_pan);
        break;

    case FLAG_TYPE_BI:
        return ARRAY_SIZE(codes_bi);
        break;
    
    case FLAG_TYPE_GENDERFLUID:
        return ARRAY_SIZE(codes_gfl);
        break;
    }
}

static const char* get_codes(flag_type_t flag_type)
{
    switch (flag_type)
    {
    default:
    case FLAG_TYPE_RAINBOW:
        return codes;
        break;

    case FLAG_TYPE_TRANS:
        return codes_tra;
        break;

    case FLAG_TYPE_NB:
        return codes_nb;
        break;

    case FLAG_TYPE_LESBIAN:
        return codes_les;
        break;

    case FLAG_TYPE_GAY:
        return codes_gay;
        break;

    case FLAG_TYPE_PAN:
        return codes_pan;
        break;

    case FLAG_TYPE_BI:
        return codes_bi;
        break;
    
    case FLAG_TYPE_GENDERFLUID:
        return codes_gfl;
        break;
    }
}

int main(int argc, char** argv)
{
    char* default_argv[] = { "-" };
    int cc = -1, i, l = 0;
    wint_t c;
    int colors    = isatty(STDOUT_FILENO);
    int force_locale = 1;
    int random = 0;
    int rgb = 0;
    double freq_h = 0.23, freq_v = 0.1;
    flag_type_t flag_type = FLAG_TYPE_RAINBOW;

    struct timeval tv;
    gettimeofday(&tv, NULL);
    double offx = (tv.tv_sec % 300) / 300.0;

    for (i = 1; i < argc; i++) {
        char* endptr;
        if (!strcmp(argv[i], "-f") || !strcmp(argv[i], "--flag")) {
            if ((++i) < argc) {
                flag_type = (flag_type_t)strtod(argv[i], &endptr);
                if (*endptr)
                    usage();
            } else {
                usage();
            }
        } else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--horizontal-frequency")) {
            if ((++i) < argc) {
                freq_h = strtod(argv[i], &endptr);
                if (*endptr)
                    usage();
            } else {
                usage();
            }
        } else if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--vertical-frequency")) {
            if ((++i) < argc) {
                freq_v = strtod(argv[i], &endptr);
                if (*endptr)
                    usage();
            } else {
                usage();
            }
        } else if (!strcmp(argv[i], "-F") || !strcmp(argv[i], "--force-color")) {
            colors = 1;
        } else if (!strcmp(argv[i], "-l") || !strcmp(argv[i], "--no-force-locale")) {
            force_locale = 0;
        } else if (!strcmp(argv[i], "-r") || !strcmp(argv[i], "--random")) {
            random = 1;
        } else if (!strcmp(argv[i], "-b") || !strcmp(argv[i], "--24bit")) {
            rgb = 1;
        } else if (!strcmp(argv[i], "--version")) {
            version();
        } else {
            if (!strcmp(argv[i], "--"))
                i++;
            break;
        }
    }

    int rand_offset = 0;
    if (random) {
        srand(time(NULL));
        rand_offset = rand();
    }
    char** inputs = argv + i;
    char** inputs_end = argv + argc;
    if (inputs == inputs_end) {
        inputs = default_argv;
        inputs_end = inputs + 1;
    }

    char* env_lang = getenv("LANG");
    if (force_locale && env_lang && !strstr(env_lang, "UTF-8")) {
        if (!setlocale(LC_ALL, "C.UTF-8")) { /* C.UTF-8 may not be available on all platforms */
            setlocale(LC_ALL, ""); /* Let's hope for the best */
        }
    } else {
        setlocale(LC_ALL, "");
    }

    i = 0;
    for (char** filename = inputs; filename < inputs_end; filename++) {
        wint_t (*this_file_read_wchar)(FILE*); /* Used for --help because fmemopen is universally broken when used with fgetwc */
        FILE* f;
        int escape_state = 0;

        if (!strcmp(*filename, "--help")) {
            this_file_read_wchar = &helpstr_hack;
            f = 0;

        } else if (!strcmp(*filename, "-")) {
            this_file_read_wchar = &fgetwc;
            f = stdin;

        } else {
            this_file_read_wchar = &fgetwc;
            f = fopen(*filename, "r");
            if (!f) {
                fwprintf(stderr, L"Cannot open input file \"%s\": %s\n", *filename, strerror(errno));
                return 2;
            }
        }

        while ((c = this_file_read_wchar(f)) != WEOF) {
            if (colors) {
                find_escape_sequences(c, &escape_state);

                if (!escape_state) {
                    if (c == '\n') {
                        l++;
                        i = 0;

                    } else {
                        if (rgb) {
                            i += wcwidth(c);
                            float theta = i * freq_h / 5.0f + l * freq_v + (offx + 2.0f * rand_offset / RAND_MAX) * M_PI;
                            float offset = 0.1;

                            uint8_t red = 0;
                            uint8_t green = 0;
                            uint8_t blue = 0;
                            get_color(flag_type, offset, theta, &red, &green, &blue);
                            wprintf(L"\033[38;2;%d;%d;%dm", red, green, blue);

                        } else {
                            int ncc = offx * get_codes_len(flag_type) + (int)((i += wcwidth(c)) * freq_h + l * freq_v);
                            if (cc != ncc)
                                wprintf(L"\033[38;5;%hhum", get_codes(flag_type)[(rand_offset + (cc = ncc)) % get_codes_len(flag_type)]);
                        }
                    }
                }
            }

            putwchar(c);

            if (escape_state == 2) /* implies "colors" */
                wprintf(L"\033[38;5;%hhum", codes[(rand_offset + cc) % ARRAY_SIZE(codes)]);
        }

        if (colors)
            wprintf(L"\033[0m");

        cc = -1;

        if (f) {
            if (ferror(f)) {
                fwprintf(stderr, L"Error reading input file \"%s\": %s\n", *filename, strerror(errno));
                fclose(f);
                return 2;
            }

            if (fclose(f)) {
                fwprintf(stderr, L"Error closing input file \"%s\": %s\n", *filename, strerror(errno));
                return 2;
            }
        }
    }
}