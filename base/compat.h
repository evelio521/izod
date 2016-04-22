// Copyright 2012  All Rights Reserved.
// Author: sunqiang
//
// Google compatibility declarations and inline definitions.
// These are assembled from OpenFst and Thrax code.

#ifndef BASE_COMPAT_H_
#define BASE_COMPAT_H_

#include <deque>
#include <iostream>
#include <istream>
#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <bits/unique_ptr.h>
#include <bits/shared_ptr.h>  // boost_specific_ptr

using std::cerr;
using std::cin;
using std::cout;
using std::deque;
using std::unique_ptr;
using std::shared_ptr;
using std::weak_ptr;
using std::vector;
using std::pair;
using std::endl;
using std::istream;
using std::map;
using std::ostream;
using std::set;
using std::stack;
using std::string;
using std::unordered_set;
using std::unordered_map;

#endif  // BASE_COMPAT_H_

