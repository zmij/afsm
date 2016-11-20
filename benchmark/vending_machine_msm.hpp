/*
 * vending_machine_msm.hpp
 *
 *  Created on: Nov 18, 2016
 *      Author: zmij
 */

#ifndef VENDING_MACHINE_MSM_HPP_
#define VENDING_MACHINE_MSM_HPP_

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wunused-variable"

#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <boost/msm/front/functor_row.hpp>
#include <boost/msm/front/euml/operator.hpp>

#pragma clang diagnostic pop

#include <pushkin/meta/callable.hpp>
#include <map>
#include <algorithm>
#include <numeric>
#include <iostream>

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
    using back_machine = ::boost::msm::back::state_machine<T...>;
    template <typename ... T>
    using mpl_vector = ::boost::mpl::vector<T...>;

    template <typename ... T>
    using tr = ::boost::msm::front::Row<T...>;
    template < typename ... T >
    using in = ::boost::msm::front::Internal< T ... >;

    using none = ::boost::msm::front::none;

    using vending_fsm = ::boost::msm::back::state_machine<vending_def>;

    //@{
    /** @name Guards */
    struct is_empty {
        template < typename Event, typename FSM, typename SourceState, typename TargetState >
        bool
        operator()(Event const&, FSM& fsm, SourceState&, TargetState&)
        {
            return root_machine(fsm).is_empty();
        }
    };
    struct prices_correct {
        template < typename Event, typename FSM, typename SourceState, typename TargetState >
        bool
        operator()(Event const&, FSM& fsm, SourceState&, TargetState&) const
        {
            return root_machine(fsm).prices_correct();
        }
    };
    struct goods_exist {
        template < typename Event, typename FSM, typename SourceState, typename TargetState >
        bool
        operator()(Event const& evt, FSM& fsm, SourceState&, TargetState&) const
        {
            return root_machine(fsm).goods_exist(evt.p_no);
        }
    };
    //@}

    struct off : state<> {
        using flag_list = mpl_vector<off>;
    };
    struct on_ : state_machine<on_> {
        using on = back_machine<on_>;
        using flag_list = mpl_vector<on>;
        //@{
        /** @name Guards */
        struct check_secret {
            template < typename Event, typename FSM, typename SourceState, typename TargetState >
            bool
            operator()(Event const& evt, FSM& fsm, SourceState&, TargetState&) const
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
        struct maintenance_ : state_machine<maintenance_> {
            using maintenance = back_machine<maintenance_>;
            using flag_list = mpl_vector<maintenance>;
            //@{
            /** @name Guards */
            struct check_amount {
                template < typename FSM, typename SourceState, typename TargetState >
                bool
                operator()(events::load_goods const& goods, FSM const&, SourceState&, TargetState&) const
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
            struct internal_transition_table : mpl_vector <
                /*  Event                   Action      Guard                               */
                in< events::set_price,      set_price,  and_< goods_exist, check_price >    >,
                in< events::withdraw_money, none,       none                                >
            >{};
            struct idle : state<> {
                using flag_list = mpl_vector<idle>;
            };
            struct loading : state<> {
                template < typename FSM >
                void
                on_entry(events::load_goods const& goods, FSM& fsm)
                {
                    root_machine(fsm).add_goods(goods);
                }
                template < typename Event, typename FSM >
                void
                on_entry(Event const&, FSM&) {}
            };

            using initial_state = idle;
            struct transition_table : mpl_vector <
                /*  State       Event                   Next        Action  Guard           */
                tr< idle,       events::load_goods,     loading,    none,   check_amount    >,
                tr< loading,    events::load_done,      idle,       none,   none            >
            > {};

            template < typename Event, typename FSM >
            void
            on_entry(Event const&, FSM& fsm)
            {
                enclosing = &fsm;
            }
            on*    enclosing;
        };
        using maintenance = back_machine<maintenance_>;

        struct serving_ : state_machine<serving_> {
            using serving = back_machine<serving_>;
            using flag_list = mpl_vector<serving>;
            //@{
            /** @name Guards */
            struct enough_money {
                template < typename FSM, typename SourceState, typename TargetState >
                bool
                operator()(events::select_item const& item, FSM const& fsm,
                        SourceState& state, TargetState&) const
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
                operator()(events::select_item const& item, FSM& fsm, SourceState&, TargetState&) const
                {
                    root_machine(fsm).dispense_product(item.p_no);
                }
            };
            //@}
            //@{
            /** @name Substates */
            struct idle : state<> {
                using flag_list = mpl_vector<idle>;
            };
            struct active : state<> {
                using flag_list = mpl_vector<active>;
                template < typename Event, typename FSM >
                void
                on_entry(Event const&, FSM&) {}
                template < typename FSM >
                void
                on_entry(events::money const& money, FSM&)
                {
                    balance += money.amount;
                }
                template < typename Event, typename FSM >
                void
                on_exit(Event const&, FSM&) {}
                template < typename FSM >
                void
                on_exit(events::select_item const& item, FSM& fsm)
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
            template < typename Event, typename FSM >
            void
            on_entry(Event const&, FSM& fsm)
            {
                enclosing = &fsm;
            }
            on*    enclosing;
        };
        using serving = back_machine<serving_>;

        struct out_of_service : state<> {
            using flag_list = mpl_vector<out_of_service>;
//            template < typename Event, typename FSM >
//            void
//            on_entry(Event const&, FSM&)
//            {
//                ::std::cerr << "Enter out_of_service state\n";
//            }
        };
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

        template < typename Event >
        void
        on_entry(Event const&, vending_fsm& fsm)
        {
            enclosing = &fsm;
        }

        vending_fsm* enclosing;
    };
    using on = back_machine<on_>;

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
    add_goods(events::load_goods const& entry)
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

inline vending_machine&
root_machine(vending_machine& m)
{
    return m;
}

template< typename T >
vending_machine&
root_machine(T& state)
{
    return root_machine(*state.enclosing);
}



}  /* namespace vending_msm */


#endif /* VENDING_MACHINE_MSM_HPP_ */
