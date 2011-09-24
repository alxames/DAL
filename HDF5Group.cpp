#include "HDF5Group.h"
#include <hdf5.h>

using namespace std;

HDF5Group::HDF5Group( const HDF5Group &other )
:
  HDF5Node(other._name),
  parent(other.parent),
  _group(other._group ? new hid_gc(*other._group) : 0)
{
}

HDF5Group::HDF5Group( const hid_gc &parent, const string &name )
:
  HDF5Node(name),
  parent(parent),
  _group(0)
{
}

HDF5Group::~HDF5Group() {
  delete _group;
}

void HDF5Group::create() {
  delete _group; _group = 0;
  _group = new hid_gc(H5Gcreate2(parent, _name.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Gclose, "Could not create group");
}

bool HDF5Group::exists() const {
  return H5Lexists(parent, _name.c_str(), H5P_DEFAULT) > 0; // does this check whether the link is a group?
}

void HDF5Group::remove() const {
  if (H5Ldelete(parent, _name.c_str(), H5P_DEFAULT) < 0)
    throw HDF5Exception("Could not delete group"); 
}

void HDF5Group::set( const HDF5Group &other, bool deepcopy ) {
  if (exists()) {
    // maybe rename the attribute instead, in case the copying fails?
    remove();
  }

  hid_gc ocpl(H5Pcreate(H5P_OBJECT_COPY), H5Pclose, "Could not create property list");

  if (!deepcopy) {
    // do a shallow copy
    if (H5Pset_copy_object(ocpl, H5O_COPY_SHALLOW_HIERARCHY_FLAG) < 0) {
      throw HDF5Exception("Could not enable shallow copy mode");
    }
  }

  // H5Ocopy allows us to copy all properties, subgroups, etc, even across files
  if (H5Ocopy(other.parent, other._name.c_str(), parent, _name.c_str(), ocpl, H5P_DEFAULT) < 0)
    throw HDF5Exception("Could not copy element");
}

hid_gc *HDF5Group::open( hid_t parent, const string &name ) const
{
  return new hid_gc(H5Gopen2(parent, name.c_str(), H5P_DEFAULT), H5Gclose, "Could not open group");
}

Attribute<string> HDF5Group::groupType()
{
  return Attribute<string>(group(), "GROUPTYPE");
}

