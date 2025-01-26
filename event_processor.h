#ifndef EVENT_PROCESOR_H_
#define EVENT_PROCESOR_H_

#include <assert.h>
// #include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void ep_init(void);
void ep_new_callback(const char *name,
                     bool (*trigger_predicate)(void));
void ep_attach(const char *cb_name,
               void *observer,
               bool (*observer_predicate)(void *, ...),
               void (*observer_notify)(void *, ...));
void ep_destroy(void);

#endif
#define EVENT_PROCESOR_H_IMPLEMENTATION
#ifdef EVENT_PROCESOR_H_IMPLEMENTATION

typedef struct Observer
{
    void *observer;
    void (*observer_notify)(void *, ...);
    bool (*observer_predicate)(void *, ...);
} Observer;

typedef struct CallbackList
{
    int count;
    int capacity;
    const char *name;
    bool (*trigger_predicate)(void);
    Observer *observers;

} CallbackList;

typedef struct EventProcesor
{
    int count;
    int capacity;
    CallbackList *callbacks;

} EventProcesor;

EventProcesor ep = {0};

void ep_init()
{
    ep.count = 0;
    ep.capacity = 8;
    ep.callbacks = (CallbackList *)malloc(sizeof(CallbackList) * ep.capacity); // TODO: Static and Dynamic allocation.
}

int _get_callback_index(const char *name)
{
    for (int idx = 0; idx < ep.count; idx++)
    {
        if (strcmp(ep.callbacks[idx].name, name) == 0)
            return idx;
    }
    return -1;
}

void ep_new_callback(const char *name, bool (*trigger_predicate)(void))
{
    assert(ep.capacity > ep.count && "Callbacks capacity exausted!"); // TODO: Dynamic allocation.
    CallbackList cb = {0};
    cb.count = 0;
    cb.capacity = 128;
    cb.name = name;
    cb.observers = (Observer *)malloc(sizeof(Observer) * cb.capacity);
    cb.trigger_predicate = trigger_predicate;
    ep.callbacks[ep.count++] = cb;
}

void ep_attach(const char *cb_name, void *observer, bool (*observer_predicate)(void *, ...), void (*observer_notify)(void *, ...))
{
    int cb_idx = _get_callback_index(cb_name);
    assert(cb_idx != -1 && "No callback registered with that name!");

    CallbackList *cb = &ep.callbacks[cb_idx]; // TODO: make dynamic alloc...

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
        if (cb->trigger_predicate())
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
    for (int cidx = 0; cidx < ep.count; cidx++)
    {
        CallbackList *cb = &ep.callbacks[cidx];
        free(cb->observers);
    }
    free(ep.callbacks);
}



#endif