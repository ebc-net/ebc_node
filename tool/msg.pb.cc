// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: msg.proto

#include "msg.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/port.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
// This is a temporary google only hack
#ifdef GOOGLE_PROTOBUF_ENFORCE_UNIQUENESS
#include "third_party/protobuf/version.h"
#endif
// @@protoc_insertion_point(includes)

namespace protobuf_msg_2eproto {
extern PROTOBUF_INTERNAL_EXPORT_protobuf_msg_2eproto ::google::protobuf::internal::SCCInfo<0> scc_info_EbcNode;
extern PROTOBUF_INTERNAL_EXPORT_protobuf_msg_2eproto ::google::protobuf::internal::SCCInfo<1> scc_info_EbcNodes;
}  // namespace protobuf_msg_2eproto
namespace config {
class EbcNodeDefaultTypeInternal {
 public:
  ::google::protobuf::internal::ExplicitlyConstructed<EbcNode>
      _instance;
} _EbcNode_default_instance_;
class EbcNodesDefaultTypeInternal {
 public:
  ::google::protobuf::internal::ExplicitlyConstructed<EbcNodes>
      _instance;
} _EbcNodes_default_instance_;
class EbcMsgDefaultTypeInternal {
 public:
  ::google::protobuf::internal::ExplicitlyConstructed<EbcMsg>
      _instance;
  ::google::protobuf::internal::ArenaStringPtr msg_;
  const ::config::EbcNodes* nodes_;
} _EbcMsg_default_instance_;
}  // namespace config
namespace protobuf_msg_2eproto {
static void InitDefaultsEbcNode() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  {
    void* ptr = &::config::_EbcNode_default_instance_;
    new (ptr) ::config::EbcNode();
    ::google::protobuf::internal::OnShutdownDestroyMessage(ptr);
  }
  ::config::EbcNode::InitAsDefaultInstance();
}

::google::protobuf::internal::SCCInfo<0> scc_info_EbcNode =
    {{ATOMIC_VAR_INIT(::google::protobuf::internal::SCCInfoBase::kUninitialized), 0, InitDefaultsEbcNode}, {}};

static void InitDefaultsEbcNodes() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  {
    void* ptr = &::config::_EbcNodes_default_instance_;
    new (ptr) ::config::EbcNodes();
    ::google::protobuf::internal::OnShutdownDestroyMessage(ptr);
  }
  ::config::EbcNodes::InitAsDefaultInstance();
}

::google::protobuf::internal::SCCInfo<1> scc_info_EbcNodes =
    {{ATOMIC_VAR_INIT(::google::protobuf::internal::SCCInfoBase::kUninitialized), 1, InitDefaultsEbcNodes}, {
      &protobuf_msg_2eproto::scc_info_EbcNode.base,}};

static void InitDefaultsEbcMsg() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  {
    void* ptr = &::config::_EbcMsg_default_instance_;
    new (ptr) ::config::EbcMsg();
    ::google::protobuf::internal::OnShutdownDestroyMessage(ptr);
  }
  ::config::EbcMsg::InitAsDefaultInstance();
}

::google::protobuf::internal::SCCInfo<1> scc_info_EbcMsg =
    {{ATOMIC_VAR_INIT(::google::protobuf::internal::SCCInfoBase::kUninitialized), 1, InitDefaultsEbcMsg}, {
      &protobuf_msg_2eproto::scc_info_EbcNodes.base,}};

void InitDefaults() {
  ::google::protobuf::internal::InitSCC(&scc_info_EbcNode.base);
  ::google::protobuf::internal::InitSCC(&scc_info_EbcNodes.base);
  ::google::protobuf::internal::InitSCC(&scc_info_EbcMsg.base);
}

}  // namespace protobuf_msg_2eproto
namespace config {
bool MsgType_IsValid(int value) {
  switch (value) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
      return true;
    default:
      return false;
  }
}

bool MsgSubType_IsValid(int value) {
  switch (value) {
    case 0:
    case 1:
    case 2:
    case 3:
      return true;
    default:
      return false;
  }
}


// ===================================================================

void EbcNode::InitAsDefaultInstance() {
}
#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int EbcNode::kIdFieldNumber;
const int EbcNode::kIpFieldNumber;
const int EbcNode::kPortNatFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

