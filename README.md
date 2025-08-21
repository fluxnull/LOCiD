# LOCiD

LOCiD is a professional-grade executable and library that generates a unique, reproducible identifier for storage devices.

## Building the project

### Linux (Debian/Ubuntu)

To build the project on a Debian-based Linux system, you will need `cmake` and a C compiler.

1.  **Install dependencies:**
    ```bash
    sudo apt-get update && sudo apt-get install -y build-essential cmake git
    ```

2.  **Clone the repository:**
    ```bash
    git clone <repo_url>
    cd locid
    ```

3.  **Create a build directory:**
    ```bash
    mkdir build
    cd build
    ```

4.  **Run cmake and make:**
    ```bash
    cmake ..
    make
    ```
    This will create the `locid` executable in the `build` directory.

### Windows (Cross-compilation from Linux with MinGW-w64)

You can build the Windows executables from a Linux environment using the `mingw-w64` cross-compiler.

1.  **Install dependencies:**
    ```bash
    sudo apt-get update && sudo apt-get install -y build-essential cmake git mingw-w64
    ```

2.  **Build the 64-bit version:**
    ```bash
    cmake -B build/win64 -DCMAKE_TOOLCHAIN_FILE=toolchain-win64.cmake .
    make -C build/win64
    ```
    The executable will be at `build/win64/locid.exe`.

3.  **Build the 32-bit version:**
    ```bash
    cmake -B build/win32 -DCMAKE_TOOLCHAIN_FILE=toolchain-win32.cmake .
    make -C build/win32
    ```
    The executable will be at `build/win32/locid.exe`.

### Alpine Linux

To build the project on Alpine Linux, you will need `cmake` and the `build-base` package.

1.  **Install dependencies:**
    ```bash
    sudo apk add build-base cmake git
    ```

2.  **Clone the repository and build:**
    Follow the same steps as for Debian/Ubuntu to clone the repository, create a build directory, and run `cmake` and `make`.

## Usage

```
locid [options]
```

### Options

*   `-h, --help`: Show the help message.
*   `--list`: Dumps a table of all loci and their values for the target device and exits.
*   `--volatility N`: Set volatility level. `0` = Hardware + Logic (Default). `1` = Hardware Only.
*   `--loci L`: Provide a comma-separated list of Loci numbers or ranges (e.g., `"1,3-5,10"`) to use for the fingerprint. Overrides `--volatility`.
*   `--chars N`: Output length of the hash (1-256). Default: 256.
*   `--device MODE N`: Select device by mode and number. Not yet implemented.
*   `--json`: Use JSON format for output.
*   `--verbose, -V`: Dump detailed info on fields used, _NA_, errors to stderr.
*   `--stability`: Rank the uniqueness quality of available loci.

## Examples

*   **Get the default LOCiD for `/dev/sda`:**
    ```bash
    sudo ./locid
    ```

*   **Get a shorter LOCiD:**
    ```bash
    sudo ./locid --chars 64
    ```

*   **List all the loci for a device:**
    ```bash
    sudo ./locid --list
    ```

*   **Get a LOCiD based on a specific set of loci:**
    ```bash
    sudo ./locid --loci "1,10,18"
    ```

*   **Get the LOCiD with a stability ranking:**
    ```bash
    sudo ./locid --stability
    ```

*   **Get the LOCiD in JSON format:**
    ```bash
    sudo ./locid --json
    ```
