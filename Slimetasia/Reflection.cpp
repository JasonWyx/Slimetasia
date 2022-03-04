#include "Reflection.h"

std::ostream& registration::operator<<(std::ostream& os, const variant& prop)
{
    os << "Name : " << prop.name << ", Type : " << prop.type << ", Offset : " << prop.offset << ", Size : " << prop.size << std::endl;
    return os;
}
