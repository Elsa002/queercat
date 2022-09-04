#define _XOPEN_SOURCE

/* *** Includes ******************************************************/
#include <stdbool.h>
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


/* *** Common ********************************************************/
/* Constants */
#define NEWLINE '\n'
#define ESCAPE_CHAR '\033'

/* Types */
typedef enum escape_state_e {
    ESCAPE_STATE_OUT = 0,
    ESCAPE_STATE_IN,
    ESCAPE_STATE_LAST
} escape_state_t;

/* Macros */
#define UNUSED(var) ((void)(var))
#define NEXT_CYCLIC_ELEMENT(array, index, array_size) \
    (((index) + 1 == (array_size)) ? (array)[0] : (array)[((index) + 1)] )
#define IS_LETTER(c) (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'))


/* *** Constants *****************************************************/

#define MAX_FLAG_STRIPES (6)
#define MAX_ANSII_CODES_PER_STRIPE (5)
#define MAX_ANSII_CODES_COUNT (MAX_FLAG_STRIPES * MAX_ANSII_CODES_PER_STRIPE)
#define MAX_FLAG_NAME_LENGTH (64)


/* *** Types *********************************************************/
/* Colors. */
typedef uint32_t hex_color_t;
typedef unsigned char ansii_code_t;
typedef struct color_s {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} color_t;

/* Color type patterns. */
typedef enum color_type_e {
    COLOR_TYPE_INVALID = -1,
    COLOR_TYPE_ANSII = 0,
    COLOR_TYPE_24_BIT,
    COLOR_TYPE_COUNT
} color_type_t;
typedef struct ansii_pattern_s {
    const unsigned int codes_count;
    const unsigned char ansii_codes[MAX_ANSII_CODES_COUNT];
} ansii_pattern_t;
typedef struct color_pattern_s {
    const uint8_t stripes_count;
    const uint32_t stripes_colors[MAX_FLAG_STRIPES];
    const float factor;
} color_pattern_t;

/* Get color function. */
typedef void(get_color_f)(const color_pattern_t *color_pattern, float theta, color_t *color);

/* Pattern. */
typedef struct pattern_s {
    const char name[MAX_FLAG_NAME_LENGTH];
    const ansii_pattern_t ansii_pattern;
    const color_pattern_t color_pattern;
    get_color_f *get_color;
} pattern_t;

/* *** A Single Global ***********************************************/
char *helpstr;

/* *** Pattern Functions *********************************************/
get_color_f get_color_rainbow;
get_color_f get_color_stripes;


