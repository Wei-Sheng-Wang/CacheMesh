# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# NO CHECKED-IN PROTOBUF GENCODE
# source: distributed-cache.proto
# Protobuf Python Version: 5.28.1
"""Generated protocol buffer code."""
from google.protobuf import descriptor as _descriptor
from google.protobuf import descriptor_pool as _descriptor_pool
from google.protobuf import runtime_version as _runtime_version
from google.protobuf import symbol_database as _symbol_database
from google.protobuf.internal import builder as _builder
_runtime_version.ValidateProtobufRuntimeVersion(
    _runtime_version.Domain.PUBLIC,
    5,
    28,
    1,
    '',
    'distributed-cache.proto'
)
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()




DESCRIPTOR = _descriptor_pool.Default().AddSerializedFile(b'\n\x17\x64istributed-cache.proto\x12\x11\x64istributed_cache\"\x19\n\nGetRequest\x12\x0b\n\x03key\x18\x01 \x01(\t\"-\n\x0bGetResponse\x12\r\n\x05value\x18\x01 \x01(\t\x12\x0f\n\x07success\x18\x02 \x01(\x08\"Z\n\nPutRequest\x12\x0b\n\x03key\x18\x01 \x01(\t\x12\r\n\x05value\x18\x02 \x01(\t\x12\x0b\n\x03ttl\x18\x03 \x01(\x03\x12\x12\n\nis_replica\x18\x04 \x01(\x08\x12\x0f\n\x07success\x18\x05 \x01(\x08\"\x1e\n\x0bPutResponse\x12\x0f\n\x07success\x18\x01 \x01(\x08\"\x1c\n\rRemoveRequest\x12\x0b\n\x03key\x18\x01 \x01(\t\"!\n\x0eRemoveResponse\x12\x0f\n\x07success\x18\x01 \x01(\x08\x32\xed\x01\n\x10\x44istributedCache\x12\x44\n\x03Get\x12\x1d.distributed_cache.GetRequest\x1a\x1e.distributed_cache.GetResponse\x12\x44\n\x03Put\x12\x1d.distributed_cache.PutRequest\x1a\x1e.distributed_cache.PutResponse\x12M\n\x06Remove\x12 .distributed_cache.RemoveRequest\x1a!.distributed_cache.RemoveResponseb\x06proto3')

_globals = globals()
_builder.BuildMessageAndEnumDescriptors(DESCRIPTOR, _globals)
_builder.BuildTopDescriptorsAndMessages(DESCRIPTOR, 'distributed_cache_pb2', _globals)
if not _descriptor._USE_C_DESCRIPTORS:
  DESCRIPTOR._loaded_options = None
  _globals['_GETREQUEST']._serialized_start=46
  _globals['_GETREQUEST']._serialized_end=71
  _globals['_GETRESPONSE']._serialized_start=73
  _globals['_GETRESPONSE']._serialized_end=118
  _globals['_PUTREQUEST']._serialized_start=120
  _globals['_PUTREQUEST']._serialized_end=210
  _globals['_PUTRESPONSE']._serialized_start=212
  _globals['_PUTRESPONSE']._serialized_end=242
  _globals['_REMOVEREQUEST']._serialized_start=244
  _globals['_REMOVEREQUEST']._serialized_end=272
  _globals['_REMOVERESPONSE']._serialized_start=274
  _globals['_REMOVERESPONSE']._serialized_end=307
  _globals['_DISTRIBUTEDCACHE']._serialized_start=310
  _globals['_DISTRIBUTEDCACHE']._serialized_end=547
# @@protoc_insertion_point(module_scope)