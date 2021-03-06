/* Copyright 2011-2012  ASTRON, Netherlands Institute for Radio Astronomy
 * This file is part of the Data Access Library (DAL).
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either 
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef DAL_ATTRIBUTE_H
#define DAL_ATTRIBUTE_H

#include <cstdlib>
#include <string>
#include <vector>
#include <ostream>
#include <hdf5.h>
#include "types/h5typemap.h"
#include "types/versiontype.h"
#include "Node.h"

namespace dal {

class Group;

/*!
 * Represents core functionality for an attribute inside a group.
 */
class AttributeBase: public Node {
public:
  /*!
   * Create a generic Attribute object, providing meta functionality.
   */
  AttributeBase( Group &parent, const std::string &name ): Node(parent, name) {}

  /*!
   * Destruct a generic Attribute object.
   */
  virtual ~AttributeBase() {}

  /*!
   * Returns whether this attribute exists in the HDF5 file.
   */
  virtual bool exists() const;

  /*!
   * Removes this element from the HDF5 file. Useful for porting, 
   * repairing, or otherwise modifying files on a structural level.
   */
  void remove() const;

  /*!
   * Returns the number of data points in this element (1 for a scalar, >= 0 for a vector)
   */
  size_t size() const;

  /*!
   * Validates the attribute by checking whether it exists, and whether it can be read
   * using the type defined by this object.
   */
  virtual bool valid() const;
};

#ifndef SWIG

template<typename T> class Attribute;

/*!
 * Interface class for Attribute<T>::value, providing Python-property-like functionality:
 *
 * Attribute<int> attr(...);
 *
 * attr.value = 42;    // set the value
 * int i = attr.value; // get the value
 * attr.del();         // remove the attribute
 *
 * where attr.value is of type AttributeValue<int>.
 *
 * The setter will create the attribute if it does not exist.
 * The getter will throw an exception if the attribute does not exist.
 * The deleter will not throw an exception if the attribute dos not exist.
 */
template<typename T> class AttributeValue {
public:
  /*!
   * Setter: allows x.value = (T)y.
   */
  AttributeValue<T>& operator=( const T& value );

  /*!
   * Allow x.value = y.value
   */
  AttributeValue<T>& operator=( const AttributeValue<T>& value );

  /*!
   * Getter: allows T y = x.value.
   */
  operator T() const;

  /*!
   * Crudely mimic Python's "del attr.value" (the property's deleter)
   */
  void del() const;

private:
  // Do not allow copying, as attr of the copy might get out of scope
  AttributeValue( const AttributeValue& );

  AttributeValue( Attribute<T> &attr ): attr(attr) {}

  Attribute<T> &attr;

  // Only Attribute<T> can create instances
  friend class Attribute<T>;
};

template<typename T> std::ostream& operator<<(std::ostream &out, const AttributeValue<T> &val);

#endif /* !SWIG */

/*!
 * Represents an attribute containing a scalar or a string.
 *
 * Python example:
 * \code
 *     # Create a new HDF5 file with some string attribute
 *     >>> f = File("example.h5", File.CREATE)
 *     >>> a = AttributeString(f, "EXAMPLE_STRING")
 *
 *     # Because we are creating the file, the attribute does initially not exist
 *     >>> a.value is None
 *     True
 *
 *     # Once we set the value, the attribute exists and can be read
 *     >>> a.value = "hello world!"
 *     >>> a.value
 *     'hello world!'
 *
 *     # The attribute can also be removed
 *     >>> del a.value
 *     >>> a.value is None
 *     True
 *
 *     # Low-level functions allow finer control ...
 *     >>> a.exists()
 *     False
 *     >>> a.create() # returns a
 *     <...>
 *     >>> a.exists()
 *     True
 *     >>> a.set("hello world!")
 *     >>> a.get()
 *     'hello world!'
 *     >>> a.remove()
 *
 *     # ... but raise errors if used incorrectly
 *     >>> a.exists()
 *     False
 *     >>> a.set("hello world!")
 *     Traceback (most recent call last):
 *     HDF5Exception: Could not open attribute
 *     >>> a.get()
 *     Traceback (most recent call last):
 *     HDF5Exception: Could not open attribute
 *     >>> a.remove()
 *     Traceback (most recent call last):
 *     HDF5Exception: Could not delete element
 *
 *     # Clean up:
 *     >>> import os
 *     >>> os.remove("example.h5")
 * \endcode
 */
