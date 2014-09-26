#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using boost::property_tree::ptree;
using boost::property_tree::read_json;
using boost::property_tree::write_json;
using boost::property_tree::json_parser::verify_json;

namespace bored
{
	typedef boost::property_tree::ptree PropertyTree;
	typedef boost::property_tree::ptree::iterator PropertyTreeIterator;

	std::string GetJSON(PropertyTree &tree);
	bool ValidateJSON(const std::string &json);
	bool ParseJSON(const std::string &json, PropertyTree &tree);
};