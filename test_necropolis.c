#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "necropolis.h"

static int tests_run = 0;
static int tests_passed = 0;
#define TEST(name) do { tests_run++; printf("  %-40s ", #name); name(); printf("PASS\n"); tests_passed++; } while(0)
#define ASSERT(x) do { if (!(x)) { printf("FAIL\n"); return; } } while(0)

static void make_stone(Tombstone *s, uint16_t id, const char *name, VesselState st) {
    memset(s, 0, sizeof(*s));
    s->vessel_id = id;
    strncpy(s->name, name, VESSEL_NAME_MAX-1);
    s->state = st;
}

// 1. graveyard init empty
static void test_graveyard_init_empty(void) {
    Graveyard gy;
    graveyard_init(&gy);
    ASSERT(gy.count == 0);
    ASSERT(gy.capacity == TOMBSTONE_MAX);
}

// 2. graveyard bury one
static void test_graveyard_bury_one(void) {
    Graveyard gy; graveyard_init(&gy);
    Tombstone s; make_stone(&s, 1, "voyager", VESSEL_DEAD);
    int idx = graveyard_bury(&gy, &s);
    ASSERT(idx == 0);
    ASSERT(gy.count == 1);
    ASSERT(gy.stones[0].vessel_id == 1);
}

// 3. graveyard bury full
static void test_graveyard_bury_full(void) {
    Graveyard gy; graveyard_init(&gy);
    Tombstone s;
    for (int i = 0; i < TOMBSTONE_MAX; i++) {
        make_stone(&s, (uint16_t)i, "x", VESSEL_DEAD);
        ASSERT(graveyard_bury(&gy, &s) >= 0);
    }
    make_stone(&s, 99, "overflow", VESSEL_DEAD);
    ASSERT(graveyard_bury(&gy, &s) == -1);
}

// 4. graveyard find by id
static void test_graveyard_find_id(void) {
    Graveyard gy; graveyard_init(&gy);
    Tombstone s; make_stone(&s, 42, "alpha", VESSEL_DEAD);
    graveyard_bury(&gy, &s);
    Tombstone *f = graveyard_find(&gy, 42);
    ASSERT(f != NULL);
    ASSERT(strcmp(f->name, "alpha") == 0);
    ASSERT(graveyard_find(&gy, 99) == NULL);
}

// 5. graveyard find by name
static void test_graveyard_find_name(void) {
    Graveyard gy; graveyard_init(&gy);
    Tombstone s; make_stone(&s, 1, "beta", VESSEL_DEAD);
    graveyard_bury(&gy, &s);
    ASSERT(graveyard_find_name(&gy, "beta") != NULL);
    ASSERT(graveyard_find_name(&gy, "gamma") == NULL);
}

// 6. graveyard count states
static void test_graveyard_count_states(void) {
    Graveyard gy; graveyard_init(&gy);
    Tombstone a, b, c;
    make_stone(&a, 1, "a", VESSEL_DEAD);
    make_stone(&b, 2, "b", VESSEL_MEMORIALIZED);
    make_stone(&c, 3, "c", VESSEL_HARVESTED);
    graveyard_bury(&gy, &a); graveyard_bury(&gy, &b); graveyard_bury(&gy, &c);
    ASSERT(graveyard_count_dead(&gy) == 3);
    ASSERT(graveyard_count_memorialized(&gy) == 2);
    ASSERT(graveyard_count_harvested(&gy) == 1);
}

// 7. afterlife init empty
static void test_afterlife_init_empty(void) {
    Afterlife al; afterlife_init(&al);
    ASSERT(al.count == 0);
}

// 8. afterlife store and find
static void test_afterlife_store_find(void) {
    Afterlife al; afterlife_init(&al);
    afterlife_store(&al, "trust", "verify before trusting", 1);
    ASSERT(al.count == 1);
    KnowledgeFragment *f = afterlife_find(&al, "trust");
    ASSERT(f != NULL);
    ASSERT(strcmp(f->value, "verify before trusting") == 0);
    ASSERT(f->source_vessel_id == 1);
}

// 9. afterlife search prefix
static void test_afterlife_search_prefix(void) {
    Afterlife al; afterlife_init(&al);
    afterlife_store(&al, "net:error", "retry", 1);
    afterlife_store(&al, "net:timeout", "backoff", 2);
    afterlife_store(&al, "git:conflict", "rebase", 3);
    KnowledgeFragment results[4];
    int n = afterlife_search(&al, "net:", results, 4);
    ASSERT(n == 2);
}

