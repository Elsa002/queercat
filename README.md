# queercat
a version of lolcat with some lgbtq+ pride flags options

## Usage
`$ queercat [-f flag_number][-h horizontal_speed] [-v vertical_speed] [--] [FILES...]`  

```
Concatenate FILE(s), or standard input, to standard output.  
With no FILE, or when FILE is -, read standard input.

--flag <d>                , -f <d>: Choose colors to use: [rainbow: 0, trans: 1, NB: 2, lesbian: 3, gay: 4, pan: 5, bi: 6, genderfluid: 7, unlabeled: 8] default is rainbow(0)
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


## Compiling
to compile with gcc: `$ gcc main.c -lm -o queercat`  

add the binary to a directory in your `PATH` viriable (`/bin` can work) to use from everywhere

## Credits
base for code: <https://github.com/jaseg/lolcat/>  
Original idea: <https://github.com/busyloop/lolcat/>