EbcNode::EbcNode()
  : ::google::protobuf::MessageLite(), _internal_metadata_(NULL) {
  ::google::protobuf::internal::InitSCC(
      &protobuf_msg_2eproto::scc_info_EbcNode.base);
  SharedCtor();
  // @@protoc_insertion_point(constructor:config.EbcNode)
}
EbcNode::EbcNode(const EbcNode& from)
  : ::google::protobuf::MessageLite(),
      _internal_metadata_(NULL) {
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  id_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  if (from.id().size() > 0) {
    id_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.id_);
  }
  ::memcpy(&ip_, &from.ip_,
    static_cast<size_t>(reinterpret_cast<char*>(&port_nat_) -
    reinterpret_cast<char*>(&ip_)) + sizeof(port_nat_));
  // @@protoc_insertion_point(copy_constructor:config.EbcNode)
}

void EbcNode::SharedCtor() {
  id_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  ::memset(&ip_, 0, static_cast<size_t>(
      reinterpret_cast<char*>(&port_nat_) -
      reinterpret_cast<char*>(&ip_)) + sizeof(port_nat_));
}

EbcNode::~EbcNode() {
  // @@protoc_insertion_point(destructor:config.EbcNode)
  SharedDtor();
}

void EbcNode::SharedDtor() {
  id_.DestroyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}

void EbcNode::SetCachedSize(int size) const {
  _cached_size_.Set(size);
}
const EbcNode& EbcNode::default_instance() {
  ::google::protobuf::internal::InitSCC(&protobuf_msg_2eproto::scc_info_EbcNode.base);
  return *internal_default_instance();
}


void EbcNode::Clear() {
// @@protoc_insertion_point(message_clear_start:config.EbcNode)
  ::google::protobuf::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  id_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  ::memset(&ip_, 0, static_cast<size_t>(
      reinterpret_cast<char*>(&port_nat_) -
      reinterpret_cast<char*>(&ip_)) + sizeof(port_nat_));
  _internal_metadata_.Clear();
}

bool EbcNode::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!GOOGLE_PREDICT_TRUE(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  ::google::protobuf::internal::LiteUnknownFieldSetter unknown_fields_setter(
      &_internal_metadata_);
  ::google::protobuf::io::StringOutputStream unknown_fields_output(
      unknown_fields_setter.buffer());
  ::google::protobuf::io::CodedOutputStream unknown_fields_stream(
      &unknown_fields_output, false);
  // @@protoc_insertion_point(parse_start:config.EbcNode)
  for (;;) {
    ::std::pair<::google::protobuf::uint32, bool> p = input->ReadTagWithCutoffNoLastTag(127u);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // bytes id = 1;
      case 1: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(10u /* 10 & 0xFF */)) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadBytes(
                input, this->mutable_id()));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // uint32 ip = 2;
      case 2: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(16u /* 16 & 0xFF */)) {

          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &ip_)));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // uint32 port_nat = 3;
      case 3: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(24u /* 24 & 0xFF */)) {

          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &port_nat_)));
        } else {
          goto handle_unusual;
        }
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormatLite::SkipField(
            input, tag, &unknown_fields_stream));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:config.EbcNode)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:config.EbcNode)
  return false;
#undef DO_
}

void EbcNode::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:config.EbcNode)
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // bytes id = 1;
  if (this->id().size() > 0) {
    ::google::protobuf::internal::WireFormatLite::WriteBytesMaybeAliased(
      1, this->id(), output);
  }

  // uint32 ip = 2;
  if (this->ip() != 0) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(2, this->ip(), output);
  }

  // uint32 port_nat = 3;
  if (this->port_nat() != 0) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(3, this->port_nat(), output);
  }

  output->WriteRaw((::google::protobuf::internal::GetProto3PreserveUnknownsDefault()   ? _internal_metadata_.unknown_fields()   : _internal_metadata_.default_instance()).data(),
                   static_cast<int>((::google::protobuf::internal::GetProto3PreserveUnknownsDefault()   ? _internal_metadata_.unknown_fields()   : _internal_metadata_.default_instance()).size()));
  // @@protoc_insertion_point(serialize_end:config.EbcNode)
}

