// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: GpsLocation.proto

#ifndef PROTOBUF_GpsLocation_2eproto__INCLUDED
#define PROTOBUF_GpsLocation_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2005000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2005000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

namespace pbmsg {

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_GpsLocation_2eproto();
void protobuf_AssignDesc_GpsLocation_2eproto();
void protobuf_ShutdownFile_GpsLocation_2eproto();

class GpsLocation;

// ===================================================================

class GpsLocation : public ::google::protobuf::Message {
 public:
  GpsLocation();
  virtual ~GpsLocation();

  GpsLocation(const GpsLocation& from);

  inline GpsLocation& operator=(const GpsLocation& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const GpsLocation& default_instance();

  void Swap(GpsLocation* other);

  // implements Message ----------------------------------------------

  GpsLocation* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const GpsLocation& from);
  void MergeFrom(const GpsLocation& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional fixed32 account_id = 1;
  inline bool has_account_id() const;
  inline void clear_account_id();
  static const int kAccountIdFieldNumber = 1;
  inline ::google::protobuf::uint32 account_id() const;
  inline void set_account_id(::google::protobuf::uint32 value);

  // optional double latitude = 2;
  inline bool has_latitude() const;
  inline void clear_latitude();
  static const int kLatitudeFieldNumber = 2;
  inline double latitude() const;
  inline void set_latitude(double value);

  // optional double longitude = 3;
  inline bool has_longitude() const;
  inline void clear_longitude();
  static const int kLongitudeFieldNumber = 3;
  inline double longitude() const;
  inline void set_longitude(double value);

  // optional bytes timestamp = 4;
  inline bool has_timestamp() const;
  inline void clear_timestamp();
  static const int kTimestampFieldNumber = 4;
  inline const ::std::string& timestamp() const;
  inline void set_timestamp(const ::std::string& value);
  inline void set_timestamp(const char* value);
  inline void set_timestamp(const void* value, size_t size);
  inline ::std::string* mutable_timestamp();
  inline ::std::string* release_timestamp();
  inline void set_allocated_timestamp(::std::string* timestamp);

  // @@protoc_insertion_point(class_scope:pbmsg.GpsLocation)
 private:
  inline void set_has_account_id();
  inline void clear_has_account_id();
  inline void set_has_latitude();
  inline void clear_has_latitude();
  inline void set_has_longitude();
  inline void clear_has_longitude();
  inline void set_has_timestamp();
  inline void clear_has_timestamp();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  double latitude_;
  double longitude_;
  ::std::string* timestamp_;
  ::google::protobuf::uint32 account_id_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(4 + 31) / 32];

  friend void  protobuf_AddDesc_GpsLocation_2eproto();
  friend void protobuf_AssignDesc_GpsLocation_2eproto();
  friend void protobuf_ShutdownFile_GpsLocation_2eproto();

  void InitAsDefaultInstance();
  static GpsLocation* default_instance_;
};
// ===================================================================


// ===================================================================

// GpsLocation

// optional fixed32 account_id = 1;
inline bool GpsLocation::has_account_id() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void GpsLocation::set_has_account_id() {
  _has_bits_[0] |= 0x00000001u;
}
inline void GpsLocation::clear_has_account_id() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void GpsLocation::clear_account_id() {
  account_id_ = 0u;
  clear_has_account_id();
}
inline ::google::protobuf::uint32 GpsLocation::account_id() const {
  return account_id_;
}
inline void GpsLocation::set_account_id(::google::protobuf::uint32 value) {
  set_has_account_id();
  account_id_ = value;
}

// optional double latitude = 2;
inline bool GpsLocation::has_latitude() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void GpsLocation::set_has_latitude() {
  _has_bits_[0] |= 0x00000002u;
}
inline void GpsLocation::clear_has_latitude() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void GpsLocation::clear_latitude() {
  latitude_ = 0;
  clear_has_latitude();
}
inline double GpsLocation::latitude() const {
  return latitude_;
}
inline void GpsLocation::set_latitude(double value) {
  set_has_latitude();
  latitude_ = value;
}

// optional double longitude = 3;
inline bool GpsLocation::has_longitude() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void GpsLocation::set_has_longitude() {
  _has_bits_[0] |= 0x00000004u;
}
inline void GpsLocation::clear_has_longitude() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void GpsLocation::clear_longitude() {
  longitude_ = 0;
  clear_has_longitude();
}
inline double GpsLocation::longitude() const {
  return longitude_;
}
inline void GpsLocation::set_longitude(double value) {
  set_has_longitude();
  longitude_ = value;
}

// optional bytes timestamp = 4;
inline bool GpsLocation::has_timestamp() const {
  return (_has_bits_[0] & 0x00000008u) != 0;
}
inline void GpsLocation::set_has_timestamp() {
  _has_bits_[0] |= 0x00000008u;
}
inline void GpsLocation::clear_has_timestamp() {
  _has_bits_[0] &= ~0x00000008u;
}
inline void GpsLocation::clear_timestamp() {
  if (timestamp_ != &::google::protobuf::internal::kEmptyString) {
    timestamp_->clear();
  }
  clear_has_timestamp();
}
inline const ::std::string& GpsLocation::timestamp() const {
  return *timestamp_;
}
inline void GpsLocation::set_timestamp(const ::std::string& value) {
  set_has_timestamp();
  if (timestamp_ == &::google::protobuf::internal::kEmptyString) {
    timestamp_ = new ::std::string;
  }
  timestamp_->assign(value);
}
inline void GpsLocation::set_timestamp(const char* value) {
  set_has_timestamp();
  if (timestamp_ == &::google::protobuf::internal::kEmptyString) {
    timestamp_ = new ::std::string;
  }
  timestamp_->assign(value);
}
inline void GpsLocation::set_timestamp(const void* value, size_t size) {
  set_has_timestamp();
  if (timestamp_ == &::google::protobuf::internal::kEmptyString) {
    timestamp_ = new ::std::string;
  }
  timestamp_->assign(reinterpret_cast<const char*>(value), size);
}
inline ::std::string* GpsLocation::mutable_timestamp() {
  set_has_timestamp();
  if (timestamp_ == &::google::protobuf::internal::kEmptyString) {
    timestamp_ = new ::std::string;
  }
  return timestamp_;
}
inline ::std::string* GpsLocation::release_timestamp() {
  clear_has_timestamp();
  if (timestamp_ == &::google::protobuf::internal::kEmptyString) {
    return NULL;
  } else {
    ::std::string* temp = timestamp_;
    timestamp_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
    return temp;
  }
}
inline void GpsLocation::set_allocated_timestamp(::std::string* timestamp) {
  if (timestamp_ != &::google::protobuf::internal::kEmptyString) {
    delete timestamp_;
  }
  if (timestamp) {
    set_has_timestamp();
    timestamp_ = timestamp;
  } else {
    clear_has_timestamp();
    timestamp_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  }
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace pbmsg

#ifndef SWIG
namespace google {
namespace protobuf {


}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_GpsLocation_2eproto__INCLUDED