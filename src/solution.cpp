#include "solution.hpp"

std::ostream& operator<<(std::ostream & out,
                         sol::ObjValue const & objValue)
{
   out << "(" << objValue.objValue() << ")";
   
   return out;
}

