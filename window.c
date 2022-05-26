/**
 * Piotr Dobiech 316625
 */

#include "window.h"

#include "utils.h"
#include <malloc.h>

void initialize_window(const size_t window_size, Window* window) {
    window->size = window_size;
    window->entries = malloc(window_size * sizeof(WindowEntry));
    window->entries_size = 0;
    window->tail_position = 0;
}

void set_timeout_time(WindowEntry* entry, const struct timeval* now) {
    timeradd(now, &PACKET_TIMEOUT_TIME, &entry->timeout_time);
}

void push_window_entry(Window* window, const uint32_t start, const struct timeval* now) {
    debug_assert(window->entries_size < window->size);

    const size_t index = (window->tail_position + window->entries_size) % window->size;
    WindowEntry* entry = &window->entries[index];
    window->entries_size++;

    entry->start = start;
    set_timeout_time(entry, now);
    // entry->timeout_time = *now;
    // entry->length = length;
}

void pull_window_entry(Window* window) {
    debug_assert(window->entries_size >= 1);

    const size_t tail_position = window->tail_position;
    window->tail_position = (tail_position + 1) % window->size;
    window->entries_size--;
}

// void peek_window_entry(const Window* window, WindowEntry* result_entry) {
//     const WindowEntry* entry = &window->entries[window->tail_position];
//     copy_window_entry(entry, result_entry);
// }

WindowEntry* peek_window_entry(Window* window) {
    return &window->entries[window->tail_position];
}

static inline void reset_iterator(WindowIterator* iterator) {
    iterator->counter = 0;
    iterator->index = iterator->window->tail_position;
}

static inline WindowEntry* iterate_next(WindowIterator* iterator) {
    const size_t index = (iterator->index + iterator->counter) % iterator->window->size;
    WindowEntry* entry = &iterator->window->entries[index];
    iterator->counter++;
    return entry;
}

void create_window_iterator(Window* window, WindowIterator* iterator) {
    iterator->window = window;
    reset_iterator(iterator);
}

bool window_iterator_has_next(WindowIterator* iterator) {
    return iterator->counter < iterator->window->entries_size;
}

// bool iterate_window(WindowIterator* iterator, WindowEntry* result_entry) {
//     const WindowEntry* entry = iterate_mut_window(iterator);
//     if (entry == NULL) {
//         return false;
//     }
//     copy_window_entry(entry, result_entry);
//     return true;
// }

WindowEntry* iterate_window(WindowIterator* iterator) {
    if (!window_iterator_has_next(iterator)) {
        return NULL;
    }
    return iterate_next(iterator);
}

// bool cyclic_iterate_window(WindowIterator* iterator, WindowEntry* result_entry) {
//     const WindowEntry* entry = cyclic_iterate_mut_window(iterator);
//     copy_window_entry(entry, result_entry);
//     return true;
// }

WindowEntry* cyclic_iterate_window(WindowIterator* iterator) {
    if (!window_iterator_has_next(iterator)) {
        reset_iterator(iterator);
    }
    return iterate_next(iterator);
}

// WindowEntry* find_mut_window_entry(
//     WindowIterator* iterator, const uint32_t start, const uint16_t length) {

//     if (is_window_empty(iterator->window)) {
//         return NULL;
//     }

//     WindowEntry* first_entry = cyclic_iterate_window(iterator);
//     if (first_entry == NULL) {
//         return NULL;
//     }
//     if (first_entry->start == start && first_entry->length == length) {
//         return first_entry;
//     }

//     WindowEntry* entry;
//     while ((entry = cyclic_iterate_window(iterator)) != NULL && entry != first_entry) {
//         if (first_entry->start == start && first_entry->length == length) {
//             return first_entry;
//         }
//     }

//     return NULL;
// }

WindowEntry* get_window_entry(Window* window, const uint32_t start) {
    debug_assert(start % PACKET_DATA_MAXUMUM_LENGTH == 0);

    if (is_window_empty(window)) {
        return NULL;
    }

    WindowEntry* first_entry = peek_window_entry(window);
    // Assume that every window entry has start being a multiple of PACKET_DATA_MAXUMUM_LENGTH.
    const int32_t signed_offset = (start - first_entry->start) / PACKET_DATA_MAXUMUM_LENGTH;
    const uint32_t offset = (uint32_t)signed_offset;
    if (signed_offset < 0 || offset >= window->entries_size) {
        return NULL;
    }
    const size_t index = (window->tail_position + offset) % window->size;
    return &window->entries[index];
}

bool is_window_empty(const Window* window) {
    return window->entries_size == 0;
}

bool is_window_full(const Window* window) {
    return window->entries_size == window->size;
}

void uninitialize_window(Window* window) {
    free(window->entries);
}
