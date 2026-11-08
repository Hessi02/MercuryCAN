/**
 * \file    main.cpp (tc_16_ContainerDeleteAll)
 * \brief   Verifies that deleteAll() empties the container safely.
 *
 * Checks that all elements are removed and that the container remains usable
 * after deletion.
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <MercuryCAN/support/list/container.hpp>

#include <cassert>

int main() {
    Generic::Container<int> values;
    values.append(1);
    values.append(2);
    values.append(3);

    values.deleteAll();
    assert(values.length() == 0);

    values.append(4);
    assert(values.length() == 1);
    assert(values.at(0) == 4);

    values.deleteAll();
    assert(values.length() == 0);
}
