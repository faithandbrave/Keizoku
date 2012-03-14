// Copyright Akira Takahashi 2012.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
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
    AMB_BLOCK(ctx) {
        amber& x = amb(ctx, 0, amb1_f);
        amber& y = amb(ctx, 1, amb2_f);
        amber& z = amb(ctx, 2, amb3_f);

        std::cout << x.get() << ", " << y.get() << ", " << z.get() << std::endl;
    };
}

/*
output:
1, 4, 6
1, 4, 7
1, 4, 8
1, 5, 6
1, 5, 7
1, 5, 8
2, 4, 6
2, 4, 7
2, 4, 8
2, 5, 6
2, 5, 7
2, 5, 8
3, 4, 6
3, 4, 7
3, 4, 8
3, 5, 6
3, 5, 7
3, 5, 8
*/
