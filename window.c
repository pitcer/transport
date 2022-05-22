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
    window->head_position = 0;
    window->tail_position = 0;
}

void push_window_entry(Window* window, const uint32_t start, const uint16_t length) {
    const size_t head_position = window->head_position;
    window->head_position = (head_position + 1) % window->size;
    window->entries_size++;
    debug_assert(window->entries_size <= window->size);

    WindowEntry* entry = &window->entries[head_position];
    entry->start = start;
    entry->length = length;
    entry->elapsed_ticks = 0;
}

void pull_window_entry(Window* window) {
    debug_assert(window->entries_size >= 1);
    const size_t tail_position = window->tail_position;
    window->tail_position = (tail_position + 1) % window->size;
    window->entries_size--;
}

static void copy_window_entry(const WindowEntry* source, WindowEntry* destination) {
    destination->start = source->start;
    destination->length = source->length;
    destination->elapsed_ticks = source->elapsed_ticks;
}

void peek_window_entry(const Window* window, WindowEntry* result_entry) {
    const WindowEntry* entry = &window->entries[window->tail_position];
    copy_window_entry(entry, result_entry);
}

WindowEntry* peek_mut_window_entry(Window* window) {
    return &window->entries[window->tail_position];
}

static inline void reset_iterator(WindowIterator* iterator) {
    iterator->index = iterator->window->tail_position;
}

static inline WindowEntry* iterate_next(WindowIterator* iterator) {
    WindowEntry* entry = &iterator->window->entries[iterator->index];
    iterator->index = (iterator->index + 1) % iterator->window->size;
    return entry;
}

void create_window_iterator(Window* window, WindowIterator* iterator) {
    iterator->window = window;
    reset_iterator(iterator);
}

bool window_iterator_has_next(WindowIterator* iterator) {
    return iterator->index != iterator->window->head_position;
}

bool iterate_window(WindowIterator* iterator, WindowEntry* result_entry) {
    const WindowEntry* entry = iterate_mut_window(iterator);
    if (entry == NULL) {
        return false;
    }
    copy_window_entry(entry, result_entry);
    return true;
}

WindowEntry* iterate_mut_window(WindowIterator* iterator) {
    if (!window_iterator_has_next(iterator)) {
        return NULL;
    }
    return iterate_next(iterator);
}

bool cyclic_iterate_window(WindowIterator* iterator, WindowEntry* result_entry) {
    const WindowEntry* entry = cyclic_iterate_mut_window(iterator);
    copy_window_entry(entry, result_entry);
    return true;
}

WindowEntry* cyclic_iterate_mut_window(WindowIterator* iterator) {
    if (!window_iterator_has_next(iterator)) {
        reset_iterator(iterator);
    }
    return iterate_next(iterator);
}

WindowEntry* find_mut_window_entry(
    WindowIterator* iterator, const uint32_t start, const uint16_t length) {

    if (is_window_empty(iterator->window)) {
        return NULL;
    }

    WindowEntry* first_entry = cyclic_iterate_mut_window(iterator);
    if (first_entry == NULL) {
        return NULL;
    }
    if (first_entry->start == start && first_entry->length == length) {
        return first_entry;
    }

    WindowEntry* entry;
    while ((entry = cyclic_iterate_mut_window(iterator)) != NULL && entry != first_entry) {
        if (first_entry->start == start && first_entry->length == length) {
            return first_entry;
        }
    }

    return NULL;
}

WindowEntry* get_window_entry(Window* window, const uint32_t start) {
    if (is_window_empty(window)) {
        return NULL;
    }

    WindowEntry* first_entry = peek_mut_window_entry(window);
    // Assume that every window entry has start being a multiple of PACKET_DATA_MAXUMUM_LENGTH.
    const uint32_t offset = (start - first_entry->start) / PACKET_DATA_MAXUMUM_LENGTH;
    if (offset >= window->entries_size) {
        return NULL;
    }
    const size_t index = (window->tail_position + offset) % window->size;
    return &window->entries[index];
}

bool is_window_empty(const Window* window) {
    debug_assert((window->size == 1 || window->head_position == window->tail_position)
        == (window->entries_size == 0));
    return window->entries_size == 0;
}

bool is_window_full(const Window* window) {
    debug_assert(((window->head_position + 1) % window->size == window->tail_position)
        == (window->entries_size == window->size));
    return window->entries_size == window->size;
}

void uninitialize_window(Window* window) {
    free(window->entries);
}