/* *** Flags *********************************************************/
const pattern_t flags[] = {
    {
        .name = "rainbow",
        .ansii_pattern = {
            .codes_count = 30,
            .ansii_codes = { 39, 38, 44, 43, 49, 48, 84, 83, 119, 118, 154, 148, 184, 178,
                214, 208, 209, 203, 204, 198, 199, 163, 164, 128, 129, 93, 99, 63, 69, 33 }
        },
        .color_pattern = { 0 },
        .get_color = get_color_rainbow
    },

    {
        .name = "transgender",
        .ansii_pattern = {
            .codes_count = 10,
            .ansii_codes = {117, 117,  225, 225,  255, 255,  225, 225,  117, 117}
        },
        .color_pattern = {
            .stripes_count = 5,
            .stripes_colors = {
                0x55cdfc, /* #55cdfd - Blue */
                0xf7a8b8, /* #f7a8b8 - Pink */
                0xffffff, /* #ffffff - White */
                0xf7a8b8, /* #f7a8b8 - Pink */
                0x55cdfc  /* #55cdfc - Blue */
            },
            .factor = 4.0f
        },
        .get_color = get_color_stripes
    },

    {
        .name = "nonbinary",
        .ansii_pattern = {
            .codes_count = 8,
            .ansii_codes = {226, 226, 255, 255, 93, 93, 234, 234}
        },
        .color_pattern = {
            .stripes_count = 4,
            .stripes_colors = {
                0xffff00, /* #ffff00 - Yellow */
                0xb000ff, /* #b000ff - Purple */
                0xffffff, /* #ffffff - White */
                0x000000  /* #000000 - Black */
            },
            .factor = 4.0f
        },
        .get_color = get_color_stripes
    },

    {
        .name = "lesbian",
        .ansii_pattern = {
            .codes_count = 5,
            .ansii_codes = {196, 208, 255, 170, 128}
        },
        .color_pattern = {
            .stripes_count = 5,
            .stripes_colors = {
                0xff0000, /* #ff0000 - Red */
                0xff993f, /* #ff993f - Orange */
                0xffffff, /* #ffffff - White */
                0xff8cbd, /* #ff8cbd - Pink */
                0xff4284  /* #ff4284 - Purple */
            },
            .factor = 2.0f
        },
        .get_color = get_color_stripes
    },

    {
        .name = "gay",
        .ansii_pattern = {
            .codes_count = 7,
            .ansii_codes = {36, 49, 121, 255, 117, 105, 92}
        },
        .color_pattern = {
            .stripes_count = 5,
            .stripes_colors = {
                0x00b685, /* #00b685 - Teal */
                0x6bffb6, /* #6bffb6 - Green */
                0xffffff, /* #ffffff - White */
                0x8be1ff, /* #8be1ff - Blue */
                0x8e1ae1  /* #8e1ae1 - Purple */
            },
            .factor = 6.0f
        },
        .get_color = get_color_stripes
    },

    {
        .name = "pansexual",
        .ansii_pattern = {
            .codes_count = 9,
            .ansii_codes = {200, 200, 200,  227, 227, 227,  45, 45, 45}
        },
        .color_pattern = {
            .stripes_count = 3,
            .stripes_colors = {
                0xff3388, /* #ff3388 - Pink */
                0xffea00, /* #ffea00 - Yellow */
                0x00dbff  /* #00dbff - Cyan */
            },
            .factor = 8.0f
        },
        .get_color = get_color_stripes
    },

    {
        .name = "bisexual",
        .ansii_pattern = {
            .codes_count = 8,
            .ansii_codes = {162, 162, 162,  129, 129, 27, 27, 27}
        },
        .color_pattern = {
            .stripes_count = 5,
            .stripes_colors = {
                0xff3b7b, /* #ff3b7b - Pink */
                0xff3b7b, /* #ff3b7b - Pink */
                0xd06bcc, /* #d06bcc - Purple */
                0x3b72ff, /* #3b72ff - Blue */
                0x3b72ff  /* #3b72ff - Blue */
            },
            .factor = 4.0f
        },
        .get_color = get_color_stripes
    },

    {
        .name = "gender_fluid",
        .ansii_pattern = {
            .codes_count = 10,
            .ansii_codes = {219, 219, 255, 255, 128, 128, 234, 234, 20, 20}
        },
        .color_pattern = {
            .stripes_count = 5,
            .stripes_colors = {
                0xffa0bc, /* #ffa0bc - Pink */
                0xffffff, /* #ffffff - White */
                0xc600e4, /* #c600e4 - Purple */
                0x000000, /* #000000 - Black */
                0x4e3cbb  /* #4e3cbb - Blue */
            },
            .factor = 2.0f
        },
        .get_color = get_color_stripes
    },

    {
        .name = "asexual",
        .ansii_pattern = {
            .codes_count = 8,
            .ansii_codes = {233, 233, 247, 247, 255, 255, 5, 5}
        },
        .color_pattern = {
            .stripes_count = 4,
            .stripes_colors = {
                0x000000, /* #000000 - Black */
                0xa3a3a3, /* #a3a3a3 - Gray */
                0xffffff, /* #ffffff - White */
                0x800080  /* #800080 - Purple */
            },
            .factor = 4.0f
        },
        .get_color = get_color_stripes
    },

    {
        .name = "unlabeled",
        .ansii_pattern = {
            .codes_count = 8,
            .ansii_codes = {194, 194, 255, 255, 195, 195, 223, 223}
        },
        .color_pattern = {
            .stripes_count = 4,
            .stripes_colors = {
                0xe6f9e3, /* #e6f9e3 - Green */
                0xfdfdfb, /* #fdfdfb - White */
                0xdeeff9, /* #deeff9 - Blue */
                0xfae1c2  /* #fae1c2 - Orange */
            },
            .factor = 4.0f
        },
        .get_color = get_color_stripes
    },

    {
        .name = "aromantic",
        .ansii_pattern = {
            .codes_count = 10,
            .ansii_codes = {
                34, 34,
                120, 120,
                255, 255,
                247, 247,
                233, 233
            }
        },
        .color_pattern = {
            .stripes_count = 5,
            .stripes_colors = {
                0x3da542, /* #3da542 - Green        */
                0xa8d379, /* #a8d379 - Light green  */
                0xffffff, /* #ffffff - White        */
                0xa9a9a9, /* #a9a9a9 - Grey         */
                0x000000  /* #000000 - Black        */
            },
            .factor = 1.0f
        },
        .get_color = get_color_stripes
    },

    {
        .name = "aroace",
        .ansii_pattern = {
            .codes_count = 10,
            .ansii_codes = {
                208, 208,
                220, 220,
                255, 255,
                75, 75,
                62, 62
            },
        },
        .color_pattern = {
            .stripes_count = 5,
            .stripes_colors = {
                0xe28d00, /* #e28d00 - Orange     */
                0xeccd00, /* #eccd00 - Yellow     */
                0xffffff, /* #ffffff - White      */
                0x62afdd, /* #62afdd - Light blue */
                0x203756  /* #203756 - Blue       */
            },
            .factor = 1.0f
        },
        .get_color = get_color_stripes
    },
    /* Add new flags above this line. */
};

