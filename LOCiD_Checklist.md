2025-08-17 13:05:00

Jules, here's the revised set of instructions to develop, build, test, debug, refine, document, and checksum the LOCiD tool based on the v18.0.0 design document. In addition to the previous steps, use this 6-step document as your guiding framework. From it, I've created a markdown checklist below for all main and sub items. Maintain and update this checklist after each step is completed—mark items as [x] done, add notes on outcomes/bugs fixed, and version the checklist (e.g., v1 after Step 1). Track it in a separate file (locid_build_checklist.md) and reference updates in progress reports.

### 6-Step Framework for LOCiD Development
1. **Refine and Complete the Source Code**: Fill in stubs with real logic, debug initial issues.
2. **Dependencies and Code Setup**: Acquire/include required libs, setup environment.
3. **Build Instructions**: Compile binaries/libraries for platforms/architectures.
4. **Rigorous Testing and Debugging**: Run units/integration/edges, fix bugs, optimize.
5. **Full Documentation**: Create user/library docs, examples.
6. **Blake3 Checksums**: Compute/verify for all outputs.

### Markdown Checklist (Initial v0 - Update After Each Step)

#### Step 1: Refine and Complete the Source Code
- [ ] Base on locid.c/h stubs.
- [ ] Implement detect_device_type (Windows: WMI for Model/Rotation; Linux: /sys/queue/rotational + /sys/class/nvme/mdadm; Mac: IOKit stubs).
- [ ] Implement collect_loci queries per matrix (e.g., SerialNumber: Windows DeviceIoControl, Linux hdparm/udevadm, Mac IOKit; handle _NA_).
- [ ] Dynamic skip for type-irrelevant loci.
- [ ] parse_loci with comma/ranges.
- [ ] compute_hash with "key:value\n" sorted, BLAKE3 extended 1024-bit.
- [ ] compute_stability_rank: score = sum(1 - change_prob for non-_NA_) / total; avail = non_na / total; final = score * avail; High >0.8, Medium >0.5, Low else.
- [ ] Output: Hex ID; append " [Stability: Rank]" if --stability.
- [ ] Verbose: Log loci, _NA_, rank.
- [ ] JSON for --list.
- [ ] Device path parsing for MODE N (pd default; Windows \\.\PhysicalDriveN/Disk#N; Linux /dev/sdX/enumerated; Mac /dev/diskN/IOKit).
- [ ] Initial debug: Compile, run with gdb/lldb/valgrind, fix leaks/crashes.

Notes: (Add after completion)

#### Step 2: Dependencies and Code Setup
- [ ] BLAKE3: Download/include c reference from github.
- [ ] CUnit: Install via apt (Debian), apk (Alpine), sourceforge (Windows).
- [ ] No other deps; use standard libs.
- [ ] Setup repo: Git init, commit initial code.

Notes: (Add after completion)

#### Step 3: Build Instructions
- [ ] Common: Clone repo, cd locid, include blake3.c/h.
- [ ] Windows x86/x64 (MSVC): cl /FeLOCiD.exe locid.c blake3.c /link /MACHINE:X64; /MACHINE:X86; DLL: /DLL /FeLOCiD.dll.
- [ ] Debian Linux x86/x64 (gcc): gcc -o LOCiD locid.c blake3.c -ldl -m64; -m32; .so: -shared -o libLOCiD.so -fPIC -m64.
- [ ] Alpine Linux x86/x64 (musl): apk add build-base cunit-dev; musl-gcc -o LOCiD locid.c blake3.c -ldl -m64; -m32; .so: -shared -o libLOCiD.so -fPIC -m64.
- [ ] Verify builds run without errors.

Notes: (Add after completion)

#### Step 4: Rigorous Testing and Debugging
- [ ] Unit tests: CUnit for locus queries (mock handles), parse_loci, hash, type detect, stability (test thresholds).
- [ ] Integration: Test on real devices (HDD/SSD/NVMe/RAID/eMMC/USB), verify IDs, dynamic skip, partial on bad enclosure mock.
- [ ] Edge: Invalid loci (1), no device (2), permission (3), empty (4), partial (5).
- [ ] Performance: Time 100 runs <5ms avg; load 50 devices.
- [ ] Cross-platform: VMs for Windows/Debian/Alpine, consistent IDs.
- [ ] Debug: Valgrind/Dr. Memory for leaks; gdb/lldb for crashes. Refine: Fix bugs, optimize queries.

Notes: (Add after completion)

#### Step 5: Full Documentation
- [ ] Binary: README.md/man with usage, options, examples (e.g., LOCiD --device pd 0 --volatility 0 --chars 64 --stability -> "abcdef... [Stability: High]"; --list --json).
- [ ] Library: Doxygen in h/c; generate with examples (LOCiD_Generate usage, error handling).

Notes: (Add after completion)

#### Step 6: Blake3 Checksums
- [ ] Compute for each binary/library: Use LOCiD itself or blake3 cmd (blake3 LOCiD.exe > checksum.txt).
- [ ] Format: e.g., LOCiD.exe (Windows x64): blake3_hash

Notes: (Add after completion)

Proceed step-by-step, update checklist after each, report progress/bugs. Use spec for all decisions.