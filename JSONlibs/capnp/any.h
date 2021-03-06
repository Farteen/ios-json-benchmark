// Copyright (c) 2013-2014 Sandstorm Development Group, Inc. and contributors
// Licensed under the MIT License:
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef CAPNP_ANY_H_
#define CAPNP_ANY_H_

#if defined(__GNUC__) && !CAPNP_HEADER_WARNINGS
#pragma GCC system_header
#endif

#include "layout.h"
#include "pointer-helpers.h"
#include "orphan.h"
#include "list.h"

namespace capnp {

class StructSchema;
class ListSchema;
class InterfaceSchema;
class Orphanage;
class ClientHook;
class PipelineHook;
struct PipelineOp;
struct AnyPointer;

struct AnyList {
  AnyList() = delete;

  class Reader;
  class Builder;
};

struct AnyStruct {
  AnyStruct() = delete;

  class Reader;
  class Builder;
  class Pipeline;
};

template<>
struct List<AnyStruct, Kind::OTHER> {
  List() = delete;

  class Reader;
  class Builder;
};

namespace _ {  // private
template <> struct Kind_<AnyPointer> { static constexpr Kind kind = Kind::OTHER; };
template <> struct Kind_<AnyStruct> { static constexpr Kind kind = Kind::OTHER; };
template <> struct Kind_<AnyList> { static constexpr Kind kind = Kind::OTHER; };
}  // namespace _ (private)

// =======================================================================================
// AnyPointer!

struct AnyPointer {
  // Reader/Builder for the `AnyPointer` field type, i.e. a pointer that can point to an arbitrary
  // object.

  AnyPointer() = delete;

  class Reader {
  public:
    typedef AnyPointer Reads;

    Reader() = default;
    inline Reader(_::PointerReader reader): reader(reader) {}

    inline MessageSize targetSize() const;
    // Get the total size of the target object and all its children.

    inline bool isNull() const;
    inline bool isStruct() {
      return reader.isStruct();
    }
    inline bool isList() {
      return reader.isList();
    }

    template <typename T>
    inline ReaderFor<T> getAs() const;
    // Valid for T = any generated struct type, interface type, List<U>, Text, or Data.

    template <typename T>
    inline ReaderFor<T> getAs(StructSchema schema) const;
    // Only valid for T = DynamicStruct.  Requires `#include <capnp/dynamic.h>`.

    template <typename T>
    inline ReaderFor<T> getAs(ListSchema schema) const;
    // Only valid for T = DynamicList.  Requires `#include <capnp/dynamic.h>`.

    template <typename T>
    inline ReaderFor<T> getAs(InterfaceSchema schema) const;
    // Only valid for T = DynamicCapability.  Requires `#include <capnp/dynamic.h>`.

#if !CAPNP_LITE
    kj::Own<ClientHook> getPipelinedCap(kj::ArrayPtr<const PipelineOp> ops) const;
    // Used by RPC system to implement pipelining.  Applications generally shouldn't use this
    // directly.
#endif  // !CAPNP_LITE

  private:
    _::PointerReader reader;
    friend struct AnyPointer;
    friend class Orphanage;
    friend class CapReaderContext;
  };

  class Builder {
  public:
    typedef AnyPointer Builds;

    Builder() = delete;
    inline Builder(decltype(nullptr)) {}
    inline Builder(_::PointerBuilder builder): builder(builder) {}

    inline MessageSize targetSize() const;
    // Get the total size of the target object and all its children.

    inline bool isNull();
    inline bool isStruct() {
      return builder.isStruct();
    }
    inline bool isList() {
      return builder.isList();
    }

    inline void clear();
    // Set to null.

    template <typename T>
    inline BuilderFor<T> getAs();
    // Valid for T = any generated struct type, List<U>, Text, or Data.

    template <typename T>
    inline BuilderFor<T> getAs(StructSchema schema);
    // Only valid for T = DynamicStruct.  Requires `#include <capnp/dynamic.h>`.

