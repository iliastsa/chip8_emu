# Build
This project is built using CMake and the only dependency it has is SDL2.

To build the project, run:
``` shell
cd build
cmake ..
make
```

# Usage
After building the project, to execute run:
''' shell
./chip8_emu <input_binary>
'''

CHIP-8 uses a hexadecimal keypad, which is mapped as follows (notation: [keypad] => [keyboard]):

| 1 => 1 | 2 => 2 | 3 => 3 | C => 4 |
|--------|--------|--------|--------|
| 4 => Q | 5 => W | 6 => E | D => R |
| 7 => A | 8 => S | 9 => D | E => F |
| A => Z | 0 => X | B => C | F => V |

# Licence
The source code of this project is licensed under the [MIT License](https://opensource.org/licenses/mit-license.php) 
