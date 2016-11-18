/*
 * vending_machine_msm.hpp
 *
 *  Created on: Nov 18, 2016
 *      Author: zmij
 */

#ifndef VENDING_MACHINE_MSM_HPP_
#define VENDING_MACHINE_MSM_HPP_

#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <boost/msm/front/functor_row.hpp>
#include <pushkin/meta/callable.hpp>
#include <map>
#include <algorithm>
#include <numeric>

namespace vending_msm {

namespace events {

struct power_on {};
struct power_off {};

struct start_maintenance {
    int             secret;
};
struct end_maintenance {};
struct withdraw_money {};
struct load_goods {
    ::std::size_t   p_no;
    int             amount;
};
struct load_done {};
struct set_price {
    ::std::size_t   p_no;
    float           price;
};

struct money {
    float           amount;
};
struct select_item {
    ::std::size_t   p_no;
};

}  /* namespace events */

struct goods_entry {
    int     amount;
    float   price;
};

using goods_storage = ::std::map<::std::size_t, goods_entry>;

struct vending_def : public ::boost::msm::front::state_machine_def<vending_def> {
    template < typename ... T >
    using and_ = ::psst::meta::and_<T...>;
    template < typename ... T >
    using or_ = ::psst::meta::or_<T...>;
    template < typename T >
    using not_ = ::psst::meta::not_<T>;

    template <typename ...T>
    using state = ::boost::msm::front::state<T...>;
    template <typename ... T>
    using state_machine = ::boost::msm::front::state_machine_def<T...>;
    template <typename ... T>
    using mpl_vector = ::boost::mpl::vector<T...>;

    template <typename ... T>
    using tr = ::boost::msm::front::Row<T...>;
    template <typename ... T>
    using in = ::boost::mpl::vector<T...>;

    using none = ::boost::msm::front::none;

    using vending_fsm = ::boost::msm::back::state_machine<vending_def>;

    //@{
    /** @name Guards */
    struct is_empty {
        template < typename Event, typename FSM, typename SourceState, typename TargetState >
        bool
        operator()(Event const&, FSM const& fsm, SourceState const&, TargetState const&)
        {
            return root_machine(fsm).is_empty();
        }
    };
    struct prices_correct {
        template < typename FSM, typename State >
        bool
        operator()(FSM const& fsm, State const&) const
        {
            return root_machine(fsm).prices_correct();
        }
    };
    struct goods_exist {
        template < typename FSM, typename State, typename Event >
        bool
        operator()(FSM const& fsm, State const&, Event const& evt) const
        {
            return root_machine(fsm).goods_exist(evt.p_no);
        }
    };
    //@}

    struct off : state<> {};
    struct on  : state_machine<on> {
        // Forward declaration
        struct maintenance;
        //@{
        /** @name Guards */
        struct check_secret {
            template < typename FSM, typename State >
            bool
            operator()(FSM const& fsm, State const&, events::start_maintenance const& evt) const
            {
                return root_machine(fsm).secret == evt.secret;
            }
        };
        //@}
        //@{
        /** @name Actions */
        //@}
        //@{
        /** @name Substates */
        struct maintenance : state_machine<maintenance> {
            //@{
            /** @name Guards */
            struct check_amount {
                template < typename FSM, typename State >
                bool
                operator()(FSM const&, State const&, events::load_goods const& goods) const
                {
                    return goods.amount >= 0;
                }
            };
            struct check_price {
                template < typename FSM, typename SourceState, typename TargetState >
                bool
                operator()(events::set_price const& price, FSM const&, SourceState&, TargetState&) const
                {
                    return price.price >= 0;
                }
            };
            //@}
            //@{
            struct set_price {
                template < typename FSM, typename SourceState, typename TargetState >
                void
                operator()(events::set_price const& price, FSM& fsm, SourceState&, TargetState&) const
                {
                    root_machine(fsm).set_price(price.p_no, price.price);
                }
            };
            //@}
            struct internal_transitions : mpl_vector <
                /*  Event                   Action      Guard                               */
                in< events::set_price,      set_price,  and_< goods_exist, check_price >    >,
                in< events::withdraw_money, none,       none                                >
            >{};
            struct idle : state<> {};
            struct loading : state<> {
                template < typename FSM >
                void
                on_enter(events::load_goods const& goods, FSM& fsm) const
                {
                    root_machine(fsm).add_goods(::std::move(goods));
                }
            };