// 10. afterlife reuse increment
static void test_afterlife_reuse_increment(void) {
    Afterlife al; afterlife_init(&al);
    afterlife_store(&al, "trust", "verify", 1);
    afterlife_increment_reuse(&al, "trust");
    afterlife_increment_reuse(&al, "trust");
    KnowledgeFragment *f = afterlife_find(&al, "trust");
    ASSERT(f->reused_count == 2);
    ASSERT(afterlife_increment_reuse(&al, "nope") == -1);
}

// 11. afterlife most reused
static void test_afterlife_most_reused(void) {
    Afterlife al; afterlife_init(&al);
    afterlife_store(&al, "a", "1", 1);
    afterlife_store(&al, "b", "2", 2);
    afterlife_store(&al, "c", "3", 3);
    afterlife_increment_reuse(&al, "c");
    afterlife_increment_reuse(&al, "c");
    afterlife_increment_reuse(&al, "a");
    KnowledgeFragment top[3];
    afterlife_most_reused(&al, top, 3);
    ASSERT(top[0].reused_count == 2);
    ASSERT(strcmp(top[0].key, "c") == 0);
    ASSERT(top[1].reused_count == 1);
}

// 12. necropolis harvest
static void test_necropolis_harvest(void) {
    Tombstone s; memset(&s, 0, sizeof(s));
    s.vessel_id = 7;
    strncpy(s.name, "sentinel", VESSEL_NAME_MAX-1);
    strncpy(s.lesson, "always validate input", LESSON_MAX-1);
    s.state = VESSEL_DEAD;
    Afterlife al; afterlife_init(&al);
    int r = necropolis_harvest(&s, &al);
    ASSERT(r == 0);
    KnowledgeFragment *f = afterlife_find(&al, "sentinel");
    ASSERT(f != NULL);
    ASSERT(strcmp(f->value, "always validate input") == 0);
    ASSERT(f->source_vessel_id == 7);
}

// 13. memorial record
static void test_memorial_record(void) {
    MemorialLog ml; memorial_init(&ml);
    int r = memorial_record(&ml, 10, 3, 2);
    ASSERT(r == 0);
    ASSERT(ml.count == 1);
    ASSERT(ml.visits[0].visitor_id == 10);
    ASSERT(ml.visits[0].stone_index == 3);
    ASSERT(ml.visits[0].lessons_taken == 2);
}

// 14. memorial visits to/by
static void test_memorial_visits_to_by(void) {
    MemorialLog ml; memorial_init(&ml);
    memorial_record(&ml, 1, 5, 1);
    memorial_record(&ml, 1, 5, 2);
    memorial_record(&ml, 2, 5, 1);
    memorial_record(&ml, 1, 9, 1);
    ASSERT(memorial_visits_to(&ml, 5) == 3);
    ASSERT(memorial_visits_by(&ml, 1) == 3);
    ASSERT(memorial_visits_by(&ml, 2) == 1);
}

// 15. tombstone defaults
static void test_tombstone_defaults(void) {
    Tombstone s; memset(&s, 0, sizeof(s));
    ASSERT(s.vessel_id == 0);
    ASSERT(s.state == VESSEL_ALIVE);
    ASSERT(s.birth_time == 0);
    ASSERT(s.cycles_lived == 0);
    ASSERT(s.commits_made == 0);
    ASSERT(s.peak_trust == 0.0f);
    ASSERT(s.knowledge_harvested == 0);
}

int main(void) {
    printf("=== flux-necropolis tests ===\n");
    TEST(test_graveyard_init_empty);
    TEST(test_graveyard_bury_one);
    TEST(test_graveyard_bury_full);
    TEST(test_graveyard_find_id);
    TEST(test_graveyard_find_name);
    TEST(test_graveyard_count_states);
    TEST(test_afterlife_init_empty);
    TEST(test_afterlife_store_find);
    TEST(test_afterlife_search_prefix);
    TEST(test_afterlife_reuse_increment);
    TEST(test_afterlife_most_reused);
    TEST(test_necropolis_harvest);
    TEST(test_memorial_record);
    TEST(test_memorial_visits_to_by);
    TEST(test_tombstone_defaults);
    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
