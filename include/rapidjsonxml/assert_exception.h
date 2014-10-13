#ifndef RAPIDJSONXML_ASSERT

#include <exception>

namespace rapidjsonxml {

    class AssertException : public std::exception {
        virtual const char* what() const throw() {
            return "Failed assertion in rapidjsonxml";
        }
    };

    void assert_raiser(bool expression) {
        static AssertException e;
        if (!expression)
            throw e;
    }

}

#define RAPIDJSONXML_ASSERT assert_raiser
#endif
