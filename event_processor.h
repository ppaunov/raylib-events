#ifndef EVENT_PROCESSOR_H_
#define EVENT_PROCESSOR_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Use #define STATIC_EVENT_PROCESSOR for static behaviour.
void ep_init(void);
void ep_new_event(const char *event_name,
                  bool (*event_predicate)(void));
void ep_attach(const char *event_name,
               void *observer,
               bool (*observer_predicate)(void *, ...),
               void (*observer_notify)(void *, ...));
void ep_destroy(void);

#ifdef RAYLIB_H
// Raylib 5.5 specific functions.

bool mouse_left_click_predicate(void)
{
    return IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

bool mouse_right_click_predicate(void)
{
    return IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
}
#endif // RAYLIB_H

#endif // EVENT_PROCESSOR_H_

#define EVENT_PROCESSOR_H_IMPLEMENTATION
#ifdef EVENT_PROCESSOR_H_IMPLEMENTATION

typedef struct ObserverStruct
{
    void *observer;
    void (*observer_notify)(void *, ...);
    bool (*observer_predicate)(void *, ...);
} Observer;

#ifdef STATIC_EVENT_PROCESSOR
// REDIFINE FOR MORE

#ifndef OBSERVERS_CAPACITY
#define OBSERVERS_CAPACITY 32
#endif

#ifndef CALLBACKS_CAPACITY
#define CALLBACKS_CAPACITY 32
#endif

#endif

typedef struct CallbackListStruct
{
    int count;
    int capacity;
    const char *name;
    bool (*event_predicate)(void);
#ifndef STATIC_EVENT_PROCESSOR
    Observer *observers;

#else
    Observer observers[OBSERVERS_CAPACITY];
#endif

} CallbackList;

typedef struct EventProcesorStruct
{
    int count;
    int capacity;
#ifndef STATIC_EVENT_PROCESSOR
    CallbackList *callbacks;
#else
    CallbackList callbacks[CALLBACKS_CAPACITY];

#endif

} EventProcesor;

static EventProcesor ep = {0};

void ep_init()
{
    ep.count = 0;
#ifndef STATIC_EVENT_PROCESSOR
    ep.capacity = 8;
    ep.callbacks = (CallbackList *)malloc(sizeof(CallbackList) * ep.capacity);
#else
    ep.capacity = CALLBACKS_CAPACITY;

#endif
}

static int _get_callback_index(const char *name)
{
    for (int idx = 0; idx < ep.count; idx++)
    {
        if (strcmp(ep.callbacks[idx].name, name) == 0)
            return idx;
    }
    return -1;
}

void ep_new_event(const char *event_name, bool (*event_predicate)(void))
{
#ifdef STATIC_EVENT_PROCESSOR

    assert(ep.capacity > ep.count && "Events capacity exausted!"); // TODO: Dynamic allocation.
#else
    if (ep.count >= ep.capacity)
    {
        printf("INFO: Events capacity exausted! Reallocating...\n");
        ep.capacity *= 2;
        ep.callbacks = (CallbackList *)realloc(ep.callbacks, sizeof(CallbackList) * ep.capacity);
    }
#endif
    CallbackList cb = {0};
    cb.count = 0;
    cb.name = event_name;

#ifdef STATIC_EVENT_PROCESSOR
    cb.capacity = OBSERVERS_CAPACITY;
#else
    cb.capacity = 2;
    cb.observers = (Observer *)malloc(sizeof(Observer) * cb.capacity);
#endif

    cb.event_predicate = event_predicate;
    ep.callbacks[ep.count++] = cb;
}

void ep_attach(const char *event_name, void *observer, bool (*observer_predicate)(void *, ...), void (*observer_notify)(void *, ...))
{
    int cb_idx = _get_callback_index(event_name);
    assert(cb_idx != -1 && "No event registered with that name!");

    CallbackList *cb = &ep.callbacks[cb_idx];

#ifndef STATIC_EVENT_PROCESSOR
    if (cb->count >= cb->capacity)
    {
        cb->capacity *= 2;
        printf("INFO: Observers capacity of %s exausted! Reallocating to new size %d...\n", event_name, cb->capacity);
        cb->observers = (Observer *)realloc(cb->observers, sizeof(Observer) * cb->capacity);
    }
#else

    assert(cb->count < cb->capacity && "Exausted observers capacity");
#endif

    cb->observers[cb->count] = (Observer){
        .observer = observer,
        .observer_predicate = observer_predicate,
        .observer_notify = observer_notify};
    cb->count++;
}

void ep_porcess_events()
{
    for (int cidx = 0; cidx < ep.count; cidx++)
    {
        CallbackList *cb = &ep.callbacks[cidx];
        if (cb->event_predicate())
        {

            for (int obs_idx = 0; obs_idx < cb->count; obs_idx++)
            {
                Observer *obs = &cb->observers[obs_idx];
                if (obs->observer_predicate(obs->observer))
                {
                    obs->observer_notify(obs->observer);
                }
            }
        }
    }
}

// void ep_dettach(void *observer)
// {
//     for (int lidx = 0; lidx < ep.count; lidx++)
//     {
//         if (ep.observers[lidx].observer == observer)
//         {
//             ep.observers[lidx] = (Observer){0};
//         }
//     }
// }

void ep_destroy()
{
#ifndef STATIC_EVENT_PROCESSOR
    for (int cidx = 0; cidx < ep.count; cidx++)
    {
        CallbackList *cb = &ep.callbacks[cidx];
        free(cb->observers);
    }
    ep.count = 0;
    free(ep.callbacks);
#endif
}

#endif // EVENT_PROCESSOR_H_IMPLEMENTATION