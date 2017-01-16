/* ***** BEGIN LICENSE BLOCK *****
 * This file is part of Natron <http://www.natron.fr/>,
 * Copyright (C) 2013-2017 INRIA and Alexandre Gauthier-Foichat
 *
 * Natron is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Natron is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Natron.  If not, see <http://www.gnu.org/licenses/gpl-2.0.html>
 * ***** END LICENSE BLOCK ***** */

#ifndef NATRON_ENGINE_IPCCOMMON_H
#define NATRON_ENGINE_IPCCOMMON_H

// ***** BEGIN PYTHON BLOCK *****
// from <https://docs.python.org/3/c-api/intro.html#include-files>:
// "Since Python may define some pre-processor definitions which affect the standard headers on some systems, you must include Python.h before any standard headers are included."
#include <Python.h>
// ***** END PYTHON BLOCK *****

#include "Engine/EngineFwd.h"

#include <string>

#if !defined(Q_MOC_RUN) && !defined(SBK_RUN)
#include <boost/shared_ptr.hpp>
#include <boost/interprocess/smart_ptr/shared_ptr.hpp>
#include <boost/interprocess/smart_ptr/weak_ptr.hpp>
#include <boost/interprocess/smart_ptr/scoped_ptr.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/managed_external_buffer.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/containers/list.hpp>
#include <boost/interprocess/containers/set.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/offset_ptr.hpp>
#endif

NATRON_NAMESPACE_ENTER;


typedef boost::interprocess::managed_external_buffer ExternalSegmentType;

typedef boost::interprocess::allocator<char, ExternalSegmentType::segment_manager> CharAllocator_ExternalSegment;
typedef boost::interprocess::basic_string<char, std::char_traits<char>, CharAllocator_ExternalSegment> String_ExternalSegment;

/**
 * @brief Function to serialize to the given memory segment the given object.
 * The object must not be a container (so it should be a pod or a struct encapsulating only pod's).
 * The only supported container is std::string
 **/
template <class T>
inline void writeMMObject(const T& object, const std::string& objectName,  ExternalSegmentType* segment)
{
    T* sharedMemObject = segment->construct<T>(objectName.c_str())();
    if (sharedMemObject) {
        *sharedMemObject = object;
    } else {
        throw std::bad_alloc();
    }
}

/**
 * @brief Template specialization for std::string
 **/
template <>
inline void writeMMObject(const std::string& object, const std::string& objectName,  ExternalSegmentType* segment)
{

    // Allocate a char array containing the string
    CharAllocator_ExternalSegment allocator(segment->get_segment_manager());
    String_ExternalSegment* sharedMemObject = segment->construct<String_ExternalSegment>(objectName.c_str())(allocator);
    if (sharedMemObject) {
        sharedMemObject->append(object.c_str());
    } else {
        throw std::bad_alloc();
    }
}


/**
 * @brief Function to read from given memory segment the given object.
 * The object must not be a container (so it should be a pod or a struct encapsulating only pod's).
 * The only supported container is std::string
 **/
template <class T>
inline void readMMObject(const std::string& objectName, ExternalSegmentType* segment, T* object)
{
    std::pair<T*, ExternalSegmentType::size_type> found = segment->find<T>(objectName.c_str());
    if (found.first) {
        *object = *found.first;
    } else {
        throw std::bad_alloc();
    }
}


/**
 * @brief Template specialization for std::string
 **/
template <>
inline void readMMObject(const std::string& objectName, ExternalSegmentType* segment, std::string* object)
{
    std::pair<String_ExternalSegment*, ExternalSegmentType::size_type> found = segment->find<String_ExternalSegment>(objectName.c_str());
    if (found.first) {
        *object = std::string(found.first->c_str());
    } else {
        throw std::bad_alloc();
    }
}

NATRON_NAMESPACE_EXIT;


#endif // NATRON_ENGINE_IPCCOMMON_H