    template <typename T>
    inline BuilderFor<T> getAs(ListSchema schema);
    // Only valid for T = DynamicList.  Requires `#include <capnp/dynamic.h>`.

    template <typename T>
    inline BuilderFor<T> getAs(InterfaceSchema schema);
    // Only valid for T = DynamicCapability.  Requires `#include <capnp/dynamic.h>`.

    template <typename T>
    inline BuilderFor<T> initAs();
    // Valid for T = any generated struct type.

    template <typename T>
    inline BuilderFor<T> initAs(uint elementCount);
    // Valid for T = List<U>, Text, or Data.

    template <typename T>
    inline BuilderFor<T> initAs(StructSchema schema);
    // Only valid for T = DynamicStruct.  Requires `#include <capnp/dynamic.h>`.

    template <typename T>
    inline BuilderFor<T> initAs(ListSchema schema, uint elementCount);
    // Only valid for T = DynamicList.  Requires `#include <capnp/dynamic.h>`.

    inline AnyList::Builder initAsAnyList(ElementSize elementSize, uint elementCount);
    // Note: Does not accept INLINE_COMPOSITE for elementSize.

    inline List<AnyStruct>::Builder initAsListOfAnyStruct(
        uint dataWordCount, uint pointerCount, uint elementCount);

    inline AnyStruct::Builder initAsAnyStruct(uint dataWordCount, uint pointerCount);

    template <typename T>
    inline void setAs(ReaderFor<T> value);
    // Valid for ReaderType = T::Reader for T = any generated struct type, List<U>, Text, Data,
    // DynamicStruct, or DynamicList (the dynamic types require `#include <capnp/dynamic.h>`).

    template <typename T>
    inline void setAs(std::initializer_list<ReaderFor<ListElementType<T>>> list);
    // Valid for T = List<?>.

    inline void set(Reader value) { builder.copyFrom(value.reader); }
    // Set to a copy of another AnyPointer.

    template <typename T>
    inline void adopt(Orphan<T>&& orphan);
    // Valid for T = any generated struct type, List<U>, Text, Data, DynamicList, DynamicStruct,
    // or DynamicValue (the dynamic types require `#include <capnp/dynamic.h>`).

    template <typename T>
    inline Orphan<T> disownAs();
    // Valid for T = any generated struct type, List<U>, Text, Data.

    template <typename T>
    inline Orphan<T> disownAs(StructSchema schema);
    // Only valid for T = DynamicStruct.  Requires `#include <capnp/dynamic.h>`.

    template <typename T>
    inline Orphan<T> disownAs(ListSchema schema);
    // Only valid for T = DynamicList.  Requires `#include <capnp/dynamic.h>`.

    template <typename T>
    inline Orphan<T> disownAs(InterfaceSchema schema);
    // Only valid for T = DynamicCapability.  Requires `#include <capnp/dynamic.h>`.

    inline Orphan<AnyPointer> disown();
    // Disown without a type.

    inline Reader asReader() const { return Reader(builder.asReader()); }
    inline operator Reader() const { return Reader(builder.asReader()); }

  private:
    _::PointerBuilder builder;
    friend class Orphanage;
    friend class CapBuilderContext;
  };

#if !CAPNP_LITE
  class Pipeline {
  public:
    typedef AnyPointer Pipelines;

    inline Pipeline(decltype(nullptr)) {}
    inline explicit Pipeline(kj::Own<PipelineHook>&& hook): hook(kj::mv(hook)) {}

    Pipeline noop();
    // Just make a copy.

    Pipeline getPointerField(uint16_t pointerIndex);
    // Deprecated. In the future, we should use .asAnyStruct.getPointerField.

    inline AnyStruct::Pipeline asAnyStruct();

    kj::Own<ClientHook> asCap();
    // Expect that the result is a capability and construct a pipelined version of it now.

