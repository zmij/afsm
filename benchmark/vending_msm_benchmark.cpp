/*
 * vending_msm_benchmark.cpp
 *
 *  Created on: Nov 18, 2016
 *      Author: zmij
 */

#include <benchmark/benchmark_api.h>

#include "vending_machine_msm.hpp"

namespace vending_msm {

void
MSM_ConstructDefault(::benchmark::State& state)
{
    while (state.KeepRunning()) {
        vending_machine vm;
    }
}

void
MSM_ConstructWithData(::benchmark::State& state)
{
    while (state.KeepRunning()) {
        vending_machine vm{ goods_storage{
            { 0, { 10, 15.0f } },
            { 1, { 100, 5.0f } }
        }};
    }
}

void
MSM_ProcessSingleEvent(::benchmark::State& state)
{
    vending_machine vm;
    while (state.KeepRunning()) {
        vm.process_event(events::power_on{});
    }
}

void
MSM_OnOffEmpty(::benchmark::State& state) // With a default transition
{
    vending_machine vm;
    while(state.KeepRunning()) {
        vm.process_event(events::power_on{});
        vm.process_event(events::power_off{});
    }
}

void
MSM_OnOffLoaded(::benchmark::State& state) // Without a default transition
{
    vending_machine vm{ goods_storage{
        { 0, { 10, 15.0f } },
        { 1, { 100, 5.0f } }
    }};
    while(state.KeepRunning()) {
        vm.process_event(events::power_on{});
        vm.process_event(events::power_off{});
    }
}

void
MSM_BuyItem(::benchmark::State& state)
{
    vending_machine vm{ goods_storage{
        { 0, { 1000000000, 15.0f } },
        { 1, { 1000000000, 5.0f } }
    }};
    vm.process_event(events::power_on{});
    while(state.KeepRunning()) {
        vm.process_event(events::money{3});
        vm.process_event(events::money{3});
        vm.process_event(events::select_item{1});
    }
}

BENCHMARK(MSM_ConstructDefault);
BENCHMARK(MSM_ConstructWithData);
BENCHMARK(MSM_ProcessSingleEvent);
BENCHMARK(MSM_OnOffEmpty);
BENCHMARK(MSM_OnOffLoaded);
BENCHMARK(MSM_BuyItem);


}  /* namespace vending_msm */

BENCHMARK_MAIN()
