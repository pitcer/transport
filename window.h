/**
 * Piotr Dobiech 316625
 */

#pragma once

#include "transport.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct {
    uint32_t start;
    uint16_t length;
    // replace with timestamp struct
    uint16_t elapsed_ticks;
    // add state to indicate if it is waiting, received, written etc
    uint8_t data_buffer[PACKET_DATA_MAXUMUM_LENGTH];
} WindowEntry;

typedef struct {
    size_t size;
    WindowEntry* entries;
    size_t entries_size;
    size_t head_position;
    size_t tail_position;
} Window;

typedef struct {
    Window* window;
    size_t index;
} WindowIterator;

void initialize_window(const size_t window_size, Window* window);

void push_window_entry(Window* window, const uint32_t start, const uint16_t length);

void pull_window_entry(Window* window);

void peek_window_entry(const Window* window, WindowEntry* result_entry);

WindowEntry* peek_mut_window_entry(Window* window);

void create_window_iterator(Window* window, WindowIterator* iterator);

bool iterate_window(WindowIterator* iterator, WindowEntry* result_entry);

WindowEntry* iterate_mut_window(WindowIterator* iterator);

bool cyclic_iterate_window(WindowIterator* iterator, WindowEntry* result_entry);

WindowEntry* cyclic_iterate_mut_window(WindowIterator* iterator);

WindowEntry* get_window_entry(Window* window, const uint32_t start);

bool is_window_empty(const Window* window);

bool is_window_full(const Window* window);

void uninitialize_window(Window* window);
