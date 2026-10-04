/**
 * \file    container.hpp
 * \brief   Defines a singly linked container for generic values.
 *
 * Container stores values in dynamically allocated linked-list elements and
 * provides insertion, removal, sorting, indexed access, and iteration.
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MERCURYCAN_SUPPORT_LIST_CONTAINER_HPP__
#define __MERCURYCAN_SUPPORT_LIST_CONTAINER_HPP__

#include <stdint.h>
#include <cstdlib>

#include <MercuryCAN/support/list/element.hpp>
#include <MercuryCAN/support/list/iterator.hpp>

namespace Generic {

/**
 * \brief   Stores values in a singly linked list.
 *
 * Container owns and deletes its list elements. Stored values are copied into
 * those elements; if a value is a pointer, the pointed-to object is not
 * managed by the container.
 *
 * \tparam  ContentType passes the type of values stored in the container.
 */
template<typename ContentType>
class Container {
public:
    /**
     * \brief   Constructs an empty container.
     */
    Container(void) : _headOfStack(nullptr) {
    }

    /**
     * \brief   Destroys the container and all of its list elements.
     *
     * The destructor does not manage the lifetime of objects referenced by
     * pointer values stored in the elements.
     */
    virtual ~Container(void) {
        Element<ContentType>* nextElement = _headOfStack;

        while (nextElement) {
            Element<ContentType>* currentElement = nextElement;
            nextElement = nextElement->getNextElement();
            delete currentElement;
        }
    }

    /**
     * \brief   Adds a copy of a value to the front of the list.
     *
     * \param   value passes the value to copy into a new element.
     */
    void append(const ContentType& value) {
        Element<ContentType>* newElement =
            new Element<ContentType>(value, _headOfStack);

        if (newElement)
            _headOfStack = newElement;
    }

    /**
     * \brief   Sorts the elements using the supplied ordering function.
     *
     * The comparator determines whether its first argument precedes its second 
     * argument. The container elements are relinked; stored values are not 
     * copied during sorting.
     *
     * \param   comparator passes the function that compares two values.
     */
    void sort(bool (*comparator)(const ContentType&, const ContentType&)) {
        Element<ContentType>* sortedElements = nullptr;
        Element<ContentType>* currentElement = _headOfStack;

        while (currentElement) {
            Element<ContentType>* nextElement = currentElement->getNextElement();
            insertSorted(currentElement, sortedElements, comparator);
            currentElement = nextElement;
        }

        _headOfStack = sortedElements;
    }

    /**
     * \brief   Removes the first element whose value equals the given value.
     *
     * The list element is deleted. If no element matches, the container is
     * unchanged.
     *
     * \param   value passes the value to find and remove.
     */
    void remove(const ContentType& value) {
        Element<ContentType>* previousElement = nullptr;
        Element<ContentType>* currentElement = _headOfStack;

        while (currentElement) {
            if (currentElement->getContent() == value) {
                if (previousElement)
                    previousElement->setNextElement(currentElement->getNextElement());
                else
                    _headOfStack = currentElement->getNextElement();

                delete currentElement;
                return;
            }

            previousElement = currentElement;
            currentElement = currentElement->getNextElement();
        }
    }

    /**
     * \brief   Returns the size of the Container object itself.
     *
     * This does not return the number of stored elements or the total memory
     * used by the dynamically allocated list.
     *
     * \return  Result of sizeof(Container).
     */
    std::size_t size(void) {
        return sizeof(*this);
    }

    /**
     * \brief   Returns the number of elements in the list.
     *
     * \return  Number of stored elements, represented as an 8-bit value.
     */
    uint8_t length(void) const {
        uint8_t loopIterations = 0;
        Element<ContentType>* nextElement = _headOfStack;

        while (nextElement) {
            nextElement = nextElement->getNextElement();
            loopIterations++;
        }

        return loopIterations;
    }

    /**
     * \brief   Returns a mutable reference to a value at the given index.
     *
     * Index zero refers to the element at the tail of the current list;
     * increasing indices move toward the head.
     *
     * \param   index passes the zero-based index of the value.
     *
     * \return  Mutable reference to the value at \p index.
     */
    ContentType& at(const uint8_t& index) const {
        Element<ContentType>* targetElement = elementAt(index);
        return targetElement->getContent();
    }

    /**
     * \brief   Deletes every list element and leaves the container empty.
     *
     * Stored pointer values are not deleted; only the list elements are
     * destroyed.
     */
    void deleteAll(void) {
        const uint8_t elementCount = length();

        for (uint8_t i = 0; i < elementCount; i++) {
            Element<ContentType>* elementPointer = elementAt(i);
            delete elementPointer;
        }

        _headOfStack = nullptr;
    }

    /**
     * \brief   Inserts a copy of a value at the given index.
     *
     * \param   content passes the value to copy into the new element.
     * \param   index passes the zero-based insertion index.
     */
    void insert(const ContentType content, const uint8_t& index) {
        Element<ContentType>* newElement =
            new Element<ContentType>(content, _headOfStack);

        if (index > 0)
            newElement->setNextElement(elementAt(index - 1));
        else
            newElement->setNextElement(nullptr);

        if (!(index >= length() - 1)) {
            Element<ContentType>* elementAfter = elementAt(index);
            elementAfter->setNextElement(newElement);
        } else {
            _headOfStack = newElement;
        }
    }

    /**
     * \brief   Returns an iterator positioned at the head of the list.
     *
     * \return  Iterator to the first element visited during traversal.
     */
    Iterator<ContentType> begin(void) {
        return Iterator(_headOfStack);
    }

    /**
     * \brief   Returns the iterator position used to terminate traversal.
     *
     * \return  Iterator positioned at the list's final indexed element.
     */
    Iterator<ContentType> end(void) {
        return Iterator(elementAt(0));
    }

private:
    /**
     * \brief   Relinks an element into a sorted list.
     *
     * \param   element passes the element to insert.
     * \param   sortedElements passes the sorted-list head and receives its
     *          possibly updated head.
     * \param   comparator passes the function that compares element values.
     */
    void insertSorted(
        Element<ContentType>* element,
        Element<ContentType>*& sortedElements,
        bool (*comparator)(const ContentType&, const ContentType&)
    ) {
        Element<ContentType>* previousElement = nullptr;
        Element<ContentType>* insertionPoint = sortedElements;

        while (insertionPoint &&
               !comparator(insertionPoint->getContent(), element->getContent())) {
            previousElement = insertionPoint;
            insertionPoint = insertionPoint->getNextElement();
        }

        element->setNextElement(insertionPoint);
        if (previousElement)
            previousElement->setNextElement(element);
        else
            sortedElements = element;
    }

    /**
     * \brief   Finds a list element by its zero-based index.
     *
     * \param   index passes the index to locate.
     *
     * \return  Pointer to the element at \p index.
     */
    Element<ContentType>* elementAt(const uint8_t& index) const {
        const uint8_t itertaionsFromTop = length() - index - 1;

        Element<ContentType>* elementPointer = _headOfStack;

        for (uint8_t i = 0; i < itertaionsFromTop; i++)
            elementPointer = elementPointer->getNextElement();

        return elementPointer;
    }

    /**
     * \brief   Pointer to the head element of the list, or \c nullptr.
     */
    Element<ContentType>* _headOfStack = nullptr;
};

}

#endif  //__MERCURYCAN_SUPPORT_LIST_CONTAINER_HPP__