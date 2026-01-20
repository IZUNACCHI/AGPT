#include "Object.h"

uint64_t Object::s_nextId = 1;

Object::Object(std::string name)
	: m_id(s_nextId++),
	m_name(std::move(name)) {
}