size_t EbcNode::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:config.EbcNode)
  size_t total_size = 0;

  total_size += (::google::protobuf::internal::GetProto3PreserveUnknownsDefault()   ? _internal_metadata_.unknown_fields()   : _internal_metadata_.default_instance()).size();

  // bytes id = 1;
  if (this->id().size() > 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::BytesSize(
        this->id());
  }

  // uint32 ip = 2;
  if (this->ip() != 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::UInt32Size(
        this->ip());
  }

  // uint32 port_nat = 3;
  if (this->port_nat() != 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::UInt32Size(
        this->port_nat());
  }

  int cached_size = ::google::protobuf::internal::ToCachedSize(total_size);
  SetCachedSize(cached_size);
  return total_size;
}

void EbcNode::CheckTypeAndMergeFrom(
    const ::google::protobuf::MessageLite& from) {
  MergeFrom(*::google::protobuf::down_cast<const EbcNode*>(&from));
}

void EbcNode::MergeFrom(const EbcNode& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:config.EbcNode)
  GOOGLE_DCHECK_NE(&from, this);
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  if (from.id().size() > 0) {

    id_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.id_);
  }
  if (from.ip() != 0) {
    set_ip(from.ip());
  }
  if (from.port_nat() != 0) {
    set_port_nat(from.port_nat());
  }
}

void EbcNode::CopyFrom(const EbcNode& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:config.EbcNode)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool EbcNode::IsInitialized() const {
  return true;
}

void EbcNode::Swap(EbcNode* other) {
  if (other == this) return;
  InternalSwap(other);
}
void EbcNode::InternalSwap(EbcNode* other) {
  using std::swap;
  id_.Swap(&other->id_, &::google::protobuf::internal::GetEmptyStringAlreadyInited(),
    GetArenaNoVirtual());
  swap(ip_, other->ip_);
  swap(port_nat_, other->port_nat_);
  _internal_metadata_.Swap(&other->_internal_metadata_);
}

::std::string EbcNode::GetTypeName() const {
  return "config.EbcNode";
}


// ===================================================================

void EbcNodes::InitAsDefaultInstance() {
}
#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int EbcNodes::kEbcNodesFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

EbcNodes::EbcNodes()
  : ::google::protobuf::MessageLite(), _internal_metadata_(NULL) {
  ::google::protobuf::internal::InitSCC(
      &protobuf_msg_2eproto::scc_info_EbcNodes.base);
  SharedCtor();
  // @@protoc_insertion_point(constructor:config.EbcNodes)
}
EbcNodes::EbcNodes(const EbcNodes& from)
  : ::google::protobuf::MessageLite(),
      _internal_metadata_(NULL),
      ebcnodes_(from.ebcnodes_) {
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  // @@protoc_insertion_point(copy_constructor:config.EbcNodes)
}

void EbcNodes::SharedCtor() {
}

EbcNodes::~EbcNodes() {
  // @@protoc_insertion_point(destructor:config.EbcNodes)
  SharedDtor();
}

void EbcNodes::SharedDtor() {
}

void EbcNodes::SetCachedSize(int size) const {
  _cached_size_.Set(size);
}
const EbcNodes& EbcNodes::default_instance() {
  ::google::protobuf::internal::InitSCC(&protobuf_msg_2eproto::scc_info_EbcNodes.base);
  return *internal_default_instance();
}


void EbcNodes::Clear() {
// @@protoc_insertion_point(message_clear_start:config.EbcNodes)
  ::google::protobuf::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  ebcnodes_.Clear();
  _internal_metadata_.Clear();
}

bool EbcNodes::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!GOOGLE_PREDICT_TRUE(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  ::google::protobuf::internal::LiteUnknownFieldSetter unknown_fields_setter(
      &_internal_metadata_);
  ::google::protobuf::io::StringOutputStream unknown_fields_output(
      unknown_fields_setter.buffer());
  ::google::protobuf::io::CodedOutputStream unknown_fields_stream(
      &unknown_fields_output, false);
  // @@protoc_insertion_point(parse_start:config.EbcNodes)
  for (;;) {
    ::std::pair<::google::protobuf::uint32, bool> p = input->ReadTagWithCutoffNoLastTag(127u);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // repeated .config.EbcNode ebcNodes = 1;
      case 1: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(10u /* 10 & 0xFF */)) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadMessage(
                input, add_ebcnodes()));
        } else {
          goto handle_unusual;
        }
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormatLite::SkipField(
            input, tag, &unknown_fields_stream));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:config.EbcNodes)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:config.EbcNodes)
  return false;
#undef DO_
}

