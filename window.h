/**
 * Piotr Dobiech 316625
 */

#pragma once

#include "transport.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct {
    struct timeval timeout_time;
    uint32_t start;
    // każdy pakiet ma 1000 bajtów, a ostatni po prostu zapiszemy do pliku krótszy
    // uint16_t length;
    // add state to indicate if it is waiting, received, written etc

    uint8_t data_buffer[PACKET_DATA_MAXUMUM_LENGTH];
} WindowEntry;

typedef struct {
    WindowEntry* entries;
    size_t size;
    size_t entries_size;
    size_t tail_position;
} Window;

typedef struct {
    Window* window;
    size_t counter;
    size_t index;
} WindowIterator;

void initialize_window(const size_t window_size, Window* window);

void set_timeout_time(WindowEntry* entry, const struct timeval* now);

void push_window_entry(Window* window, const uint32_t start, const struct timeval* now);

void pull_window_entry(Window* window);

WindowEntry* peek_window_entry(Window* window);

void create_window_iterator(Window* window, WindowIterator* iterator);

void pull_window_iterator(WindowIterator* iterator);

WindowEntry* iterate_window(WindowIterator* iterator);

WindowEntry* get_window_entry(Window* window, const uint32_t start);

bool is_window_empty(const Window* window);

bool is_window_full(const Window* window);

void uninitialize_window(Window* window);
