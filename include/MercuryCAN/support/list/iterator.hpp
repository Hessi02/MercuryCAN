/**
 * \file    iterator.hpp
 * \brief   Defines an iterator for the Generic linked-list container.
 *
 * Iterator holds a pointer to the current list element and supports 
 * dereferencing, pre-increment, and comparison with another iterator.
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MERCURYCAN_SUPPORT_LIST_ITERATOR_HPP__
#define __MERCURYCAN_SUPPORT_LIST_ITERATOR_HPP__

#include <MercuryCAN/support/list/element.hpp>

namespace Generic {

/**
 * \brief   Traverses elements in a Generic linked-list container.
 *
 * \tparam  ContentType passes the type of values stored in the elements.
 */
template<typename ContentType>
class Iterator {
public:
    /**
     * \brief   Constructs an iterator positioned at a list element.
     *
     * \param   element passes the element at which iteration starts.
     */
    Iterator(Element<ContentType>* element) : _currentElement(element) {
    }

    /**
     * \brief   Compares this iterator with another iterator.
     *
     * The comparison checks whether both iterators refer to different
     * elements. A null element represents the end of the list.
     *
     * \param   other passes the iterator position to compare against.
     *
     * \return  True if this element differs from the element \p other.
     */
    bool operator!=(const Iterator& other) const {
        return _currentElement != other._currentElement;
    }

    /**
     * \brief   Returns a copy of the value at the current position.
     *
     * \return  Copy of the current element's stored value.
     */
    ContentType operator*(void) const {
        return _currentElement->getContent();
    }

    /**
     * \brief   Advances the iterator to the next element.
     *
     * \return  Reference to this iterator after it has been advanced.
     */
    const Iterator& operator++(void) {
        _currentElement = _currentElement->getNextElement();
        return *this;
    }

private:
    /**
     * \brief   Element currently referenced by the iterator.
     */
    Element<ContentType>* _currentElement;
};

}

#endif  //__MERCURYCAN_SUPPORT_LIST_ITERATOR_HPP__