void EbcNodes::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:config.EbcNodes)
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // repeated .config.EbcNode ebcNodes = 1;
  for (unsigned int i = 0,
      n = static_cast<unsigned int>(this->ebcnodes_size()); i < n; i++) {
    ::google::protobuf::internal::WireFormatLite::WriteMessage(
      1,
      this->ebcnodes(static_cast<int>(i)),
      output);
  }

  output->WriteRaw((::google::protobuf::internal::GetProto3PreserveUnknownsDefault()   ? _internal_metadata_.unknown_fields()   : _internal_metadata_.default_instance()).data(),
                   static_cast<int>((::google::protobuf::internal::GetProto3PreserveUnknownsDefault()   ? _internal_metadata_.unknown_fields()   : _internal_metadata_.default_instance()).size()));
  // @@protoc_insertion_point(serialize_end:config.EbcNodes)
}

size_t EbcNodes::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:config.EbcNodes)
  size_t total_size = 0;

  total_size += (::google::protobuf::internal::GetProto3PreserveUnknownsDefault()   ? _internal_metadata_.unknown_fields()   : _internal_metadata_.default_instance()).size();

  // repeated .config.EbcNode ebcNodes = 1;
  {
    unsigned int count = static_cast<unsigned int>(this->ebcnodes_size());
    total_size += 1UL * count;
    for (unsigned int i = 0; i < count; i++) {
      total_size +=
        ::google::protobuf::internal::WireFormatLite::MessageSize(
          this->ebcnodes(static_cast<int>(i)));
    }
  }

  int cached_size = ::google::protobuf::internal::ToCachedSize(total_size);
  SetCachedSize(cached_size);
  return total_size;
}

void EbcNodes::CheckTypeAndMergeFrom(
    const ::google::protobuf::MessageLite& from) {
  MergeFrom(*::google::protobuf::down_cast<const EbcNodes*>(&from));
}

void EbcNodes::MergeFrom(const EbcNodes& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:config.EbcNodes)
  GOOGLE_DCHECK_NE(&from, this);
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  ebcnodes_.MergeFrom(from.ebcnodes_);
}

void EbcNodes::CopyFrom(const EbcNodes& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:config.EbcNodes)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool EbcNodes::IsInitialized() const {
  return true;
}

void EbcNodes::Swap(EbcNodes* other) {
  if (other == this) return;
  InternalSwap(other);
}
void EbcNodes::InternalSwap(EbcNodes* other) {
  using std::swap;
  CastToBase(&ebcnodes_)->InternalSwap(CastToBase(&other->ebcnodes_));
  _internal_metadata_.Swap(&other->_internal_metadata_);
}

::std::string EbcNodes::GetTypeName() const {
  return "config.EbcNodes";
}


// ===================================================================

void EbcMsg::InitAsDefaultInstance() {
}
void EbcMsg::set_allocated_nodes(::config::EbcNodes* nodes) {
  ::google::protobuf::Arena* message_arena = GetArenaNoVirtual();
  clear_body();
  if (nodes) {
    ::google::protobuf::Arena* submessage_arena = NULL;
    if (message_arena != submessage_arena) {
      nodes = ::google::protobuf::internal::GetOwnedMessage(
          message_arena, nodes, submessage_arena);
    }
    set_has_nodes();
    body_.nodes_ = nodes;
  }
  // @@protoc_insertion_point(field_set_allocated:config.EbcMsg.nodes)
}
#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int EbcMsg::kHeadFieldNumber;
const int EbcMsg::kVersionFieldNumber;
const int EbcMsg::kSrcIdFieldNumber;
const int EbcMsg::kDstIdFieldNumber;
const int EbcMsg::kTtlFieldNumber;
const int EbcMsg::kIdFieldNumber;
const int EbcMsg::kTypeFieldNumber;
const int EbcMsg::kSubTypeFieldNumber;
const int EbcMsg::kLengthFieldNumber;
const int EbcMsg::kMsgFieldNumber;
const int EbcMsg::kNodesFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

