# CPU and Memory Performance Guidelines

Companion to `c99-code-standards.md`. Same running example (`Img` / `IMG_`),
same conventions — every snippet here obeys the standards file.

**These are not conventions.** The standards file contains rules with one right
answer that a reviewer can check on sight. This file contains judgement calls
that depend entirely on whether the code is hot. Applying them to cold code
costs you readability and buys nothing. **Profile first, every time.**

---

## 1. The cost model

Single-threaded CPU performance has plateaued. Code that is slow because of how
it touches memory will not get faster on next year's hardware.

Memory is a hierarchy — registers, L1, L2, L3, main memory, disk — and each
level down is dramatically slower. The number that matters:

> Fetching a 64-byte cache line from main memory costs roughly **200–300
> cycles**. In that time the CPU could have done hundreds of arithmetic
> operations.

Two consequences drive everything below:

- **The cache line is the unit of transfer.** You never load 4 bytes; you load
  64, starting at the requested address and running forward. Whatever else is
  in those 64 bytes came along free.
- **The win is in the bytes you didn't have to fetch**, not in the instructions
  you removed. Most "optimisation" here is really data layout.

---

## 2. Data layout

### Fill the cache line with data you're about to use

Arrange data used together so it sits together. This is the whole game, and
every rule in this section is a restatement of it.

### Use the smallest type that holds the range

An enum with three states is a `uint8_t` field, not an `int` — see *Field
ordering and width* in the standards. Eight times as many fit in a line.

One honest caveat for C: on x86-64, arithmetic on `uint8_t` promotes to `int`
anyway, so this is a **memory density** win, not a compute win. It pays off in
arrays and struct fields; it does nothing for a loop counter, and
`for (uint8_t i = ...)` is worse than `size_t` for no benefit.

### Array of structs vs struct of arrays

C has no reference types, so an array of structs is already packed — you get
locality by default, and the class-vs-struct advice from managed languages
doesn't apply. The remaining question is AoS vs SoA.

```c
// Array of structs — the default. A pass loads every field whether or not
// it reads them.
typedef struct ImgTile {
    uint64_t hash;
    uint32_t offset;
    uint16_t width;
    uint8_t level;
    uint8_t flags;
} ImgTile;

// Struct of arrays — a pass over flags touches only the flags array.
typedef struct ImgTileSet {
    uint64_t *hashes;
    uint32_t *offsets;
    uint16_t *widths;
    uint8_t *levels;
    uint8_t *flags;
    size_t count;
} ImgTileSet;
```

**AoS is the default.** Reach for SoA only when profiling shows a hot pass that
reads one or two fields out of many — then that pass walks a dense array
instead of loading 16 bytes to read one. SoA costs you: five allocations to
keep in step, no single `ImgTile` to pass around, and any pass that *does* use
every field now touches five streams instead of one.

### Hot/cold splitting

The cheaper version of the same idea, and usually the one to try first. Keep
the fields the hot loop reads in one array, and everything else in a parallel
array indexed the same way.

```c
typedef struct ImgTileHot {   // walked every frame
    uint32_t offset;
    uint8_t flags;
} ImgTileHot;

typedef struct ImgTileCold {  // touched on load and on error paths
    const char *debugLabel;
    uint64_t sourceHash;
    ImgSize originalSize;
} ImgTileCold;
```

### Don't chase pointers you don't have to

Deeply nested pointer structures scatter access across memory. A linked list is
fine when you hold a direct reference to a node and splice locally; it is a bad
default for a collection you iterate or index, where an array wins on every
access.

The standards file's *Handle granularity* rule is this rule applied to API
design: a handle per element means a pointer chase per element.

---

## 3. Access patterns

### Provide bulk entry points

A per-item update function called N times is N calls and N chances to miss
cache. A bulk function walks the array once.

```c
// Per item — fine for a handful, wrong as the hot path.
void ImgTileUpdate(ImgTile *tile, float deltaTime);

// Bulk — one call, one linear walk, prefetcher engaged.
void ImgTileSetUpdate(ImgTileSet *tiles, float deltaTime);
```