const int FLAG_COUNT = sizeof(flags)/sizeof(flags[0]);

/* *** Functions Declarations ****************************************/
/* Info */
static void usage(void);
static void version(void);

/* Helpers */
static void build_helpstr();
static void find_escape_sequences(wint_t current_char, escape_state_t *state);
static wint_t helpstr_hack(FILE * _ignored);

/* Colors handling */
static void mix_colors(uint32_t color1, uint32_t color2, float balance, float factor, color_t *output_color);
static void print_color(const pattern_t *pattern, color_type_t color_type, int char_index, int line_index, double freq_h, double freq_v, double offx, int rand_offset, int cc);

/* *** Functions *****************************************************/
static void usage(void)
{
    wprintf(L"Usage: queercat [-h horizontal_speed] [-v vertical_speed] [--] [FILES...]\n");
    exit(1);
}

static void version(void)
{
    wprintf(L"queercat version 2.0, (c) 2022 elsa002\n");
    exit(0);
}

static void build_helpstr() {
    if(helpstr != NULL)
        return;

    static char helpstr_head[] = "\n"
        "Usage: queercat [-f flag_number][-h horizontal_speed] [-v vertical_speed] [--] [FILES...]\n"
        "\n"
        "Concatenate FILE(s), or standard input, to standard output.\n"
        "With no FILE, or when FILE is -, read standard input.\n"
        "\n"
        "                --flag <d>, -f <d>: Choose colors to use (default: 0 (rainbow)):\n";

    static char helpstr_indent[] = "                                      ";

    static char helpstr_tail[] =
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

    /* old version of what this generates, for reference:
     * "                                    [rainbow: 0, trans: 1, NB: 2, lesbian: 3,\n"
     * "                                    gay: 4, pan: 5, bi: 6, genderfluid: 7, asexual: 8,\n"
     * "                                    unlabeled: 9, aromantic: 10, aroace: 11]\n"
     * would be nice to have the dynamic word-wrap back, but that's
     * more clever than I currently feel like trying to be
     */
    const int line_max_len = strlen(helpstr_indent) + MAX_FLAG_NAME_LENGTH + strlen(": 000\n") ;
    char lines[FLAG_COUNT][line_max_len];
    size_t lines_total_len = 0;

    for(int i = 0; i < FLAG_COUNT; ++i) {
        lines_total_len += snprintf(lines[i], line_max_len, "%s%s: %d\n", helpstr_indent, flags[i].name, i);
    }

    size_t helpstr_len = strlen(helpstr_head) + lines_total_len + strlen(helpstr_tail);

    char *out = malloc(helpstr_len);
    char *out_pos = out;

    out_pos = mempcpy(out, helpstr_head, strlen(helpstr_head));

    for(int i = 0; i < FLAG_COUNT; ++i) {
        char* this_line = lines[i];
        out_pos = mempcpy(out_pos, this_line, strlen(this_line));
    }

    memcpy(out_pos, helpstr_tail, strlen(helpstr_tail));

    // TODO maybe free() this at some point? should be cleaned up at program exit, but
    // it feels a bit gross to rely on that... could refactor helpstr_hack to not rely
    // on helpstr being global maybe?
    helpstr = out;
}