EbcMsg::EbcMsg()
  : ::google::protobuf::MessageLite(), _internal_metadata_(NULL) {
  ::google::protobuf::internal::InitSCC(
      &protobuf_msg_2eproto::scc_info_EbcMsg.base);
  SharedCtor();
  // @@protoc_insertion_point(constructor:config.EbcMsg)
}
EbcMsg::EbcMsg(const EbcMsg& from)
  : ::google::protobuf::MessageLite(),
      _internal_metadata_(NULL) {
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  version_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  if (from.version().size() > 0) {
    version_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.version_);
  }
  src_id_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  if (from.src_id().size() > 0) {
    src_id_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.src_id_);
  }
  dst_id_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  if (from.dst_id().size() > 0) {
    dst_id_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.dst_id_);
  }
  ::memcpy(&head_, &from.head_,
    static_cast<size_t>(reinterpret_cast<char*>(&length_) -
    reinterpret_cast<char*>(&head_)) + sizeof(length_));
  clear_has_body();
  switch (from.body_case()) {
    case kMsg: {
      set_msg(from.msg());
      break;
    }
    case kNodes: {
      mutable_nodes()->::config::EbcNodes::MergeFrom(from.nodes());
      break;
    }
    case BODY_NOT_SET: {
      break;
    }
  }
  // @@protoc_insertion_point(copy_constructor:config.EbcMsg)
}

void EbcMsg::SharedCtor() {
  version_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  src_id_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  dst_id_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  ::memset(&head_, 0, static_cast<size_t>(
      reinterpret_cast<char*>(&length_) -
      reinterpret_cast<char*>(&head_)) + sizeof(length_));
  clear_has_body();
}

EbcMsg::~EbcMsg() {
  // @@protoc_insertion_point(destructor:config.EbcMsg)
  SharedDtor();
}

void EbcMsg::SharedDtor() {
  version_.DestroyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  src_id_.DestroyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  dst_id_.DestroyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  if (has_body()) {
    clear_body();
  }
}

void EbcMsg::SetCachedSize(int size) const {
  _cached_size_.Set(size);
}
const EbcMsg& EbcMsg::default_instance() {
  ::google::protobuf::internal::InitSCC(&protobuf_msg_2eproto::scc_info_EbcMsg.base);
  return *internal_default_instance();
}


void EbcMsg::clear_body() {
// @@protoc_insertion_point(one_of_clear_start:config.EbcMsg)
  switch (body_case()) {
    case kMsg: {
      body_.msg_.DestroyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
      break;
    }
    case kNodes: {
      delete body_.nodes_;
      break;
    }
    case BODY_NOT_SET: {
      break;
    }
  }
  _oneof_case_[0] = BODY_NOT_SET;
}


void EbcMsg::Clear() {
// @@protoc_insertion_point(message_clear_start:config.EbcMsg)
  ::google::protobuf::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  version_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  src_id_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  dst_id_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  ::memset(&head_, 0, static_cast<size_t>(
      reinterpret_cast<char*>(&length_) -
      reinterpret_cast<char*>(&head_)) + sizeof(length_));
  clear_body();
  _internal_metadata_.Clear();
}

bool EbcMsg::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!GOOGLE_PREDICT_TRUE(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  ::google::protobuf::internal::LiteUnknownFieldSetter unknown_fields_setter(
      &_internal_metadata_);
  ::google::protobuf::io::StringOutputStream unknown_fields_output(
      unknown_fields_setter.buffer());
  ::google::protobuf::io::CodedOutputStream unknown_fields_stream(
      &unknown_fields_output, false);
  // @@protoc_insertion_point(parse_start:config.EbcMsg)
  for (;;) {
    ::std::pair<::google::protobuf::uint32, bool> p = input->ReadTagWithCutoffNoLastTag(127u);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // uint32 head = 1;
      case 1: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(8u /* 8 & 0xFF */)) {

          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &head_)));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // string version = 2;
      case 2: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(18u /* 18 & 0xFF */)) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_version()));
          DO_(::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
            this->version().data(), static_cast<int>(this->version().length()),
            ::google::protobuf::internal::WireFormatLite::PARSE,
            "config.EbcMsg.version"));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // bytes src_id = 3;
      case 3: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(26u /* 26 & 0xFF */)) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadBytes(
                input, this->mutable_src_id()));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // bytes dst_id = 4;
      case 4: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(34u /* 34 & 0xFF */)) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadBytes(
                input, this->mutable_dst_id()));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // uint32 ttl = 5;
      case 5: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(40u /* 40 & 0xFF */)) {

          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &ttl_)));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // uint32 id = 6;
      case 6: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(48u /* 48 & 0xFF */)) {

          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &id_)));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // .config.MsgType type = 7;
      case 7: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(56u /* 56 & 0xFF */)) {
          int value;
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   int, ::google::protobuf::internal::WireFormatLite::TYPE_ENUM>(
                 input, &value)));
          set_type(static_cast< ::config::MsgType >(value));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // .config.MsgSubType sub_type = 8;
      case 8: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(64u /* 64 & 0xFF */)) {
          int value;
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   int, ::google::protobuf::internal::WireFormatLite::TYPE_ENUM>(
                 input, &value)));
          set_sub_type(static_cast< ::config::MsgSubType >(value));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // uint32 length = 9;
      case 9: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(72u /* 72 & 0xFF */)) {

          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &length_)));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // bytes msg = 10;
      case 10: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(82u /* 82 & 0xFF */)) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadBytes(
                input, this->mutable_msg()));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // .config.EbcNodes nodes = 11;
      case 11: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(90u /* 90 & 0xFF */)) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadMessage(
               input, mutable_nodes()));
        } else {
          goto handle_unusual;
        }
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormatLite::SkipField(
            input, tag, &unknown_fields_stream));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:config.EbcMsg)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:config.EbcMsg)
  return false;