    inline kj::Own<PipelineHook> releasePipelineHook() { return kj::mv(hook); }
    // For use by RPC implementations.

    template <typename T, typename = kj::EnableIf<CAPNP_KIND(FromClient<T>) == Kind::INTERFACE>>
    inline operator T() { return T(asCap()); }

  private:
    kj::Own<PipelineHook> hook;
    kj::Array<PipelineOp> ops;

    inline Pipeline(kj::Own<PipelineHook>&& hook, kj::Array<PipelineOp>&& ops)
        : hook(kj::mv(hook)), ops(kj::mv(ops)) {}

    friend class LocalClient;
    friend class PipelineHook;
  };
#endif  // !CAPNP_LITE
};

template <>
class Orphan<AnyPointer> {
  // An orphaned object of unknown type.

public:
  Orphan() = default;
  KJ_DISALLOW_COPY(Orphan);
  Orphan(Orphan&&) = default;
  Orphan& operator=(Orphan&&) = default;

  template <typename T>
  inline Orphan(Orphan<T>&& other): builder(kj::mv(other.builder)) {}
  template <typename T>
  inline Orphan& operator=(Orphan<T>&& other) { builder = kj::mv(other.builder); return *this; }
  // Cast from typed orphan.

  // It's not possible to get an AnyPointer::{Reader,Builder} directly since there is no
  // underlying pointer (the pointer would normally live in the parent, but this object is
  // orphaned).  It is possible, however, to request typed readers/builders.

  template <typename T>
  inline BuilderFor<T> getAs();
  template <typename T>
  inline BuilderFor<T> getAs(StructSchema schema);
  template <typename T>
  inline BuilderFor<T> getAs(ListSchema schema);
  template <typename T>
  inline typename T::Client getAs(InterfaceSchema schema);
  template <typename T>
  inline ReaderFor<T> getAsReader() const;
  template <typename T>
  inline ReaderFor<T> getAsReader(StructSchema schema) const;
  template <typename T>
  inline ReaderFor<T> getAsReader(ListSchema schema) const;
  template <typename T>
  inline typename T::Client getAsReader(InterfaceSchema schema) const;

  template <typename T>
  inline Orphan<T> releaseAs();
  template <typename T>
  inline Orphan<T> releaseAs(StructSchema schema);
  template <typename T>
  inline Orphan<T> releaseAs(ListSchema schema);
  template <typename T>
  inline Orphan<T> releaseAs(InterfaceSchema schema);
  // Down-cast the orphan to a specific type.

  inline bool operator==(decltype(nullptr)) const { return builder == nullptr; }
  inline bool operator!=(decltype(nullptr)) const { return builder != nullptr; }

private:
  _::OrphanBuilder builder;

  inline Orphan(_::OrphanBuilder&& builder)
      : builder(kj::mv(builder)) {}

  template <typename, Kind>
  friend struct _::PointerHelpers;
  friend class Orphanage;
  template <typename U>
  friend class Orphan;
  friend class AnyPointer::Builder;
};

template <Kind k> struct AnyTypeFor_;
template <> struct AnyTypeFor_<Kind::STRUCT> { typedef AnyStruct Type; };
template <> struct AnyTypeFor_<Kind::LIST> { typedef AnyList Type; };

template <typename T>
using AnyTypeFor = typename AnyTypeFor_<CAPNP_KIND(T)>::Type;

template <typename T>
inline ReaderFor<AnyTypeFor<FromReader<T>>> toAny(T&& value) {
  return ReaderFor<AnyTypeFor<FromReader<T>>>(
      _::PointerHelpers<FromReader<T>>::getInternalReader(value));
}
template <typename T>
inline BuilderFor<AnyTypeFor<FromBuilder<T>>> toAny(T&& value) {
  return BuilderFor<AnyTypeFor<FromBuilder<T>>>(
      _::PointerHelpers<FromBuilder<T>>::getInternalBuilder(kj::mv(value)));
}

template <>
struct List<AnyPointer, Kind::OTHER> {
  // Note: This cannot be used for a list of structs, since such lists are not encoded as pointer
  //   lists! Use List<AnyStruct>.

