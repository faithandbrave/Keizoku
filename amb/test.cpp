// Copyright Akira Takahashi 2012.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/detail/lightweight_test.hpp>

#include <vector>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/comparison.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/range/algorithm/equal.hpp>
#include "amb.hpp"

void amb1_f(amber& cont)
{
    cont.suspend(1);
    cont.suspend(2);
    cont.suspend_break(3);
}

void amb2_f(amber& cont)
{
    cont.suspend(4);
    cont.suspend_break(5);
}

void amb3_f(amber& cont)
{
    cont.suspend(6);
    cont.suspend(7);
    cont.suspend_break(8);
}

int main()
{
	namespace fusion = boost::fusion;

	typedef fusion::vector<int, int, int> value_type;
	std::vector<value_type> vec;

    AMB_BLOCK(ctx) {
        amber& x = amb(ctx, 0, amb1_f);
        amber& y = amb(ctx, 1, amb2_f);
        amber& z = amb(ctx, 2, amb3_f);

        vec.push_back(value_type(x.get(), y.get(), z.get()));
    };

	const std::vector<value_type> expected = boost::assign::list_of<value_type>
		(1, 4, 6)
		(1, 4, 7)
		(1, 4, 8)
		(1, 5, 6)
		(1, 5, 7)
		(1, 5, 8)
		(2, 4, 6)
		(2, 4, 7)
		(2, 4, 8)
		(2, 5, 6)
		(2, 5, 7)
		(2, 5, 8)
		(3, 4, 6)
		(3, 4, 7)
		(3, 4, 8)
		(3, 5, 6)
		(3, 5, 7)
		(3, 5, 8)
		;

	BOOST_TEST(boost::equal(vec, expected));

	return boost::report_errors();
}

