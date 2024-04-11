// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_FOOTER_MINKNOW_READSFORMAT_H_
#define FLATBUFFERS_GENERATED_FOOTER_MINKNOW_READSFORMAT_H_

#include "flatbuffers/flatbuffers.h"

namespace Minknow {
namespace ReadsFormat {

struct EmbeddedFile;
struct EmbeddedFileBuilder;

struct Footer;
struct FooterBuilder;

enum ContentType : int16_t {
  ContentType_ReadsTable = 0,
  ContentType_SignalTable = 1,
  ContentType_ReadIdIndex = 2,
  ContentType_OtherIndex = 3,
  ContentType_RunInfoTable = 4,
  ContentType_MIN = ContentType_ReadsTable,
  ContentType_MAX = ContentType_RunInfoTable
};

inline const ContentType (&EnumValuesContentType())[5] {
  static const ContentType values[] = {
    ContentType_ReadsTable,
    ContentType_SignalTable,
    ContentType_ReadIdIndex,
    ContentType_OtherIndex,
    ContentType_RunInfoTable
  };
  return values;
}

inline const char * const *EnumNamesContentType() {
  static const char * const names[6] = {
    "ReadsTable",
    "SignalTable",
    "ReadIdIndex",
    "OtherIndex",
    "RunInfoTable",
    nullptr
  };
  return names;
}

inline const char *EnumNameContentType(ContentType e) {
  if (flatbuffers::IsOutRange(e, ContentType_ReadsTable, ContentType_RunInfoTable)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesContentType()[index];
}

enum Format : int16_t {
  Format_FeatherV2 = 0,
  Format_MIN = Format_FeatherV2,
  Format_MAX = Format_FeatherV2
};

inline const Format (&EnumValuesFormat())[1] {
  static const Format values[] = {
    Format_FeatherV2
  };
  return values;
}

inline const char * const *EnumNamesFormat() {
  static const char * const names[2] = {
    "FeatherV2",
    nullptr
  };
  return names;
}

inline const char *EnumNameFormat(Format e) {
  if (flatbuffers::IsOutRange(e, Format_FeatherV2, Format_FeatherV2)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesFormat()[index];
}

struct EmbeddedFile FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef EmbeddedFileBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_OFFSET = 4,
    VT_LENGTH = 6,
    VT_FORMAT = 8,
    VT_CONTENT_TYPE = 10
  };
  int64_t offset() const {
    return GetField<int64_t>(VT_OFFSET, 0);
  }
  int64_t length() const {
    return GetField<int64_t>(VT_LENGTH, 0);
  }
  Minknow::ReadsFormat::Format format() const {
    return static_cast<Minknow::ReadsFormat::Format>(GetField<int16_t>(VT_FORMAT, 0));
  }
  Minknow::ReadsFormat::ContentType content_type() const {
    return static_cast<Minknow::ReadsFormat::ContentType>(GetField<int16_t>(VT_CONTENT_TYPE, 0));
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int64_t>(verifier, VT_OFFSET) &&
           VerifyField<int64_t>(verifier, VT_LENGTH) &&
           VerifyField<int16_t>(verifier, VT_FORMAT) &&
           VerifyField<int16_t>(verifier, VT_CONTENT_TYPE) &&
           verifier.EndTable();
  }
};

struct EmbeddedFileBuilder {
  typedef EmbeddedFile Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_offset(int64_t offset) {
    fbb_.AddElement<int64_t>(EmbeddedFile::VT_OFFSET, offset, 0);
  }
  void add_length(int64_t length) {
    fbb_.AddElement<int64_t>(EmbeddedFile::VT_LENGTH, length, 0);
  }
  void add_format(Minknow::ReadsFormat::Format format) {
    fbb_.AddElement<int16_t>(EmbeddedFile::VT_FORMAT, static_cast<int16_t>(format), 0);
  }
  void add_content_type(Minknow::ReadsFormat::ContentType content_type) {
    fbb_.AddElement<int16_t>(EmbeddedFile::VT_CONTENT_TYPE, static_cast<int16_t>(content_type), 0);
  }
  explicit EmbeddedFileBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<EmbeddedFile> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<EmbeddedFile>(end);
    return o;
  }
};

inline flatbuffers::Offset<EmbeddedFile> CreateEmbeddedFile(
    flatbuffers::FlatBufferBuilder &_fbb,
    int64_t offset = 0,
    int64_t length = 0,
    Minknow::ReadsFormat::Format format = Minknow::ReadsFormat::Format_FeatherV2,
    Minknow::ReadsFormat::ContentType content_type = Minknow::ReadsFormat::ContentType_ReadsTable) {
  EmbeddedFileBuilder builder_(_fbb);
  builder_.add_length(length);
  builder_.add_offset(offset);
  builder_.add_content_type(content_type);
  builder_.add_format(format);
  return builder_.Finish();
}

struct Footer FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef FooterBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_FILE_IDENTIFIER = 4,
    VT_SOFTWARE = 6,
    VT_POD5_VERSION = 8,
    VT_CONTENTS = 10
  };
  const flatbuffers::String *file_identifier() const {
    return GetPointer<const flatbuffers::String *>(VT_FILE_IDENTIFIER);
  }
  const flatbuffers::String *software() const {
    return GetPointer<const flatbuffers::String *>(VT_SOFTWARE);
  }
  const flatbuffers::String *pod5_version() const {
    return GetPointer<const flatbuffers::String *>(VT_POD5_VERSION);
  }
  const flatbuffers::Vector<flatbuffers::Offset<Minknow::ReadsFormat::EmbeddedFile>> *contents() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<Minknow::ReadsFormat::EmbeddedFile>> *>(VT_CONTENTS);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_FILE_IDENTIFIER) &&
           verifier.VerifyString(file_identifier()) &&
           VerifyOffset(verifier, VT_SOFTWARE) &&
           verifier.VerifyString(software()) &&
           VerifyOffset(verifier, VT_POD5_VERSION) &&
           verifier.VerifyString(pod5_version()) &&
           VerifyOffset(verifier, VT_CONTENTS) &&
           verifier.VerifyVector(contents()) &&
           verifier.VerifyVectorOfTables(contents()) &&
           verifier.EndTable();
  }
};