  List() = delete;

  class Reader {
  public:
    typedef List<AnyPointer> Reads;

    Reader() = default;
    inline explicit Reader(_::ListReader reader): reader(reader) {}

    inline uint size() const { return reader.size() / ELEMENTS; }
    inline AnyPointer::Reader operator[](uint index) const {
      KJ_IREQUIRE(index < size());
      return AnyPointer::Reader(reader.getPointerElement(index * ELEMENTS));
    }

    typedef _::IndexingIterator<const Reader, typename AnyPointer::Reader> Iterator;
    inline Iterator begin() const { return Iterator(this, 0); }
    inline Iterator end() const { return Iterator(this, size()); }

  private:
    _::ListReader reader;
    template <typename U, Kind K>
    friend struct _::PointerHelpers;
    template <typename U, Kind K>
    friend struct List;
    friend class Orphanage;
    template <typename U, Kind K>
    friend struct ToDynamic_;
  };

  class Builder {
  public:
    typedef List<AnyPointer> Builds;

    Builder() = delete;
    inline Builder(decltype(nullptr)) {}
    inline explicit Builder(_::ListBuilder builder): builder(builder) {}

    inline operator Reader() { return Reader(builder.asReader()); }
    inline Reader asReader() { return Reader(builder.asReader()); }

    inline uint size() const { return builder.size() / ELEMENTS; }
    inline AnyPointer::Builder operator[](uint index) {
      KJ_IREQUIRE(index < size());
      return AnyPointer::Builder(builder.getPointerElement(index * ELEMENTS));
    }

    typedef _::IndexingIterator<Builder, typename AnyPointer::Builder> Iterator;
    inline Iterator begin() { return Iterator(this, 0); }
    inline Iterator end() { return Iterator(this, size()); }

  private:
    _::ListBuilder builder;
    template <typename, Kind>
    friend struct _::PointerHelpers;
    friend class Orphanage;
    template <typename, Kind>
    friend struct ToDynamic_;
  };
};

class AnyStruct::Reader {
public:
  Reader() = default;
  inline Reader(_::StructReader reader): _reader(reader) {}

#if !_MSC_VER  // TODO(msvc): MSVC ICEs on this. Try restoring when compiler improves.
  template <typename T, typename = kj::EnableIf<CAPNP_KIND(FromReader<T>) == Kind::STRUCT>>
  inline Reader(T&& value)
      : _reader(_::PointerHelpers<FromReader<T>>::getInternalReader(kj::fwd<T>(value))) {}
#endif

  Data::Reader getDataSection() {
    return _reader.getDataSectionAsBlob();
  }
  List<AnyPointer>::Reader getPointerSection() {
    return List<AnyPointer>::Reader(_reader.getPointerSectionAsList());
  }

  template <typename T>
  ReaderFor<T> as();
  // T must be a struct type.
private:
  _::StructReader _reader;

  template <typename, Kind>
  friend struct _::PointerHelpers;
};

class AnyStruct::Builder {
public:
  inline Builder(decltype(nullptr)) {}
  inline Builder(_::StructBuilder builder): _builder(builder) {}

#if !_MSC_VER  // TODO(msvc): MSVC ICEs on this. Try restoring when compiler improves.
  template <typename T, typename = kj::EnableIf<CAPNP_KIND(FromBuilder<T>) == Kind::STRUCT>>
  inline Builder(T&& value)
      : _builder(_::PointerHelpers<FromBuilder<T>>::getInternalBuilder(kj::fwd<T>(value))) {}
#endif

  inline Data::Builder getDataSection() {
    return _builder.getDataSectionAsBlob();
  }
  List<AnyPointer>::Builder getPointerSection() {
    return List<AnyPointer>::Builder(_builder.getPointerSectionAsList());
  }