#undef DO_
}

void EbcMsg::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:config.EbcMsg)
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // uint32 head = 1;
  if (this->head() != 0) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(1, this->head(), output);
  }

  // string version = 2;
  if (this->version().size() > 0) {
    ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
      this->version().data(), static_cast<int>(this->version().length()),
      ::google::protobuf::internal::WireFormatLite::SERIALIZE,
      "config.EbcMsg.version");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      2, this->version(), output);
  }

  // bytes src_id = 3;
  if (this->src_id().size() > 0) {
    ::google::protobuf::internal::WireFormatLite::WriteBytesMaybeAliased(
      3, this->src_id(), output);
  }

  // bytes dst_id = 4;
  if (this->dst_id().size() > 0) {
    ::google::protobuf::internal::WireFormatLite::WriteBytesMaybeAliased(
      4, this->dst_id(), output);
  }

  // uint32 ttl = 5;
  if (this->ttl() != 0) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(5, this->ttl(), output);
  }

  // uint32 id = 6;
  if (this->id() != 0) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(6, this->id(), output);
  }

  // .config.MsgType type = 7;
  if (this->type() != 0) {
    ::google::protobuf::internal::WireFormatLite::WriteEnum(
      7, this->type(), output);
  }

  // .config.MsgSubType sub_type = 8;
  if (this->sub_type() != 0) {
    ::google::protobuf::internal::WireFormatLite::WriteEnum(
      8, this->sub_type(), output);
  }

  // uint32 length = 9;
  if (this->length() != 0) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(9, this->length(), output);
  }

  // bytes msg = 10;
  if (has_msg()) {
    ::google::protobuf::internal::WireFormatLite::WriteBytesMaybeAliased(
      10, this->msg(), output);
  }

  // .config.EbcNodes nodes = 11;
  if (has_nodes()) {
    ::google::protobuf::internal::WireFormatLite::WriteMessage(
      11, this->_internal_nodes(), output);
  }

  output->WriteRaw((::google::protobuf::internal::GetProto3PreserveUnknownsDefault()   ? _internal_metadata_.unknown_fields()   : _internal_metadata_.default_instance()).data(),
                   static_cast<int>((::google::protobuf::internal::GetProto3PreserveUnknownsDefault()   ? _internal_metadata_.unknown_fields()   : _internal_metadata_.default_instance()).size()));
  // @@protoc_insertion_point(serialize_end:config.EbcMsg)
}

size_t EbcMsg::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:config.EbcMsg)
  size_t total_size = 0;

  total_size += (::google::protobuf::internal::GetProto3PreserveUnknownsDefault()   ? _internal_metadata_.unknown_fields()   : _internal_metadata_.default_instance()).size();

  // string version = 2;
  if (this->version().size() > 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::StringSize(
        this->version());
  }

  // bytes src_id = 3;
  if (this->src_id().size() > 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::BytesSize(
        this->src_id());
  }

  // bytes dst_id = 4;
  if (this->dst_id().size() > 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::BytesSize(
        this->dst_id());
  }

  // uint32 head = 1;
  if (this->head() != 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::UInt32Size(
        this->head());
  }

  // uint32 ttl = 5;
  if (this->ttl() != 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::UInt32Size(
        this->ttl());
  }

  // uint32 id = 6;
  if (this->id() != 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::UInt32Size(
        this->id());
  }

  // .config.MsgType type = 7;
  if (this->type() != 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::EnumSize(this->type());
  }

  // .config.MsgSubType sub_type = 8;
  if (this->sub_type() != 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::EnumSize(this->sub_type());
  }

  // uint32 length = 9;
  if (this->length() != 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::UInt32Size(
        this->length());
  }

  switch (body_case()) {
    // bytes msg = 10;
    case kMsg: {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::BytesSize(
          this->msg());
      break;
    }
    // .config.EbcNodes nodes = 11;
    case kNodes: {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::MessageSize(
          *body_.nodes_);
      break;
    }
    case BODY_NOT_SET: {
      break;
    }
  }
  int cached_size = ::google::protobuf::internal::ToCachedSize(total_size);
  SetCachedSize(cached_size);
  return total_size;
}

