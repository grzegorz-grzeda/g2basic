# G2Basic Documentation

This directory contains the Doxygen-generated documentation for the G2Basic BASIC language interpreter.

## Overview

The G2Basic project is fully documented using Doxygen comments throughout the source code. The documentation includes:

- **API Reference**: Complete documentation of all public functions and data structures
- **Internal Functions**: Documentation of key internal implementation functions
- **Code Examples**: Usage examples and code snippets
- **Architecture Overview**: High-level description of the interpreter design
- **Memory Management**: Details about dynamic memory allocation and cleanup

## Generating Documentation

### Prerequisites

Install Doxygen on your system:

```bash
# Ubuntu/Debian
sudo apt-get install doxygen

# macOS with Homebrew
brew install doxygen

# Windows - Download from http://www.doxygen.nl/download.html
```

### Generate HTML Documentation

From the project root directory:

```bash
# Generate documentation
doxygen Doxyfile

# Open the documentation
open docs/html/index.html    # macOS
xdg-open docs/html/index.html # Linux
start docs/html/index.html   # Windows
```

The documentation will be generated in the `docs/html/` directory.

## Documentation Structure

### Main Sections

1. **Main Page** - Project overview and getting started guide
2. **Modules** - Organized view of functionality:
   - **Public API** - Functions for integrating G2Basic
   - **Memory Management** - Internal memory handling functions
   - **Parser Implementation** - Expression and statement parsing
   - **Built-in Functions** - Mathematical and utility functions

3. **Files** - Source file documentation:
   - `g2basic.h` - Public API header
   - `g2basic.c` - Main implementation
   - `main.c` - Interactive interpreter example

4. **Data Structures** - Documentation of key structures:
   - `Variable` - Variable storage
   - `Function` - Function registration
   - `ProgramLine` - Program line storage
   - `ForLoop` - FOR loop state
   - `GosubStackEntry` - GOSUB call stack

### Key Features Documented

- **Dynamic Memory Management**: All data structures use linked lists with no fixed limits
- **BASIC Language Support**: Complete implementation of core BASIC features
- **Expression Parser**: Recursive descent parser with operator precedence
- **Control Flow**: FOR/NEXT loops, IF/THEN statements, GOTO/GOSUB
- **Function System**: Built-in math functions and custom function registration
- **Error Handling**: Comprehensive error reporting and recovery

## Using the Documentation

### For Library Users

If you're integrating G2Basic into your project:

1. Start with the **Main Page** for an overview
2. Read the **Public API** section for `g2basic.h`
3. Look at the **Examples** section for usage patterns
4. Check **Data Structures** for understanding data types

### For Contributors

If you're working on G2Basic development:

1. Review the **Architecture Overview**
2. Study the **Memory Management** section
3. Examine **Parser Implementation** details
4. Understand **Internal Functions** documentation

### Navigation Tips

- Use the **Search** box to quickly find functions or concepts
- The **Class Hierarchy** shows relationships between data structures
- **File List** provides quick access to source file documentation
- **Examples** section contains practical usage code

## Documentation Standards

The G2Basic project follows these documentation standards:

### Function Documentation

Every public function includes:
- Brief description
- Detailed description with usage context
- Parameter documentation with types and meanings
- Return value explanation
- Usage notes and warnings
- Code examples where helpful
- Cross-references to related functions

### Data Structure Documentation

Every structure includes:
- Purpose and usage context
- Member variable documentation
- Memory management notes
- Usage examples
- Related functions

### Code Examples

Documentation includes:
- Basic usage patterns
- Error handling examples
- Complete program examples
- Integration examples

## Building Documentation in CI

The documentation can be automatically built in CI/CD pipelines:

```yaml
# Add to GitHub Actions workflow
- name: Install Doxygen
  run: sudo apt-get install doxygen

- name: Generate Documentation
  run: doxygen Doxyfile

- name: Deploy Documentation
  uses: peaceiris/actions-gh-pages@v3
  with:
    github_token: ${{ secrets.GITHUB_TOKEN }}
    publish_dir: ./docs/html
```

## Contributing to Documentation

When contributing to G2Basic:

1. **Add Doxygen comments** to all new public functions
2. **Update existing comments** when changing function behavior
3. **Include examples** for complex features
4. **Cross-reference related functions** using `@see` tags
5. **Test documentation** by generating and reviewing output

### Comment Style

Use this format for functions:

```c
/**
 * @brief Brief one-line description
 * 
 * Detailed description explaining the function's purpose,
 * behavior, and any important usage notes.
 * 
 * @param param_name Description of parameter
 * @param[in,out] param2 Parameter that is modified
 * @return Description of return value
 * 
 * @note Important usage notes
 * @warning Critical warnings
 * @see related_function()
 * @since Version when function was added
 * 
 * @code
 * // Usage example
 * result = my_function(arg1, arg2);
 * @endcode
 */
```

## Additional Resources

- [Doxygen Manual](http://www.doxygen.nl/manual/)
- [Doxygen Comment Blocks](http://www.doxygen.nl/manual/docblocks.html)
- [G2Basic GitHub Repository](https://github.com/grzegorz-grzeda/g2basic)
- [Project Issues and Discussions](https://github.com/grzegorz-grzeda/g2basic/issues)