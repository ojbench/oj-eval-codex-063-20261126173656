# MemoryRiver

`MemoryRiver.hpp` implements the required template. The file contains the
`info_len` public integers, a private free-list head, and fixed-size records.
Indices are byte offsets. Zero marks the end of the free list; every record
starts after the header, so zero cannot be a valid record index.

Deleting a record stores the previous free-list head in that record and makes
it the new head. Writing consumes the head slot if available, otherwise it
appends a record. All list metadata is persisted in the same file. Updates
are flushed before returning, and stream ownership closes the file on
destruction. Initialisation truncates the file and resets all metadata.

Each operation uses a constant number of seeks and transfers. Record reads
and writes transfer `sizeof(T)` bytes. Auxiliary memory is constant, and
file size is `(info_len + 1) * sizeof(int) + peak_records * sizeof(T)`.
As required by the binary-storage interface, records must support raw-byte
serialization. Deleted indices must not be accessed or deleted again until
reallocated. Concurrent writes and crash recovery are outside the contract.

## Local validation

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

The test checks 100,000 deterministic randomized operations against an
in-memory model, metadata persistence, reopening, file-size bounds,
reinitialisation, and visibility between two instances.

## Judge result

- Problem: ACMOJ 1337.
- Submission: 879512.
- Submitted source: `MemoryRiver.hpp`, language `cpp`.
- Result: Accepted, all eight groups, 200/200 points.
- Attempts used: 1 of 5.
- Submitted implementation commit: `457dd229fa288378ffb5108c83bc8bc32a6604fe`.

The live problem API accepts C++ source for this template assignment; the
judge supplies the test driver. The CMake `code` executable is the local
regression test driver.