  inline operator Reader() const { return Reader(_builder.asReader()); }
  inline Reader asReader() const { return Reader(_builder.asReader()); }
private:
  _::StructBuilder _builder;
  friend class Orphanage;
  friend class CapBuilderContext;
};

#if !CAPNP_LITE
class AnyStruct::Pipeline {
public:
  Pipeline getPointerField(uint16_t pointerIndex);
  // Return a new Promise representing a sub-object of the result.  `pointerIndex` is the index
  // of the sub-object within the pointer section of the result (the result must be a struct).
  //
  // TODO(perf):  On GCC 4.8 / Clang 3.3, use rvalue qualifiers to avoid the need for copies.
  //   Also make `ops` into a Vector to optimize this.

private:
  kj::Own<PipelineHook> hook;
  kj::Array<PipelineOp> ops;

  inline Pipeline(kj::Own<PipelineHook>&& hook, kj::Array<PipelineOp>&& ops)
      : hook(kj::mv(hook)), ops(kj::mv(ops)) {}

};
#endif  // !CAPNP_LITE

class List<AnyStruct, Kind::OTHER>::Reader {
public:
  typedef List<AnyStruct> Reads;

  Reader() = default;
  inline explicit Reader(_::ListReader reader): reader(reader) {}

  inline uint size() const { return reader.size() / ELEMENTS; }
  inline AnyStruct::Reader operator[](uint index) const {
    KJ_IREQUIRE(index < size());
    return AnyStruct::Reader(reader.getStructElement(index * ELEMENTS));
  }

  typedef _::IndexingIterator<const Reader, typename AnyStruct::Reader> Iterator;
  inline Iterator begin() const { return Iterator(this, 0); }
  inline Iterator end() const { return Iterator(this, size()); }

private:
  _::ListReader reader;
  template <typename U, Kind K>
  friend struct _::PointerHelpers;
  template <typename U, Kind K>
  friend struct List;
  friend class Orphanage;
  template <typename U, Kind K>
  friend struct ToDynamic_;
};


class List<AnyStruct, Kind::OTHER>::Builder {
public:
  typedef List<AnyStruct> Builds;

  Builder() = delete;
  inline Builder(decltype(nullptr)) {}
  inline explicit Builder(_::ListBuilder builder): builder(builder) {}

  inline operator Reader() { return Reader(builder.asReader()); }
  inline Reader asReader() { return Reader(builder.asReader()); }

  inline uint size() const { return builder.size() / ELEMENTS; }
  inline AnyStruct::Builder operator[](uint index) {
    KJ_IREQUIRE(index < size());
    return AnyStruct::Builder(builder.getStructElement(index * ELEMENTS));
  }

  typedef _::IndexingIterator<Builder, typename AnyStruct::Builder> Iterator;
  inline Iterator begin() { return Iterator(this, 0); }
  inline Iterator end() { return Iterator(this, size()); }

private:
  _::ListBuilder builder;
  template <typename U, Kind K>
  friend struct _::PointerHelpers;
  friend class Orphanage;
  template <typename U, Kind K>
  friend struct ToDynamic_;
};

class AnyList::Reader {
public:
  Reader() = default;
  inline Reader(_::ListReader reader): _reader(reader) {}

#if !_MSC_VER  // TODO(msvc): MSVC ICEs on this. Try restoring when compiler improves.
  template <typename T, typename = kj::EnableIf<CAPNP_KIND(FromReader<T>) == Kind::LIST>>
  inline Reader(T&& value)
      : _reader(_::PointerHelpers<FromReader<T>>::getInternalReader(kj::fwd<T>(value))) {}
#endif

  inline ElementSize getElementSize() { return _reader.getElementSize(); }
  inline uint size() { return _reader.size() / ELEMENTS; }

  template <typename T> ReaderFor<T> as() {
    // T must be List<U>.
    return ReaderFor<T>(_reader);
  }
private:
  _::ListReader _reader;

