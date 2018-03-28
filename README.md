# D&D Dice Roller

This is a standard D&D (or any role playing game) dice roller which prints each
roll, any constant bonuses, and finally the total of all rolls and bonuses.

The program accepts a dice rolling language most role players will probably
understand:
`<roll> := <number> | <number>d<number> [+ <roll>]`

Or to give some examples:

    ./roll "2d4 + 3"
    6 (2d4) + 3 (bonus) => 9

    ./roll "1d20 + 6"
    17 (1d20) + 6 (bonus) => 23

    ./roll "5 => 5"
    5 (bonus) => 5

    ./roll "3d6 + 1d4"
    11 (3d6) + 2 (1d4) => 13

## How do I use it?

Simply clone this repository, travel to the directory, and run `make`. The name
of the program is simply `roll`.

If, for some reason, you don't want to print each roll and bonuses and just want
to print a single total, you can run `make dont_show_rolls`.

The dice roller parses the first argument as a string. So, the argument must be
in quotes, e.g. `./roll "1d20"`.  The dice roller also accepts arguments from
`stdin`, e.g. `echo "2d4" | ./roll`.

## How did you do this?

The random number generator is called [The Mersenne
Twister](https://www.mcs.anl.gov/~kazutomo/hugepage-old/twister.c). The
`twister.c` was lifted from that URL and I take no credit for that work.
Otherwise, I wrote everything else such as the uniform distribution (hopefully
correctly) and the parser.
