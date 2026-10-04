#ifndef __MERCURYCAN_SUPPORT_LIST_ITERATOR_HPP__
#define __MERCURYCAN_SUPPORT_LIST_ITERATOR_HPP__

#include <MercuryCAN/support/list/element.hpp>

namespace Generic {

template<typename ContentType>
class Iterator {
public:
    Iterator(Element<ContentType>* element) : _currentElement(element) {
    }

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

#endif  //__MERCURYCAN_SUPPORT_LIST_ITERATOR_HPP__