  template <typename, Kind>
  friend struct _::PointerHelpers;
};

class AnyList::Builder {
public:
  inline Builder(decltype(nullptr)) {}
  inline Builder(_::ListBuilder builder): _builder(builder) {}

#if !_MSC_VER  // TODO(msvc): MSVC ICEs on this. Try restoring when compiler improves.
  template <typename T, typename = kj::EnableIf<CAPNP_KIND(FromBuilder<T>) == Kind::LIST>>
  inline Builder(T&& value)
      : _builder(_::PointerHelpers<FromBuilder<T>>::getInternalBuilder(kj::fwd<T>(value))) {}
#endif

  inline ElementSize getElementSize() { return _builder.getElementSize(); }
  inline uint size() { return _builder.size() / ELEMENTS; }

  template <typename T> BuilderFor<T> as() {
    // T must be List<U>.
    return BuilderFor<T>(_builder);
  }

  inline operator Reader() const { return Reader(_builder.asReader()); }
  inline Reader asReader() const { return Reader(_builder.asReader()); }

private:
  _::ListBuilder _builder;
};

// =======================================================================================
// Pipeline helpers
//
// These relate to capabilities, but we don't declare them in capability.h because generated code
// for structs needs to know about these, even in files that contain no interfaces.

#if !CAPNP_LITE

struct PipelineOp {
  // Corresponds to rpc.capnp's PromisedAnswer.Op.

  enum Type {
    NOOP,  // for convenience

    GET_POINTER_FIELD

    // There may be other types in the future...
  };

  Type type;
  union {
    uint16_t pointerIndex;  // for GET_POINTER_FIELD
  };
};

class PipelineHook {
  // Represents a currently-running call, and implements pipelined requests on its result.

public:
  virtual kj::Own<PipelineHook> addRef() = 0;
  // Increment this object's reference count.

  virtual kj::Own<ClientHook> getPipelinedCap(kj::ArrayPtr<const PipelineOp> ops) = 0;
  // Extract a promised Capability from the results.

  virtual kj::Own<ClientHook> getPipelinedCap(kj::Array<PipelineOp>&& ops);
  // Version of getPipelinedCap() passing the array by move.  May avoid a copy in some cases.
  // Default implementation just calls the other version.

