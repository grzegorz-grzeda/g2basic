# G2Basic

[![Build Linux Executable](https://github.com/grzegorz-grzeda/g2basic/actions/workflows/build-linux.yml/badge.svg)](https://github.com/grzegorz-grzeda/g2basic/actions/workflows/build-linux.yml)
Simple BASIC interpreter for microcontrollers with dynamic memory management and comprehensive language support.

## Features

- **Dynamic Memory Management**: All data structures use linked lists for unlimited nesting
- **BASIC Language Support**: Variables, functions, control flow (FOR/NEXT, IF/THEN, GOTO, GOSUB/RETURN)
- **Mathematical Functions**: Built-in math library with common functions
- **Line-based Programming**: Traditional BASIC line number support
- **Configurable Output**: Customizable print function for different environments

## Quick Start

### Download Pre-built Binaries

Visit the [Releases](../../releases) page or check the [Actions](../../actions) tab for the latest builds.

### Building from Source

```bash
# Clone the repository
git clone https://github.com/grzegorz-grzeda/g2basic.git
cd g2basic

# Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Run the interactive interpreter
./examples/interactive/g2basic-interactive
```

### Example Usage

```basic
10 PRINT "Hello, World!"
20 FOR I = 1 TO 10
30 PRINT "Count: " I
40 NEXT I
50 END
```

## CI/CD

This project includes automated builds for Linux. See [CI Documentation](.github/CI_README.md) for details.