            using initial_state = idle;
            struct transition_table : mpl_vector <
                /*  State       Event                   Next        Action  Guard           */
                tr< idle,       events::load_goods,     loading,    none,   check_amount    >,
                tr< loading,    events::load_done,      idle,       none,   none            >
            > {};
        };
        struct serving : state_machine<serving> {
            //@{
            /** @name Guards */
            struct enough_money {
                template < typename FSM, typename State >
                bool
                operator()(FSM const& fsm, State const& state, events::select_item const& item) const
                {
                    return root_machine(fsm).get_price(item.p_no) <= state.balance;
                }
            };
            //@}
            //@{
            /** @name Actions */
            struct dispense {
                template < typename FSM, typename SourceState, typename TargetState >
                void
                operator()(events::select_item&& item, FSM& fsm, SourceState&, TargetState&) const
                {
                    root_machine(fsm).dispense_product(item.p_no);
                }
            };
            //@}
            //@{
            /** @name Substates */
            struct idle : state<> {};
            struct active : state<> {
                template < typename FSM >
                void
                on_enter(events::money&& money, FSM&)
                {
                    balance += money.amount;
                }
                template < typename FSM >
                void
                on_exit(events::select_item&& item, FSM& fsm)
                {
                    // Subtract balance
                    auto& root = root_machine(fsm);
                    balance -= root.get_price(item.p_no);
                    if (balance > 0) {
                        // Give change
                    }
                }

                float       balance{0};
            };
            //@}
            using initial_state = idle;

            struct transition_table : mpl_vector<
                /*  Start       Event                   Next    Action      Guard                               */
                tr< idle,       events::money,          active, none,       none                                >,
                tr< active,     events::money,          active, none,       none                                >,
                tr< active,     events::select_item,    idle,   dispense,   and_< goods_exist, enough_money >   >
            >{};
        };
        struct out_of_service : state<> {};
        //@}

        using initial_state = serving;
        struct transition_table : mpl_vector <
            /*  Start           Event                       Next            Action  Guard                           */
            /* Default transitions                                                                                  */
            /*-----------------+---------------------------+---------------+-------+--------------------------------*/
            tr< serving,        none,                       out_of_service, none,   is_empty                        >,
            tr< out_of_service, none,                       serving,        none,   not_<is_empty>                  >,
            /*-----------------+---------------------------+---------------+-------+--------------------------------*/
            /* Normal transitions                                                                                   */
            tr< serving,        events::start_maintenance,  maintenance,    none,   check_secret                    >,
            tr< out_of_service, events::start_maintenance,  maintenance,    none,   check_secret                    >,
            tr< maintenance,    events::end_maintenance,    serving,        none,   or_< is_empty, prices_correct > >
        >{};

    };

    using initial_state = off;
    struct transition_table : mpl_vector <
        /*  Start       Event               Next        */
        tr< off,        events::power_on,   on,   none, none>,
        tr< on,         events::power_off,  off,  none, none>
    > {};

    template <typename FSM, typename Event>
    void
    no_transition(Event const&, FSM&, int) {}

    static const int factory_code   = 2147483647;

    vending_def()
        : secret{factory_code}, goods{}, balance{0} {}
    vending_def(int code)
        : secret{code}, goods{}, balance{0} {}
    vending_def(goods_storage const& goods)
        : secret{factory_code}, goods{goods}, balance{0} {}
    vending_def(int code, goods_storage&& goods)
        : secret{code}, goods{::std::move(goods)}, balance{0} {}

    bool
    is_empty() const
    {
        return count() == 0;
    }

    ::std::size_t
    count() const
    {
        return ::std::accumulate(
            goods.begin(), goods.end(), 0ul,
            [](::std::size_t cnt, goods_storage::value_type const& i)
            {
                return cnt + i.second.amount;
            }
        );
    }

    void
    add_goods(events::load_goods&& entry)
    {
        auto f = goods.find(entry.p_no);
        if (f == goods.end()) {
            goods.emplace(entry.p_no, goods_entry{entry.amount, 0});
        } else {
            f->second.amount += entry.amount;
        }
        rebind().process_event(events::load_done{});
    }

    bool
    prices_correct() const
    {
        auto f = ::std::find_if(goods.begin(), goods.end(),
            [](goods_storage::value_type const& i)
            {
                return i.second.price <= 0;
            });
        return f == goods.end();
    }

    bool
    goods_exist(::std::size_t p_no) const
    {
        auto f = goods.find(p_no);
        return f != goods.end();
    }

    void
    set_price(::std::size_t p_no, float price)
    {
        auto f = goods.find(p_no);
        if (f != goods.end()) {
            f->second.price = price;
        }
    }
    float
    get_price(::std::size_t p_no) const
    {
        auto f = goods.find(p_no);
        if (f != goods.end()) {
            return f->second.price;
        }
        return 0;
    }
    void
    dispense_product(::std::size_t p_no)
    {
        auto f = goods.find(p_no);
        if (f != goods.end() && f->second.amount > 0) {
            --f->second.amount;
            balance += f->second.price;
        }
    }
    void
    add_balance(float amount)
    {
        balance += amount;
    }
    void
    clear_balance()
    { balance = 0; }

    vending_fsm&
    rebind()
    { return static_cast<vending_fsm&>(*this); }
    vending_fsm const&
    rebind() const
    { return static_cast<vending_fsm const&>(*this); }

    int             secret;
    goods_storage   goods;
    float           balance;
};

using vending_machine = ::boost::msm::back::state_machine<vending_def>;

}  /* namespace vending_msm */


#endif /* VENDING_MACHINE_MSM_HPP_ */
