#ifndef __GENERIC_ELEMENT_HPP__
#define __GENERIC_ELEMENT_HPP__

namespace Generic {

template<typename ContentType>
class Element {
public:
    Element(const ContentType& content, Element* nextElement)
        : _content(content), _nextElement(nextElement) {
    }

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

#endif  //__GENERIC_ELEMENT_HPP__