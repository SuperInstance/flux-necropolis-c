#ifndef NECROPOLIS_H
#define NECROPOLIS_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    VESSEL_ALIVE = 0,
    VESSEL_DYING = 1,
    VESSEL_DEAD = 2,
    VESSEL_MEMORIALIZED = 3,
    VESSEL_HARVESTED = 4
} VesselState;

#define VESSEL_NAME_MAX 32
#define CAUSE_MAX 64
#define LESSON_MAX 128
#define TOMBSTONE_MAX 64

typedef struct {
    uint16_t vessel_id;
    char name[VESSEL_NAME_MAX];
    VesselState state;
    char cause[CAUSE_MAX];
    char lesson[LESSON_MAX];
    uint64_t birth_time;
    uint64_t death_time;
    uint64_t cycles_lived;
    uint32_t commits_made;
    uint32_t repos_touched;
    float peak_trust;
    float avg_confidence;
    uint8_t knowledge_harvested;
} Tombstone;

typedef struct {
    Tombstone stones[TOMBSTONE_MAX];
    uint16_t count;
    uint16_t capacity;
} Graveyard;

#define FRAGMENT_KEY_MAX 32
#define FRAGMENT_VAL_MAX 256
#define FRAGMENTS_MAX 128

typedef struct {
    char key[FRAGMENT_KEY_MAX];
    char value[FRAGMENT_VAL_MAX];
    uint16_t source_vessel_id;
    uint8_t reused_count;
} KnowledgeFragment;

typedef struct {
    KnowledgeFragment frags[FRAGMENTS_MAX];
    uint16_t count;
} Afterlife;

typedef struct {
    uint16_t visitor_id;
    uint16_t stone_index;
    uint64_t visit_time;
    uint8_t lessons_taken;
} MemorialVisit;

#define MEMORIAL_LOG_MAX 256
typedef struct {
    MemorialVisit visits[MEMORIAL_LOG_MAX];
    uint16_t count;
} MemorialLog;

void graveyard_init(Graveyard *gy);
int graveyard_bury(Graveyard *gy, const Tombstone *stone);
Tombstone* graveyard_find(Graveyard *gy, uint16_t vessel_id);
Tombstone* graveyard_find_name(Graveyard *gy, const char *name);
int graveyard_count_dead(Graveyard *gy);
int graveyard_count_memorialized(Graveyard *gy);
int graveyard_count_harvested(Graveyard *gy);

void afterlife_init(Afterlife *al);
int afterlife_store(Afterlife *al, const char *key, const char *value, uint16_t source_id);
KnowledgeFragment* afterlife_find(Afterlife *al, const char *key);
int afterlife_search(Afterlife *al, const char *prefix, KnowledgeFragment *results, int max_results);
int afterlife_increment_reuse(Afterlife *al, const char *key);
void afterlife_most_reused(Afterlife *al, KnowledgeFragment *top, int count);

int necropolis_harvest(const Tombstone *stone, Afterlife *al);

void memorial_init(MemorialLog *log);
int memorial_record(MemorialLog *log, uint16_t visitor, uint16_t stone, uint8_t lessons);
int memorial_visits_to(MemorialLog *log, uint16_t stone);
int memorial_visits_by(MemorialLog *log, uint16_t visitor);

#endif