struct FooterBuilder {
  typedef Footer Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_file_identifier(flatbuffers::Offset<flatbuffers::String> file_identifier) {
    fbb_.AddOffset(Footer::VT_FILE_IDENTIFIER, file_identifier);
  }
  void add_software(flatbuffers::Offset<flatbuffers::String> software) {
    fbb_.AddOffset(Footer::VT_SOFTWARE, software);
  }
  void add_pod5_version(flatbuffers::Offset<flatbuffers::String> pod5_version) {
    fbb_.AddOffset(Footer::VT_POD5_VERSION, pod5_version);
  }
  void add_contents(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Minknow::ReadsFormat::EmbeddedFile>>> contents) {
    fbb_.AddOffset(Footer::VT_CONTENTS, contents);
  }
  explicit FooterBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<Footer> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Footer>(end);
    return o;
  }
};

inline flatbuffers::Offset<Footer> CreateFooter(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> file_identifier = 0,
    flatbuffers::Offset<flatbuffers::String> software = 0,
    flatbuffers::Offset<flatbuffers::String> pod5_version = 0,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Minknow::ReadsFormat::EmbeddedFile>>> contents = 0) {
  FooterBuilder builder_(_fbb);
  builder_.add_contents(contents);
  builder_.add_pod5_version(pod5_version);
  builder_.add_software(software);
  builder_.add_file_identifier(file_identifier);
  return builder_.Finish();
}

inline flatbuffers::Offset<Footer> CreateFooterDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *file_identifier = nullptr,
    const char *software = nullptr,
    const char *pod5_version = nullptr,
    const std::vector<flatbuffers::Offset<Minknow::ReadsFormat::EmbeddedFile>> *contents = nullptr) {
  auto file_identifier__ = file_identifier ? _fbb.CreateString(file_identifier) : 0;
  auto software__ = software ? _fbb.CreateString(software) : 0;
  auto pod5_version__ = pod5_version ? _fbb.CreateString(pod5_version) : 0;
  auto contents__ = contents ? _fbb.CreateVector<flatbuffers::Offset<Minknow::ReadsFormat::EmbeddedFile>>(*contents) : 0;
  return Minknow::ReadsFormat::CreateFooter(
      _fbb,
      file_identifier__,
      software__,
      pod5_version__,
      contents__);
}

}  // namespace ReadsFormat
}  // namespace Minknow

#endif  // FLATBUFFERS_GENERATED_FOOTER_MINKNOW_READSFORMAT_H_
