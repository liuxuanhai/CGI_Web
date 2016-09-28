#include "reader.h"
#include "value.h"
#include <utility>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <iostream>
#include <stdexcept>

#include "byteorder.h"

#if !defined(__GNUC__) || (__GNUC__ == 2 && __GNUC_MINOR__ < 96)
#define likely(x)       if((x))
#define unlikely(x)     if((x))
#else
#define likely(x)       if(__builtin_expect((x) != 0, 1))
#define unlikely(x)     if(__builtin_expect((x) != 0, 0))
#endif

namespace Json
{

bool BinaryReader::parse(const char * pszData, size_t size, Value & root)
{
	document_.assign(pszData, size);
	const char *begin = document_.c_str();
	const char *end = begin + document_.length();
	return parse(begin, end, root);
}
	
bool BinaryReader::parse(const std::string &document, Value &root)
{
	document_ = document;
	const char *begin = document_.c_str();
	const char *end = begin + document_.length();
	return parse(begin, end, root);
}

bool BinaryReader::parse(const char *beginDoc, const char *endDoc, Value &root)
{
	begin_ = beginDoc;
	end_ = endDoc;
	current_ = begin_;

	unlikely (*current_++ != 0) return false;

	if (current_ == end_) return true;

	while (!nodes_.empty())
		nodes_.pop();
	nodes_.push(&root);

	return readValue();
}

bool BinaryReader::readValue()
{
	bool successful = false;

	Node node;
	unlikely(!readNode(node)) return false;

	switch (node.type_)
	{
	case Integer1:
	case Integer2:
	case Integer4:
	case Integer8:
		successful = readInteger(node);
		break;
	case String1:
	case String2:
	case String4:
		successful = readString(node);
		break;
	case Vector:
		successful= readVector(node);
		break;
	case Map:
		successful = readMap(node);
		break;
	default:
		return false;
	}

	return successful;
}

bool BinaryReader::readInteger(Node & node)
{
	switch(node.size_)
	{
	case 1:
	{
		currentValue() = *(unsigned char *)current_;
		break;
	}
	case 2:
	{
		currentValue() = ntohs(*(unsigned short *)current_);
		break;
	}
	case 4:
	{
		currentValue() = ntohl(*(unsigned int *)current_);
		break;
	}
	case 8:
	{
		char buf[8];
		buf[7] = current_[0];
		buf[6] = current_[1];
		buf[5] = current_[2];
		buf[4] = current_[3];
		buf[3] = current_[4];
		buf[2] = current_[5];
		buf[1] = current_[6];
		buf[0] = current_[7];
		currentValue() = (unsigned int)*(long long*)buf;
		break;
	}
	default:
		return false;
	}

	current_ += node.size_;
	unlikely(current_ > end_) return false;
	
	return true;
}

bool BinaryReader::readString(Node & node)
{
	unlikely (current_ + node.size_ > end_) return false;
	currentValue() = std::string(current_, node.size_);
	current_ += node.size_;
	return true;
}

bool BinaryReader::readVector(Node & node)
{
	currentValue() = Value(arrayValue);
	for (unsigned int index = 0; index < node.size_; index++)
	{
		Value &value = currentValue()[index];
		nodes_.push(&value);
		bool ok = readValue();
		nodes_.pop();
		unlikely (!ok) return false;
	}
	return true;
}

bool BinaryReader::readMap(Node & node)
{
	std::string name;
	currentValue() = Value(objectValue);
	for (unsigned int index = 0; index < node.size_; index++)
	{
		unsigned char ucNameLen =  *(unsigned char*)current_; 
		current_++;
		unlikely (current_ > end_) return false;
		name.assign(current_, ucNameLen);
		current_+= ucNameLen;
		unlikely (current_ > end_) return false;
		Value &value = currentValue()[name];
		nodes_.push(&value);
		bool ok = readValue();
		nodes_.pop();
		unlikely (!ok) return false;
	}
	return true;
}

bool BinaryReader::readNode(Node & node)
{
	node.type_ = *(unsigned char *) current_;
	current_++;
	unlikely (current_ > end_) return false;

	switch (node.type_)
	{
	case Integer1:
		node.size_ = 1;
		break;
	case Integer2:
		node.size_ = 2;
		break;
	case Integer4:
		node.size_ = 4;
		break;
	case Integer8:
		node.size_ = 8;
		break;
	case String1:
		node.size_ = *(unsigned char *) current_;
		current_ += 1;
		break;
	case String2:
		node.size_ = ntohs(*(unsigned short *) current_);
		current_ += 2;
		break;
	case String4:
		node.size_ = ntohl(*(unsigned long *) current_);
		current_ += 4;
		break;
	case Vector:
		node.size_ = ntohl(*(unsigned long *) current_);
		current_ += 4;
		break;
	case Map:
		node.size_ = ntohl(*(unsigned long *) current_);
		current_ += 4;
		break;
	default:
		return false;
	}

	unlikely (current_ > end_) return false;

	return true;
}

Value & BinaryReader::currentValue()
{
	return *(nodes_.top());
}

} // namespace Json
