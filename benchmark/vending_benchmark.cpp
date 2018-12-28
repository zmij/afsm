/*
 * vending_benchmark.cpp
 *
 *  Created on: Nov 18, 2016
 *      Author: zmij
 */

#include "vending_machine.hpp"

#include <benchmark/benchmark.h>

namespace vending {

void
AFSM_ConstructDefault(::benchmark::State& state)
{
    while (state.KeepRunning()) {
        vending_machine vm;
    }
}

void
AFSM_ConstructWithData(::benchmark::State& state)
{
    while (state.KeepRunning()) {
        // clang-format off
        vending_machine vm{ goods_storage{
            { 0, { 10, 15.0f } },
            { 1, { 100, 5.0f } }
        }};
        // clang-format on
    }
}

void
AFSM_ProcessSingleEvent(::benchmark::State& state)
{
    // clang-format off
    vending_machine vm{ goods_storage{
        { 0, { 10, 15.0f } },
        { 1, { 100, 5.0f } }
    }};
    // clang-format on
    vm.process_event(events::power_on{});
    while (state.KeepRunning()) {
        vm.process_event(events::money{100});
    }
}

void
AFSM_OnOffEmpty(::benchmark::State& state)    // With a default transition
{
    vending_machine vm;
    while (state.KeepRunning()) {
        vm.process_event(events::power_on{});
        vm.process_event(events::power_off{});
    }
}

void
AFSM_OnOffLoaded(::benchmark::State& state)    // Without a default transition
{
    // clang-format off
    vending_machine vm{ goods_storage{
        { 0, { 10, 15.0f } },
        { 1, { 100, 5.0f } }
    }};
    // clang-format on
    while (state.KeepRunning()) {
        vm.process_event(events::power_on{});
        vm.process_event(events::power_off{});
    }
}

void
AFSM_BuyItem(::benchmark::State& state)
{
    // clang-format off
    vending_machine vm{ goods_storage{
        { 0, { 1000000000, 15.0f } },
        { 1, { 1000000000, 5.0f } }
    }};
    // clang-format on
    vm.process_event(events::power_on{});
    while (state.KeepRunning()) {
        vm.process_event(events::money{3});
        vm.process_event(events::money{3});
        vm.process_event(events::select_item{1});
    }
}

BENCHMARK(AFSM_ConstructDefault);
BENCHMARK(AFSM_ConstructWithData);
BENCHMARK(AFSM_ProcessSingleEvent);
BENCHMARK(AFSM_OnOffEmpty);
BENCHMARK(AFSM_OnOffLoaded);
BENCHMARK(AFSM_BuyItem);

} /* namespace vending */

BENCHMARK_MAIN();
