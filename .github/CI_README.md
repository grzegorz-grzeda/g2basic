# CI/CD Pipeline Documentation

This repository includes GitHub Actions workflows for automated building and testing of the G2Basic interpreter.

## Workflows

### 1. Build Linux Executable (`build-linux.yml`)

**Triggers:**
- Push to `main` or `develop` branches
- Pull requests to `main` branch
- GitHub releases

**Features:**
- Builds the `g2basic-interactive` executable for Linux x64
- Runs basic functionality tests
- Uploads build artifacts for each commit
- Automatically attaches binaries to GitHub releases

**Artifacts:**
- `g2basic-interactive-linux-x64`: The compiled executable

## Using the Built Executables

### Download from GitHub

1. Go to the [Actions](../../actions) tab
2. Click on the latest successful workflow run
3. Download the desired artifact
4. Extract and run the executable

### From Releases

For official releases, pre-built binaries are automatically attached to the release page.

### Testing the Executable

The CI pipeline includes basic functionality tests. You can run similar tests locally:

```bash
# Create a test program
cat > test.bas << EOF
10 PRINT "Hello from G2Basic!"
20 FOR I = 1 TO 5
30 PRINT "Number: " I  
40 NEXT I
50 END
EOF

# Run it
./g2basic-interactive < test.bas
```

## Local Development

To build locally (matching the CI environment):

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install build-essential cmake

# Build
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Test
echo "10 PRINT \"Local build test\"" | ./examples/interactive/g2basic-interactive
```

## Build Configuration

The CI uses the following CMake configuration:
- **Release builds:** `-DCMAKE_BUILD_TYPE=Release`
- **Optimized builds:** `-DCMAKE_C_FLAGS="-O3 -march=x86-64 -mtune=generic"`
- **Math library:** Automatically linked (`-lm`)

## Troubleshooting

### Build Failures

Check the Actions logs for detailed error messages. Common issues:
- Missing dependencies
- Compiler warnings treated as errors (`-Werror`)
- Test failures

### Artifact Downloads

Artifacts are retained for:
- Regular builds: 7-30 days
- Release builds: 90 days

### Adding New Tests

To add tests to the CI pipeline, modify the test sections in the workflow files:

```yaml
- name: Your test name
  run: |
    echo "Your test BASIC program" > test.bas
    ./build/examples/interactive/g2basic-interactive < test.bas
```