static void find_escape_sequences(wint_t current_char, escape_state_t *state)
{
    if (current_char == '\033') {
        *state = ESCAPE_STATE_IN;
    } else if (*state == ESCAPE_STATE_IN) {
        *state = IS_LETTER(current_char) ? ESCAPE_STATE_LAST : ESCAPE_STATE_IN;
    } else {
        *state = ESCAPE_STATE_OUT;
    }
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

static void mix_colors(uint32_t color1, uint32_t color2, float balance, float factor, color_t *output_color)
{
    uint8_t red_1   = (color1 & 0xff0000) >> 16;
    uint8_t green_1 = (color1 & 0x00ff00) >>  8;
    uint8_t blue_1  = (color1 & 0x0000ff) >>  0;

    uint8_t red_2   = (color2 & 0xff0000) >> 16;
    uint8_t green_2 = (color2 & 0x00ff00) >>  8;
    uint8_t blue_2  = (color2 & 0x0000ff) >>  0;

    balance = pow(balance, factor);

    output_color->red = lrintf(red_1 * balance + red_2 * (1.0f - balance));
    output_color->green = lrintf(green_1 * balance + green_2 * (1.0f - balance));
    output_color->blue = lrintf(blue_1 * balance + blue_2 * (1.0f - balance));
}

void get_color_rainbow (const color_pattern_t *color_pattern, float theta, color_t *color)
{
    /* Unused variables. */
    UNUSED(color_pattern);

    /* Get theta in range. */
    while (theta < 0) theta += 2.0f * (float)M_PI;
    while (theta >= 2.0f * (float)M_PI) theta -= 2.0f * (float)M_PI;

    /* Generate the color. */
    color->red   = lrintf((1.0f * (0.5f + 0.5f * sin(theta + 0            ))) * 255.0f);
    color->green = lrintf((1.0f * (0.5f + 0.5f * sin(theta + 2 * M_PI / 3 ))) * 255.0f);
    color->blue  = lrintf((1.0f * (0.5f + 0.5f * sin(theta + 4 * M_PI / 3 ))) * 255.0f);
}

void get_color_stripes (const color_pattern_t *color_pattern, float theta, color_t *color)
{
    /* Get theta in range. */
    while (theta < 0) theta += 2.0f * (float)M_PI;
    while (theta >= 2.0f * (float)M_PI) theta -= 2.0f * (float)M_PI;

    /* Find the stripe based on theta and generate the color. */
    for (int i = 0; i < color_pattern->stripes_count; i++) {
        float stripe_size = (2.0f * M_PI) / color_pattern->stripes_count;
        float min_theta = i * stripe_size;
        float max_theta = (i + 1) * stripe_size;

        if (min_theta <= theta && max_theta > theta) {
            float balance = 1 - ((theta - min_theta) / stripe_size);
            mix_colors(
                    color_pattern->stripes_colors[i],
                    NEXT_CYCLIC_ELEMENT(color_pattern->stripes_colors, i, color_pattern->stripes_count),
                    balance,
                    color_pattern->factor,
                    color);
            return;
        }
    }
}

static void print_color(const pattern_t *pattern, color_type_t color_type, int char_index, int line_index, double freq_h, double freq_v, double offx, int rand_offset, int cc)
{
    float theta;
    color_t color = { 0 };

    int ncc;

    switch (color_type) {
        case COLOR_TYPE_24_BIT:
            theta = char_index * freq_h / 5.0f + line_index * freq_v + (offx + 2.0f * rand_offset / (float)RAND_MAX) * M_PI;

            pattern->get_color(&pattern->color_pattern, theta, &color);
            wprintf(L"\033[38;2;%d;%d;%dm", color.red, color.green, color.blue);
            break;

        case COLOR_TYPE_ANSII:
            ncc = offx * pattern->ansii_pattern.codes_count + (int)(char_index * freq_h + line_index * freq_v);
            if (cc != ncc)
                wprintf(L"\033[38;5;%hhum", pattern->ansii_pattern.ansii_codes[(rand_offset + (cc = ncc)) % pattern->ansii_pattern.codes_count]);
            break;

        default:
            exit(1);
    }
}

int main(int argc, char** argv)
{
    char* default_argv[] = { "-" };
    int cc = -1;
    int i = 0;
    int char_index = 0;
    int line_index = 0;
    wint_t current_char = '\0';
    bool print_colors = isatty(STDOUT_FILENO);
    bool force_locale = true;
    bool random = false;
    color_type_t color_type = COLOR_TYPE_ANSII;
    double freq_h = 0.23;
    double freq_v = 0.1;
    int flag_type = 0; // default to rainbow
    const pattern_t *pattern;

    struct timeval tv;
    gettimeofday(&tv, NULL);
    double offx = (tv.tv_sec % 300) / 300.0;

    build_helpstr();

    /* Handle flags. */
    for (i = 1; i < argc; i++) {
        char* endptr;
        if (!strcmp(argv[i], "-f") || !strcmp(argv[i], "--flag")) {
            if ((++i) < argc) {
                // TODO? use strtoul instead of strtod?
                flag_type = (int)strtod(argv[i], &endptr);
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
            print_colors = true;
        } else if (!strcmp(argv[i], "-l") || !strcmp(argv[i], "--no-force-locale")) {
            force_locale = false;
        } else if (!strcmp(argv[i], "-r") || !strcmp(argv[i], "--random")) {
            random = true;
        } else if (!strcmp(argv[i], "-b") || !strcmp(argv[i], "--24bit")) {
            color_type = COLOR_TYPE_24_BIT;
        } else if (!strcmp(argv[i], "--version")) {
            version();
        } else {
            if (!strcmp(argv[i], "--"))
                i++;
            break;
        }
    }

    /* Get pattern. */
    if (flag_type < 0 || flag_type >= FLAG_COUNT) {
        fprintf(stderr, "Invalid flag: %d\n", flag_type);
        exit(1);
    }

    pattern = &flags[flag_type];

    /* Handle randomness. */
    int rand_offset = 0;
    if (random) {
        srand(time(NULL));
        rand_offset = rand();
    }

    /* Get inputs. */
    char** inputs = argv + i;
    char** inputs_end = argv + argc;
    if (inputs == inputs_end) {
        inputs = default_argv;
        inputs_end = inputs + 1;
    }

    /* Handle locale. */
    char* env_lang = getenv("LANG");
    if (force_locale && env_lang && !strstr(env_lang, "UTF-8")) {
        if (!setlocale(LC_ALL, "C.UTF-8")) { /* C.UTF-8 may not be available on all platforms */
            setlocale(LC_ALL, ""); /* Let's hope for the best */
        }
    } else {
        setlocale(LC_ALL, "");
    }

    /* For file in inputs. */
    for (char** filename = inputs; filename < inputs_end; filename++) {
        wint_t (*this_file_read_wchar)(FILE*); /* Used for --help because fmemopen is universally broken when used with fgetwc */
        FILE* f;
        escape_state_t escape_state = ESCAPE_STATE_OUT;

        /* Handle "--help", "-" (STDIN) and file names. */
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

        /* While there are chars to read. */
        while ((current_char = this_file_read_wchar(f)) != WEOF) {

            /* If set to print colors, handle the colors. */
            if (print_colors) {

                /* Skip escape sequences. */
                find_escape_sequences(current_char, &escape_state);
                if (escape_state == ESCAPE_STATE_OUT) {

                    /* Handle newlines. */
                    if (current_char == '\n') {
                        line_index++;
                        char_index = 0;
                    } else {
                        char_index += wcwidth(current_char);
                        print_color(pattern, color_type, char_index, line_index, freq_h, freq_v, offx, rand_offset, cc);
                    }
                }
            }

            /* Print the char. */
            putwchar(current_char);

            if (escape_state == ESCAPE_STATE_LAST) {  /* implies "print_colors" */
                print_color(pattern, color_type, char_index, line_index, freq_h, freq_v, offx, rand_offset, cc);
            }
        }

        if (print_colors)
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
