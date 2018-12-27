/*
 * vending_starter.cpp
 *
 *  Created on: Nov 15, 2016
 *      Author: zmij
 */

#include <afsm/fsm.hpp>

#include <algorithm>
#include <iostream>
#include <map>

namespace vending {

namespace events {

struct power_on {};
struct power_off {};

struct start_maintenance {};
struct end_maintenance {};
struct out_of_goods {};

struct set_price {
    ::std::size_t p_no;
    float         price;
};
struct withdraw_money {};

} /* namespace events */

struct goods_entry {
    int   amount;
    float price;
};

using goods_storage = ::std::map<::std::size_t, goods_entry>;

struct vending_def : ::afsm::def::state_machine<vending_def> {
    //@{
    /** @name Substates definition */
    struct off : state<off> {};
    struct on : state_machine<on> {
        //@{
        /** @name Substates definition */
        struct serving : state<serving> {};
        struct maintenance : state<maintenance> {
            //@{
            /** @name Actions */
            struct set_price {
                template <typename FSM, typename SourceState, typename TargetState>
                void
                operator()(events::set_price&& price, FSM& fsm, SourceState&, TargetState&) const
                {
                    root_machine(fsm).set_price(price.p_no, price.price);
                }
            };
            struct clear_balance {
                template <typename FSM, typename SourceState, typename TargetState>
                void
                operator()(events::withdraw_money&&, FSM& fsm, SourceState&, TargetState&) const
                {
                    root_machine(fsm).clear_balance();
                }
            };
            //@}
            /** @name In-state transitions */
            // clang-format off
            using internal_transitions = transition_table<
                /*  Event                   Action          */
                in< events::set_price,      set_price       >,
                in< events::withdraw_money, clear_balance   >
            >;
            // clang-format on
        };
        struct out_of_service : state<out_of_service> {};
        //@}

        /** Initial state machine state */
        using initial_state = serving;
        /** State transition table */
        // clang-format off
        using transitions = transition_table<
            /*  Start           Event                       Next            */
            tr< serving,        events::start_maintenance,  maintenance     >,
            tr< serving,        events::out_of_goods,       out_of_service  >,
            tr< out_of_service, events::start_maintenance,  maintenance     >,
            tr< maintenance,    events::end_maintenance,    serving         >
        >;
        // clang-format on
    };
    //@}

    /** Initial state machine state */
    using initial_state = off;

    /** State transition table */
    // clang-format off
    using transitions = transition_table <
        /*  Start   Event               Next    */
        tr< off,    events::power_on,   on      >,
        tr< on,     events::power_off,  off     >
    >;
    // clang-format on

    /** Default constructor */
    vending_def() : goods{}, balance{0} {}
    /** Constructor, moving goods container to data member */
    vending_def(goods_storage&& g) : goods{::std::move(g)}, balance{0} {}

    /**
     * Set price for an item
     * @param p_no
     * @param price
     */
    void
    set_price(::std::size_t p_no, float price)
    {
        auto f = goods.find(p_no);
        if (f != goods.end()) {
            f->second.price = price;
        }
    }
    void
    clear_balance()
    {
        balance = 0;
    }

    goods_storage goods;
    float         balance;
};

using vending_sm = ::afsm::state_machine<vending_def>;

::std::ostream&
operator<<(::std::ostream& os, vending_sm const& val)
{
    ::std::ostream::sentry s(os);
    if (s) {
        os << (val.is_in_state<vending_sm::on>() ? "ON" : "OFF");
    }
    return os;
}

void
use()
{
    vending_sm vm{goods_storage{{1, {10, 15.0f}}, {5, {100, 5.0f}}}};
    ::std::cout << "Machine is " << vm << "\n";
    vm.process_event(events::power_on{});
    ::std::cout << "Machine is " << vm << "\n";
    vm.process_event(events::power_off{});
    ::std::cout << "Machine is " << vm << "\n";

    vm.process_event(events::power_on{});
    ::std::cout << "Machine is " << vm << "\n";
    vm.process_event(events::start_maintenance{});
    if (vm.is_in_state<vending_sm::on::maintenance>()) {
        ::std::cout << "Machine is being maintained\n";
    } else {
        ::std::cout << "Something went wrong\n";
    }
    vm.process_event(events::power_off{});
    ::std::cout << "Machine is " << vm << "\n";
    vm.process_event(events::power_on{});
    ::std::cout << "Machine is " << vm << "\n";
    if (vm.is_in_state<vending_sm::on::maintenance>()) {
        ::std::cout << "Something went wrong\n";
    } else {
        ::std::cout << "Machine is not being maintained\n";
    }
}

} /* namespace vending */

int
main(int, char*[])
try {
    vending::use();
    return 0;
} catch (::std::exception const& e) {
    ::std::cerr << "Exception: " << e.what() << "\n";
    return 1;
} catch (...) {
    ::std::cerr << "Unexpected exception\n";
    return 2;
}