Design the bulk form first and let the single-item form be the special case,
not the other way round.

### Decide ahead of the loop, not inside it

An `if` in a hot loop is a decision you are re-making for every element. Split
the data by state instead, and let array membership carry the information.

```c
// Re-tests state for every tile, every frame.
for (size_t i = 0; i < set->count; ++i) {
    if (set->states[i] == IMG_TILE_DIRTY) {
        UploadTile(set, i);
    }
}

// Dirty tiles live in their own array; the state is out of band.
for (size_t i = 0; i < dirtySet->count; ++i) {
    UploadTile(dirtySet, i);
}
```

The cost is real and should be stated: you now maintain N collections and move
elements between them on state change. Worth it for a per-frame loop over
thousands of items; not worth it for a list of eight.

### Deleting from a collection

**If order doesn't matter, swap with the last element.** Removing from the
middle of an array otherwise shifts every following element.

```c
/**
 * Removes the tile at `index` by moving the last tile into its slot.
 *
 * Does not preserve order: the element previously at the end takes the
 * removed element's index. Callers iterating by index must re-test the
 * current index after a removal rather than advancing past it.
 *
 * @param set   Tile set to modify. Non-NULL.
 * @param index Index of the tile to remove. Must be less than set->count.
 */
void ImgTileSetRemoveSwap(ImgTileSet *set, size_t index)
{
    IMG_ASSERT(set != NULL);
    IMG_ASSERT(index < set->count);

    set->count -= 1;
    set->offsets[index] = set->offsets[set->count];
    set->flags[index] = set->flags[set->count];
}
```

**If order matters, batch the removals.** Mark, then compact once — a single
pass rather than a shift per deletion.

```c
/**
 * Compacts a tile set in place, dropping every tile marked for removal.
 *
 * Preserves the relative order of the surviving tiles. Intended to be called
 * once at the end of a processing pass, not per removal.
 *
 * @param set Tile set to compact. Non-NULL.
 * @return The number of tiles remaining.
 */
size_t ImgTileSetCompact(ImgTileSet *set)
{
    IMG_ASSERT(set != NULL);

    size_t write = 0;
    for (size_t read = 0; read < set->count; ++read) {
        if ((set->flags[read] & IMG_TILE_REMOVED) == 0) {
            set->offsets[write] = set->offsets[read];
            set->flags[write] = set->flags[read];
            write += 1;
        }
    }

    set->count = write;
    return write;
}
```

Yes, that `if` is inside a loop, which the previous rule argues against. It
earns its place: one linear pass replaces a shift per deletion, and the
alternative — partitioning by removal state — costs more than it saves for a
once-per-frame compaction.

### Keep callbacks out of hot loops

A function pointer called per element is an indirect call the compiler can't
inline, and worse, it's arbitrary code: it might allocate, log, hit the disk or
evict the cache you just warmed. Hoist it, inline it, or batch the results and
call once.

This applies directly to `ImgAllocator`, which is a callback struct: allocate
one array of N, never call the allocator N times.

### Know what the collection is doing underneath

`List`-style removal copies. Hash lookups hash, then chase. `malloc` walks a
free list. None of these are free, and all of them are built on the same arrays
and pointer chases described above.

---

## 4. Allocation

The standards file requires an explicit `ImgAllocator` on every allocating
function. This is what that requirement is *for*: strategy becomes a caller
decision, with no API change.

### A linear (arena) allocator

The simplest and fastest allocator there is — a bump pointer. Perfect when
allocations share a lifetime and are released together: a frame, a load, a
parse.

