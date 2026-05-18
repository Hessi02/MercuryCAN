#ifndef __GENERIC_CONTAINER_HPP__
#define __GENERIC_CONTAINER_HPP__

#include <stdint.h>

#include "element.hpp"
#include "iterator.hpp"

namespace Generic
{

/**
 *  \class      Container
 *  \brief      Generic singly linked list container.
 *  \tparam     ContentType Type of the data stored inside the container.
 *
 *  The class `Container` implements a simple generic container using a
 *  singly linked list as its underlying data structure. Each stored value is
 *  wrapped inside an 'Element' object which maintains a pointer to the next
 *  element in the sequence. New elements are appended at the head of the list, 
 *  resulting in a stack-like insertion behavior where the most recently 
 *  inserted element becomes the first element of the container. Memory 
 *  allocation and deallocation of list nodes are handled internally.
 */
template<typename ContentType>
class Container
{
public:
    Container(void)
        : _headOfStack(nullptr)
    {}

    virtual ~Container(void) {
        Element<ContentType>* nextElement = _headOfStack;

        while (nextElement) {
            Element<ContentType>* currentElement = nextElement;
            nextElement = nextElement->getNextElement();
            delete currentElement;
        }
    }

    void append(const ContentType& value) {
        Element<ContentType>* newElement = 
            new Element<ContentType>(value, _headOfStack);
        
        if (newElement)
            _headOfStack = newElement;
    }

    std::size_t size(void) {
        return sizeof(*this);
    }

    uint8_t length(void) const {
        uint8_t loopIterations = 0;
        Element<ContentType>* nextElement = _headOfStack;

        while (nextElement) {
            nextElement = nextElement->getNextElement();
            loopIterations++;
        }

        return loopIterations;
    }

    ContentType& at(const uint8_t& index) const {
        Element<ContentType>* targetElement = elementAt(index);
        return targetElement->getContent();
    }

    void deleteAll(void) {
        const uint8_t elementCount = length();

        for (uint8_t i = 0; i < elementCount; i++) {
            Element<ContentType>* elementPointer = elementAt(i);
            delete elementPointer;
        }

        _headOfStack = nullptr;
    }

    void insert(const ContentType content, const uint8_t& index) {
        Element<ContentType>* newElement = 
            new Element<ContentType>(content, _headOfStack);

        if (index > 0) {
            newElement->setNextElement(elementAt(index - 1));
        } else {
            newElement->setNextElement(nullptr);
        }

        if (!(index >= length() - 1)) {
            Element<ContentType>* elementAfter = elementAt(index);
            elementAfter->setNextElement(newElement);
        } else {
            _headOfStack = newElement;
        }
    }

    Iterator<ContentType> begin(void) { 
        return Iterator(_headOfStack); 
    }

    Iterator<ContentType> end(void) { 
        return Iterator(elementAt(0)); 
    }

private:
    Element<ContentType>* elementAt(const uint8_t& index) const {
        const uint8_t itertaionsFromTop = length() - index - 1;

        Element<ContentType>* elementPointer = _headOfStack;

        for (uint8_t i = 0; i < itertaionsFromTop; i++) {
            elementPointer = elementPointer->getNextElement();
        }

        return elementPointer;
    }

    Element<ContentType>* _headOfStack = nullptr;
};

}

#endif // __GENERIC_CONTAINER_HPP__