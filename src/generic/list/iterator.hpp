#ifndef __GENERIC_ITERATOR_HPP__
#define __GENERIC_ITERATOR_HPP__

#include "element.hpp"

namespace Generic
{

/**
 *  \class      Iterator
 *  \brief      Forward iterator for traversing linked list elements.
 *  \tparam     ContentType Type of the data stored in the container.
 *
 *  The class 'Iterator' provides a lightweight forward iterator implementation
 *  for navigating through the singly linked list used by 'Generic::Container'.
 *  Internally, the iterator stores a pointer to the currently referenced
 *  'Element' object and advances through the list by following the linked
 *  successor nodes.
 *
 *  The iterator supports:
 *
 *  - dereferencing using the `*` operator,
 *  - forward iteration using the prefix increment operator (`++`),
 *  - comparison using the inequality operator (`!=`).
 *
 *  This allows the container to be traversed in a manner compatible with
 *  range-based loops and iterator-style iteration patterns.
 */
template<typename ContentType>
class Iterator
{
public:
    Iterator(Element<ContentType>* element)
        : _currentElement(element)
    {}

    bool operator!=(const Iterator& other) const {
        return (_currentElement != other._currentElement->getNextElement());
    }

    ContentType operator*(void) const { 
        return _currentElement->getContent(); 
    }

    const Iterator& operator++(void) {
        _currentElement = _currentElement->getNextElement();
        return *this;
    }

private:
    Element<ContentType>* _currentElement;
};

}

#endif //__GENERIC_ITERATOR_HPP__