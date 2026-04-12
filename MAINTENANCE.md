# MAINTENANCE.md

## Overview

`flux-necropolis` is a zero-dependency C11 library. Maintenance should be minimal.

## Building

```sh
make clean && make
```

All 15 tests must pass. CI should run `make` and fail on non-zero exit.

## Changing Limits

All capacity constants are `#define`s in `necropolis.h`:
- `TOMBSTONE_MAX` (default 64)
- `FRAGMENTS_MAX` (default 128)
- `MEMORIAL_LOG_MAX` (default 256)
- String field sizes (`VESSEL_NAME_MAX`, `CAUSE_MAX`, etc.)

Changing these changes struct sizes. No malloc means no fragmentation concerns, but large values increase stack/static usage.

## Adding Functionality

- Keep it zero-malloc. If you need dynamic sizing, require the caller to pass pre-allocated buffers.
- Add tests in `test_necropolis.c` following the existing `TEST(name)` pattern.
- Keep the public API in `necropolis.h`; implementation in `necropolis.c`.

## Potential Improvements

- **Persistence**: Add serialize/deserialize to flat buffers for saving to disk.
- **Eviction**: When graveyard/afterlife is full, allow LRU eviction of oldest entries.
- **Query**: Add more search patterns (substring match, regex-lite).
- **Thread safety**: Currently not thread-safe. Add mutex wrappers if needed.
