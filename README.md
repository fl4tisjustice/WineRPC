# WineRPC

**Disclaimer:** This software is very early in development and as such may not function properly in many systems. Please feel free to create an issue should the need arise. Aditionally, both installation steps and requirements are subject to change in the future as the scope of the project grows (i.e. the inclusion of a GUI/compatibility with 64-bit compilers as well as other systems).

WineRPC allows Discord Rich Presence to function with games and software running under Wine/Proton.

## Installation

Prerequisites:
1. Install `make` and `i686-w64-mingw32-gcc` or an equivalent from your package manager (32-bit MinGW GCC for C).
2. Run `make` in the project root.
3. Lastly, just run the `winerpcbridge` located in the `bin` folder under wine **and** int the same wine prefix as the game/software you intend to have Rich Presence work with.