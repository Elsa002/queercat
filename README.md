# queercat
a version of lolcat with some lgbtq+ pride flags options

## Usage
`$ queercat [-f flag_number][-h horizontal_speed] [-v vertical_speed] [--] [FILES...]`  

```
Concatenate FILE(s), or standard input, to standard output.  
With no FILE, or when FILE is -, read standard input.

--flag <d>                , -f <d>: Choose colors to use: [rainbow: 0, trans: 1, NB: 2, lesbian: 3, gay: 4, pan: 5, bi: 6, genderfluid: 7, asexual: 8, unlabeled: 9, aromantic : 10, aroace: 11] default is rainbow(0)
--horizontal-frequency <d>, -h <d>: Horizontal rainbow frequency (default: 0.23)  
  --vertical-frequency <d>, -v <d>: Vertical rainbow frequency (default: 0.1)  
                 --force-color, -F: Force color even when stdout is not a tty  
             --no-force-locale, -l: Use encoding from system locale instead of assuming UTF-8  
                    --random, -r: Random colors  
                       --24bit, -b: Output in 24-bit "true" RGB mode (slower and
                                    not supported by all terminals)  
                         --version: Print version and exit  
                            --help: Show this message
```

## Adding a flag
### Step 1: Define the pattern
To add a flag, first create an instance of `pattern_t` for it in the `main.c` file.  
under the section `/* *** Flags *********************************************************/`

Example:
``` c
const pattern_t nonbinary = {
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
};
```

### Step 2: Add to the enum
Next, add a value of your flag to the enum `flag_type_t` (just before `FLAG_TYPE_END`)
``` c
/* Patterns enum. */
typedef enum flag_type_e
{
    FLAG_TYPE_INVALID = -1,
    FLAG_TYPE_RAINBOW = 0,
    FLAG_TYPE_TRANS,
    ...
    FLAG_TYPE_ASEXUAL,
    /* FLAG_TYPE_YOUR_NEW_FLAG goes here. */
    FLAG_TYPE_END
} flag_type_t;
```

### Step 3: Handle in the `get_pattern` function
Add a `case` to the `switch` in the function.
``` c
    switch (flag_type) {
        case FLAG_TYPE_RAINBOW:
            return &rainbow;

        case FLAG_TYPE_TRANS:
            return &transgender;

        ...

        case FLAG_TYPE_ASEXUAL:
            return &asexual;

        case FLAG_TYPE_YOUR_NEW_FLAG:
            return &your_new_flag;

        default:
            return NULL;
    }

```

### Step 4: Add in the `README.md` and help string.
**Pay attension to the number it gets from the enum!**

`main.c`
``` c
/* *** Constants *****************************************************/
static char helpstr[] = "\n"
                        ...
                        "--flag <d>                , -f <d>: Choose colors to use:\n"
                        "                                    [rainbow: 0, trans: 1, NB: 2, lesbian: 3, \n"
                        "                                    gay: 4, pan: 5, bi: 6, genderfluid: 7, asexual: 8, \n"
                        "                                    your_new_flag: 9]\n"
                        "                                    default is rainbow (0)\n"
                        ...
```

`README.md`
Extend the line for `--flag` under `Usage` the same way as in the `main.c`.

*Note that in the readme it is a single line.*

### Step 5: Pull request :)

## Compiling
to compile with gcc: `$ gcc main.c -lm -o queercat`  

add the binary to a directory in your `PATH` variable (`/bin` can work) to use from everywhere

## Credits
base for code: <https://github.com/jaseg/lolcat/>  
Original idea: <https://github.com/busyloop/lolcat/>
