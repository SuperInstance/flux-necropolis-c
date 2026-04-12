# flux-necropolis

Fleet graveyard — when vessels die, their knowledge lives on.

## What

A pure C11 library for managing death records of autonomous fleet vessels. When a vessel dies, it's memorialized in a graveyard, its knowledge is harvested into an afterlife, and living vessels can visit to learn from the dead.

Zero dependencies. No malloc. Stack and static allocation only.

## API

### Graveyard
```c
graveyard_init(&gy);                          // init
graveyard_bury(&gy, &stone);                  // returns index or -1
graveyard_find(&gy, vessel_id);               // find by id
graveyard_find_name(&gy, "name");             // find by name
graveyard_count_dead(&gy);                    // count DEAD+
graveyard_count_memorialized(&gy);            // count MEMORIALIZED+
graveyard_count_harvested(&gy);               // count HARVESTED
```

### Afterlife
```c
afterlife_init(&al);
afterlife_store(&al, "key", "value", source_id);
afterlife_find(&al, "key");
afterlife_search(&al, "prefix", results, max);
afterlife_increment_reuse(&al, "key");
afterlife_most_reused(&al, top, count);
```

### Harvest
```c
necropolis_harvest(&tombstone, &afterlife);   // extract lesson into afterlife
```

### Memorial
```c
memorial_init(&log);
memorial_record(&log, visitor, stone, lessons);
memorial_visits_to(&log, stone);
memorial_visits_by(&log, visitor);
```

## Build

```sh
make          # build library + run tests
make clean    # remove artifacts
```

## Linking

```c
#include "necropolis.h"
// Link with -lnecropolis
```

## Constraints

- C11 only, no POSIX extensions required
- Zero dynamic allocation
- Max 64 tombstones, 128 knowledge fragments, 256 memorial entries
- All sizes configurable via `#define` in `necropolis.h`