template<typename T> class Attribute: public AttributeBase {
public:
  typedef T value_type;

  /*!
   * Represent an attribute called `name' within group `parent'.
   */
  Attribute( Group &parent, const std::string &name ): AttributeBase(parent, name), value(*this) {}

  Attribute( const Attribute &other ): AttributeBase(other), value(*this) {}

  /*!
   * Destruct an Attribute object.
   */
  virtual ~Attribute() {}

  /*!
   * Creates this attribute.
   * Returns a reference to the attribute, so you can create and set in one expression: attr.create().set(val).
   */
  Attribute<T>& create();

  /*!
   * Returns the value of this attribute, retrieved from the HDF5 file. An exception
   * is thrown if the attribute does not exist.
   */
  T get() const;

  /*!
   * Stores the value of this attribute in the HDF5 file. An exception is thrown
   * if the attribute does not exist.
   */
  void set( const T &value );

  /*!
   * Validates the attribute by checking whether it exists, and whether it can be read
   * using the type defined by this object.
   *
   * Python example:
   * \code
   *     # Create a new HDF5 file with some string attribute
   *     >>> f = File("example.h5", File.CREATE)
   *
   *     # Refer to a string attribute
   *     >>> a = AttributeString(f, "EXAMPLE_STRING")
   *
   *     # The attribute has not been written yet, so it's not valid
   *     >>> a.exists()
   *     False
   *     >>> a.valid()
   *     False
   *
   *     # Create and set the attribute
   *     >>> a.value = "hello world"
   *
   *     # This attribute is valid because it exists and the type we specified
   *     # matches the type stored on disk.
   *     >>> a.valid()
   *     True
   *
   *     # Read the same value as an integer
   *     >>> b = AttributeUInt(f, "EXAMPLE_STRING")
   *     >>> b.valid()
   *     False
   *
   *     # Clean up:
   *     >>> import os
   *     >>> os.remove("example.h5")
   * \endcode
   */
  virtual bool valid() const;

  AttributeValue<T> value;
};

/*!
 * Represents an attribute containing a vector of scalars or strings.
 */
template<typename T> class Attribute< std::vector<T> >: public AttributeBase {
public:
  typedef std::vector<T> value_type;

  /*!
   * Represent an attribute called `name' within group `parent'.
   */
  Attribute( Group &parent, const std::string &name ): AttributeBase(parent, name), value(*this) {}

  Attribute( const Attribute &other ): AttributeBase(other), value(*this) {}

  /*!
   * Destruct an Attribute object.
   */
  virtual ~Attribute() {}

  /*!
   * Creates this attribute, reserving a certain length.
   * If a different length is needed later, the attribute will be automatically recreated.
   * Returns a reference to the attribute, so you can create and set in one expression: attr.create(val.size()).set(val).
   */
  Attribute< std::vector<T> > &create( size_t length = 0 );

  /*!
   * Returns the value of this attribute, retrieved from the HDF5 file. An exception
   * is thrown if the attribute does not already exist.
   */
  std::vector<T> get() const;

  /*!
   * Stores the value of this attribute in the HDF5 file. An exception is thrown
   * if the attribute does not exist.
   */
  void set( const std::vector<T> &value );

  /*!
   * Validates the attribute by checking whether it exists, and whether it can be read
   * using the type defined by this object.
   */
  virtual bool valid() const;

  AttributeValue< std::vector<T> > value;
};

}

#include "Attribute.tcc"

#endif

