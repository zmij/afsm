/**
 * machine_gun.hpp
 *
 *  Created on: Nov 9, 2019
 *      Author: ser-fedorov
 */

#ifndef EXAMPLES_MACHINE_GUN_HPP_
#define EXAMPLES_MACHINE_GUN_HPP_

#include <afsm/fsm.hpp>

#include <functional>
#include <type_traits>

namespace guns::events {

struct trigger_pull {};
struct trigger_release {};

struct reload {};

struct safety_lever_up {};
struct safety_lever_down {};

struct tick {
    std::uint64_t frame = 0;
};

}    // namespace guns::events

namespace guns {

// Guards fwd
struct depleted;
struct firing;
struct want_this_frame;

// Actions fwd
struct emit_bullet;
struct change_magazine;
struct update_safety;

enum class safety_lever { safe, automatic, single };

struct machine_gun_def : afsm::def::state_machine<machine_gun_def> {
    using fsm_type = afsm::state_machine<machine_gun_def>;

    //@{
    /** @name States */
    struct selector : state_machine<selector> {
        //@{
        /** @name Selector substates */
        struct safe : state<safe> {
            static constexpr safety_lever lever = safety_lever::safe;
        };
        struct single : state<single> {
            static constexpr safety_lever lever = safety_lever::single;
        };
        struct automatic : state<automatic> {
            static constexpr safety_lever lever = safety_lever::automatic;
        };
        //@}

        using initial_state = safe;

        // Type aliases to shorten the transition table
        using down = events::safety_lever_down;
        using up   = events::safety_lever_up;

        // clang-format off
        // selector transition table
        using transitions = transition_table<
        /*  State       | Event | Next      | Action        | Guard         */
        tr< safe        , down  , automatic , update_safety , none          >,
        tr< automatic   , up    , safe      , update_safety , not_<firing>  >,
        tr< automatic   , down  , single    , update_safety , not_<firing>  >,
        tr< single      , up    , automatic , update_safety , not_<firing>  >
        >;
        // clang-format on
    };

    struct trigger : state_machine<trigger> {
        //@{
        /** @name Trigger substates */
        struct initial_state : state<initial_state> {};
        struct ready : state<ready> {};
        struct empty : state<empty> {
            template <typename FSM>
            void
            on_enter(afsm::none&&, FSM& fsm)
            {
                root_machine(fsm).ammo_depleted();
            }
            template <typename FSM>
            void
            on_exit(events::reload const&, FSM& fsm)
            {
                root_machine(fsm).ammo_replentished();
            }
        };
        struct fire : state<fire> {};
        //@}

        // Type aliases to shorten the transition table
        using init    = initial_state;
        using pull    = events::trigger_pull;
        using release = events::trigger_release;
        using reload  = events::reload;
        using tick    = events::tick;

        // clang-format off
        // trigger transition table
        using transitions = transition_table<
        /*  State   | Event     | Next      | Action            | Guard                             */
        tr< init    , none      , ready     , none              , not_<depleted>                    >,
        tr< init    , none      , empty     , none              , depleted                          >,
        tr< empty   , reload    , ready     , change_magazine   , none                              >,
        tr< ready   , pull      , fire      , emit_bullet       , not_<in_state<selector::safe>>    >,
        tr< ready   , reload    , ready     , change_magazine   , none                              >,
        tr< fire    , release   , ready     , none              , none                              >,
        tr< fire    , tick      , fire      , emit_bullet       , and_<in_state<selector::automatic>,
                                                                       want_this_frame,
                                                                       not_<depleted>>              >,
        tr< fire    , none      , empty     , none              , depleted                          >
        >;
        // clang-format on
    };
    using orthogonal_regions = type_tuple<trigger, selector>;
    //@}

    using callback        = std::function<void()>;
    using safety_callback = std::function<void(safety_lever)>;

    bool
    empty() const
    {
        return ammo_ == 0;
    }
    int
    ammo() const
    {
        return ammo_;
    }

    void
    set_on_reload(callback cb)
    {
        on_reload_ = cb;
    }

    void
    set_on_emit_bullet(callback cb)
    {
        on_emit_bullet_ = cb;
    }

    void
    set_on_empty(callback cb)
    {
        on_empty_ = cb;
    }

    void
    set_on_safety_change(safety_callback cb)
    {
        on_change_safety_ = cb;
    }

    void
    reload()
    {
        ammo_ = 30;
        if (on_reload_)
            on_reload_();
    }

    void
    emit_bullet()
    {
        --ammo_;
        if (on_emit_bullet_)
            on_emit_bullet_();
    }

    void
    update_safety(safety_lever s)
    {
        if (on_change_safety_) {
            on_change_safety_(s);
        }
    }

    void
    ammo_depleted()
    {
        if (on_empty_) {
            on_empty_();
        }
    }

    void
    ammo_replentished()
    {
        // Effectively do nothing here, it's just for demo purposes
    }

private:
    int ammo_ = 0;

    callback on_emit_bullet_;
    callback on_empty_;
    callback on_reload_;

    safety_callback on_change_safety_;
};

using machine_gun = afsm::state_machine<machine_gun_def>;

//@{
/** @name Guards */
struct depleted {
    template <typename FSM, typename State>
    bool
    operator()(FSM const& fsm, State const&) const
    {
        return root_machine(fsm).empty();
    }
};

struct want_this_frame {
    template <typename FSM, typename State>
    bool
    operator()(FSM const&, State const&, events::tick const& t) const
    {
        return t.frame % 10 == 0;
    }
};

struct firing : machine_gun_def::in_state<machine_gun_def::trigger::fire> {};
//@}

//@{
/** @name Actions */
struct emit_bullet {
    template <typename Event, typename FSM>
    void
    operator()(Event const&, FSM& fsm) const
    {
        root_machine(fsm).emit_bullet();
    }
};

struct change_magazine {
    template <typename Event, typename FSM>
    void
    operator()(Event const&, FSM& fsm) const
    {
        root_machine(fsm).reload();
    }
};

struct update_safety {
    template <typename Event, typename FSM, typename SourceState, typename TargetState>
    void
    operator()(Event const&, FSM& fsm, SourceState&, TargetState&) const
    {
        root_machine(fsm).update_safety(TargetState::lever);
    }
};
//@}

using machine_gun = afsm::state_machine<machine_gun_def>;

constexpr char const*
to_chars(safety_lever v)
{
    switch (v) {
    case safety_lever::safe:
        return "safe";
    case safety_lever::automatic:
        return "auto";
    case safety_lever::single:
        return "sngl";
    }
    return "";
}

}    // namespace guns

#endif /* EXAMPLES_MACHINE_GUN_HPP_ */
