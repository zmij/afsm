/*
 * common_base_test.cpp
 *
 *  Created on: 31 мая 2016 г.
 *      Author: sergey.fedorov
 */

#include "test_observer.hpp"
#include <afsm/fsm.hpp>

#include <gtest/gtest.h>

#include <iostream>

namespace afsm {
namespace test {

struct wake {};
struct wash {};
struct food {};
struct do_work {};

struct alarm {};
struct pillow {};
struct sleep {};

struct human_interface {
    virtual ~human_interface() {}

    virtual void
    work()
        = 0;

    virtual void
    sleep()
        = 0;
};

struct dummy_action {
    template <typename FSM>
    void
    operator()(wash const&, FSM&)
    {
        std::cerr << "Brrr!\n";
    }
    template <typename FSM>
    void
    operator()(do_work const&, FSM&)
    {
        std::cerr << "Enough work!\n";
    }
};

struct sleep_action {
    template <typename FSM>
    void
    operator()(sleep const&, FSM& fsm) const
    {
        if (fsm.fatigue > 0)
            --fsm.fatigue;
    }
};

struct work_action {
    template <typename FSM>
    void
    operator()(do_work const&, FSM& fsm) const
    {
        std::cerr << "Getting tired! " << ++fsm.fatigue << "\n";
    }
};

using common_base_tag = ::afsm::def::tags::common_base<human_interface>;

struct human_def : ::afsm::def::state_machine<human_def, common_base_tag> {
    using fsm_type = ::afsm::state_machine<human_def, none, test_fsm_observer>;

    struct sleeping : state<sleeping> {
        void
        work() override
        {
            std::cerr << "Zzzzzz!\n";
        }
        void
        sleep() override
        {
            std::cerr << "ZzzZ.Zzzz!\n";
        }

        // clang-format off
        using internal_transitions = transition_table <
            in< test::sleep,      sleep_action,     none >
        >;
        // clang-format on
    };

    struct awake : state_machine<awake> {
        using fsm_type = ::afsm::inner_state_machine<awake, human_def::fsm_type>;

        template <typename Event>
        void
        on_enter(Event&&, human_def::fsm_type&)
        {
            std::cerr << "Woken up!\n";
        }

        template <typename Event>
        void
        on_exit(Event&&, human_def::fsm_type&)
        {
            std::cerr << "Going to sleep\n";
        }

        struct is_tired {
            template <typename FSM, typename State>
            bool
            operator()(FSM const& fsm, State const&) const
            {
                std::cerr << "Check tired " << fsm.fatigue << "\n";
                return fsm.fatigue >= 5;
            }
        };

        struct woken_up : state<woken_up> {
            using deferred_events = type_tuple<do_work>;
            void
            work() override
            {
                std::cerr << "Nay!\n";
            }
            void
            sleep() override
            {
                std::cerr << "Nay!\n";
            }

            using internal_transitions = transition_table<in<food, none, none>>;
        };
        struct fresh : state<fresh> {
            void
            work() override
            {
                std::cerr << "OK!\n";
            }
            void
            sleep() override
            {
                std::cerr << "Nay!\n";
            }

            using internal_transitions = transition_table<in<do_work, work_action, not_<is_tired>>>;
        };
        struct tired : state<tired> {
            void
            work() override
            {
                std::cerr << "Noooo!\n";
            }
            void
            sleep() override
            {
                std::cerr << "Sooner the better!\n";
            }
        };
        struct sleepy : state<sleepy> {
            void
            work() override
            {
                std::cerr << "Noooo...\n";
            }
            void
            sleep() override
            {
                std::cerr << "Yaaawn!\n";
            }
        };

        using initial_state = woken_up;
        // clang-format off
        using transitions = transition_table<
            tr< woken_up,   wash,       fresh,      dummy_action                >,
            tr< fresh,      do_work,    tired,      dummy_action,   is_tired    >,
            tr< tired,      food,       sleepy,     none                        >
        >;
        // clang-format on

        awake() : fatigue{0} { std::cerr << "Construct awake\n"; }
        fsm_type&
        fsm()
        {
            return static_cast<fsm_type&>(*this);
        }
        void
        work() override
        {
            fsm().current_state_base().work();
        }
        void
        sleep() override
        {
            fsm().current_state_base().sleep();
        }

        int fatigue = 0;
    };

    using initial_state = sleeping;
    // clang-format off
    using transitions = transition_table <
        tr< sleeping,   alarm,      awake    >,
        tr< awake,      pillow,     sleeping >
    >;
    // clang-format ofn

    fsm_type&
    fsm()
    {
        return static_cast<fsm_type&>(*this);
    }

    void
    work() override
    {
        fsm().current_state_base().work();
    }
    void
    sleep() override
    {
        fsm().current_state_base().sleep();
    }

    int fatigue = 0;
};

using human_fsm = ::afsm::state_machine<human_def, none, test_fsm_observer>;

TEST(FSM, CommonBase)
{
    using afsm::actions::event_process_result;
    human_fsm hfsm;
    hfsm.make_observer("human_def");

    EXPECT_TRUE(hfsm.is_in_state< human_fsm::sleeping >());
    hfsm.work();
    hfsm.sleep();
    EXPECT_TRUE(hfsm.is_in_state< human_fsm::sleeping >());

    EXPECT_EQ(event_process_result::process, hfsm.process_event(alarm{}));
    EXPECT_TRUE(hfsm.is_in_state< human_fsm::awake >());
    EXPECT_TRUE(hfsm.is_in_state< human_fsm::awake::woken_up >());

    hfsm.work();
    hfsm.sleep();
    EXPECT_EQ(event_process_result::defer, hfsm.process_event(do_work{}));
    EXPECT_EQ(event_process_result::defer, hfsm.process_event(do_work{}));
    EXPECT_EQ(event_process_result::defer, hfsm.process_event(do_work{}));
    EXPECT_EQ(event_process_result::process, hfsm.process_event(wash{}));
    hfsm.work();
    hfsm.sleep();
    EXPECT_EQ(event_process_result::process_in_state, hfsm.process_event(do_work{}));
    EXPECT_EQ(event_process_result::process_in_state, hfsm.process_event(do_work{}));
    EXPECT_EQ(event_process_result::process, hfsm.process_event(do_work{}));
    hfsm.work();
    hfsm.sleep();
}

} // namespace test
}  /* namespace afsm */
