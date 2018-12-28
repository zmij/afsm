/*
 * defer_benchmark.cpp
 *
 *  Created on: Dec 21, 2016
 *      Author: zmij
 */

#include <afsm/fsm.hpp>

#include <benchmark/benchmark.h>

namespace afsm {
namespace bench {

namespace events {

struct a_to_b {};
struct b_to_a {};
struct b_to_c {};
struct c_to_a {};
struct c_to_b {};

} /* namespace events */

struct defer_fsm_def : ::afsm::def::state_machine_def<defer_fsm_def> {

    struct state_a : state<state_a> {};
    struct state_b : state<state_b> {
        using deferred_events = type_tuple<events::a_to_b>;
    };
    struct state_c : state<state_c> {
        using deferred_events = type_tuple<events::a_to_b>;
    };

    using initial_state = state_a;

    // clang-format off
    using transitions = transition_table<
        tr< state_a, events::a_to_b, state_b >,
        tr< state_b, events::b_to_c, state_c >,
        tr< state_b, events::b_to_a, state_a >,
        tr< state_c, events::c_to_a, state_a >,
        tr< state_c, events::c_to_b, state_b >
    >;
    // clang-format on
};

using defer_fsm = ::afsm::state_machine<defer_fsm_def>;

namespace {

void
enqueue_events(defer_fsm& fsm, int n)
{
    for (int i = 0; i < n; ++i) {
        fsm.process_event(events::a_to_b{});
    }
}

} /* namespace  */

void
DeferNoDefer(::benchmark::State& state)
{
    while (state.KeepRunning()) {
        defer_fsm fsm;
        fsm.process_event(events::a_to_b{});
    }
}

void
DeferReject(::benchmark::State& state)
{
    while (state.KeepRunning()) {
        defer_fsm fsm;
        fsm.process_event(events::a_to_b{});
    }
}

void
DeferEnqueue(::benchmark::State& state)
{
    defer_fsm fsm;

    fsm.process_event(events::a_to_b{});

    while (state.KeepRunning()) {
        ::benchmark::DoNotOptimize(fsm.process_event(events::a_to_b{}));
    }
}

void
DeferIgnore(::benchmark::State& state)
{
    while (state.KeepRunning()) {
        state.PauseTiming();
        defer_fsm fsm;
        fsm.process_event(events::a_to_b{});
        // Enqueue N events
        enqueue_events(fsm, state.range(0));
        state.ResumeTiming();
        fsm.process_event(events::b_to_c{});
    }
    state.SetComplexityN(state.range(0));
}

void
DeferProcessOne(::benchmark::State& state)
{
    while (state.KeepRunning()) {
        state.PauseTiming();
        defer_fsm fsm;
        fsm.process_event(events::a_to_b{});
        // Enqueue N events
        enqueue_events(fsm, state.range(0));
        state.ResumeTiming();
        fsm.process_event(events::b_to_a{});
    }
    state.SetComplexityN(state.range(0));
}

BENCHMARK(DeferNoDefer);
BENCHMARK(DeferReject);
BENCHMARK(DeferEnqueue);
BENCHMARK(DeferIgnore)->RangeMultiplier(10)->Range(1, 100000)->Complexity();
BENCHMARK(DeferProcessOne)->RangeMultiplier(10)->Range(1, 100000)->Complexity();

} /* namespace bench */
} /* namespace afsm */
