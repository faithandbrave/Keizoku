// Copyright Akira Takahashi 2012.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef AMB_INCLUDE
#define AMB_INCLUDE

#include <vector>
#include <memory>
#include <boost/context/all.hpp>
#include <boost/function.hpp>
#include <boost/utility/value_init.hpp>
#include <boost/foreach.hpp>
#include <boost/range/algorithm_ext/all_of.hpp>
#include <boost/range/combine.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/adaptor/indirected.hpp>
#include <boost/range/adaptor/taken.hpp>
#include <boost/range/adaptor/dropped.hpp>

class amber {
    boost::contexts::context ctx_;
    boost::function<void(amber&)> fn_;
    boost::initialized<bool> started_;
    boost::initialized<bool> is_break_;
    boost::initialized<int> now_value_;

    void trampoline_()
    { fn_(*this); }

public:
    amber(boost::function<void(amber&)> const& fn)
    {
        ctx_.swap(boost::move(boost::contexts::context(
                    &amber::trampoline_,
                    this,
                    boost::contexts::default_stacksize(),
                    boost::contexts::stack_unwind,
                    boost::contexts::return_to_caller)));

        fn_ = fn;
    }

    void resume()
    {
        if (!started_.data()) {
            started_.data() = true;
            is_break_.data() = false;
            now_value_.data() = ctx_.start();
        }
        else {
            now_value_.data() = ctx_.resume();
        }
    }

    void restart()
    {
        started_.data() = false;
        is_break_.data() = false;
        ctx_.swap(boost::move(boost::contexts::context(
                    &amber::trampoline_,
                    this,
                    boost::contexts::default_stacksize(),
                    boost::contexts::stack_unwind,
                    boost::contexts::return_to_caller)));
        resume();
    }

    int suspend(int vp = 0)
    { return ctx_.suspend(vp); }

    int suspend_break(int vp = 0)
    {
        is_break_.data() = true;
        return ctx_.suspend(vp);
    }

    bool is_complete() const
    { return is_break_.data() || ctx_.is_complete(); }

    void unwind_stack() { ctx_.unwind_stack(); }

    int get() const { return now_value_.data(); }
};

class amb_block_t {
    std::vector<std::unique_ptr<amber>> ambers_;
    boost::initialized<bool> initialized_;
    boost::function<void(amb_block_t&)> fn_;
public:
    amb_block_t& operator=(boost::function<void(amb_block_t&)> fn)
    {
        fn_ = fn;
        return *this;
    }

    ~amb_block_t()
    {
        execute();
    }

    void add(std::unique_ptr<amber> x)
    {
        if (initialized_.data())
            return;
        ambers_.push_back(boost::move(x));
    }

    amber& at(std::size_t i)
    {
        return *ambers_.at(i);
    }

    void execute()
    {
        using namespace boost::adaptors;

        while (true) {
            fn_(*this);

            if (!initialized_.data()) {
                initialized_.data() = true;
            }

            if (is_all_complete()) {
                break;
            }

            auto rev = ambers_ | reversed;
            BOOST_FOREACH (auto& adj, boost::combine(rev | taken(ambers_.size() - 1), rev | dropped(1))) {
                amber& x = *boost::get<0>(adj);
                amber& y = *boost::get<1>(adj);

                if (x.is_complete()) {
                    x.restart();
                    if (y.is_complete())
                        continue;
                    y.resume();
                    break;
                } else {
                    x.resume();
                    break;
                }
            }
        }
    }

    bool is_initialized() const
    { return initialized_.data(); }

private:
    bool is_all_complete() const
    {
        using namespace boost::adaptors;
        return boost::all_of(ambers_ | indirected, [](amber& x) { return x.is_complete(); });
    }
};

inline amber& amb(amb_block_t& ctx, std::size_t i, boost::function<void(amber&)> f)
{
    if (ctx.is_initialized())
        return ctx.at(i);

    std::unique_ptr<amber> p(new amber(f));
    p->resume();
    ctx.add(boost::move(p));
    return ctx.at(i);
}

#define AMB_BLOCK(context_variable) amb_block_t() = [&](amb_block_t& context_variable)

#endif // AMB_INCLUDE