  template <typename Pipeline, typename = FromPipeline<Pipeline>>
  static inline kj::Own<PipelineHook> from(Pipeline&& pipeline);

private:
  template <typename T> struct FromImpl;
};

#endif  // !CAPNP_LITE

// =======================================================================================
// Inline implementation details

inline MessageSize AnyPointer::Reader::targetSize() const {
  return reader.targetSize().asPublic();
}

inline bool AnyPointer::Reader::isNull() const {
  return reader.isNull();
}

template <typename T>
inline ReaderFor<T> AnyPointer::Reader::getAs() const {
  return _::PointerHelpers<T>::get(reader);
}

inline MessageSize AnyPointer::Builder::targetSize() const {
  return asReader().targetSize();
}

inline bool AnyPointer::Builder::isNull() {
  return builder.isNull();
}

inline void AnyPointer::Builder::clear() {
  return builder.clear();
}

template <typename T>
inline BuilderFor<T> AnyPointer::Builder::getAs() {
  return _::PointerHelpers<T>::get(builder);
}

template <typename T>
inline BuilderFor<T> AnyPointer::Builder::initAs() {
  return _::PointerHelpers<T>::init(builder);
}

template <typename T>
inline BuilderFor<T> AnyPointer::Builder::initAs(uint elementCount) {
  return _::PointerHelpers<T>::init(builder, elementCount);
}

inline AnyList::Builder AnyPointer::Builder::initAsAnyList(
    ElementSize elementSize, uint elementCount) {
  return AnyList::Builder(builder.initList(elementSize, elementCount * ELEMENTS));
}

inline List<AnyStruct>::Builder AnyPointer::Builder::initAsListOfAnyStruct(
    uint dataWordCount, uint pointerCount, uint elementCount) {
  return List<AnyStruct>::Builder(builder.initStructList(elementCount * ELEMENTS,
      _::StructSize(dataWordCount * WORDS, pointerCount * POINTERS)));
}

inline AnyStruct::Builder AnyPointer::Builder::initAsAnyStruct(uint dataWordCount, uint pointerCount) {
  return AnyStruct::Builder(builder.initStruct(
      _::StructSize(dataWordCount * WORDS, pointerCount * POINTERS)));
}

template <typename T>
inline void AnyPointer::Builder::setAs(ReaderFor<T> value) {
  return _::PointerHelpers<T>::set(builder, value);
}

template <typename T>
inline void AnyPointer::Builder::setAs(
    std::initializer_list<ReaderFor<ListElementType<T>>> list) {
  return _::PointerHelpers<T>::set(builder, list);
}

template <typename T>
inline void AnyPointer::Builder::adopt(Orphan<T>&& orphan) {
  _::PointerHelpers<T>::adopt(builder, kj::mv(orphan));
}

template <typename T>
inline Orphan<T> AnyPointer::Builder::disownAs() {
  return _::PointerHelpers<T>::disown(builder);
}

inline Orphan<AnyPointer> AnyPointer::Builder::disown() {
  return Orphan<AnyPointer>(builder.disown());
}

template <> struct ReaderFor_ <AnyPointer, Kind::OTHER> { typedef AnyPointer::Reader Type; };
template <> struct BuilderFor_<AnyPointer, Kind::OTHER> { typedef AnyPointer::Builder Type; };
template <> struct ReaderFor_ <AnyStruct, Kind::OTHER> { typedef AnyStruct::Reader Type; };
template <> struct BuilderFor_<AnyStruct, Kind::OTHER> { typedef AnyStruct::Builder Type; };

template <>
struct Orphanage::GetInnerReader<AnyPointer, Kind::OTHER> {
  static inline _::PointerReader apply(const AnyPointer::Reader& t) {
    return t.reader;
  }
};

template <>
struct Orphanage::GetInnerBuilder<AnyPointer, Kind::OTHER> {
  static inline _::PointerBuilder apply(AnyPointer::Builder& t) {
    return t.builder;
  }
};

template <typename T>
inline BuilderFor<T> Orphan<AnyPointer>::getAs() {
  return _::OrphanGetImpl<T>::apply(builder);
}
template <typename T>
inline ReaderFor<T> Orphan<AnyPointer>::getAsReader() const {
  return _::OrphanGetImpl<T>::applyReader(builder);
}
template <typename T>
inline Orphan<T> Orphan<AnyPointer>::releaseAs() {
  return Orphan<T>(kj::mv(builder));
}

// Using AnyPointer as the template type should work...

template <>
inline typename AnyPointer::Reader AnyPointer::Reader::getAs<AnyPointer>() const {
  return *this;
}
template <>
inline typename AnyPointer::Builder AnyPointer::Builder::getAs<AnyPointer>() {
  return *this;
}
template <>
inline typename AnyPointer::Builder AnyPointer::Builder::initAs<AnyPointer>() {
  clear();
  return *this;
}
template <>
inline void AnyPointer::Builder::setAs<AnyPointer>(AnyPointer::Reader value) {
  return builder.copyFrom(value.reader);
}
template <>
inline void AnyPointer::Builder::adopt<AnyPointer>(Orphan<AnyPointer>&& orphan) {
  builder.adopt(kj::mv(orphan.builder));
}
template <>
inline Orphan<AnyPointer> AnyPointer::Builder::disownAs<AnyPointer>() {
  return Orphan<AnyPointer>(builder.disown());
}
template <>
inline Orphan<AnyPointer> Orphan<AnyPointer>::releaseAs() {
  return kj::mv(*this);
}

namespace _ {  // private

// Specialize PointerHelpers for AnyPointer.

template <>
struct PointerHelpers<AnyPointer, Kind::OTHER> {
  static inline AnyPointer::Reader get(PointerReader reader,
                                       const void* defaultValue = nullptr,
                                       uint defaultBytes = 0) {
    return AnyPointer::Reader(reader);
  }
  static inline AnyPointer::Builder get(PointerBuilder builder,
                                        const void* defaultValue = nullptr,
                                        uint defaultBytes = 0) {
    return AnyPointer::Builder(builder);
  }
  static inline void set(PointerBuilder builder, AnyPointer::Reader value) {
    AnyPointer::Builder(builder).set(value);
  }
  static inline void adopt(PointerBuilder builder, Orphan<AnyPointer>&& value) {
    builder.adopt(kj::mv(value.builder));
  }
  static inline Orphan<AnyPointer> disown(PointerBuilder builder) {
    return Orphan<AnyPointer>(builder.disown());
  }
};

template <>
struct PointerHelpers<AnyStruct, Kind::OTHER> {
  static inline AnyStruct::Reader get(
      PointerReader reader, const word* defaultValue = nullptr) {
    return AnyStruct::Reader(reader.getStruct(defaultValue));
  }
  static inline AnyStruct::Builder get(
      PointerBuilder builder, const word* defaultValue = nullptr) {
    // TODO(someday): Allow specifying the size somehow?
    return AnyStruct::Builder(builder.getStruct(
        _::StructSize(0 * WORDS, 0 * POINTERS), defaultValue));
  }
  static inline void set(PointerBuilder builder, AnyStruct::Reader value) {
    builder.setStruct(value._reader);
  }
  static inline AnyStruct::Builder init(
      PointerBuilder builder, uint dataWordCount, uint pointerCount) {
    return AnyStruct::Builder(builder.initStruct(
        StructSize(dataWordCount * WORDS, pointerCount * POINTERS)));
  }
};

template <>
struct PointerHelpers<AnyList, Kind::OTHER> {
  static inline AnyList::Reader get(
      PointerReader reader, const word* defaultValue = nullptr) {
    return AnyList::Reader(reader.getListAnySize(defaultValue));
  }
  static inline AnyList::Builder get(
      PointerBuilder builder, const word* defaultValue = nullptr) {
    return AnyList::Builder(builder.getListAnySize(defaultValue));
  }
  static inline void set(PointerBuilder builder, AnyList::Reader value) {
    builder.setList(value._reader);
  }
  static inline AnyList::Builder init(
      PointerBuilder builder, ElementSize elementSize, uint elementCount) {
    return AnyList::Builder(builder.initList(elementSize, elementCount * ELEMENTS));
  }
  static inline AnyList::Builder init(
      PointerBuilder builder, uint dataWordCount, uint pointerCount, uint elementCount) {
    return AnyList::Builder(builder.initStructList(
        elementCount * ELEMENTS, StructSize(dataWordCount * WORDS, pointerCount * POINTERS)));
  }
};

}  // namespace _ (private)

#if !CAPNP_LITE

template <typename T>
struct PipelineHook::FromImpl {
  static inline kj::Own<PipelineHook> apply(typename T::Pipeline&& pipeline) {
    return from(kj::mv(pipeline._typeless));
  }
};

template <>
struct PipelineHook::FromImpl<AnyPointer> {
  static inline kj::Own<PipelineHook> apply(AnyPointer::Pipeline&& pipeline) {
    return kj::mv(pipeline.hook);
  }
};

template <typename Pipeline, typename T>
inline kj::Own<PipelineHook> PipelineHook::from(Pipeline&& pipeline) {
  return FromImpl<T>::apply(kj::fwd<Pipeline>(pipeline));
}

#endif  // !CAPNP_LITE

}  // namespace capnp

#endif  // CAPNP_ANY_H_