```c
typedef struct ImgArena {
    uint8_t *base;
    size_t capacity;
    size_t used;
} ImgArena;

/**
 * Allocates from an arena by bumping its offset.
 *
 * @param userData  The ImgArena to allocate from. Non-NULL.
 * @param size      Bytes requested.
 * @param alignment Required alignment. Must be a power of two.
 * @return The allocation, or NULL if the arena is exhausted.
 */
static void *ArenaAlloc(void *userData, size_t size, size_t alignment)
{
    IMG_ASSERT(userData != NULL);
    IMG_ASSERT(alignment != 0 && (alignment & (alignment - 1)) == 0);

    ImgArena *arena = (ImgArena *)userData;
    const size_t aligned = (arena->used + alignment - 1) & ~(alignment - 1);
    if (aligned + size > arena->capacity) {
        return NULL;
    }

    arena->used = aligned + size;
    return arena->base + aligned;
}

/**
 * No-op: an arena releases everything at once via ImgArenaReset.
 *
 * @param userData The ImgArena. Unused.
 * @param pointer  The allocation. Unused.
 */
static void ArenaFree(void *userData, void *pointer)
{
    (void)userData;
    (void)pointer;
}
```

Wiring it up is an ordinary designated initializer — no compound literal, per
the standards:

```c
ImgArena arena = { .base = storage, .capacity = sizeof storage, .used = 0 };

const ImgAllocator allocator = {
    .alloc = ArenaAlloc,
    .free = ArenaFree,
    .userData = &arena,
};

ImgImage *image = NULL;
const ImgResult result = ImgImageLoadFromFile("sprite.png", &allocator,
                                              &image);
```

The constraint to document loudly wherever an arena is used: **individual frees
do nothing.** The lifetime is the arena's, and `ImgImageDestroy` becomes a
formality. That's safe only because the standards require every handle to
remember the allocator that built it.

### General allocation

The general-purpose heap hands out non-contiguous blocks to avoid fragmentation
and exhaustion. Freeing is not "returning a quantity" — memory has an address
as well as a size, which is why a general allocator is orders of magnitude more
complex than a bump pointer. Use it for long-lived, individually-managed
objects; use an arena or a pool for everything with a shared lifetime.

---

## 5. Doing less work at the wrong time

- **Bulk.** Once you've paid to set up a task — code in i-cache, data in
  d-cache, lookup tables warm — do as much of it as you can before moving on.
- **Ahead of time.** Anything that doesn't have to happen at frame time
  shouldn't: bake it, precompute it at startup, generate it at build time.
- **In parallel.** Aim for independent streams of work that need little
  synchronisation and combine cheaply at the end. Each core has its own L1, so
  a single-threaded bottleneck wastes most of the machine's fast memory.

Note the interaction with the standards: thread safety is the caller's
responsibility and no handle locks itself, so a parallel design must partition
data so that threads don't touch the same handle — and must pass a thread-safe
allocator, or one arena per thread.

---

## 6. Measuring

- **Profile before you change anything.** Every rule here trades clarity for
  speed, and paying that price in cold code is a straight loss.
- **Benchmark the alternatives** rather than reasoning about them. Cache
  behaviour routinely defeats intuition, and the only way to know whether SoA
  beat AoS for your access pattern is to run both.
- **Improvements multiply, they don't add.** A dozen sensible layout decisions
  compound into an order of magnitude, which is why "micro-optimisation" is the
  wrong frame for data layout — it's design, and it's very hard to retrofit.
- Watch for the case where the profiler points at something absurd. The
  canonical example is GTA Online spending most of its several-minute load
  parsing a 10 MB JSON file — not a hot loop anyone had thought about.

---

## 7. Where this document defers

When speed and the standards file conflict, these hold:

- Naming, formatting, doc comments and error handling are **not negotiable for
  performance**. A hot loop uses the same conventions as everything else.
- `const`-unless-mutated and declare-at-first-use cost nothing at runtime.
  Never drop them "for speed".
- The explicit-allocator rule stands even in hot paths — the fix for allocator
  overhead is fewer allocations, not a direct `malloc`.
- Assertions are compiled out in release, so `IMG_ASSERT` in a hot loop is
  free. Keep them.

Not yet covered: `restrict` and strict-aliasing rules, SIMD and alignment
requirements, prefetch intrinsics, false sharing between cores, and
allocator-level pool implementations.