void EbcMsg::CheckTypeAndMergeFrom(
    const ::google::protobuf::MessageLite& from) {
  MergeFrom(*::google::protobuf::down_cast<const EbcMsg*>(&from));
}

void EbcMsg::MergeFrom(const EbcMsg& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:config.EbcMsg)
  GOOGLE_DCHECK_NE(&from, this);
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  if (from.version().size() > 0) {

    version_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.version_);
  }
  if (from.src_id().size() > 0) {

    src_id_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.src_id_);
  }
  if (from.dst_id().size() > 0) {

    dst_id_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.dst_id_);
  }
  if (from.head() != 0) {
    set_head(from.head());
  }
  if (from.ttl() != 0) {
    set_ttl(from.ttl());
  }
  if (from.id() != 0) {
    set_id(from.id());
  }
  if (from.type() != 0) {
    set_type(from.type());
  }
  if (from.sub_type() != 0) {
    set_sub_type(from.sub_type());
  }
  if (from.length() != 0) {
    set_length(from.length());
  }
  switch (from.body_case()) {
    case kMsg: {
      set_msg(from.msg());
      break;
    }
    case kNodes: {
      mutable_nodes()->::config::EbcNodes::MergeFrom(from.nodes());
      break;
    }
    case BODY_NOT_SET: {
      break;
    }
  }
}

void EbcMsg::CopyFrom(const EbcMsg& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:config.EbcMsg)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool EbcMsg::IsInitialized() const {
  return true;
}

void EbcMsg::Swap(EbcMsg* other) {
  if (other == this) return;
  InternalSwap(other);
}
void EbcMsg::InternalSwap(EbcMsg* other) {
  using std::swap;
  version_.Swap(&other->version_, &::google::protobuf::internal::GetEmptyStringAlreadyInited(),
    GetArenaNoVirtual());
  src_id_.Swap(&other->src_id_, &::google::protobuf::internal::GetEmptyStringAlreadyInited(),
    GetArenaNoVirtual());
  dst_id_.Swap(&other->dst_id_, &::google::protobuf::internal::GetEmptyStringAlreadyInited(),
    GetArenaNoVirtual());
  swap(head_, other->head_);
  swap(ttl_, other->ttl_);
  swap(id_, other->id_);
  swap(type_, other->type_);
  swap(sub_type_, other->sub_type_);
  swap(length_, other->length_);
  swap(body_, other->body_);
  swap(_oneof_case_[0], other->_oneof_case_[0]);
  _internal_metadata_.Swap(&other->_internal_metadata_);
}

::std::string EbcMsg::GetTypeName() const {
  return "config.EbcMsg";
}


// @@protoc_insertion_point(namespace_scope)
}  // namespace config
namespace google {
namespace protobuf {
template<> GOOGLE_PROTOBUF_ATTRIBUTE_NOINLINE ::config::EbcNode* Arena::CreateMaybeMessage< ::config::EbcNode >(Arena* arena) {
  return Arena::CreateInternal< ::config::EbcNode >(arena);
}
template<> GOOGLE_PROTOBUF_ATTRIBUTE_NOINLINE ::config::EbcNodes* Arena::CreateMaybeMessage< ::config::EbcNodes >(Arena* arena) {
  return Arena::CreateInternal< ::config::EbcNodes >(arena);
}
template<> GOOGLE_PROTOBUF_ATTRIBUTE_NOINLINE ::config::EbcMsg* Arena::CreateMaybeMessage< ::config::EbcMsg >(Arena* arena) {
  return Arena::CreateInternal< ::config::EbcMsg >(arena);
}
}  // namespace protobuf
}  // namespace google

// @@protoc_insertion_point(global_scope)
