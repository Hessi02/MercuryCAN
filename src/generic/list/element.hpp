#ifndef __GENERIC_ELEMENT_HPP__
#define __GENERIC_ELEMENT_HPP__

namespace Generic
{

/**
 *  \class      Element
 *  \brief      Represents a single node of a singly linked list.
 *  \tparam     ContentType specifies the data type to store in the node.
 *
 *  The class 'Element' encapsulates the structure and behavior of a single
 *  node used within the linked list 'Container'. Each node contains a data 
 *  object of type 'ContentType' and a pointer to the next node in the list. By 
 *  linking multiple 'Element' instances together, a linear dynamic data 
 *  structure is created where each node knows its direct  successor. The last 
 *  element of the list typically stores 'nullptr' as its next element. The 
 *  class provides basic accessor and modifier methods to access the stored 
 *  content, retrieve the next element and modify the linkage between nodes at 
 *  runtime.
 */
template<typename ContentType>
class Element
{
public:
    Element(const ContentType& content, Element* nextElement)
        : _content(content)
        , _nextElement(nextElement)
    {}

    ContentType& getContent(void) { 
        return _content; 
    }

    Element* getNextElement(void) const { 
        return _nextElement; 
    }

    void setNextElement(Element* nextElement) { 
        _nextElement = nextElement; 
    }

private:
    ContentType _content;
    Element* _nextElement;
};

}

#endif //__GENERIC_ELEMENT_HPP__