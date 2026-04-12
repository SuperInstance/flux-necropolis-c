#include "necropolis.h"
#include <string.h>

// --- helpers ---
static size_t safe_strncpy(char *dst, const char *src, size_t n) {
    size_t i;
    for (i = 0; i < n - 1 && src[i]; i++) dst[i] = src[i];
    dst[i] = '\0';
    return i;
}

static int str_starts(const char *s, const char *prefix) {
    while (*prefix) { if (*s++ != *prefix++) return 0; }
    return 1;
}

// --- Graveyard ---
void graveyard_init(Graveyard *gy) {
    memset(gy, 0, sizeof(*gy));
    gy->capacity = TOMBSTONE_MAX;
}

int graveyard_bury(Graveyard *gy, const Tombstone *stone) {
    if (gy->count >= gy->capacity) return -1;
    gy->stones[gy->count] = *stone;
    return (int)gy->count++;
}

Tombstone* graveyard_find(Graveyard *gy, uint16_t vessel_id) {
    for (uint16_t i = 0; i < gy->count; i++)
        if (gy->stones[i].vessel_id == vessel_id) return &gy->stones[i];
    return NULL;
}

Tombstone* graveyard_find_name(Graveyard *gy, const char *name) {
    for (uint16_t i = 0; i < gy->count; i++)
        if (strcmp(gy->stones[i].name, name) == 0) return &gy->stones[i];
    return NULL;
}

int graveyard_count_dead(Graveyard *gy) {
    int c = 0;
    for (uint16_t i = 0; i < gy->count; i++)
        if (gy->stones[i].state >= VESSEL_DEAD) c++;
    return c;
}

int graveyard_count_memorialized(Graveyard *gy) {
    int c = 0;
    for (uint16_t i = 0; i < gy->count; i++)
        if (gy->stones[i].state >= VESSEL_MEMORIALIZED) c++;
    return c;
}

int graveyard_count_harvested(Graveyard *gy) {
    int c = 0;
    for (uint16_t i = 0; i < gy->count; i++)
        if (gy->stones[i].state >= VESSEL_HARVESTED) c++;
    return c;
}

// --- Afterlife ---
void afterlife_init(Afterlife *al) {
    memset(al, 0, sizeof(*al));
}

int afterlife_store(Afterlife *al, const char *key, const char *value, uint16_t source_id) {
    if (al->count >= FRAGMENTS_MAX) return -1;
    KnowledgeFragment *f = &al->frags[al->count++];
    safe_strncpy(f->key, key, FRAGMENT_KEY_MAX);
    safe_strncpy(f->value, value, FRAGMENT_VAL_MAX);
    f->source_vessel_id = source_id;
    f->reused_count = 0;
    return 0;
}

KnowledgeFragment* afterlife_find(Afterlife *al, const char *key) {
    for (uint16_t i = 0; i < al->count; i++)
        if (strcmp(al->frags[i].key, key) == 0) return &al->frags[i];
    return NULL;
}

int afterlife_search(Afterlife *al, const char *prefix, KnowledgeFragment *results, int max_results) {
    int found = 0;
    for (uint16_t i = 0; i < al->count && found < max_results; i++)
        if (str_starts(al->frags[i].key, prefix))
            results[found++] = al->frags[i];
    return found;
}

int afterlife_increment_reuse(Afterlife *al, const char *key) {
    KnowledgeFragment *f = afterlife_find(al, key);
    if (!f) return -1;
    if (f->reused_count < 255) f->reused_count++;
    return 0;
}

static void swap_frags(KnowledgeFragment *a, KnowledgeFragment *b) {
    KnowledgeFragment tmp = *a; *a = *b; *b = tmp;
}

void afterlife_most_reused(Afterlife *al, KnowledgeFragment *top, int count) {
    // copy and partial sort
    int n = (int)al->count < count ? (int)al->count : count;
    for (int i = 0; i < n; i++) top[i] = al->frags[i];
    // insertion sort by reused_count desc
    for (int i = 1; i < n; i++) {
        KnowledgeFragment v = top[i];
        int j = i;
        while (j > 0 && top[j-1].reused_count < v.reused_count) {
            top[j] = top[j-1]; j--;
        }
        top[j] = v;
    }
    // check remaining
    for (uint16_t i = (uint16_t)n; i < al->count; i++) {
        if (al->frags[i].reused_count > top[n-1].reused_count) {
            top[n-1] = al->frags[i];
            // re-sort last element
            for (int j = n-1; j > 0 && top[j-1].reused_count < top[j].reused_count; j--)
                swap_frags(&top[j], &top[j-1]);
        }
    }
}

// --- Harvest ---
int necropolis_harvest(const Tombstone *stone, Afterlife *al) {
    if (!stone || !stone->lesson[0]) return -1;
    return afterlife_store(al, stone->name, stone->lesson, stone->vessel_id);
}

// --- Memorial ---
void memorial_init(MemorialLog *log) {
    memset(log, 0, sizeof(*log));
}

int memorial_record(MemorialLog *log, uint16_t visitor, uint16_t stone, uint8_t lessons) {
    if (log->count >= MEMORIAL_LOG_MAX) return -1;
    MemorialVisit *v = &log->visits[log->count++];
    v->visitor_id = visitor;
    v->stone_index = stone;
    v->lessons_taken = lessons;
    v->visit_time = 0; // caller can set
    return 0;
}

int memorial_visits_to(MemorialLog *log, uint16_t stone) {
    int c = 0;
    for (uint16_t i = 0; i < log->count; i++)
        if (log->visits[i].stone_index == stone) c++;
    return c;
}

int memorial_visits_by(MemorialLog *log, uint16_t visitor) {
    int c = 0;
    for (uint16_t i = 0; i < log->count; i++)
        if (log->visits[i].visitor_id == visitor) c++;
    return c;
}
