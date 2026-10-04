/**
 * \file    element.hpp
 * \brief   Defines a linked-list element for the Generic container.
 *
 * Each Element stores one value and a pointer to the next element in the
 * singly linked list.
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MERCURYCAN_SUPPORT_LIST_ELEMENT_HPP__
#define __MERCURYCAN_SUPPORT_LIST_ELEMENT_HPP__

namespace Generic {

/**
 * \brief   Stores a value and a link to the next list element.
 *
 * \tparam  ContentType passes the type of the value stored in the element.
 */
template<typename ContentType>
class Element {
public:
    /**
     * \brief   Constructs an element with a value and its next element.
     *
     * \param   content passes the value to store.
     * \param   nextElement passes the next element, or \c nullptr if there is
     *          no next element.
     */
    Element(const ContentType& content, Element* nextElement)
        : _content(content), _nextElement(nextElement) {
    }

    /**
     * \brief   Returns a mutable reference to the stored value.
     *
     * \return  Reference to the value held by this element.
     */
    ContentType& getContent(void) {
        return _content;
    }

    /**
     * \brief   Returns the next element in the list.
     *
     * \return  Pointer to the next element, or \c nullptr if there is none.
     */
    Element* getNextElement(void) const {
        return _nextElement;
    }

    /**
     * \brief   Sets the next element in the list.
     *
     * \param   nextElement passes the next element, or \c nullptr to terminate.
     */
    void setNextElement(Element* nextElement) {
        _nextElement = nextElement;
    }

private:
    /**
     * \brief   Value stored by this element.
     */
    ContentType _content;

    /**
     * \brief   Pointer to the next element in the linked list.
     */
    Element* _nextElement;
};

}

#endif  //__MERCURYCAN_SUPPORT_LIST_ELEMENT_HPP__